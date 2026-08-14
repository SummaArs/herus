#include "memory_collection_finale.h"
#include "memory_capture.h"
#include "memory_consolidation.h"
#include "memory_extract.h"
#include "memory_collection_index.h"
#include "memory_retrieval_present.h"
#include <stdio.h>
#include <string.h>

#define TEST_VAULT_ID 0x4d313446u      /* M14F, non-secret fixture context. */
#define TEST_COLLECTION_ID 0x4d314346u /* M1CF, non-secret fixture context. */

typedef struct {
    uint8_t root[MEMORY_VAULT_ROOT_LEN];
    uint8_t blob[MEMORY_VAULT_BLOB_LEN];
    uint32_t floor;
    uint8_t have_blob;
} fake_vault_store_t;

typedef struct {
    uint8_t prepared[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t committed[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t has_prepared;
    uint8_t has_committed;
    uint32_t floor;
} fake_collection_store_t;

static fake_vault_store_t *VAULT_ROOT = 0;
static uint8_t COLLECTION_ROOT[MEMORY_COLLECTION_ROOT_LEN];
static int FAILED = 0;

int memory_vault_platform_load_root(uint32_t vault_id,
                                    uint8_t out[MEMORY_VAULT_ROOT_LEN])
{
    if (!VAULT_ROOT || vault_id != TEST_VAULT_ID) return -1;
    memcpy(out, VAULT_ROOT->root, sizeof(VAULT_ROOT->root));
    return 0;
}

int memory_collection_platform_load_root(uint32_t collection_id,
                                         uint8_t out[MEMORY_COLLECTION_ROOT_LEN])
{
    if (collection_id != TEST_COLLECTION_ID) return -1;
    memcpy(out, COLLECTION_ROOT, MEMORY_COLLECTION_ROOT_LEN);
    return 0;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static int vault_store(void *ctx, const uint8_t blob[MEMORY_VAULT_BLOB_LEN])
{
    fake_vault_store_t *s = (fake_vault_store_t *)ctx;
    memcpy(s->blob, blob, sizeof(s->blob));
    s->have_blob = 1u;
    return 0;
}

static int vault_load(void *ctx, uint8_t blob[MEMORY_VAULT_BLOB_LEN])
{
    fake_vault_store_t *s = (fake_vault_store_t *)ctx;
    if (!s->have_blob) return -1;
    memcpy(blob, s->blob, sizeof(s->blob));
    return 0;
}

static int vault_erase(void *ctx)
{
    fake_vault_store_t *s = (fake_vault_store_t *)ctx;
    memset(s->blob, 0, sizeof(s->blob));
    s->have_blob = 0u;
    return 0;
}

static int vault_load_floor(void *ctx, uint32_t *floor)
{
    *floor = ((fake_vault_store_t *)ctx)->floor;
    return 0;
}

static int vault_commit_floor(void *ctx, uint32_t generation)
{
    fake_vault_store_t *s = (fake_vault_store_t *)ctx;
    if (generation <= s->floor) return -1;
    s->floor = generation;
    return 0;
}

static int collection_store_prepared(void *ctx,
                                     const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memcpy(s->prepared, blob, sizeof(s->prepared));
    s->has_prepared = 1u;
    return 0;
}

static int collection_load_prepared(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (!s->has_prepared) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, s->prepared, sizeof(s->prepared));
    return 0;
}

static int collection_erase_prepared(void *ctx)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memset(s->prepared, 0, sizeof(s->prepared));
    s->has_prepared = 0u;
    return 0;
}

static int collection_store_committed(void *ctx,
                                      const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memcpy(s->committed, blob, sizeof(s->committed));
    s->has_committed = 1u;
    return 0;
}

static int collection_load_committed(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (!s->has_committed) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, s->committed, sizeof(s->committed));
    return 0;
}

static int collection_erase_committed(void *ctx)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memset(s->committed, 0, sizeof(s->committed));
    s->has_committed = 0u;
    return 0;
}

static int collection_load_floor(void *ctx, uint32_t *floor)
{
    *floor = ((fake_collection_store_t *)ctx)->floor;
    return 0;
}

static int collection_commit_floor(void *ctx, uint32_t generation)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (generation <= s->floor) return -1;
    s->floor = generation;
    return 0;
}

