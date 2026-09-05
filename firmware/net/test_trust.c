/* test_trust.c — executable contract for Advance 7 trust lifecycle. */
#include "trust.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t blob[TRUST_STORE_BLOB_LEN];
    int present;
    int fail_store;
    int fail_erase;
    unsigned stores;
    unsigned erases;
} vault_t;

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static int zeroed(const void *p, size_t n)
{
    const uint8_t *b = p;
    uint8_t any = 0;
    for (size_t i = 0; i < n; i++) any |= b[i];
    return any == 0;
}

static int store_active(void *ctx, const uint8_t blob[TRUST_STORE_BLOB_LEN])
{
    vault_t *v = ctx;
    v->stores++;
    if (v->fail_store) return -1;
    memcpy(v->blob, blob, sizeof(v->blob));
    v->present = 1;
    return 0;
}

static int load_active(void *ctx, uint8_t blob[TRUST_STORE_BLOB_LEN])
{
    vault_t *v = ctx;
    if (v->present) memcpy(blob, v->blob, sizeof(v->blob));
    else memset(blob, 0, TRUST_STORE_BLOB_LEN);
    return 0;
}

static int erase(void *ctx)
{
    vault_t *v = ctx;
    v->erases++;
    if (v->fail_erase) return -1;
    secure_zero(v->blob, sizeof(v->blob));
    v->present = 0;
    return 0;
}

static trust_storage_t storage(vault_t *v)
{
    trust_storage_t s;
    s.ctx = v; s.store_active = store_active; s.load_active = load_active; s.erase = erase;
    return s;
}

static void material(uint8_t secret[SHA256_LEN], uint8_t core[TRUST_NONCE_LEN],
                     uint8_t nucleus[TRUST_NONCE_LEN], uint8_t seed)
{
    for (unsigned i = 0; i < SHA256_LEN; i++) secret[i] = (uint8_t)(seed + i);
    for (unsigned i = 0; i < TRUST_NONCE_LEN; i++) {
        core[i] = (uint8_t)(seed + 0x20u + i);
        nucleus[i] = (uint8_t)(seed + 0x60u + i);
    }
}

