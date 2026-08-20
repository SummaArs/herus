#include "autonomy_policy.h"

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

herus_policy_status_t herus_policy_validate(
    const herus_autonomy_envelope_t *envelope)
{
    if (!envelope || envelope->level > HERUS_A4_CONFIRMED ||
        envelope->scope > HERUS_SCOPE_ACTUATE ||
        !canonical_bool(envelope->proactive) ||
        !canonical_bool(envelope->attention_window) ||
        !canonical_bool(envelope->proactive_consent) ||
        !canonical_bool(envelope->explicit_confirmation) ||
        !canonical_bool(envelope->confirmation_consumed) ||
        !canonical_bool(envelope->sensitive_context) ||
        !canonical_bool(envelope->third_party_context))
        return HERUS_POLICY_FORMAT;
    if (envelope->level == HERUS_A0_SILENT && envelope->scope != HERUS_SCOPE_NONE)
        return HERUS_POLICY_SCOPE;
    if (envelope->level == HERUS_A4_CONFIRMED &&
        envelope->scope == HERUS_SCOPE_NONE)
        return HERUS_POLICY_SCOPE;
    if (envelope->confirmation_consumed &&
        (!envelope->explicit_confirmation || envelope->confirmed_scope == HERUS_SCOPE_NONE ||
         envelope->confirmed_scope != envelope->scope))
        return HERUS_POLICY_REVOKED;
    if (envelope->sensitive_context || envelope->third_party_context) {
        if (envelope->proactive || envelope->level >= HERUS_A2_CONTEXTUAL)
            return HERUS_POLICY_REJECTED;
    }
    return HERUS_POLICY_OK;
}

herus_policy_status_t herus_policy_classify(
    const herus_autonomy_envelope_t *envelope)
{
    herus_policy_status_t status = herus_policy_validate(envelope);
    if (status != HERUS_POLICY_OK) return status;
    if (envelope->sensitive_context || envelope->third_party_context)
        return HERUS_POLICY_REJECTED;
    if (envelope->level == HERUS_A0_SILENT ||
        (envelope->proactive &&
         (envelope->attention_window != 1u ||
          envelope->proactive_consent != 1u)))
        return HERUS_POLICY_SILENT;
    if (envelope->scope == HERUS_SCOPE_TRANSMIT ||
        envelope->scope == HERUS_SCOPE_ACTUATE ||
        envelope->scope == HERUS_SCOPE_REMEMBER ||
        envelope->scope == HERUS_SCOPE_PREPARE) {
        if (envelope->level < HERUS_A4_CONFIRMED ||
            envelope->explicit_confirmation != 1u ||
            envelope->confirmation_consumed != 1u)
            return HERUS_POLICY_NEEDS_CONFIRMATION;
    }
    if (envelope->level == HERUS_A4_CONFIRMED &&
        envelope->confirmation_consumed != 1u)
        return HERUS_POLICY_NEEDS_CONFIRMATION;
    return HERUS_POLICY_OK;
}

herus_policy_status_t herus_policy_consume_confirmation(
    herus_autonomy_envelope_t *envelope,
    uint32_t proposal_id,
    uint32_t confirmation_id)
{
    herus_policy_status_t status;
    if (!envelope || proposal_id == 0u || confirmation_id == 0u)
        return HERUS_POLICY_FORMAT;
    status = herus_policy_validate(envelope);
    if (status != HERUS_POLICY_OK) return status;
    if (envelope->proposal_id == 0u || envelope->proposal_id != proposal_id ||
        envelope->confirmation_id != confirmation_id ||
        envelope->explicit_confirmation != 1u ||
        envelope->confirmation_consumed == 1u)
        return HERUS_POLICY_REVOKED;
    envelope->level = HERUS_A4_CONFIRMED;
    envelope->confirmation_consumed = 1u;
    envelope->confirmed_scope = envelope->scope;
    return HERUS_POLICY_OK;
}
