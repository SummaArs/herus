/* memory_consolidation.c — bounded human review around the Step-4 vault. */
#include "memory_consolidation.h"
#include "crypto.h"
#include <limits.h>
#include <string.h>

static int config_valid(const memory_consolidation_config_t *cfg)
{
    return cfg && cfg->review_window_ms != 0u && cfg->review_window_ms <= INT32_MAX;
}

static int proposal_valid(const memory_consolidation_proposal_t *proposal)
{
    memory_assessment_t assessment;
    if (!proposal || proposal->card_id == 0u ||
        proposal->signal.session_authorized != 1u ||
        (proposal->signal.explicit_remember != 0u &&
         proposal->signal.explicit_remember != 1u) ||
        proposal->signal.kind <= MEMORY_KIND_NONE ||
        proposal->signal.kind >= MEMORY_KIND_COUNT ||
        proposal->signal.scope != MEMORY_SCOPE_SELF ||
        proposal->signal.sensitivity != MEMORY_SENSITIVITY_ORDINARY ||
        proposal->signal.confidence_pct > 100u ||
        proposal->signal.novelty_pct > 100u ||
        proposal->signal.future_value_pct > 100u ||
        proposal->signal.consequence_pct > 100u ||
        proposal->origin <= MEMORY_EXTRACT_ORIGIN_NONE ||
        proposal->origin >= MEMORY_EXTRACT_ORIGIN_COUNT ||
        proposal->extract_reasons == 0u)
        return 0;
    if (memory_policy_assess(&proposal->signal, &assessment) != MEMORY_POLICY_OK)
        return 0;
    return assessment.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE;
}

static int access_valid(const memory_consolidation_access_t *access)
{
    return access && access->physical_session_id != 0u &&
           access->physical_confirmed == 1u;
}

static int review_elapsed(const memory_consolidation_t *c, uint32_t now_ms)
{
    return (uint32_t)(now_ms - c->started_at_ms) >= c->cfg.review_window_ms;
}

static void scrub_pending(memory_consolidation_t *c)
{
    secure_zero(&c->pending, sizeof(c->pending));
    c->active_physical_session_id = 0u;
    c->started_at_ms = 0u;
}

static int expire_if_due(memory_consolidation_t *c, uint32_t now_ms)
{
    if (c->state != MEMORY_CONSOLIDATION_REVIEWING &&
        c->state != MEMORY_CONSOLIDATION_CONFLICTED)
        return 0;
    if (!review_elapsed(c, now_ms)) return 0;
    scrub_pending(c);
    c->state = MEMORY_CONSOLIDATION_EXPIRED;
    c->metrics.expired++;
    return 1;
}

void memory_consolidation_config_default(memory_consolidation_config_t *cfg)
{
    if (!cfg) return;
    cfg->review_window_ms = MEMORY_CONSOLIDATION_REVIEW_WINDOW_MS;
}

int memory_consolidation_init(memory_consolidation_t *c,
                              const memory_consolidation_config_t *cfg)
{
    if (!c || !cfg) return MEMORY_CONSOLIDATION_E_ARG;
    memset(c, 0, sizeof(*c));
    if (!config_valid(cfg)) return MEMORY_CONSOLIDATION_E_CONFIG;
    c->cfg = *cfg;
    c->state = MEMORY_CONSOLIDATION_IDLE;
    c->next_review_receipt_id = 1u;
    return MEMORY_CONSOLIDATION_OK;
}

int memory_consolidation_begin(memory_consolidation_t *c,
                               const memory_consolidation_proposal_t *proposal,
                               uint32_t physical_session_id, uint32_t now_ms)
{
    if (!c || !proposal) return MEMORY_CONSOLIDATION_E_ARG;
    if (c->state != MEMORY_CONSOLIDATION_IDLE) return MEMORY_CONSOLIDATION_E_STATE;
    if (physical_session_id == 0u || !proposal_valid(proposal)) {
        c->metrics.rejected_proposals++;
        return MEMORY_CONSOLIDATION_E_PROPOSAL;
    }
    c->pending = *proposal;
    c->active_physical_session_id = physical_session_id;
    c->started_at_ms = now_ms;
    c->state = MEMORY_CONSOLIDATION_REVIEWING;
    c->metrics.begun++;
    return MEMORY_CONSOLIDATION_OK;
}

int memory_consolidation_expire(memory_consolidation_t *c, uint32_t now_ms)
{
    if (!c) return MEMORY_CONSOLIDATION_E_ARG;
    if (c->state != MEMORY_CONSOLIDATION_REVIEWING &&
        c->state != MEMORY_CONSOLIDATION_CONFLICTED)
        return MEMORY_CONSOLIDATION_E_STATE;
    (void)expire_if_due(c, now_ms);
    return c->state == MEMORY_CONSOLIDATION_EXPIRED ?
           MEMORY_CONSOLIDATION_E_EXPIRED : MEMORY_CONSOLIDATION_OK;
}

int memory_consolidation_mark_conflict(memory_consolidation_t *c,
                                      uint32_t competing_card_id, uint32_t now_ms)
{
    if (!c || competing_card_id == 0u) return MEMORY_CONSOLIDATION_E_ARG;
    if (expire_if_due(c, now_ms)) return MEMORY_CONSOLIDATION_E_EXPIRED;
    if (c->state == MEMORY_CONSOLIDATION_CONFLICTED) return MEMORY_CONSOLIDATION_E_CONFLICT;
    if (c->state != MEMORY_CONSOLIDATION_REVIEWING) return MEMORY_CONSOLIDATION_E_STATE;
    if (competing_card_id == c->pending.card_id) return MEMORY_CONSOLIDATION_E_ARG;
    c->state = MEMORY_CONSOLIDATION_CONFLICTED;
    c->metrics.conflicts++;
    return MEMORY_CONSOLIDATION_OK;
}

