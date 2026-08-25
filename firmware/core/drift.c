#include "drift.h"
#include "crypto.h"
#include <string.h>

void drift_store_init(drift_store_t *s)
{
    if (!s) return;
    memset(s, 0, sizeof *s);
}

static void derive(const uint8_t key[32], const char *label, uint32_t counter,
                   uint8_t out[SHA256_LEN])
{
    uint8_t msg[8];
    size_t n = strlen(label);
    uint8_t buf[16];
    if (n > 8u) n = 8u;
    memcpy(buf, label, n);
    msg[0] = (uint8_t)(counter >> 24);
    msg[1] = (uint8_t)(counter >> 16);
    msg[2] = (uint8_t)(counter >> 8);
    msg[3] = (uint8_t)(counter);
    memcpy(buf + n, msg, 4);
    hmac_sha256(key, 32u, buf, n + 4u, out);
}

static void nonce_for(uint32_t counter, uint8_t nonce[12])
{
    memset(nonce, 0, 12);
    nonce[8]  = (uint8_t)(counter >> 24);
    nonce[9]  = (uint8_t)(counter >> 16);
    nonce[10] = (uint8_t)(counter >> 8);
    nonce[11] = (uint8_t)(counter);
}

drift_status_t drift_seal(const uint8_t key[32], uint32_t counter,
                          const uint8_t wire[HIR_WIRE_BYTES],
                          uint8_t payload_class, uint32_t now, uint32_t ttl_s,
                          drift_bundle_t *out)
{
    uint8_t k[SHA256_LEN], idk[SHA256_LEN], nonce[12];
    uint8_t addr[DRIFT_ADDR_BYTES];

    if (!key || !wire || !out) return DRIFT_E_ARG;
    if (payload_class != 1u) return DRIFT_E_CLASS;   /* LDR_CLASS_ESSENTIAL */
    if (ttl_s == 0u) return DRIFT_E_ARG;

    memset(out, 0, sizeof *out);
    derive(key, "addr", counter, k);
    addr[0] = k[0];
    addr[1] = k[1];
    derive(key, "id", counter, idk);
    memcpy(out->id, idk, DRIFT_ID_BYTES);

    nonce_for(counter, nonce);
    out->onair[0] = addr[0];
    out->onair[1] = addr[1];
    aead_encrypt(key, nonce, addr, DRIFT_ADDR_BYTES,
                 wire, HIR_WIRE_BYTES,
                 out->onair + DRIFT_ADDR_BYTES,
                 out->onair + DRIFT_ADDR_BYTES + HIR_WIRE_BYTES,
                 DRIFT_TAG_BYTES);

    out->hops_left = DRIFT_MAX_HOPS;
    out->fanout_left = DRIFT_MAX_FANOUT;
    out->payload_class = payload_class;
    out->expires_at = now + ttl_s;

    secure_zero(k, sizeof k);
    secure_zero(idk, sizeof idk);
    return DRIFT_OK;
}

drift_status_t drift_open(const uint8_t key[32], uint32_t counter_base,
                          const drift_bundle_t *b, uint8_t wire[HIR_WIRE_BYTES])
{
    uint32_t i;

    if (!key || !b || !wire) return DRIFT_E_ARG;
    memset(wire, 0, HIR_WIRE_BYTES);

    for (i = 0; i < DRIFT_OPEN_WINDOW; i++) {
        uint32_t counter = counter_base + i;
        uint8_t k[SHA256_LEN], nonce[12], addr[DRIFT_ADDR_BYTES];
        int rc;

        derive(key, "addr", counter, k);
        addr[0] = k[0];
        addr[1] = k[1];
        secure_zero(k, sizeof k);
        if (addr[0] != b->onair[0] || addr[1] != b->onair[1]) continue;

        nonce_for(counter, nonce);
        rc = aead_decrypt(key, nonce, addr, DRIFT_ADDR_BYTES,
                          b->onair + DRIFT_ADDR_BYTES, HIR_WIRE_BYTES,
                          b->onair + DRIFT_ADDR_BYTES + HIR_WIRE_BYTES,
                          DRIFT_TAG_BYTES, wire);
        if (rc == 0) return DRIFT_OK;
        memset(wire, 0, HIR_WIRE_BYTES);
        return DRIFT_E_AUTH;      /* address matched, tag did not: refuse */
    }
    return DRIFT_E_AUTH;
}

