/*
 * semantic_life.h — deterministic host composition of personal semantic life.
 *
 * It composes the real firmware semantic-evidence index with the host physical
 * adapter. Inputs are symbolic facts and typed scores only; this is not a speech
 * recognizer, recorder, identity engine or claim about human understanding.
 */
#ifndef HERUS_SEMANTIC_LIFE_H
#define HERUS_SEMANTIC_LIFE_H

#include <stdint.h>
#include "personal_sim.h"
#include "memory_semantic_evidence.h"

#define SL_USER_SUBJECT       SR_SYMBOL_LEGACY(40u)
#define SL_PRED_PREFERENCE    SR_SYMBOL_LEGACY(41u)
#define SL_PRED_GOAL          SR_SYMBOL_LEGACY(42u)
#define SL_PRED_CONTEXT       SR_SYMBOL_LEGACY(43u)

typedef enum {
    SL_MEMORY_NONE = 0u,
    SL_MEMORY_DISCARDED_NO_AUTHORITY,
    SL_MEMORY_RETAINED,
    SL_MEMORY_REJECTED,
    SL_MEMORY_CONFLICTED
} sl_memory_disposition_t;

typedef struct {
    pps_event_t presence;
    uint8_t has_memory_candidate;
    uint8_t explicit_memory_confirmation;
    memory_vault_card_t card;
    sr_fact_t fact;
    uint32_t memory_valid_until_generation;
    uint8_t reboot;
    uint32_t recovered_semantic_floor;
} sl_event_t;

typedef struct {
    int presence_result;
    pps_status_t presence_status;
    sl_memory_disposition_t memory_disposition;
    uint32_t memory_reason;
    uint32_t expired_count;
    uint32_t generation_floor;
    uint8_t scrubbed_on_reboot;
    uint8_t quarantined;
} sl_trace_t;

typedef struct {
    pps_device_t physical;
    mse_index_t semantic_index;
    uint32_t last_generation;
    uint32_t durable_semantic_floor;
    uint32_t memory_candidates;
    uint32_t memory_retained;
    uint32_t memory_discarded;
    uint32_t memory_rejected;
    uint32_t memory_conflicted;
    uint8_t quarantined;
} sl_life_t;

enum {
    SL_OK = 0,
    SL_E_ARG = -1,
    SL_E_FORMAT = -2,
    SL_E_TIME = -3,
    SL_E_FLOOR = -4
};

void sl_init(sl_life_t *life, const pps_config_t *config);

/* Advance one event. Reboot scrubs active semantic evidence before floor import. */
int sl_step(sl_life_t *life, const sl_event_t *event, sl_trace_t *trace);

int sl_query(const sl_life_t *life, const sr_pattern_t *pattern,
            uint32_t generation, mse_query_result_t *out);

#endif /* HERUS_SEMANTIC_LIFE_H */