static void fixture_init(fake_vault_store_t *vault, fake_collection_store_t *collection)
{
    size_t i;
    memset(vault, 0, sizeof(*vault));
    memset(collection, 0, sizeof(*collection));
    for (i = 0u; i < sizeof(vault->root); ++i) vault->root[i] = (uint8_t)(0x35u + i);
    for (i = 0u; i < sizeof(COLLECTION_ROOT); ++i) COLLECTION_ROOT[i] = (uint8_t)(0x58u + i);
    VAULT_ROOT = vault;
}

static memory_vault_config_t vault_config(fake_vault_store_t *s)
{
    memory_vault_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.vault_id = TEST_VAULT_ID;
    cfg.storage.ctx = s;
    cfg.storage.store_sealed = vault_store;
    cfg.storage.load_sealed = vault_load;
    cfg.storage.erase_sealed = vault_erase;
    cfg.storage.load_generation_floor = vault_load_floor;
    cfg.storage.commit_generation_floor = vault_commit_floor;
    return cfg;
}

static memory_collection_config_t collection_config(fake_collection_store_t *s)
{
    memory_collection_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.collection_id = TEST_COLLECTION_ID;
    cfg.storage.ctx = s;
    cfg.storage.store_prepared = collection_store_prepared;
    cfg.storage.load_prepared = collection_load_prepared;
    cfg.storage.erase_prepared = collection_erase_prepared;
    cfg.storage.store_committed = collection_store_committed;
    cfg.storage.load_committed = collection_load_committed;
    cfg.storage.erase_committed = collection_erase_committed;
    cfg.storage.load_generation_floor = collection_load_floor;
    cfg.storage.commit_generation_floor = collection_commit_floor;
    return cfg;
}

static memory_consolidation_access_t physical(uint32_t session)
{
    memory_consolidation_access_t access;
    access.physical_session_id = session;
    access.physical_confirmed = 1u;
    return access;
}

static memory_collection_access_t collection_access(memory_physical_session_t *gate,
                                                   memory_physical_purpose_t purpose,
                                                   uint32_t session)
{
    memory_collection_access_t access;
    uint8_t uses = purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY ? 2u : 1u;
    (void)memory_physical_session_begin(gate, purpose, session, session ^ 0xC33Cu,
                                        1u, uses, session * 100u);
    access.gate = gate;
    access.physical_session_id = session;
    access.observed_at_ms = session * 100u + 1u;
    return access;
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

static memory_collection_finale_snapshot_t safe_snapshot(memory_retrieval_status_t status)
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
    s.index_status = status;
    s.query_result_card_auto_opened = 0u;
    s.unit_vault_fallback_used = 0u;
    s.presentation_physical_access = 1u;
    s.presentation_one_shot_enforced = 1u;
    s.presentation_contract_valid = 1u;
    s.model_in_memory_path = 0u;
    return s;
}