/* HERUS_CRITICAL_SINK: memory-review-persist operation=memory_vault_seal( */
int memory_consolidation_confirm_store(memory_consolidation_t *c,
                                       memory_vault_t *vault,
                                       const memory_consolidation_access_t *access,
                                       uint32_t now_ms)
{
    memory_vault_card_t card;
    memory_vault_write_authorization_t auth;
    int rc;

    if (!c || !vault || !access) return MEMORY_CONSOLIDATION_E_ARG;
    if (expire_if_due(c, now_ms)) return MEMORY_CONSOLIDATION_E_EXPIRED;
    if (c->state == MEMORY_CONSOLIDATION_CONFLICTED) return MEMORY_CONSOLIDATION_E_CONFLICT;
    if (c->state != MEMORY_CONSOLIDATION_REVIEWING) return MEMORY_CONSOLIDATION_E_STATE;
    if (!access_valid(access) ||
        access->physical_session_id != c->active_physical_session_id) {
        c->metrics.rejected_access++;
        return MEMORY_CONSOLIDATION_E_ACCESS;
    }
    if (c->next_review_receipt_id == 0u) {
        scrub_pending(c);
        c->state = MEMORY_CONSOLIDATION_FAILED;
        return MEMORY_CONSOLIDATION_E_STATE;
    }

    memset(&card, 0, sizeof(card));
    card.card_id = c->pending.card_id;
    card.review_receipt_id = c->next_review_receipt_id;
    card.signal = c->pending.signal;
    card.origin = c->pending.origin;
    card.extract_reasons = c->pending.extract_reasons;
    auth.card_id = card.card_id;
    auth.review_receipt_id = card.review_receipt_id;
    auth.human_confirmed = 1u;

    rc = memory_vault_seal(vault, &auth, &card);
    secure_zero(&card, sizeof(card));
    secure_zero(&auth, sizeof(auth));
    scrub_pending(c);
    if (rc != MEMORY_VAULT_OK) {
        c->metrics.vault_failures++;
        c->state = MEMORY_CONSOLIDATION_FAILED;
        return MEMORY_CONSOLIDATION_E_VAULT;
    }
    c->next_review_receipt_id++;
    c->state = MEMORY_CONSOLIDATION_IDLE;
    c->metrics.persisted++;
    return MEMORY_CONSOLIDATION_OK;
}

int memory_consolidation_cancel(memory_consolidation_t *c)
{
    if (!c) return MEMORY_CONSOLIDATION_E_ARG;
    if (c->state != MEMORY_CONSOLIDATION_REVIEWING &&
        c->state != MEMORY_CONSOLIDATION_CONFLICTED &&
        c->state != MEMORY_CONSOLIDATION_EXPIRED)
        return MEMORY_CONSOLIDATION_E_STATE;
    scrub_pending(c);
    c->state = MEMORY_CONSOLIDATION_IDLE;
    c->metrics.cancelled++;
    return MEMORY_CONSOLIDATION_OK;
}

int memory_consolidation_recall(memory_consolidation_t *c, memory_vault_t *vault,
                                uint32_t expected_card_id,
                                const memory_consolidation_access_t *access,
                                memory_vault_card_t *out)
{
    int rc;
    if (!c || !vault || !access || !out || expected_card_id == 0u)
        return MEMORY_CONSOLIDATION_E_ARG;
    memset(out, 0, sizeof(*out));
    if (c->state != MEMORY_CONSOLIDATION_IDLE) return MEMORY_CONSOLIDATION_E_STATE;
    if (!access_valid(access)) {
        c->metrics.rejected_access++;
        return MEMORY_CONSOLIDATION_E_ACCESS;
    }
    rc = memory_vault_open(vault, expected_card_id, out);
    if (rc != MEMORY_VAULT_OK) {
        c->metrics.vault_failures++;
        c->state = MEMORY_CONSOLIDATION_FAILED;
        memset(out, 0, sizeof(*out));
        return MEMORY_CONSOLIDATION_E_VAULT;
    }
    c->metrics.recalled++;
    return MEMORY_CONSOLIDATION_OK;
}

int memory_consolidation_erase(memory_consolidation_t *c, memory_vault_t *vault,
                               const memory_consolidation_access_t *access)
{
    int rc;
    if (!c || !vault || !access) return MEMORY_CONSOLIDATION_E_ARG;
    if (c->state != MEMORY_CONSOLIDATION_IDLE) return MEMORY_CONSOLIDATION_E_STATE;
    if (!access_valid(access)) {
        c->metrics.rejected_access++;
        return MEMORY_CONSOLIDATION_E_ACCESS;
    }
    rc = memory_vault_erase(vault);
    if (rc != MEMORY_VAULT_OK) {
        c->metrics.vault_failures++;
        c->state = MEMORY_CONSOLIDATION_FAILED;
        return MEMORY_CONSOLIDATION_E_VAULT;
    }
    c->metrics.erased++;
    return MEMORY_CONSOLIDATION_OK;
}

const memory_consolidation_metrics_t *memory_consolidation_metrics(
    const memory_consolidation_t *c)
{
    return c ? &c->metrics : 0;
}
