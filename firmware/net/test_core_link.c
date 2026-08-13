/* test_core_link.c — executable contract for Advance 6 companion control link. */
#include "core_link.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static core_link_key_t key_fixture(void)
{
    core_link_key_t key;
    for (unsigned i = 0; i < SHA256_LEN; i++) key.pair_key[i] = (uint8_t)(0x31u + i);
    key.pair_id = 0x4E55434Cu; /* "NUCL", provisioning-local only */
    return key;
}

static intent_observation_t observation(void)
{
    intent_observation_t o;
    o.source = INTENT_SOURCE_NUCLEUS;
    o.session_id = 77;
    o.command = VOICE_COMMAND_ARRIVE;
    o.minutes = 12;
    o.confidence_pct = 91;
    o.runner_up_pct = 20;
    return o;
}

int main(void)
{
    core_link_key_t key = key_fixture();
    core_link_tx_t tx;
    core_link_rx_t rx;
    intent_observation_t obs = observation();
    core_link_intent_t out;
    uint8_t wire[CORE_LINK_WIRE_LEN], tampered[CORE_LINK_WIRE_LEN];

    printf("\n== L1  authenticated Core/Nucleus companion control ==\n");
    core_link_tx_init(&tx);
    core_link_rx_init(&rx);
    {
        intent_observation_t wrong_session = obs;
        wrong_session.session_id = 78;
        ok(core_link_seal_nucleus_intent(&tx, &key, 100, 77, 200, &wrong_session, wire) == CORE_LINK_E_ARG &&
           core_link_seal_nucleus_intent(&tx, &key, 100, 77, 8101, &obs, wire) == CORE_LINK_E_ARG &&
           tx.next_seq == 1,
           "L1 mismatched session and overlong TTL cannot consume a control sequence");
    }
    ok(core_link_seal_nucleus_intent(&tx, &key, 100, 77, 200, &obs, wire) == CORE_LINK_OK &&
       core_link_open_nucleus_intent(&rx, &key, wire, sizeof(wire), 100, &out) == CORE_LINK_OK &&
       out.session_id == 77 && out.observation.source == INTENT_SOURCE_NUCLEUS &&
       out.observation.command == VOICE_COMMAND_ARRIVE && out.observation.minutes == 12 &&
       out.observation.confidence_pct == 91 && rx.last_seq == 1,
       "L1 authenticated envelope preserves only the typed local observation");
    ok(core_link_open_nucleus_intent(&rx, &key, wire, sizeof(wire), 100, &out) == CORE_LINK_E_REPLAY &&
       rx.last_seq == 1,
       "L1 accepted ciphertext cannot be replayed into the same Core link state");

    core_link_seal_nucleus_intent(&tx, &key, 100, 77, 300, &obs, wire);
    memcpy(tampered, wire, sizeof(wire));
    tampered[CORE_LINK_HEADER_LEN] ^= 0x01u;
    ok(core_link_open_nucleus_intent(&rx, &key, tampered, sizeof(tampered), 100, &out) == CORE_LINK_E_AUTH &&
       out.session_id == 0 && rx.last_seq == 1,
       "L1 one altered encrypted byte exposes no observation and consumes no sequence");
    ok(core_link_open_nucleus_intent(&rx, &key, wire, sizeof(wire), 100, &out) == CORE_LINK_OK &&
       rx.last_seq == 2,
       "L1 original authenticated ciphertext remains usable after a failed forgery");

    core_link_seal_nucleus_intent(&tx, &key, 50, 77, 100, &obs, wire);
    ok(core_link_open_nucleus_intent(&rx, &key, wire, sizeof(wire), 100, &out) == CORE_LINK_E_EXPIRED &&
       rx.last_seq == 3 && out.session_id == 0,
       "L1 valid expired ciphertext is discarded and its sequence is consumed");
    ok(core_link_open_nucleus_intent(&rx, &key, wire, sizeof(wire), 50, &out) == CORE_LINK_E_REPLAY,
       "L1 moving the clock backwards cannot revive an expired envelope");

    core_link_tx_init(&tx);
    core_link_rx_init(&rx);
    core_link_seal_nucleus_intent(&tx, &key, 100, 77, 300, &obs, wire);
    {
        core_link_key_t wrong_pair = key;
        wrong_pair.pair_id ^= 1u;
        ok(core_link_open_nucleus_intent(&rx, &wrong_pair, wire, sizeof(wire), 100, &out) == CORE_LINK_E_PAIR &&
           rx.last_seq == 0,
           "L1 ciphertext from a different companion binding cannot enter this Core state");
    }
    wire[1] = 0;
    ok(core_link_open_nucleus_intent(&rx, &key, wire, sizeof(wire), 100, &out) == CORE_LINK_E_DIR &&
       rx.last_seq == 0,
       "L1 an envelope in an unauthorized direction never reaches AEAD plaintext");

    if (FAILED) {
        printf("CORE LINK TESTS FAILED\n");
        return 1;
    }
    printf("CORE LINK INVARIANTS HOLD — authenticated companion control is session-scoped and replay-safe.\n");
    return 0;
}
