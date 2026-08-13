/* assurance.c — pure Grand Finale composition policy. */
#include "assurance.h"

static int exactly_one(uint8_t value)
{
    return value == 1u;
}

int assurance_decide(const assurance_snapshot_t *snapshot,
                     assurance_decision_t *out)
{
    uint32_t failures = ASSURANCE_FAIL_NONE;
    if (!snapshot || !out) return ASSURANCE_E_ARG;

    if (snapshot->source != ASSURANCE_SOURCE_CORE &&
        snapshot->source != ASSURANCE_SOURCE_NUCLEUS)
        failures |= ASSURANCE_FAIL_SOURCE;
    if (!exactly_one(snapshot->physical_session_current))
        failures |= ASSURANCE_FAIL_PHYSICAL;
    if (!exactly_one(snapshot->intent_accepted))
        failures |= ASSURANCE_FAIL_INTENT;
    if (!exactly_one(snapshot->physical_confirmation))
        failures |= ASSURANCE_FAIL_CONFIRM;
    if (!exactly_one(snapshot->handoff_unused))
        failures |= ASSURANCE_FAIL_HANDOFF;

    /* Revocation is terminal and dominates a previously valid trust/link path. */
    if (snapshot->trust_revoked != 0u)
        failures |= ASSURANCE_FAIL_REVOKED;
    if (snapshot->source == ASSURANCE_SOURCE_NUCLEUS) {
        if (!exactly_one(snapshot->trust_active))
            failures |= ASSURANCE_FAIL_TRUST;
        if (!exactly_one(snapshot->control_link_authenticated))
            failures |= ASSURANCE_FAIL_LINK_AUTH;
        if (!exactly_one(snapshot->control_link_fresh))
            failures |= ASSURANCE_FAIL_LINK_FRESH;
    }

    /* A disabled model needs no profile. Once a model is enabled, A9 evidence and
     * the display-only boundary are both mandatory. */
    if (snapshot->local_model_enabled != 0u) {
        if (!exactly_one(snapshot->local_model_accepted))
            failures |= ASSURANCE_FAIL_MODEL;
        if (!exactly_one(snapshot->model_reply_display_only))
            failures |= ASSURANCE_FAIL_AGENCY;
    }

    out->failures = failures;
    out->handoff_permitted = failures == ASSURANCE_FAIL_NONE;
    return out->handoff_permitted ? ASSURANCE_OK : ASSURANCE_E_BLOCKED;
}
