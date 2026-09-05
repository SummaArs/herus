/* test_capstone.c — Grand Finale end-to-end safety composition proof. */
#include "interaction.h"
#include "dialogue.h"
#include "model_lab.h"
#include "trust.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t blob[TRUST_STORE_BLOB_LEN];
    int present;
    int fail_erase;
} vault_t;

static int FAILED = 0;
static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static int store_active(void *ctx, const uint8_t blob[TRUST_STORE_BLOB_LEN])
{
    vault_t *v = ctx;
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

static int erase_vault(void *ctx)
{
    vault_t *v = ctx;
    if (v->fail_erase) return -1;
    memset(v->blob, 0, sizeof(v->blob));
    v->present = 0;
    return 0;
}

static trust_storage_t storage(vault_t *v)
{
    trust_storage_t s;
    s.ctx = v;
    s.store_active = store_active;
    s.load_active = load_active;
    s.erase = erase_vault;
    return s;
}

static int action_reply(void *ctx, const dialogue_request_t *request,
                        dialogue_model_reply_t *out)
{
    (void)ctx;
    (void)request;
    memset(out, 0, sizeof(*out));
    strcpy(out->reply, "envie ajuda agora");
    out->reply_len = (uint8_t)strlen(out->reply);
    out->topic = DIALOGUE_TOPIC_SAFETY;
    return 0;
}

static void accepted_model(model_lab_profile_t *p, model_lab_budget_t *b,
                           model_lab_decision_t *d)
{
    memset(p, 0, sizeof(*p));
    memset(b, 0, sizeof(*b));
    p->measured_on_target = 1; p->local_only = 1; p->model_digest[0] = 1;
    p->model_flash_bytes = 1; p->peak_internal_bytes = 1; p->peak_psram_bytes = 1;
    p->p95_latency_ms = 1; p->energy_per_turn_uj = 1;
    p->functional_cases = p->functional_passed = 1;
    p->adversarial_cases = p->adversarial_rejected = 1;
    b->max_model_flash_bytes = 1; b->max_peak_internal_bytes = 1;
    b->max_peak_psram_bytes = 1; b->max_p95_latency_ms = 1;
    b->max_energy_per_turn_uj = 1; b->min_functional_cases = 1;
    b->min_adversarial_cases = 1;
    (void)model_lab_decide(p, b, d);
}

static assurance_snapshot_t snapshot_for(interaction_t *it,
                                         const model_lab_decision_t *model)
{
    assurance_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.source = ASSURANCE_SOURCE_CORE;
    s.physical_session_current = interaction_session_id(it) != 0u;
    s.intent_accepted = it->state == INTERACTION_READY_SEND;
    s.physical_confirmation = it->state == INTERACTION_READY_SEND;
    s.handoff_unused = it->state == INTERACTION_READY_SEND;
    s.local_model_enabled = 1;
    s.local_model_accepted = model->accepted;
    s.model_reply_display_only = 1;
    return s;
}

static intent_observation_t observation(uint32_t session)
{
    intent_observation_t o;
    o.source = INTENT_SOURCE_CORE;
    o.session_id = session;
    o.command = VOICE_COMMAND_ARRIVE;
    o.minutes = 0;
    o.confidence_pct = 95;
    o.runner_up_pct = 0;
    return o;
}

static void material(uint8_t secret[SHA256_LEN], uint8_t core[TRUST_NONCE_LEN],
                     uint8_t nucleus[TRUST_NONCE_LEN])
{
    for (unsigned i = 0; i < SHA256_LEN; i++) secret[i] = (uint8_t)(0x21u + i);
    for (unsigned i = 0; i < TRUST_NONCE_LEN; i++) {
        core[i] = (uint8_t)(0x61u + i);
        nucleus[i] = (uint8_t)(0x91u + i);
    }
}

int main(void)
{
    dialogue_t dialogue;
    dialogue_model_t model;
    dialogue_topic_t topic;
    char reply[DIALOGUE_REPLY_MAX];
    model_lab_profile_t profile;
    model_lab_budget_t budget;
    model_lab_decision_t model_decision;
    interaction_t it;
    intent_observation_t obs;
    assurance_snapshot_t snap;
    hcp_msg_t sent;
    trust_t trust;
    vault_t vault;
    trust_storage_t store;
    core_link_tx_t tx;
    core_link_rx_t rx;
    uint8_t secret[SHA256_LEN], core_nonce[TRUST_NONCE_LEN], nucleus_nonce[TRUST_NONCE_LEN];
    uint8_t wire[CORE_LINK_WIRE_LEN];
    uint32_t sas;
    trust_revoke_authorization_t revoke_auth;

    printf("\n== C10  Grand Finale chain: dialogue -> model -> intent -> trust ==\n");
    accepted_model(&profile, &budget, &model_decision);
    model.ctx = NULL;
    model.generate_local = action_reply;
    dialogue_init(&dialogue, NULL, &model);
    ok(model_decision.accepted && dialogue_begin_turn(&dialogue, 33, 0) == DIALOGUE_OK &&
       dialogue_submit_utterance(&dialogue, "preciso de ajuda", 16, 1) == DIALOGUE_OK &&
       dialogue_take_reply(&dialogue, reply, sizeof(reply), &topic) == DIALOGUE_OK &&
       !strcmp(reply, "envie ajuda agora") && topic == DIALOGUE_TOPIC_SAFETY,
       "C10 a measured local-model candidate may produce action-looking UX text only");

    interaction_init(&it, NULL);
    ok(interaction_take_send(&it, &sent) == INTERACTION_E_STATE,
       "C10 dialogue output creates neither an interaction draft nor a send handoff");
    ok(interaction_push_to_talk(&it, 10) == INTERACTION_OK &&
       interaction_asr_result(&it, &(intent_observation_t){
           INTENT_SOURCE_CORE, interaction_session_id(&it), VOICE_COMMAND_ARRIVE,
           0, 95, 0 }, NULL, 11) == INTERACTION_OK &&
       interaction_confirm(&it, 1, 12) == INTERACTION_OK,
       "C10 a separate physical command session still creates the only candidate handoff");
    snap = snapshot_for(&it, &model_decision);
    ok(interaction_take_send_assured(&it, &snap, &sent) == INTERACTION_OK &&
       it.state == INTERACTION_IDLE && sent.intent == it.cfg.lexicon.intent_arrive,
       "C10 only all local gates plus accepted display-only model evidence release one handoff");

    interaction_init(&it, NULL);
    interaction_push_to_talk(&it, 20);
    obs = observation(interaction_session_id(&it));
    interaction_asr_result(&it, &obs, NULL, 21);
    interaction_confirm(&it, 1, 22);
    snap = snapshot_for(&it, &model_decision);
    snap.model_reply_display_only = 0;
    ok(interaction_take_send_assured(&it, &snap, &sent) == INTERACTION_E_UNTRUSTED &&
       it.state == INTERACTION_READY_SEND,
       "C10 suspected model agency blocks rather than consumes a confirmed local draft");

    memset(&vault, 0, sizeof(vault));
    store = storage(&vault);
    material(secret, core_nonce, nucleus_nonce);
    trust_init(&trust);
    core_link_tx_init(&tx);
    core_link_rx_init(&rx);
    trust_begin(&trust, 1, 1, secret, core_nonce, nucleus_nonce, 30);
    trust_sas(&trust, &sas);
    ok(trust_confirm(&trust, 1, sas, 1, sas, 31, &store) == TRUST_OK &&
       trust.state == TRUST_ACTIVE,
       "C10 a Nucleus path becomes eligible only after the real paired trust ceremony");
    revoke_auth.physical_session_id = 9001u;
    revoke_auth.active_generation = trust.generation;
    revoke_auth.core_confirmed = 1u;
    revoke_auth.nucleus_confirmed = 1u;
    vault.fail_erase = 1;
    ok(trust_revoke(&trust, &tx, &rx, &store, &revoke_auth) == TRUST_E_STORAGE &&
       trust.state == TRUST_REVOKED &&
       trust_seal_nucleus_intent(&trust, &tx, 32, 1, 33, &obs, wire) == TRUST_E_STATE,
       "C10 erase failure leaves trust revoked and forbids future authenticated envelopes");

    interaction_init(&it, NULL);
    interaction_push_to_talk(&it, 40);
    obs = observation(interaction_session_id(&it));
    interaction_asr_result(&it, &obs, NULL, 41);
    interaction_confirm(&it, 1, 42);
    snap = snapshot_for(&it, &model_decision);
    snap.source = ASSURANCE_SOURCE_NUCLEUS;
    snap.trust_active = trust.state == TRUST_ACTIVE;
    snap.control_link_authenticated = 1;
    snap.control_link_fresh = 1;
    snap.trust_revoked = trust.state == TRUST_REVOKED;
    ok(interaction_take_send_assured(&it, &snap, &sent) == INTERACTION_E_UNTRUSTED &&
       it.state == INTERACTION_READY_SEND,
       "C10 revocation has precedence over a previously confirmed handoff and claimed fresh link");

    if (FAILED) {
        printf("CAPSTONE TESTS FAILED\n");
        return 1;
    }
    printf("CAPSTONE INVARIANTS HOLD — no dialogue, model or revoked companion state bypasses physical confirmation.\n");
    return 0;
}