int main(void)
{
    fake_vault_store_t vault_store_state;
    fake_collection_store_t collection_store_state;
    memory_vault_config_t vault_cfg;
    memory_collection_config_t collection_cfg;
    memory_capture_config_t capture_cfg;
    memory_consolidation_config_t consolidation_cfg;
    memory_collection_index_config_t index_cfg;
    memory_capture_t capture;
    memory_extract_t extract;
    memory_candidate_t candidate;
    memory_assessment_t assessment;
    memory_consolidation_t consolidation;
    memory_consolidation_proposal_t proposal;
    memory_consolidation_access_t review_access;
    memory_collection_access_t collection_access_assertion;
    memory_physical_session_t collection_gate;
    memory_physical_session_config_t collection_gate_cfg;
    memory_vault_t vault;
    memory_vault_card_t card;
    memory_vault_write_authorization_t auth;
    memory_collection_t collection;
    memory_collection_t reopened;
    memory_collection_index_t index;
    memory_retrieval_query_t query;
    memory_retrieval_result_t result;
    memory_retrieval_present_t presenter;
    memory_retrieval_presentation_t presentation;
    memory_collection_finale_snapshot_t snapshot;
    memory_collection_finale_decision_t decision;
    uint32_t receipt_id;
    uint32_t opens_before;
    uint32_t capture_id;
    char idea[] = "lembre esta ideia: nucleo privado complementar";

    printf("\n== M14 collection Grand Finale keeps human authority, abstention and no fallback ==\n");
    fixture_init(&vault_store_state, &collection_store_state);
    vault_cfg = vault_config(&vault_store_state);
    collection_cfg = collection_config(&collection_store_state);
    ok(memory_vault_init(&vault, &vault_cfg) == MEMORY_VAULT_OK &&
       memory_collection_init(&collection, &collection_cfg) == MEMORY_COLLECTION_OK,
       "M14 fixture begins with separate RAM contracts for unit authorization and collection transaction");

    memory_capture_config_default(&capture_cfg);
    capture_cfg.window_ms = 1000u;
    memory_capture_init(&capture, &capture_cfg, 0);
    memory_extract_init(&extract);
    ok(memory_capture_begin(&capture, 7u, 100u) == MEMORY_CAPTURE_OK &&
       (capture_id = memory_capture_session_id(&capture)) != 0u &&
       memory_extract_text(&extract, &capture, capture_id, idea, strlen(idea), 96u,
                           &candidate) == MEMORY_EXTRACT_OK &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE,
       "M14 only a bounded physical capture and typed eligible signal reach human consolidation");

    memory_consolidation_config_default(&consolidation_cfg);
    consolidation_cfg.review_window_ms = 100u;
    ok(memory_consolidation_init(&consolidation, &consolidation_cfg) == MEMORY_CONSOLIDATION_OK,
       "M14 human review is an independent bounded authority boundary");
    memset(&proposal, 0, sizeof(proposal));
    proposal.card_id = 1401u;
    proposal.signal = candidate.signal;
    proposal.origin = candidate.origin;
    proposal.extract_reasons = candidate.reasons;
    review_access = physical(701u);
    receipt_id = consolidation.next_review_receipt_id;
    ok(memory_consolidation_begin(&consolidation, &proposal, review_access.physical_session_id, 200u) ==
           MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_confirm_store(&consolidation, &vault, &review_access, 201u) ==
           MEMORY_CONSOLIDATION_OK,
       "M14 same-session human confirmation emits the existing minimal-card authority before collection admission");

    memset(&card, 0, sizeof(card));
    card.card_id = proposal.card_id;
    card.review_receipt_id = receipt_id;
    card.signal = proposal.signal;
    card.origin = proposal.origin;
    card.extract_reasons = proposal.extract_reasons;
    auth.card_id = card.card_id;
    auth.review_receipt_id = card.review_receipt_id;
    auth.human_confirmed = 1u;
    memory_physical_session_config_default(&collection_gate_cfg);
    collection_gate_cfg.window_ms = 1000u;
    ok(memory_physical_session_init(&collection_gate, &collection_gate_cfg) ==
           MEMORY_PHYSICAL_SESSION_OK,
       "M14 collection uses a separate purpose-bound session gate without claiming a real gesture");
    collection_access_assertion = collection_access(&collection_gate,
                                                     MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                                     801u);
    ok(memory_collection_insert(&collection, &auth, &card, &collection_access_assertion) ==
           MEMORY_COLLECTION_OK && collection.state == MEMORY_COLLECTION_READY &&
       collection.metrics.inserts == 1u,
       "M14 collection accepts only the separately human-authorised minimal card under INSERT purpose");
    (void)memory_capture_cancel(&capture, 7u, 202u);

    ok(memory_collection_init(&reopened, &collection_cfg) == MEMORY_COLLECTION_OK &&
       reopened.state == MEMORY_COLLECTION_READY && reopened.generation == collection.generation,
       "M14 reopened collection accepts only its authenticated committed topology before indexing");
    memory_collection_index_config_default(&index_cfg);
    index_cfg.max_queries_per_session = 2u;
    ok(memory_collection_index_init(&index, &index_cfg) == MEMORY_COLLECTION_INDEX_OK,
       "M14 in-RAM index begins without a card cache, query or automatic open");
    query = idea_query();
    collection_access_assertion = collection_access(&collection_gate,
                                                     MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                                     802u);
    opens_before = reopened.metrics.opens;
    ok(memory_collection_index_query(&index, &reopened, &collection_access_assertion, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && result.status == MEMORY_RETRIEVAL_MATCH &&
       result.card_id == proposal.card_id && reopened.metrics.opens == opens_before,
       "M14 typed physical query returns only a minimal match and does not open the collection card");

    memory_retrieval_present_init(&presenter);
    ok(memory_retrieval_present_show(&presenter, &review_access, &result, &presentation) ==
           MEMORY_RETRIEVAL_PRESENT_OK &&
       presentation.phrase == MEMORY_RETRIEVAL_PHRASE_MATCH_AVAILABLE &&
       presentation.kind == MEMORY_KIND_IDEA && presentation.reasons != 0u,
       "M14 one-shot presentation exposes no opaque identifier or free content after collection match");
    snapshot = safe_snapshot(result.status);
    ok(memory_collection_finale_audit(&snapshot, &decision) == MEMORY_COLLECTION_FINALE_OK &&
       decision.chain_consistent == 1u && decision.failures == MEMORY_COLLECTION_FINALE_FAIL_NONE,
       "M14 composed evidence is diagnostic only and creates no new collection, index or presentation authority");

    ok(memory_physical_session_cancel(&collection_gate) == MEMORY_PHYSICAL_SESSION_OK,
       "M14 unfinished query capability is explicitly cancelled before a new retrieval purpose");
    query.preferred_kind = MEMORY_KIND_COMMITMENT;
    review_access = physical(702u);
    collection_access_assertion = collection_access(&collection_gate,
                                                     MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                                     803u);
    ok(memory_collection_index_query(&index, &reopened, &collection_access_assertion, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && result.status == MEMORY_RETRIEVAL_NO_MATCH &&
       result.card_id == 0u && result.reasons == 0u,
       "M14 no-match remains a terminal abstention with no collection enumeration");
    memory_retrieval_present_init(&presenter);
    ok(memory_retrieval_present_show(&presenter, &review_access, &result, &presentation) ==
           MEMORY_RETRIEVAL_PRESENT_OK &&
       presentation.phrase == MEMORY_RETRIEVAL_PHRASE_NO_MATCH && presentation.kind == MEMORY_KIND_NONE,
       "M14 no-match reaches only symbolic presentation, not a retry or fallback");
    snapshot = safe_snapshot(MEMORY_RETRIEVAL_NO_MATCH);
    ok(memory_collection_finale_audit(&snapshot, &decision) == MEMORY_COLLECTION_FINALE_OK &&
       decision.chain_consistent == 1u,
       "M14 abstention is a coherent composed result, never a permission to select a card");

    snapshot = safe_snapshot(MEMORY_RETRIEVAL_AMBIGUOUS);
    ok(memory_collection_finale_audit(&snapshot, &decision) == MEMORY_COLLECTION_FINALE_OK &&
       decision.chain_consistent == 1u,
       "M14 ambiguity is coherent only as uncertainty and cannot imply a winner");

    snapshot = safe_snapshot(MEMORY_RETRIEVAL_MATCH);
    snapshot.policy_disposition = MEMORY_DISPOSITION_REVIEW;
    snapshot.human_review_confirmed = 0u;
    snapshot.collection_state = MEMORY_COLLECTION_BLOCKED;
    snapshot.collection_physical_session_bound = 0u;
    snapshot.index_budget_respected = 0u;
    snapshot.query_result_card_auto_opened = 1u;
    snapshot.unit_vault_fallback_used = 1u;
    snapshot.model_in_memory_path = 1u;
    ok(memory_collection_finale_audit(&snapshot, &decision) == MEMORY_COLLECTION_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_POLICY) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_HUMAN_REVIEW) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_STATE) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_SESSION) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_INDEX_BUDGET) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_AUTO_OPEN) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_LEGACY_FALLBACK) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_MODEL_AGENCY) && !decision.chain_consistent,
       "M14 review, blocked collection/session, budget bypass, automatic open, legacy fallback and model agency all dominate success");

    snapshot = safe_snapshot(MEMORY_RETRIEVAL_MATCH);
    snapshot.capture_physical_validated = 2u;
    snapshot.collection_record_authenticated = 0u;
    snapshot.presentation_contract_valid = 0u;
    ok(memory_collection_finale_audit(&snapshot, &decision) == MEMORY_COLLECTION_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_CAPTURE) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_AUTH) &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_PRESENTATION_CONTRACT),
       "M14 noncanonical physical evidence, unauthenticated record and malformed presentation fail closed together");

    snapshot = safe_snapshot(MEMORY_RETRIEVAL_MATCH);
    snapshot.index_status = (memory_retrieval_status_t)99;
    ok(memory_collection_finale_audit(&snapshot, &decision) == MEMORY_COLLECTION_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_COLLECTION_FINALE_FAIL_INDEX_STATUS),
       "M14 unknown retrieval status cannot inherit a permissive terminal state");
    ok(memory_collection_finale_audit(0, &decision) == MEMORY_COLLECTION_FINALE_E_ARG &&
       memory_collection_finale_audit(&snapshot, 0) == MEMORY_COLLECTION_FINALE_E_ARG,
       "M14 missing audit input never creates a composed evidence decision");

    if (FAILED) {
        printf("COLLECTION FINALE INVARIANTS FAILED\n");
        return 1;
    }
    printf("COLLECTION FINALE INVARIANTS HOLD — human authority, transactional recovery and abstention compose without model, fallback or auto-open authority.\n");
    return 0;
}
