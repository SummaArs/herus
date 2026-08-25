#include "hir.h"
#include "crypto.h"
#include <string.h>

void hir_init(hir_t *h)
{
    if (!h) return;
    memset(h, 0, sizeof *h);
    h->version = HIR_VERSION;
}

hir_status_t hir_put(hir_t *h, uint8_t role, uint16_t filler)
{
    uint8_t i;
    if (!h) return HIR_E_ARG;
    if (role == HIR_ROLE_NONE || role >= HIR_ROLE_COUNT) return HIR_E_RANGE;
    if (filler > HIR_FILLER_MAX) return HIR_E_RANGE;
    for (i = 0; i < h->slot_count; i++) {
        if (h->slot[i].role != role) continue;
        if (h->slot[i].filler == filler) return HIR_OK;   /* idempotent */
        return HIR_E_DUPLICATE_ROLE;                      /* conflicting fill */
    }
    if (h->slot_count >= HIR_MAX_SLOTS) return HIR_E_FULL;
    h->slot[h->slot_count].role = role;
    h->slot[h->slot_count].filler = filler;
    h->slot_count++;
    return HIR_OK;
}

hir_status_t hir_validate(const hir_t *h)
{
    uint8_t i, j;
    if (!h) return HIR_E_ARG;
    if (h->version != HIR_VERSION) return HIR_E_RANGE;
    if (h->op == HIR_OP_NONE || h->op >= HIR_OP_MAX) return HIR_E_RANGE;
    if (h->polarity > 1u) return HIR_E_RANGE;
    if (h->urgency > (uint8_t)HIR_URG_SOCORRO) return HIR_E_RANGE;
    if (h->persistence > (uint8_t)HIR_PERSIST_PERMANENTE) return HIR_E_RANGE;
    if (h->slot_count > HIR_MAX_SLOTS) return HIR_E_RANGE;
    if (h->slot_count == 0) return HIR_E_EMPTY;
    for (i = 0; i < h->slot_count; i++) {
        if (h->slot[i].role == HIR_ROLE_NONE || h->slot[i].role >= HIR_ROLE_COUNT) return HIR_E_RANGE;
        if (h->slot[i].filler > HIR_FILLER_MAX) return HIR_E_RANGE;
        for (j = (uint8_t)(i + 1u); j < h->slot_count; j++) {
            if (h->slot[i].role == h->slot[j].role) return HIR_E_DUPLICATE_ROLE;
        }
    }
    return HIR_OK;
}

static void sort_slots(hir_slot_t *s, uint8_t n)
{
    uint8_t i, j;
    for (i = 1; i < n; i++) {
        hir_slot_t key = s[i];
        j = i;
        while (j > 0 && (s[j-1].role > key.role ||
                        (s[j-1].role == key.role && s[j-1].filler > key.filler))) {
            s[j] = s[j-1];
            j--;
        }
        s[j] = key;
    }
}

size_t hir_canon(const hir_t *h, uint8_t out[HIR_CANON_MAX])
{
    hir_slot_t s[HIR_MAX_SLOTS];
    size_t n = 0;
    uint8_t i;

    if (!h || !out) return 0;
    if (hir_validate(h) != HIR_OK) return 0;

    memcpy(s, h->slot, sizeof(hir_slot_t) * h->slot_count);
    sort_slots(s, h->slot_count);

    out[n++] = 'H';
    out[n++] = h->version;
    out[n++] = h->op;
    out[n++] = (uint8_t)(h->polarity | (uint8_t)(h->urgency << 1) | (uint8_t)(h->persistence << 3));
    out[n++] = h->slot_count;
    out[n++] = 0x00;                       /* canonical separator */
    for (i = 0; i < h->slot_count; i++) {
        out[n++] = s[i].role;
        out[n++] = (uint8_t)(s[i].filler >> 8);
        out[n++] = (uint8_t)(s[i].filler & 0xffu);
    }
    return n;
}

