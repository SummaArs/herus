#include "magic_trigger.h"
#include <string.h>

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

magic_trigger_status_t magic_trigger_begin(magic_trigger_t *trigger,
                                            const magic_context_t *context,
                                            uint32_t generation,
                                            uint32_t ttl_generations,
                                            uint8_t max_proposals)
{
    if (!trigger || !context || generation == 0u || ttl_generations == 0u ||
        max_proposals == 0u || max_proposals > MAGIC_TRIGGER_MAX_PROPOSALS ||
        !canonical_bool(context->attention_window))
        return MAGIC_TRIGGER_E_ARG;
    if (generation > UINT32_MAX - ttl_generations) return MAGIC_TRIGGER_E_FORMAT;
    if (context->attention_window != 1u) {
        memset(trigger, 0, sizeof(*trigger));
        return MAGIC_TRIGGER_SILENT;
    }
    memset(trigger, 0, sizeof(*trigger));
    trigger->active = 1u;
    trigger->max_proposals = max_proposals;
    trigger->context = *context;
    trigger->started_generation = generation;
    trigger->expires_generation = generation + ttl_generations;
    return MAGIC_TRIGGER_OK;
}

magic_trigger_status_t magic_trigger_offer(magic_trigger_t *trigger,
                                           const sr_reasoner_t *base,
                                           const mse_index_t *memory,
                                           uint32_t generation,
                                           const magic_policy_t *policy,
                                           sr_reasoner_t *scratch,
                                           magic_proposal_t *out)
{
    magic_status_t proposal_status;
    if (out) memset(out, 0, sizeof(*out));
    if (!trigger || !base || !memory || !policy || !scratch || !out ||
        generation == 0u)
        return MAGIC_TRIGGER_E_ARG;
    if (!trigger->active || generation < trigger->started_generation ||
        generation > trigger->expires_generation ||
        trigger->max_proposals == 0u ||
        trigger->max_proposals > MAGIC_TRIGGER_MAX_PROPOSALS ||
        trigger->proposals_served >= trigger->max_proposals)
        return MAGIC_TRIGGER_SILENT;
    proposal_status = magic_propose(base, memory, generation, &trigger->context,
                                    policy, scratch, out);
    if (proposal_status != MAGIC_SILENT &&
        proposal_status != MAGIC_SENSITIVE_BLOCK)
        trigger->proposals_served++;
    if (proposal_status == MAGIC_SILENT ||
        proposal_status == MAGIC_SENSITIVE_BLOCK)
        return MAGIC_TRIGGER_SILENT;
    return MAGIC_TRIGGER_OK;
}

void magic_trigger_close(magic_trigger_t *trigger)
{
    if (!trigger) return;
    memset(trigger, 0, sizeof(*trigger));
}
