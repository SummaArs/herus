/* test_memory_prehardware_finale.c — final host-chain composition adversarial suite. */
#include "memory_collection_index.h"
#include "memory_prehardware_finale.h"

#include <stdio.h>
#include <string.h>

#define TEST_COLLECTION_ID 0x47463138u /* GF18, non-secret fixture context. */

typedef struct {
    uint8_t prepared[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t committed[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t has_prepared;
    uint8_t has_committed;
    uint32_t floor;
} fake_collection_store_t;

static uint8_t COLLECTION_ROOT[MEMORY_COLLECTION_ROOT_LEN];
static int FAILED = 0;

int memory_collection_platform_load_root(uint32_t collection_id,
                                         uint8_t out[MEMORY_COLLECTION_ROOT_LEN])
{
    if (collection_id != TEST_COLLECTION_ID) return -1;
    memcpy(out, COLLECTION_ROOT, sizeof(COLLECTION_ROOT));
    return 0;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static int store_prepared(void *ctx, const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memcpy(s->prepared, blob, sizeof(s->prepared));
    s->has_prepared = 1u;
    return 0;
}

static int load_prepared(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (!s->has_prepared) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, s->prepared, sizeof(s->prepared));
    return 0;
}

static int erase_prepared(void *ctx)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memset(s->prepared, 0, sizeof(s->prepared));
    s->has_prepared = 0u;
    return 0;
}

static int store_committed(void *ctx, const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memcpy(s->committed, blob, sizeof(s->committed));
    s->has_committed = 1u;
    return 0;
}

static int load_committed(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (!s->has_committed) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, s->committed, sizeof(s->committed));
    return 0;
}

static int erase_committed(void *ctx)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memset(s->committed, 0, sizeof(s->committed));
    s->has_committed = 0u;
    return 0;
}

static int load_floor(void *ctx, uint32_t *out)
{
    *out = ((fake_collection_store_t *)ctx)->floor;
    return 0;
}

static int commit_floor(void *ctx, uint32_t generation)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (generation <= s->floor) return -1;
    s->floor = generation;
    return 0;
}

static void fixture_init(fake_collection_store_t *s)
{
    size_t i;
    memset(s, 0, sizeof(*s));
    for (i = 0u; i < sizeof(COLLECTION_ROOT); ++i)
        COLLECTION_ROOT[i] = (uint8_t)(0x51u + i);
}

static memory_collection_config_t collection_config(fake_collection_store_t *s)
{
    memory_collection_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.collection_id = TEST_COLLECTION_ID;
    cfg.storage.ctx = s;
    cfg.storage.store_prepared = store_prepared;
    cfg.storage.load_prepared = load_prepared;
    cfg.storage.erase_prepared = erase_prepared;
    cfg.storage.store_committed = store_committed;
    cfg.storage.load_committed = load_committed;
    cfg.storage.erase_committed = erase_committed;
    cfg.storage.load_generation_floor = load_floor;
    cfg.storage.commit_generation_floor = commit_floor;
    return cfg;
}

static memory_vault_card_t eligible_card(void)
{
    memory_vault_card_t card;
    memset(&card, 0, sizeof(card));
    card.card_id = 1801u;
    card.review_receipt_id = 1802u;
    card.signal.session_authorized = 1u;
    card.signal.explicit_remember = 1u;
    card.signal.kind = MEMORY_KIND_IDEA;
    card.signal.scope = MEMORY_SCOPE_SELF;
    card.signal.sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    card.signal.confidence_pct = 96u;
    card.signal.novelty_pct = 95u;
    card.signal.future_value_pct = 95u;
    card.signal.consequence_pct = 80u;
    card.origin = MEMORY_EXTRACT_EXPLICIT;
    card.extract_reasons = MEMORY_EXTRACT_REASON_EXPLICIT | MEMORY_EXTRACT_REASON_IDEA;
    return card;
}

static memory_vault_write_authorization_t authorization(const memory_vault_card_t *card)
{
    memory_vault_write_authorization_t auth;
    auth.card_id = card->card_id;
    auth.review_receipt_id = card->review_receipt_id;
    auth.human_confirmed = 1u;
    return auth;
}

static memory_collection_access_t access_for(memory_physical_session_t *gate,
                                             uint32_t session, uint32_t observed)
{
    memory_collection_access_t access;
    access.gate = gate;
    access.physical_session_id = session;
    access.observed_at_ms = observed;
    return access;
}