int main(void)
{
    trust_t t, restored;
    vault_t v;
    trust_storage_t s;
    uint8_t secret[SHA256_LEN], core[TRUST_NONCE_LEN], nucleus[TRUST_NONCE_LEN];
    core_link_tx_t tx;
    core_link_rx_t rx;
    intent_observation_t obs;
    core_link_intent_t opened;
    uint8_t wire[CORE_LINK_WIRE_LEN];
    uint32_t sas;
    trust_revoke_authorization_t revoke_auth;

    memset(&v, 0, sizeof(v));
    s = storage(&v);
    material(secret, core, nucleus, 0x11u);
    trust_init(&t);
    obs.source = INTENT_SOURCE_NUCLEUS; obs.session_id = 9;
    obs.command = VOICE_COMMAND_ARRIVE; obs.minutes = 0;
    obs.confidence_pct = 90; obs.runner_up_pct = 0;
    core_link_tx_init(&tx);
    core_link_rx_init(&rx);

    printf("\n== T1  explicit pairing, key lifecycle and revocation ==\n");
    ok(trust_begin(&t, 1, 0, secret, core, nucleus, 100) == TRUST_E_PHYSICAL &&
       t.state == TRUST_UNPAIRED,
       "T1 one-sided physical pairing mode cannot create an offer");

    ok(trust_begin(&t, 1, 1, secret, core, nucleus, 100) == TRUST_OK &&
       trust_sas(&t, &sas) == TRUST_OK && sas < TRUST_SAS_MODULO &&
       trust_seal_nucleus_intent(&t, &tx, 101, 9, 200, &obs, wire) == TRUST_E_STATE,
       "T1 a pending offer has a six-digit SAS but cannot seal a control envelope");
    ok(trust_confirm(&t, 1, sas, 0, sas, 200, &s) == TRUST_E_CONFIRM &&
       t.state == TRUST_UNPAIRED && !v.present,
       "T1 absent human confirmation zeroizes the offer and stores nothing");

    trust_begin(&t, 1, 1, secret, core, nucleus, 250);
    trust_sas(&t, &sas);
    ok(trust_confirm(&t, 1, sas, 1, (sas + 1u) % TRUST_SAS_MODULO, 260, &s) == TRUST_E_CONFIRM &&
       t.state == TRUST_UNPAIRED && !v.present,
       "T1 a divergent six-digit SAS cannot activate or persist a link key");

    trust_begin(&t, 1, 1, secret, core, nucleus, 300);
    ok(trust_tick(&t, 300 + TRUST_OFFER_TTL_MS) == TRUST_E_TIMEOUT &&
       t.state == TRUST_UNPAIRED,
       "T1 an unattended association offer expires without activating trust");

    trust_begin(&t, 1, 1, secret, core, nucleus, 350);
    trust_sas(&t, &sas);
    v.fail_store = 1;
    ok(trust_confirm(&t, 1, sas, 1, sas, 360, &s) == TRUST_E_STORAGE &&
       t.state == TRUST_UNPAIRED && !v.present && zeroed(t.transport_secret, sizeof(t.transport_secret)),
       "T1 failed protected persistence zeroizes derived material and exposes no link key");
    v.fail_store = 0;

    trust_begin(&t, 1, 1, secret, core, nucleus, 400);
    trust_sas(&t, &sas);
    ok(trust_confirm(&t, 1, sas, 1, sas, 500, &s) == TRUST_OK && t.state == TRUST_ACTIVE &&
       v.present,
       "T1 dual confirmation plus successful protected-store operation creates an active link");

    core_link_tx_init(&tx);
    core_link_rx_init(&rx);
    ok(trust_seal_nucleus_intent(&t, &tx, 600, 9, 900, &obs, wire) == CORE_LINK_OK &&
       trust_open_nucleus_intent(&t, &rx, wire, sizeof(wire), 700, &opened) == CORE_LINK_OK,
       "T1 only an active trust link can seal and open an A6 control envelope");
    ok(trust_begin(&t, 1, 1, secret, core, nucleus, 700) == TRUST_E_STATE,
       "T1 an active link cannot be silently replaced by a new offer");

    revoke_auth.physical_session_id = 7001u;
    revoke_auth.active_generation = t.generation;
    revoke_auth.core_confirmed = 1u;
    revoke_auth.nucleus_confirmed = 0u;
    ok(trust_revoke(&t, &tx, &rx, &s, &revoke_auth) == TRUST_E_PHYSICAL &&
       t.state == TRUST_ACTIVE && v.present && tx.next_seq == 2 && rx.last_seq == 1,
       "T1 one-sided revocation authority cannot erase trust or reset replay state");
    revoke_auth.nucleus_confirmed = 1u;
    v.fail_erase = 1;
    ok(tx.next_seq == 2 && rx.last_seq == 1 &&
       trust_revoke(&t, &tx, &rx, &s, &revoke_auth) == TRUST_E_STORAGE && t.state == TRUST_REVOKED &&
       zeroed(t.active_blob, sizeof(t.active_blob)) && zeroed(t.transport_secret, sizeof(t.transport_secret)) &&
       zeroed(t.core_nonce, sizeof(t.core_nonce)) && zeroed(t.nucleus_nonce, sizeof(t.nucleus_nonce)) &&
       tx.next_seq == 1 && rx.last_seq == 0 &&
       trust_seal_nucleus_intent(&t, &tx, 750, 9, 900, &obs, wire) == TRUST_E_STATE &&
       trust_begin(&t, 1, 1, secret, core, nucleus, 800) == TRUST_E_STATE,
       "T1 erase failure fails closed and zeroizes RAM: the link cannot export or re-pair");
    v.fail_erase = 0;
    ok(trust_retry_erase(&t, &s) == TRUST_OK && t.state == TRUST_UNPAIRED && !v.present &&
       v.erases == 2,
       "T1 successful retry erases protected record before re-provisioning is allowed");

    material(secret, core, nucleus, 0x51u);
    ok(trust_begin(&t, 1, 1, secret, core, nucleus, 900) == TRUST_OK &&
       trust_sas(&t, &sas) == TRUST_OK &&
       trust_confirm(&t, 1, sas, 1, sas, 1000, &s) == TRUST_OK &&
       trust_open_nucleus_intent(&t, &rx, wire, sizeof(wire), 850, &opened) != CORE_LINK_OK &&
       t.generation == 2,
       "T1 re-provisioning after revocation derives a new binding that rejects the prior envelope");

    trust_init(&restored);
    core_link_tx_init(&tx);
    ok(trust_restore(&restored, &s) == TRUST_OK && restored.state == TRUST_ACTIVE &&
       trust_seal_nucleus_intent(&restored, &tx, 1100, 9, 1200, &obs, wire) == CORE_LINK_OK,
       "T1 only an intact active protected record restores a usable control link");
    secure_zero(v.blob, sizeof(v.blob));
    trust_init(&restored);
    ok(trust_restore(&restored, &s) == TRUST_E_RECORD && restored.state == TRUST_UNPAIRED,
       "T1 a revoked or malformed record cannot resurrect a companion trust link");

    if (FAILED) {
        printf("TRUST LIFECYCLE TESTS FAILED\n");
        return 1;
    }
    printf("TRUST LIFECYCLE INVARIANTS HOLD — pairing is explicit, revocable and non-resurrectable.\n");
    return 0;
}