int drift_holds(const drift_store_t *s, const uint8_t id[DRIFT_ID_BYTES])
{
    uint8_t i;
    if (!s || !id) return 0;
    for (i = 0; i < s->count; i++)
        if (memcmp(s->slot[i].id, id, DRIFT_ID_BYTES) == 0) return 1;
    return 0;
}

static int seen(const drift_store_t *s, const uint8_t id[DRIFT_ID_BYTES])
{
    uint8_t i;
    for (i = 0; i < s->seen_count; i++)
        if (memcmp(s->seen[i], id, DRIFT_ID_BYTES) == 0) return 1;
    return 0;
}

static void mark_seen(drift_store_t *s, const uint8_t id[DRIFT_ID_BYTES])
{
    if (seen(s, id)) return;
    if (s->seen_count >= DRIFT_MAX_SEEN) {
        /* Oldest-out. A forgotten identifier can only cause a re-carry, never a
         * second delivery: delivery is gated by drift_deliver_once as well. */
        memmove(s->seen[0], s->seen[1], (size_t)(DRIFT_MAX_SEEN - 1) * DRIFT_ID_BYTES);
        s->seen_count = (uint8_t)(DRIFT_MAX_SEEN - 1u);
    }
    memcpy(s->seen[s->seen_count++], id, DRIFT_ID_BYTES);
}

drift_status_t drift_accept(drift_store_t *s, const drift_bundle_t *b, uint32_t now)
{
    uint8_t i;
    if (!s || !b) return DRIFT_E_ARG;
    if (b->payload_class != 1u) return DRIFT_E_CLASS;
    if (b->expires_at <= now) return DRIFT_E_EXPIRED;
    if (b->hops_left == 0u) return DRIFT_E_EXHAUSTED;
    if (seen(s, b->id)) return DRIFT_E_DUPLICATE;      /* already consumed here */

    for (i = 0; i < s->count; i++) {
        if (memcmp(s->slot[i].id, b->id, DRIFT_ID_BYTES) != 0) continue;
        if (b->hops_left > s->slot[i].hops_left) {
            s->slot[i] = *b;                            /* keep the better replica */
            return DRIFT_UPGRADED;
        }
        return DRIFT_E_DUPLICATE;
    }
    if (s->count >= DRIFT_MAX_CUSTODY) return DRIFT_E_FULL;
    s->slot[s->count++] = *b;
    return DRIFT_OK;
}

drift_status_t drift_deliver_once(drift_store_t *s, const uint8_t id[DRIFT_ID_BYTES])
{
    uint8_t i;
    if (!s || !id) return DRIFT_E_ARG;
    for (i = 0; i < s->count; i++) {
        if (memcmp(s->slot[i].id, id, DRIFT_ID_BYTES) != 0) continue;
        /* consume: the slot is dropped, the identifier stays remembered */
        memmove(&s->slot[i], &s->slot[i+1],
                (size_t)(s->count - i - 1u) * sizeof(drift_bundle_t));
        s->count--;
        memset(&s->slot[s->count], 0, sizeof(drift_bundle_t));
        mark_seen(s, id);
        return DRIFT_OK;
    }
    return DRIFT_E_DUPLICATE;
}

uint8_t drift_gc(drift_store_t *s, uint32_t now)
{
    uint8_t i = 0, dropped = 0;
    if (!s) return 0;
    while (i < s->count) {
        if (s->slot[i].expires_at <= now) {
            memmove(&s->slot[i], &s->slot[i+1],
                    (size_t)(s->count - i - 1u) * sizeof(drift_bundle_t));
            s->count--;
            memset(&s->slot[s->count], 0, sizeof(drift_bundle_t));
            dropped++;
            continue;
        }
        i++;
    }
    return dropped;
}

uint8_t drift_contact(drift_store_t *from, drift_store_t *to, uint32_t now)
{
    uint8_t i, moved = 0;
    if (!from || !to) return 0;
    for (i = 0; i < from->count; i++) {
        drift_bundle_t copy;
        drift_status_t st;
        if (from->slot[i].fanout_left == 0u) continue;
        if (from->slot[i].hops_left <= 1u) continue;
        if (from->slot[i].expires_at <= now) continue;

        copy = from->slot[i];
        copy.hops_left = (uint8_t)(copy.hops_left - 1u);
        copy.fanout_left = DRIFT_MAX_FANOUT;
        st = drift_accept(to, &copy, now);
        if (st != DRIFT_OK && st != DRIFT_UPGRADED) continue;

        from->slot[i].fanout_left = (uint8_t)(from->slot[i].fanout_left - 1u);
        from->transmissions++;
        moved++;
    }
    return moved;
}
