/* test_memory_finale.c — Grand Finale selective-memory composition proof. */
#include "memory_finale.h"
#include "memory_capture.h"
#include "memory_extract.h"
#include "memory_consolidation.h"
#include "memory_retrieval_present.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

typedef struct {
    uint8_t root[MEMORY_VAULT_ROOT_LEN];
    uint8_t blob[MEMORY_VAULT_BLOB_LEN];
    uint32_t floor;
    int have_blob;
} fake_backend_t;

static fake_backend_t *ROOT_BACKEND = 0;

int memory_vault_platform_load_root(uint32_t vault_id,
                                    uint8_t out[MEMORY_VAULT_ROOT_LEN])
{
    if (!ROOT_BACKEND || vault_id != 0x48455255u) return -1;
    memcpy(out, ROOT_BACKEND->root, sizeof(ROOT_BACKEND->root));
    return 0;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static int fake_store(void *ctx, const uint8_t blob[MEMORY_VAULT_BLOB_LEN])
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    memcpy(f->blob, blob, sizeof(f->blob));
    f->have_blob = 1;
    return 0;
}

static int fake_load(void *ctx, uint8_t blob[MEMORY_VAULT_BLOB_LEN])
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    if (!f->have_blob) return -1;
    memcpy(blob, f->blob, sizeof(f->blob));
    return 0;
}

static int fake_erase(void *ctx)
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    memset(f->blob, 0, sizeof(f->blob));
    f->have_blob = 0;
    return 0;
}

static int fake_load_floor(void *ctx, uint32_t *out)
{
    *out = ((fake_backend_t *)ctx)->floor;
    return 0;
}

static int fake_commit_floor(void *ctx, uint32_t generation)
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    if (generation <= f->floor) return -1;
    f->floor = generation;
    return 0;
}

static void fake_backend_init(fake_backend_t *f)
{
    size_t i;
    memset(f, 0, sizeof(*f));
    for (i = 0u; i < sizeof(f->root); ++i) f->root[i] = (uint8_t)(0x31u + i);
}

static memory_vault_config_t vault_config(fake_backend_t *f)
{
    memory_vault_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.vault_id = 0x48455255u;
    cfg.storage.ctx = f;
    cfg.storage.store_sealed = fake_store;
    cfg.storage.load_sealed = fake_load;
    cfg.storage.erase_sealed = fake_erase;
    cfg.storage.load_generation_floor = fake_load_floor;
    cfg.storage.commit_generation_floor = fake_commit_floor;
    return cfg;
}

static memory_consolidation_access_t physical(uint32_t session)
{
    memory_consolidation_access_t a;
    a.physical_session_id = session;
    a.physical_confirmed = 1u;
    return a;
}

static memory_finale_snapshot_t safe_snapshot(memory_retrieval_status_t status)
{
    memory_finale_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.capture_physical_validated = 1u;
    s.extraction_typed = 1u;
    s.policy_disposition = MEMORY_DISPOSITION_AUTO_ELIGIBLE;
    s.human_review_confirmed = 1u;
    s.consolidation_conflicted = 0u;
    s.vault_sealed = 1u;
    s.retrieval_physical_access = 1u;
    s.retrieval_status = status;
    s.presentation_physical_access = 1u;
    s.presentation_one_shot_enforced = 1u;
    s.presentation_contract_valid = 1u;
    s.model_in_memory_path = 0u;
    return s;
}

static memory_retrieval_query_t idea_query(void)
{
    memory_retrieval_query_t q;
    memset(&q, 0, sizeof(q));
    q.preferred_kind = MEMORY_KIND_IDEA;
    q.preferred_origin = MEMORY_EXTRACT_EXPLICIT;
    q.require_explicit = 1u;
    q.minimum_confidence_pct = 90u;
    return q;
}

