/*
 * HERUS memory_semantic_evidence — bounded typed facts backed by reviewed cards.
 *
 * This is an in-RAM semantic index, not a recorder or a second vault. It stores
 * only symbolic handles plus opaque card/review provenance and monotonic local
 * generations. It has no text, audio, timestamp, identity or location.
 */
#ifndef HERUS_MEMORY_SEMANTIC_EVIDENCE_H
#define HERUS_MEMORY_SEMANTIC_EVIDENCE_H

#include "memory_vault.h"
#include "symbolic_reasoner.h"
#include <stddef.h>
#include <stdint.h>

#define MSE_MAX_EVIDENCE 16u

typedef enum {
    MSE_EVIDENCE_NONE = 0u,
    MSE_EVIDENCE_ACTIVE,
    MSE_EVIDENCE_SUPERSEDED,
    MSE_EVIDENCE_CONFLICTED,
    MSE_EVIDENCE_EXPIRED
} mse_evidence_status_t;

typedef enum {
    MSE_QUERY_NO_MATCH = 0u,
    MSE_QUERY_MATCH,
    MSE_QUERY_AMBIGUOUS,
    MSE_QUERY_CONTRADICTED
} mse_query_status_t;

typedef enum {
    MSE_OK = 0,
    MSE_NO_CHANGE = 1,
    MSE_E_ARG = -1,
    MSE_E_CARD = -2,
    MSE_E_FACT = -3,
    MSE_E_ROLLBACK = -4,
    MSE_E_FULL = -5,
    MSE_E_FORMAT = -6,
    MSE_E_FLOOR = -7
} mse_status_t;

typedef int (*mse_functional_predicate_fn)(sr_symbol_t predicate, void *user);

typedef struct {
    sr_fact_t fact;
    uint32_t card_id;
    uint32_t review_receipt_id;
    uint32_t observed_generation;
    uint32_t valid_until_generation; /* zero means no generation expiry */
    mse_evidence_status_t status;
} mse_evidence_t;

typedef struct {
    uint16_t evidence_count;
    uint32_t generation_floor; /* durable semantic floor; zero means cold start */
    uint32_t additions;
    uint32_t superseded;
    uint32_t conflicts;
    uint32_t expired;
    uint32_t rejected;
    mse_functional_predicate_fn is_functional;
    void *policy_user;
    mse_evidence_t evidence[MSE_MAX_EVIDENCE];
} mse_index_t;

typedef struct {
    mse_query_status_t status;
    uint16_t active_matches;
    uint16_t conflict_matches;
    sr_fact_t fact;
    uint32_t selected_card_id;
    uint32_t selected_review_receipt_id;
    uint32_t selected_generation;
} mse_query_result_t;

void mse_init(mse_index_t *index,
              mse_functional_predicate_fn is_functional,
              void *policy_user);

/* Sets a nondecreasing semantic generation floor only on an empty index. It is an
 * anti-replay boundary for reindexing after reboot; it is not a wall-clock claim. */
mse_status_t mse_set_generation_floor(mse_index_t *index, uint32_t generation_floor);

/* Validates the bounded in-RAM representation without modifying it. */
mse_status_t mse_validate(const mse_index_t *index);

/* Adds evidence only from an already-authorised card. Functional-predicate
 * conflicts are marked, never resolved. Newer exact evidence supersedes older
 * exact evidence; older generations are rejected. */
mse_status_t mse_add(mse_index_t *index,
                     const memory_vault_card_t *card,
                     const sr_fact_t *fact,
                     uint32_t observed_generation,
                     uint32_t valid_until_generation);

/* Advances local generation state without using wall-clock time. */
unsigned mse_expire(mse_index_t *index, uint32_t current_generation);

/* Queries typed evidence. An all-variable query is rejected to prevent bounded
 * memory enumeration from becoming an implicit listing API. */
mse_status_t mse_query(const mse_index_t *index,
                       const sr_pattern_t *pattern,
                       uint32_t current_generation,
                       mse_query_result_t *out);

#endif /* HERUS_MEMORY_SEMANTIC_EVIDENCE_H */
