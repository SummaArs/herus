/*
 * HERUS magic_trigger — bounded temporal context window.
 *
 * This is a local attention/relevance gate. It has no clock service, sensor,
 * network, text or executor; generations are supplied by the caller.
 */
#ifndef HERUS_MAGIC_TRIGGER_H
#define HERUS_MAGIC_TRIGGER_H

#include "magic_anticipation.h"
#include <stdint.h>

#define MAGIC_TRIGGER_MAX_PROPOSALS 4u

typedef enum {
    MAGIC_TRIGGER_OK = 0,
    MAGIC_TRIGGER_SILENT = 1,
    MAGIC_TRIGGER_E_ARG = -1,
    MAGIC_TRIGGER_E_FORMAT = -2
} magic_trigger_status_t;

typedef struct {
    uint8_t active;
    uint8_t proposals_served;
    uint8_t max_proposals;
    magic_context_t context;
    uint32_t started_generation;
    uint32_t expires_generation;
} magic_trigger_t;

/* Opens a bounded local relevance window. `ttl_generations` and
 * `max_proposals` are explicit resource budgets, not wall-clock claims. */
magic_trigger_status_t magic_trigger_begin(magic_trigger_t *trigger,
                                            const magic_context_t *context,
                                            uint32_t generation,
                                            uint32_t ttl_generations,
                                            uint8_t max_proposals);

/* Offers at most one proposal during the active window. A successful proposal,
 * contradiction, abstention or known gap consumes one bounded presentation slot;
 * silent and privacy-blocked results do not create action authority. */
magic_trigger_status_t magic_trigger_offer(magic_trigger_t *trigger,
                                           const sr_reasoner_t *base,
                                           const mse_index_t *memory,
                                           uint32_t generation,
                                           const magic_policy_t *policy,
                                           sr_reasoner_t *scratch,
                                           magic_proposal_t *out);

void magic_trigger_close(magic_trigger_t *trigger);

#endif /* HERUS_MAGIC_TRIGGER_H */
