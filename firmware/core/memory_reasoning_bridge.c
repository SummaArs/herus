#include "memory_reasoning_bridge.h"
#include <string.h>

static void meta_clear(mrb_meta_t *meta)
{
    if (meta) memset(meta, 0, sizeof(*meta));
}

static mrb_status_t map_reasoner_status(int status, mrb_meta_t *meta)
{
    if (meta) meta->reasoner_status = status;
    if (status == SR_OK) return MRB_OK;
    if (status == SR_E_NO_EVIDENCE) return MRB_NO_EVIDENCE;
    if (status == SR_E_CONTRADICTION) return MRB_CONTRADICTED;
    if (status == SR_E_AMBIGUOUS) return MRB_AMBIGUOUS;
    if (status == SR_E_LIMIT || status == SR_E_FULL) return MRB_LIMIT;
    return MRB_E_ARG;
}

static int evidence_current(const mse_evidence_t *item,
                            uint32_t current_generation)
{
    return item && item->status == MSE_EVIDENCE_ACTIVE &&
           (item->valid_until_generation == 0u ||
            current_generation <= item->valid_until_generation);
}

mrb_status_t mrb_query(const sr_reasoner_t *base,
                       const mse_index_t *memory,
                       uint32_t current_generation,
                       const sr_pattern_t *query,
                       uint32_t max_steps,
                       sr_reasoner_t *scratch,
                       sr_answer_t *out,
                       mrb_meta_t *meta)
{
    mse_query_result_t direct_memory;
    mse_status_t memory_status;
    int reasoner_status;
    if (out) memset(out, 0, sizeof(*out));
    meta_clear(meta);
    if (!base || !memory || !query || current_generation == 0u ||
        max_steps == 0u || !scratch || !out)
        return MRB_E_ARG;
    if (scratch == base) return MRB_E_SCRATCH;

    /* A direct memory contradiction or ambiguity is authoritative abstention.
     * An all-variable query is rejected here by mse_query, so this bridge cannot
     * become a private evidence enumeration API. */
    memory_status = mse_query(memory, query, current_generation, &direct_memory);
    if (memory_status != MSE_OK) return MRB_E_ARG;
    if (meta) {
        meta->memory_status = direct_memory.status;
        meta->memory_conflicts = direct_memory.conflict_matches;
        meta->selected_card_id = direct_memory.selected_card_id;
        meta->selected_review_receipt_id = direct_memory.selected_review_receipt_id;
        meta->selected_generation = direct_memory.selected_generation;
    }
    if (direct_memory.status == MSE_QUERY_CONTRADICTED) return MRB_CONTRADICTED;
    if (direct_memory.status == MSE_QUERY_AMBIGUOUS) return MRB_AMBIGUOUS;

    memcpy(scratch, base, sizeof(*scratch));
    for (uint16_t i = 0u; i < memory->evidence_count; i++) {
        const mse_evidence_t *item = &memory->evidence[i];
        int add_status;
        if (!evidence_current(item, current_generation)) {
            if (meta) meta->memory_skipped++;
            continue;
        }
        add_status = sr_add_fact(scratch, item->fact);
        if (add_status == SR_E_FULL) return MRB_LIMIT;
        if (add_status != SR_OK && add_status != SR_NO_CHANGE)
            return MRB_E_ARG;
        if (meta) meta->memory_imported++;
    }
    if (meta) {
        for (uint16_t i = 0u; i < memory->evidence_count; i++) {
            const mse_evidence_t *item = &memory->evidence[i];
            if (item->status == MSE_EVIDENCE_CONFLICTED &&
                (item->valid_until_generation == 0u ||
                 current_generation <= item->valid_until_generation))
                meta->memory_conflicts++;
        }
    }

    reasoner_status = sr_saturate(scratch, max_steps);
    if (reasoner_status != SR_OK)
        return map_reasoner_status(reasoner_status, meta);
    reasoner_status = sr_query(scratch, query, out);
    return map_reasoner_status(reasoner_status, meta);
}