hir_status_t hir_digest(const hir_t *h, uint8_t out[HIR_DIGEST_BYTES])
{
    uint8_t canon[HIR_CANON_MAX];
    uint8_t full[SHA256_LEN];
    size_t n;

    if (!h || !out) return HIR_E_ARG;
    n = hir_canon(h, canon);
    if (n == 0) return HIR_E_ARG;
    sha256(canon, n, full);
    memcpy(out, full, HIR_DIGEST_BYTES);
    return HIR_OK;
}

int hir_equal(const hir_t *a, const hir_t *b)
{
    uint8_t ca[HIR_CANON_MAX], cb[HIR_CANON_MAX];
    size_t na, nb;
    if (!a || !b) return 0;
    na = hir_canon(a, ca);
    nb = hir_canon(b, cb);
    if (na == 0 || na != nb) return 0;
    return memcmp(ca, cb, na) == 0;
}

hir_status_t hir_encode_wire(const hir_t *h, uint8_t out[HIR_WIRE_BYTES])
{
    hir_slot_t s[HIR_MAX_SLOTS];
    uint8_t i;
    size_t off;

    if (!h || !out) return HIR_E_ARG;
    if (hir_validate(h) != HIR_OK) return HIR_E_RANGE;

    memcpy(s, h->slot, sizeof(hir_slot_t) * h->slot_count);
    sort_slots(s, h->slot_count);
    memset(out, 0, HIR_WIRE_BYTES);

    out[0] = (uint8_t)((h->version << 4) | (h->op & 0x0fu));
    out[1] = (uint8_t)(h->polarity |
                      (uint8_t)(h->urgency << 1) |
                      (uint8_t)(h->persistence << 3));
    out[2] = h->slot_count;
    off = 3;
    for (i = 0; i < h->slot_count; i++) {
        uint16_t packed = (uint16_t)(((uint16_t)(s[i].role & HIR_ROLE_MAX) << HIR_FILLER_BITS) |
                                     (uint16_t)(s[i].filler & HIR_FILLER_MAX));
        out[off++] = (uint8_t)(packed >> 8);
        out[off++] = (uint8_t)(packed & 0xffu);
    }
    return HIR_OK;
}

hir_status_t hir_decode_wire(const uint8_t in[HIR_WIRE_BYTES], hir_t *h)
{
    uint8_t i, count;
    size_t off;

    if (!in || !h) return HIR_E_ARG;
    hir_init(h);

    /* Reserved bits and reserved tail bytes must be zero. A frame that carries
     * anything there is a covert channel, not a meaning: refuse it. */
    if ((in[1] & 0xe0u) != 0u) return HIR_E_RANGE;

    h->version = (uint8_t)(in[0] >> 4);
    h->op = (uint8_t)(in[0] & 0x0fu);
    h->polarity = (uint8_t)(in[1] & 0x01u);
    h->urgency = (uint8_t)((in[1] >> 1) & 0x03u);
    h->persistence = (uint8_t)((in[1] >> 3) & 0x03u);
    count = in[2];
    if (count > HIR_MAX_SLOTS) return HIR_E_RANGE;

    off = 3;
    for (i = 0; i < count; i++) {
        uint16_t packed = (uint16_t)(((uint16_t)in[off] << 8) | (uint16_t)in[off+1]);
        uint8_t role = (uint8_t)(packed >> HIR_FILLER_BITS);
        uint16_t filler = (uint16_t)(packed & HIR_FILLER_MAX);
        hir_status_t st = hir_put(h, role, filler);
        if (st != HIR_OK) return st;
        off += 2;
    }
    for (; off < HIR_WIRE_BYTES; off++) {
        if (in[off] != 0u) return HIR_E_RANGE;
    }
    if (h->slot_count != count) return HIR_E_RANGE;
    return hir_validate(h);
}

int hir_requires_confirmation(const hir_t *h)
{
    if (!h) return 1;
    switch (h->op) {
    case HIR_OP_PERGUNTAR: return 0;
    default:               return 1;
    }
}
