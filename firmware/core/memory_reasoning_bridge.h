/*
 * HERUS memory_reasoning_bridge — local composition of reviewed memory and rules.
 *
 * The caller supplies scratch storage. The base reasoner and semantic-memory index
 * are read-only; the Core is absent from this API. Conflicted/expired evidence is
 * never imported, and ambiguous memory never becomes a selected answer.
 */
#ifndef HERUS_MEMORY_REASONING_BRIDGE_H
#define HERUS_MEMORY_REASONING_BRIDGE_H

#include "memory_semantic_evidence.h"
#include "symbolic_reasoner.h"
#include <stdint.h>

typedef enum {
    MRB_OK = 0,
    MRB_NO_EVIDENCE = 1,
    MRB_CONTRADICTED = 2,
    MRB_AMBIGUOUS = 3,
    MRB_LIMIT = 4,
    MRB_E_ARG = -1,
    MRB_E_SCRATCH = -2
} mrb_status_t;

typedef struct {
    mse_query_status_t memory_status;
    uint16_t memory_imported;
    uint16_t memory_skipped;
    uint16_t memory_conflicts;
    uint32_t selected_card_id;
    uint32_t selected_review_receipt_id;
    uint32_t selected_generation;
    int reasoner_status;
} mrb_meta_t;

/* Compose `base` with current reviewed memory into caller-owned `scratch` and
 * query the resulting local reasoner. `scratch` must be distinct from `base`.
 * No input index, base reasoner or persistent vault is mutated. */
mrb_status_t mrb_query(const sr_reasoner_t *base,
                       const mse_index_t *memory,
                       uint32_t current_generation,
                       const sr_pattern_t *query,
                       uint32_t max_steps,
                       sr_reasoner_t *scratch,
                       sr_answer_t *out,
                       mrb_meta_t *meta);

#endif /* HERUS_MEMORY_REASONING_BRIDGE_H */
