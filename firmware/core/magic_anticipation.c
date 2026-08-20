#include "magic_anticipation.h"
#include <string.h>

void magic_policy_default(magic_policy_t *policy)
{
    if (!policy) return;
    policy->max_steps = 32u;
    policy->allow_personal_explicit = 1u;
}

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int context_valid(const magic_context_t *context)
{
    return context && context->privacy_class >= MAGIC_PRIVACY_ORDINARY &&
           context->privacy_class <= MAGIC_PRIVACY_THIRD_PARTY &&
           context->request_kind >= MAGIC_REQUEST_EXPLICIT &&
           context->request_kind <= MAGIC_REQUEST_CONTEXTUAL &&
           canonical_bool(context->attention_window) &&
           canonical_bool(context->proactive_consent);
}

magic_status_t magic_propose(const sr_reasoner_t *base,
                             const mse_index_t *memory,
                             uint32_t current_generation,
                             const magic_context_t *context,
                             const magic_policy_t *policy,
                             sr_reasoner_t *scratch,
                             magic_proposal_t *out)
{
    mrb_status_t composed;
    if (out) memset(out, 0, sizeof(*out));
    if (!out || !base || !memory || !scratch || !context || !policy ||
        current_generation == 0u || !context_valid(context) ||
        policy->max_steps == 0u || !canonical_bool(policy->allow_personal_explicit))
        return MAGIC_ABSTAIN;
    if (context->privacy_class == MAGIC_PRIVACY_SENSITIVE ||
        context->privacy_class == MAGIC_PRIVACY_THIRD_PARTY) {
        out->status = MAGIC_SENSITIVE_BLOCK;
        return out->status;
    }
    if (context->privacy_class == MAGIC_PRIVACY_PERSONAL &&
        (context->request_kind != MAGIC_REQUEST_EXPLICIT ||
         policy->allow_personal_explicit != 1u)) {
        out->status = MAGIC_SENSITIVE_BLOCK;
        return out->status;
    }
    if (context->request_kind == MAGIC_REQUEST_CONTEXTUAL &&
        (context->attention_window != 1u || context->proactive_consent != 1u)) {
        out->status = MAGIC_SILENT;
        return out->status;
    }
    composed = mrb_query(base, memory, current_generation, &context->cue,
                         policy->max_steps, scratch, &out->answer,
                         &out->composition);
    if (composed == MRB_OK) {
        out->status = out->answer.kind == SR_ANSWER_DIRECT
                          ? MAGIC_RECALL : MAGIC_CONNECTION;
        out->requires_confirmation = 0u;
        out->explanation_available = 1u;
        out->explanation_code = out->composition.selected_card_id;
        return out->status;
    }
    if (composed == MRB_NO_EVIDENCE) {
        out->status = MAGIC_KNOWN_GAP;
        out->explanation_available = 1u;
        return out->status;
    }
    if (composed == MRB_CONTRADICTED) {
        out->status = MAGIC_CONTRADICTION;
        out->explanation_available = 1u;
        out->requires_confirmation = 1u;
        return out->status;
    }
    if (composed == MRB_AMBIGUOUS) {
        out->status = MAGIC_ABSTAIN;
        out->explanation_available = 1u;
        return out->status;
    }
    if (composed == MRB_LIMIT) {
        out->status = MAGIC_LIMIT;
        out->explanation_available = 0u;
        return out->status;
    }
    out->status = MAGIC_ABSTAIN;
    return out->status;
}
