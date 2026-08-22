#include "haptic_semantic_bridge.h"

#include <string.h>

static void base_event(hl_event_t *event)
{
    memset(event, 0, sizeof(*event));
    event->version = HL_VERSION_1;
    event->fragment_index = 0u;
    event->fragment_total = 1u;
}

static void mark_unknown(hs_signal_t *out, uint8_t scope)
{
    out->event.scope = scope;
    out->event.class_code = HL_CLASS_ERROR;
    out->event.state = HL_STATE_UNKNOWN;
    out->event.urgency = HL_URGENCY_U1;
    out->abstained = 1u;
    out->confirmation_required = 0u;
    out->actionable = 0u;
}

int hs_from_compiler(const sc_unit_t *unit, const sc_bridge_result_t *bridge,
                     hs_signal_t *out)
{
    if (!unit || !out) return HL_E_ARG;
    memset(out, 0, sizeof(*out));
    base_event(&out->event);
    if (unit->status != SC_OK || unit->exact_parse != 1u) {
        out->event.scope = HL_SCOPE_SFTY;
        out->event.class_code = unit->status == SC_E_SENSITIVE
                                    ? HL_CLASS_PRIVACY : HL_CLASS_ERROR;
        out->event.state = unit->status == SC_E_SENSITIVE
                               ? HL_STATE_DENIED : HL_STATE_UNKNOWN;
        out->event.urgency = unit->status == SC_E_SENSITIVE
                                 ? HL_URGENCY_U2 : HL_URGENCY_U1;
        out->abstained = 1u;
        out->actionable = 0u;
        return HL_OK;
    }

    if (unit->kind == SC_UNIT_REJECT) {
        out->event.scope = HL_SCOPE_MEM;
        out->event.class_code = HL_CLASS_PRIVACY;
        out->event.state = HL_STATE_DENIED;
        out->event.urgency = HL_URGENCY_U1;
        out->abstained = 0u;
        out->actionable = 0u;
        return HL_OK;
    }

    if (unit->kind == SC_UNIT_FACT || unit->kind == SC_UNIT_RULE) {
        out->event.scope = unit->kind == SC_UNIT_FACT ? HL_SCOPE_MEM : HL_SCOPE_PLAN;
        out->event.class_code = HL_CLASS_NOTICE;
        out->event.state = HL_STATE_PENDING;
        out->event.urgency = HL_URGENCY_U1;
        out->confirmation_required = unit->requires_confirmation;
        out->actionable = 0u;
        if (!bridge) return HL_OK;
        if (bridge->status == SC_BRIDGE_OK && bridge->state_changed != 0u) {
            out->event.class_code = HL_CLASS_ACK;
            out->event.state = HL_STATE_CONFIRMED;
            out->confirmation_required = 0u;
        } else if (bridge->status == SC_BRIDGE_E_LIMIT ||
                   bridge->status == SC_BRIDGE_E_ABSTAIN) {
            mark_unknown(out, out->event.scope);
        } else if (bridge->status == SC_BRIDGE_E_AUTH) {
            out->event.state = HL_STATE_PENDING;
            out->confirmation_required = 1u;
        }
        return HL_OK;
    }

    if (unit->kind == SC_UNIT_QUERY) {
        out->event.scope = HL_SCOPE_MEM;
        out->event.class_code = HL_CLASS_QUERY;
        out->event.state = HL_STATE_PENDING;
        out->event.urgency = HL_URGENCY_U1;
        out->actionable = 0u;
        if (!bridge) return HL_OK;
        if (bridge->status == SC_BRIDGE_OK) {
            out->event.state = HL_STATE_CONFIRMED;
        } else if (bridge->status == SC_BRIDGE_E_LIMIT ||
                   bridge->status == SC_BRIDGE_E_ABSTAIN) {
            mark_unknown(out, HL_SCOPE_MEM);
        }
        return HL_OK;
    }

    if (unit->kind == SC_UNIT_GOAL) {
        out->event.scope = HL_SCOPE_PLAN;
        out->event.class_code = HL_CLASS_QUERY;
        out->event.state = HL_STATE_PENDING;
        out->event.urgency = HL_URGENCY_U1;
        out->confirmation_required = 1u;
        out->actionable = 0u;
        if (!bridge) return HL_OK;
        if (bridge->status == SC_BRIDGE_OK && bridge->plan.plan_length > 0u) {
            out->event.state = HL_STATE_PENDING;
        } else if (bridge->status == SC_BRIDGE_E_NO_PLAN ||
                   bridge->status == SC_BRIDGE_E_LIMIT ||
                   bridge->status == SC_BRIDGE_E_ABSTAIN) {
            mark_unknown(out, HL_SCOPE_PLAN);
        }
        return HL_OK;
    }

    mark_unknown(out, HL_SCOPE_SFTY);
    return HL_OK;
}
