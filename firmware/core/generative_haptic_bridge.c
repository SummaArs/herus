#include "generative_haptic_bridge.h"

#include <string.h>

static void clear_event(gh_signal_t *out)
{
    memset(out, 0, sizeof(*out));
    out->event.version = HL_VERSION_1;
    out->event.fragment_index = 0u;
    out->event.fragment_total = 1u;
}

static int fill_result_event(const gc_result_t *result, gh_signal_t *out)
{
    out->event.urgency = HL_URGENCY_U1;
    if (result->authority == GC_AUTH_CONFIRMATION_REQUIRED ||
        result->requires_confirmation != 0u) {
        out->event.scope = HL_SCOPE_PLAN;
        out->event.class_code = HL_CLASS_QUERY;
        out->event.state = HL_STATE_PENDING;
        out->confirmation_required = 1u;
        return HL_OK;
    }
    if (result->kind == GC_KIND_POLICY_BLOCKED ||
        result->abstain_reason == GC_ABSTAIN_POLICY) {
        out->event.scope = HL_SCOPE_SFTY;
        out->event.class_code = HL_CLASS_PRIVACY;
        out->event.state = HL_STATE_DENIED;
        out->abstained = 1u;
        return HL_OK;
    }
    if (result->status == GC_STATUS_ABSTAIN ||
        result->status == GC_STATUS_LIMIT ||
        result->kind == GC_KIND_UNKNOWN ||
        result->kind == GC_KIND_AMBIGUOUS ||
        result->kind == GC_KIND_CONTRADICTED ||
        result->kind == GC_KIND_UNSUPPORTED ||
        result->kind == GC_KIND_LIMIT) {
        out->event.scope = result->grounded != 0u ? HL_SCOPE_MEM : HL_SCOPE_COM;
        out->event.class_code = result->kind == GC_KIND_CONTRADICTED ?
                                HL_CLASS_ALERT : HL_CLASS_ERROR;
        out->event.state = HL_STATE_UNKNOWN;
        out->abstained = 1u;
        return HL_OK;
    }
    if (result->kind == GC_KIND_PLAN) {
        out->event.scope = HL_SCOPE_PLAN;
        out->event.class_code = HL_CLASS_QUERY;
        out->event.state = HL_STATE_PENDING;
        out->confirmation_required = result->requires_confirmation;
        return HL_OK;
    }
    out->event.scope = result->grounded != 0u ? HL_SCOPE_MEM : HL_SCOPE_COM;
    out->event.class_code = result->kind == GC_KIND_COUNTERFACTUAL ?
                            HL_CLASS_QUERY : HL_CLASS_ACK;
    out->event.state = HL_STATE_CONFIRMED;
    out->event.urgency = result->kind == GC_KIND_COUNTERFACTUAL ?
                         HL_URGENCY_U1 : HL_URGENCY_U0;
    return HL_OK;
}

int gh_from_result(const gc_result_t *result, gh_signal_t *out)
{
    if (result == NULL || out == NULL) return HL_E_ARG;
    clear_event(out);
    out->actionable = 0u;
    return fill_result_event(result, out);
}
