/*
 * HERUS magic_anticipation — typed, explainable surprise proposals.
 *
 * This layer is deliberately a proposal engine. It has no text, audio, network,
 * clock, identity, location, executor or automatic memory-write path.
 */
#ifndef HERUS_MAGIC_ANTICIPATION_H
#define HERUS_MAGIC_ANTICIPATION_H

#include "memory_reasoning_bridge.h"
#include <stdint.h>

typedef enum {
    MAGIC_PRIVACY_ORDINARY = 1u,
    MAGIC_PRIVACY_PERSONAL = 2u,
    MAGIC_PRIVACY_SENSITIVE = 3u,
    MAGIC_PRIVACY_THIRD_PARTY = 4u
} magic_privacy_class_t;

typedef enum {
    MAGIC_REQUEST_EXPLICIT = 1u,
    MAGIC_REQUEST_CONTEXTUAL = 2u
} magic_request_kind_t;

typedef enum {
    MAGIC_NO_PROPOSAL = 0u,
    MAGIC_RECALL = 1u,
    MAGIC_CONNECTION = 2u,
    MAGIC_KNOWN_GAP = 3u,
    MAGIC_CONTRADICTION = 4u,
    MAGIC_ABSTAIN = 5u,
    MAGIC_SILENT = 6u,
    MAGIC_SENSITIVE_BLOCK = 7u,
    MAGIC_LIMIT = 8u
} magic_status_t;

typedef struct {
    sr_pattern_t cue;
    magic_privacy_class_t privacy_class;
    magic_request_kind_t request_kind;
    uint8_t attention_window; /* exactly 1 permits a contextual proposal */
} magic_context_t;

typedef struct {
    uint32_t max_steps;
    uint8_t allow_personal_explicit; /* exactly 1 allows explicit personal query */
} magic_policy_t;

typedef struct {
    magic_status_t status;
    uint8_t requires_confirmation;
    uint8_t explanation_available;
    uint32_t explanation_code;
    sr_answer_t answer;
    mrb_meta_t composition;
} magic_proposal_t;

void magic_policy_default(magic_policy_t *policy);

/* Creates a local proposal only. It clears `out` first, rejects sensitive or
 * third-party contexts before reasoning, and never mutates `base` or `memory`. */
magic_status_t magic_propose(const sr_reasoner_t *base,
                             const mse_index_t *memory,
                             uint32_t current_generation,
                             const magic_context_t *context,
                             const magic_policy_t *policy,
                             sr_reasoner_t *scratch,
                             magic_proposal_t *out);

#endif /* HERUS_MAGIC_ANTICIPATION_H */