int main(void)
{
    fake_backend_t f;
    memory_capture_t capture;
    memory_capture_config_t capture_cfg;
    memory_extract_t extract;
    memory_candidate_t candidate;
    memory_assessment_t assessment;
    memory_consolidation_t consolidation;
    memory_consolidation_config_t consolidation_cfg;
    memory_consolidation_proposal_t proposal;
    memory_consolidation_access_t access;
    memory_vault_config_t vcfg;
    memory_vault_t vault;
    memory_vault_card_t card;
    memory_retrieval_t retrieval;
    memory_retrieval_query_t query;
    memory_retrieval_result_t retrieved;
    memory_retrieval_present_t presenter;
    memory_retrieval_presentation_t presentation;
    memory_finale_snapshot_t snapshot;
    memory_finale_decision_t decision;
    char idea[] = "lembre esta ideia: nucleo como segundo cerebro";
    char other[] = "lembre fato de projeto: saude de outra pessoa";
    uint32_t capture_id;

    printf("\n== M8 Grand Finale: selective memory remains human, private and fail-closed ==\n");
    fake_backend_init(&f);
    ROOT_BACKEND = &f;
    vcfg = vault_config(&f);
    ok(memory_vault_init(&vault, &vcfg) == MEMORY_VAULT_OK,
       "M8 fixture vault begins with a protected-generation contract");

    memory_capture_config_default(&capture_cfg);
    capture_cfg.window_ms = 1000u;
    memory_capture_init(&capture, &capture_cfg, 0);
    memory_extract_init(&extract);
    ok(memory_capture_begin(&capture, 5u, 100u) == MEMORY_CAPTURE_OK &&
       memory_capture_session_id(&capture) != 0u,
       "M8 the chain starts only in an explicit bounded physical capture session");
    capture_id = memory_capture_session_id(&capture);
    ok(memory_extract_text(&extract, &capture, capture_id, idea, strlen(idea), 96u,
                           &candidate) == MEMORY_EXTRACT_OK &&
       candidate.signal.scope == MEMORY_SCOPE_SELF &&
       candidate.signal.sensitivity == MEMORY_SENSITIVITY_ORDINARY &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE,
       "M8 conservative extraction and policy produce only a typed eligible candidate, never content retention");

    memory_consolidation_config_default(&consolidation_cfg);
    consolidation_cfg.review_window_ms = 100u;
    ok(memory_consolidation_init(&consolidation, &consolidation_cfg) == MEMORY_CONSOLIDATION_OK,
       "M8 human consolidation begins independently of capture and storage");
    memset(&proposal, 0, sizeof(proposal));
    proposal.card_id = 41u;
    proposal.signal = candidate.signal;
    proposal.origin = candidate.origin;
    proposal.extract_reasons = candidate.reasons;
    access = physical(77u);
    ok(memory_consolidation_begin(&consolidation, &proposal, access.physical_session_id, 200u) ==
       MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_confirm_store(&consolidation, &vault, &access, 201u) ==
       MEMORY_CONSOLIDATION_OK && f.have_blob && f.floor == 1u,
       "M8 eligible proposal reaches the vault only through separate same-session human confirmation");
    (void)memory_capture_cancel(&capture, 5u, 202u);

    ok(memory_consolidation_recall(&consolidation, &vault, proposal.card_id, &access, &card) ==
       MEMORY_CONSOLIDATION_OK && card.card_id == proposal.card_id,
       "M8 controlled recall returns only the minimal typed card under physical access");
    memory_retrieval_init(&retrieval);
    query = idea_query();
    ok(memory_retrieval_query(&retrieval, &access, &query, &card, 1u, &retrieved) ==
       MEMORY_RETRIEVAL_OK && retrieved.status == MEMORY_RETRIEVAL_MATCH &&
       retrieved.card_id == proposal.card_id,
       "M8 local retrieval ranks the authorised typed card without opening storage or using a model");
    memory_retrieval_present_init(&presenter);
    ok(memory_retrieval_present_show(&presenter, &access, &retrieved, &presentation) ==
       MEMORY_RETRIEVAL_PRESENT_OK &&
       presentation.phrase == MEMORY_RETRIEVAL_PHRASE_MATCH_AVAILABLE &&
       presentation.kind == MEMORY_KIND_IDEA && presentation.reasons != 0u,
       "M8 presentation exposes a bounded local status without identifier or free content");
    snapshot = safe_snapshot(retrieved.status);
    ok(memory_finale_audit(&snapshot, &decision) == MEMORY_FINALE_OK &&
       decision.chain_consistent == 1u && decision.failures == MEMORY_FINALE_FAIL_NONE,
       "M8 the entire authorised fixture chain is compositionally consistent without authorising any new action");

    memory_retrieval_present_init(&presenter);
    memset(&retrieved, 0, sizeof(retrieved));
    retrieved.status = MEMORY_RETRIEVAL_AMBIGUOUS;
    retrieved.score_pct = 82u;
    retrieved.runner_up_score_pct = 77u;
    ok(memory_retrieval_present_show(&presenter, &access, &retrieved, &presentation) ==
       MEMORY_RETRIEVAL_PRESENT_OK &&
       presentation.phrase == MEMORY_RETRIEVAL_PHRASE_AMBIGUOUS_REVIEW &&
       presentation.kind == MEMORY_KIND_NONE && presentation.reasons == 0u,
       "M8 an ambiguous recovery remains a valid final status but never exposes or selects a contender");
    snapshot = safe_snapshot(MEMORY_RETRIEVAL_AMBIGUOUS);
    ok(memory_finale_audit(&snapshot, &decision) == MEMORY_FINALE_OK &&
       decision.chain_consistent == 1u,
       "M8 ambiguity is evidence of uncertainty, not a broken chain or permission to guess");

    memory_capture_init(&capture, &capture_cfg, 0);
    (void)memory_capture_begin(&capture, 6u, 300u);
    capture_id = memory_capture_session_id(&capture);
    ok(memory_extract_text(&extract, &capture, capture_id, other, strlen(other), 95u,
                           &candidate) == MEMORY_EXTRACT_OK &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_REVIEW,
       "M8 third-party sensitive fixture remains review-only before consolidation");
    proposal.card_id = 42u;
    proposal.signal = candidate.signal;
    proposal.origin = candidate.origin;
    proposal.extract_reasons = candidate.reasons;
    ok(memory_consolidation_begin(&consolidation, &proposal, access.physical_session_id, 301u) ==
       MEMORY_CONSOLIDATION_E_PROPOSAL && f.floor == 1u,
       "M8 review-only third-party sensitive input cannot cross into persistence even with a session");
    snapshot = safe_snapshot(MEMORY_RETRIEVAL_MATCH);
    snapshot.policy_disposition = MEMORY_DISPOSITION_REVIEW;
    ok(memory_finale_audit(&snapshot, &decision) == MEMORY_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_FINALE_FAIL_POLICY) && !decision.chain_consistent,
       "M8 final audit records policy refusal as a blocking cross-layer failure");

    memory_consolidation_init(&consolidation, &consolidation_cfg);
    proposal.card_id = 43u;
    proposal.signal = card.signal;
    proposal.origin = card.origin;
    proposal.extract_reasons = card.extract_reasons;
    ok(memory_consolidation_begin(&consolidation, &proposal, access.physical_session_id, 400u) ==
       MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_mark_conflict(&consolidation, 99u, 401u) == MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_confirm_store(&consolidation, &vault, &access, 402u) ==
       MEMORY_CONSOLIDATION_E_CONFLICT && f.floor == 1u,
       "M8 explicit conflict blocks a later confirmation and cannot overwrite the sealed card");
    snapshot = safe_snapshot(MEMORY_RETRIEVAL_MATCH);
    snapshot.consolidation_conflicted = 1u;
    ok(memory_finale_audit(&snapshot, &decision) == MEMORY_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_FINALE_FAIL_CONFLICT),
       "M8 conflict dominates all otherwise plausible downstream evidence");

    snapshot = safe_snapshot(MEMORY_RETRIEVAL_MATCH);
    snapshot.model_in_memory_path = 1u;
    snapshot.capture_physical_validated = 2u;
    ok(memory_finale_audit(&snapshot, &decision) == MEMORY_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_FINALE_FAIL_MODEL_AGENCY) &&
       (decision.failures & MEMORY_FINALE_FAIL_CAPTURE),
       "M8 model presence and noncanonical physical evidence fail closed even with all other fields set");

    snapshot = safe_snapshot(MEMORY_RETRIEVAL_NO_MATCH);
    snapshot.presentation_contract_valid = 0u;
    ok(memory_finale_audit(&snapshot, &decision) == MEMORY_FINALE_E_BLOCKED &&
       (decision.failures & MEMORY_FINALE_FAIL_PRESENTATION_CONTRACT),
       "M8 malformed or unverified presentation cannot be treated as harmless UI after retrieval");

    if (FAILED) {
        printf("MEMORY GRAND FINALE TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY GRAND FINALE INVARIANTS HOLD — the composed chain is private, human-gated, uncertainty-safe and has zero model authority.\n");
    return 0;
}