static memory_collection_finale_snapshot_t safe_collection_snapshot(void)
{
    memory_collection_finale_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.capture_physical_validated = 1u;
    s.extraction_typed = 1u;
    s.policy_disposition = MEMORY_DISPOSITION_AUTO_ELIGIBLE;
    s.human_review_confirmed = 1u;
    s.write_authorization_bound = 1u;
    s.collection_inserted = 1u;
    s.collection_state = MEMORY_COLLECTION_READY;
    s.collection_recovery_consistent = 1u;
    s.collection_record_authenticated = 1u;
    s.collection_physical_session_bound = 1u;
    s.index_physical_access = 1u;
    s.index_typed_query = 1u;
    s.index_budget_respected = 1u;
    s.index_status = MEMORY_RETRIEVAL_MATCH;
    s.query_result_card_auto_opened = 0u;
    s.unit_vault_fallback_used = 0u;
    s.presentation_physical_access = 1u;
    s.presentation_one_shot_enforced = 1u;
    s.presentation_contract_valid = 1u;
    s.model_in_memory_path = 0u;
    return s;
}

static void host_tm04(threat_model_snapshot_t *s)
{
    memset(s, 1, sizeof(*s));
}

static memory_physical_session_recovery_snapshot_t committed_reservation(uint32_t id)
{
    memory_physical_session_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_reservation_id = id;
    s.durable_reservation_floor = id;
    s.committed_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.committed_uses = 2u;
    return s;
}

static memory_retrieval_query_t idea_query(void)
{
    memory_retrieval_query_t query;
    memset(&query, 0, sizeof(query));
    query.preferred_kind = MEMORY_KIND_IDEA;
    query.preferred_origin = MEMORY_EXTRACT_EXPLICIT;
    query.require_explicit = 1u;
    query.minimum_confidence_pct = 90u;
    return query;
}

