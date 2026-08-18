/* memory_proposal.c — deterministic compiler for untrusted typed model output. */
#include "memory_proposal.h"
#include <string.h>

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_kind(memory_kind_t kind)
{
    return kind > MEMORY_KIND_NONE && kind < MEMORY_KIND_COUNT;
}

static int valid_scope(memory_scope_t scope)
{
    return scope > MEMORY_SCOPE_NONE && scope < MEMORY_SCOPE_COUNT;
}

static int valid_sensitivity(memory_sensitivity_t sensitivity)
{
    return sensitivity > MEMORY_SENSITIVITY_NONE &&
           sensitivity < MEMORY_SENSITIVITY_COUNT;
}

static void clear_candidate(memory_candidate_t *out)
{
    memset(out, 0, sizeof(*out));
}

void memory_proposal_init(memory_proposal_t *compiler)
{
    if (compiler) memset(compiler, 0, sizeof(*compiler));
}

int memory_proposal_compile(memory_proposal_t *compiler,
                            const memory_capture_t *capture,
                            uint32_t capture_session_id,
                            const memory_model_proposal_t *proposal,
                            memory_candidate_t *out)
{
    memory_signal_t *signal;

    if (!compiler || !capture || !proposal || !out)
        return MEMORY_PROPOSAL_E_ARG;
    clear_candidate(out);
    compiler->metrics.calls++;

    if (capture_session_id == 0u ||
        capture_session_id != memory_capture_session_id(capture)) {
        compiler->metrics.rejected_session++;
        return MEMORY_PROPOSAL_E_SESSION;
    }
    if (proposal->schema_version != MEMORY_PROPOSAL_SCHEMA_VERSION) {
        compiler->metrics.rejected_schema++;
        return MEMORY_PROPOSAL_E_SCHEMA;
    }
    if (!canonical_bool(proposal->abstain)) {
        compiler->metrics.rejected_value++;
        return MEMORY_PROPOSAL_E_VALUE;
    }
    if (proposal->abstain == 1u) {
        compiler->metrics.abstained++;
        return MEMORY_PROPOSAL_NO_CANDIDATE;
    }
    if (!valid_kind(proposal->kind) || !valid_scope(proposal->scope) ||
        !valid_sensitivity(proposal->sensitivity) ||
        proposal->confidence_pct > 100u || proposal->novelty_pct > 100u ||
        proposal->future_value_pct > 100u || proposal->consequence_pct > 100u) {
        compiler->metrics.rejected_value++;
        return MEMORY_PROPOSAL_E_VALUE;
    }

    signal = &out->signal;
    signal->session_authorized = 1u;
    /* A model proposal cannot manufacture human confirmation. */
    signal->explicit_remember = 0u;
    signal->scope = proposal->scope;
    signal->sensitivity = proposal->sensitivity;
    signal->confidence_pct = proposal->confidence_pct;
    signal->novelty_pct = proposal->novelty_pct;
    signal->future_value_pct = proposal->future_value_pct;
    signal->consequence_pct = proposal->consequence_pct;
    signal->kind = proposal->kind;

    out->origin = MEMORY_EXTRACT_CONTROLLED_INFERENCE;
    out->reasons = MEMORY_EXTRACT_REASON_NONE;
    switch (proposal->kind) {
    case MEMORY_KIND_IDEA:
        out->reasons |= MEMORY_EXTRACT_REASON_IDEA;
        break;
    case MEMORY_KIND_DECISION:
        out->reasons |= MEMORY_EXTRACT_REASON_DECISION;
        break;
    case MEMORY_KIND_COMMITMENT:
        out->reasons |= MEMORY_EXTRACT_REASON_COMMITMENT;
        break;
    case MEMORY_KIND_PREFERENCE:
        out->reasons |= MEMORY_EXTRACT_REASON_PREFERENCE;
        break;
    case MEMORY_KIND_PROJECT_FACT:
        out->reasons |= MEMORY_EXTRACT_REASON_PROJECT;
        break;
    case MEMORY_KIND_ROUTINE:
        out->reasons |= MEMORY_EXTRACT_REASON_ROUTINE;
        break;
    default:
        break;
    }
    if (proposal->confidence_pct < MEMORY_POLICY_MIN_CONFIDENCE_PCT)
        out->reasons |= MEMORY_EXTRACT_REASON_AMBIGUOUS;
    if (proposal->scope != MEMORY_SCOPE_SELF)
        out->reasons |= MEMORY_EXTRACT_REASON_THIRD_PARTY;
    if (proposal->sensitivity != MEMORY_SENSITIVITY_ORDINARY)
        out->reasons |= MEMORY_EXTRACT_REASON_SENSITIVE;

    compiler->metrics.compiled++;
    return MEMORY_PROPOSAL_OK;
}

const memory_proposal_metrics_t *memory_proposal_metrics(const memory_proposal_t *compiler)
{
    return compiler ? &compiler->metrics : 0;
}