int main(void)
{
    fake_collection_store_t store;
    memory_collection_config_t collection_cfg;
    memory_physical_session_config_t session_cfg;
    memory_collection_t collection;
    memory_collection_t reopened;
    memory_collection_index_t index;
    memory_collection_index_config_t index_cfg;
    memory_physical_session_t seed_gate;
    memory_physical_session_t boot_gate;
    memory_vault_card_t card;
    memory_vault_write_authorization_t auth;
    memory_collection_access_t access;
    memory_collection_finale_snapshot_t collection_snapshot;
    memory_physical_session_recovery_snapshot_t reservation;
    threat_model_snapshot_t threat_snapshot;
    memory_prehardware_finale_input_t input;
    memory_prehardware_finale_decision_t decision;
    memory_retrieval_query_t query;
    memory_retrieval_result_t result;
    uint32_t opens_before;

    printf("\n== G1 pre-hardware Grand Finale composes boot quarantine and memory authority ==\n");
    fixture_init(&store);
    collection_cfg = collection_config(&store);
    memory_physical_session_config_default(&session_cfg);
    memory_collection_index_config_default(&index_cfg);
    index_cfg.max_queries_per_session = 2u;
    card = eligible_card();
    auth = authorization(&card);

    access = access_for(&seed_gate, 1u, 2u);
    ok(memory_collection_init(&collection, &collection_cfg) == MEMORY_COLLECTION_OK &&
       memory_physical_session_init(&seed_gate, &session_cfg) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_begin(&seed_gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     1u, 2u, 1u, 1u, 1u) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_collection_insert(&collection, &auth, &card, &access) == MEMORY_COLLECTION_OK &&
       memory_collection_init(&reopened, &collection_cfg) == MEMORY_COLLECTION_OK &&
       memory_collection_index_init(&index, &index_cfg) == MEMORY_COLLECTION_INDEX_OK,
       "G1 fixture persists an authorized minimal card and reopens only authenticated collection topology");

    reservation = committed_reservation(100u);
    collection_snapshot = safe_collection_snapshot();
    host_tm04(&threat_snapshot);
    input.session_config = &session_cfg;
    input.reservation_snapshot = &reservation;
    input.collection_snapshot = &collection_snapshot;
    input.threat_snapshot = &threat_snapshot;
    ok(memory_prehardware_finale_audit(&boot_gate, &input, &decision) ==
           MEMORY_PREHARDWARE_FINALE_OK &&
       decision.ready_for_target_validation == 1u &&
       decision.failures == MEMORY_PREHARDWARE_FINALE_FAIL_NONE &&
       decision.recovery_action == MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED &&
       decision.recovered_session_floor == 100u &&
       boot_gate.state == MEMORY_PHYSICAL_SESSION_IDLE && boot_gate.active_session_id == 0u &&
       boot_gate.active_event_nonce == 0u && boot_gate.active_purpose == MEMORY_PHYSICAL_PURPOSE_NONE,
       "G1 final composition imports only committed floor and leaves no active post-reboot authority");

    query = idea_query();
    access = access_for(&boot_gate, 101u, 11u);
    ok(memory_collection_index_query(&index, &reopened, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_E_ACCESS && result.status == MEMORY_RETRIEVAL_NO_MATCH &&
       memory_physical_session_begin(&boot_gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     100u, 3u, 1u, 2u, 10u) == MEMORY_PHYSICAL_SESSION_E_FORMAT &&
       memory_physical_session_begin(&boot_gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     101u, 3u, 0u, 2u, 10u) == MEMORY_PHYSICAL_SESSION_E_FORMAT,
       "G1 no stale/ floor ID or unasserted event can query collection after a coherent finale");

    ok(memory_physical_session_begin(&boot_gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     101u, 3u, 1u, 2u, 10u) == MEMORY_PHYSICAL_SESSION_OK &&
       (access = access_for(&boot_gate, 101u, 11u), 1) &&
       (opens_before = reopened.metrics.opens, 1) &&
       memory_collection_index_query(&index, &reopened, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && result.status == MEMORY_RETRIEVAL_MATCH &&
       result.card_id == card.card_id && reopened.metrics.opens == opens_before,
       "G1 only a new successor session reaches typed query and still cannot auto-open a card");
    ok(memory_physical_session_cancel(&boot_gate) == MEMORY_PHYSICAL_SESSION_OK &&
       boot_gate.state == MEMORY_PHYSICAL_SESSION_CANCELLED && boot_gate.active_session_id == 0u,
       "G1 unfinished post-boot query remains explicitly cancellable and scrubs active evidence");

    collection_snapshot.query_result_card_auto_opened = 1u;
    ok(memory_prehardware_finale_audit(&boot_gate, &input, &decision) ==
           MEMORY_PREHARDWARE_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_PREHARDWARE_FINALE_FAIL_COLLECTION) &&
       decision.ready_for_target_validation == 0u && boot_gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED &&
       boot_gate.active_session_id == 0u,
       "G1 automatic open in composed evidence blocks and scrubs the final gate");

    collection_snapshot = safe_collection_snapshot();
    threat_snapshot.memory_physical_session_bootstrap_quarantined = 0u;
    ok(memory_prehardware_finale_audit(&boot_gate, &input, &decision) ==
           MEMORY_PREHARDWARE_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_PREHARDWARE_FINALE_FAIL_THREAT_MODEL) &&
       boot_gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED,
       "G1 removing only TM-04 boot-quarantine evidence blocks final readiness");

    host_tm04(&threat_snapshot);
    reservation.committed_authenticated = 0u;
    ok(memory_prehardware_finale_audit(&boot_gate, &input, &decision) ==
           MEMORY_PREHARDWARE_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_PREHARDWARE_FINALE_FAIL_BOOTSTRAP) &&
       decision.recovery_action == MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED &&
       boot_gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED,
       "G1 unauthenticated durable reservation cannot cross the final composition");

    reservation = committed_reservation(100u);
    ok(memory_prehardware_finale_audit(0, &input, &decision) == MEMORY_PREHARDWARE_FINALE_E_ARG &&
       memory_prehardware_finale_audit(&boot_gate, 0, &decision) == MEMORY_PREHARDWARE_FINALE_E_ARG &&
       boot_gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED &&
       memory_prehardware_finale_audit(&boot_gate, &input, 0) == MEMORY_PREHARDWARE_FINALE_E_ARG &&
       boot_gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED,
       "G1 missing finale inputs never produce readiness or preserve a permissive gate");

    if (FAILED) {
        printf("PREHARDWARE FINALE TESTS FAILED\n");
        return 1;
    }
    printf("PREHARDWARE FINALE INVARIANTS HOLD — a coherent host chain reaches only idle quarantine, never active authority.\n");
    return 0;
}
