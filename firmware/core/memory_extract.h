/* memory_extract.h — local, transient and conservative memory-candidate extractor.
 *
 * This module receives one borrowed, already-normalized local-ASR fixture/input
 * during an authorised memory-capture session. It is not ASR, an LLM, a recorder,
 * database or a communication API. It stores no input pointer or text, creates no
 * summary, and has no path to persistence, HCP, radio, trust, dialogue or keys.
 *
 * Its only product is a typed, uncertain candidate signal for memory_policy. A
 * candidate is an interpretation of a narrow grammar, never a statement of truth.
 */
#ifndef HERUS_MEMORY_EXTRACT_H
#define HERUS_MEMORY_EXTRACT_H

#include <stddef.h>
#include <stdint.h>
#include "memory_capture.h"
#include "memory_policy.h"

#define MEMORY_EXTRACT_TEXT_MAX 192u

typedef enum {
    MEMORY_EXTRACT_ORIGIN_NONE = 0,
    MEMORY_EXTRACT_EXPLICIT,
    MEMORY_EXTRACT_CONTROLLED_INFERENCE,
    MEMORY_EXTRACT_ORIGIN_COUNT
} memory_extract_origin_t;

enum {
    MEMORY_EXTRACT_REASON_NONE          = 0u,
    MEMORY_EXTRACT_REASON_EXPLICIT      = 1u << 0,
    MEMORY_EXTRACT_REASON_IDEA          = 1u << 1,
    MEMORY_EXTRACT_REASON_DECISION      = 1u << 2,
    MEMORY_EXTRACT_REASON_COMMITMENT    = 1u << 3,
    MEMORY_EXTRACT_REASON_PREFERENCE    = 1u << 4,
    MEMORY_EXTRACT_REASON_PROJECT       = 1u << 5,
    MEMORY_EXTRACT_REASON_ROUTINE       = 1u << 6,
    MEMORY_EXTRACT_REASON_THIRD_PARTY   = 1u << 7,
    MEMORY_EXTRACT_REASON_SENSITIVE     = 1u << 8,
    MEMORY_EXTRACT_REASON_AMBIGUOUS     = 1u << 9,
    MEMORY_EXTRACT_REASON_UNRECOGNIZED  = 1u << 10
};

typedef struct {
    memory_signal_t       signal;
    memory_extract_origin_t origin;
    uint32_t              reasons;
} memory_candidate_t;

typedef struct {
    uint32_t calls;
    uint32_t candidates;
    uint32_t no_candidate;
    uint32_t rejected_session;
    uint32_t rejected_input;
    uint32_t low_confidence;
    uint32_t sensitive_or_other;
} memory_extract_metrics_t;

typedef struct {
    memory_extract_metrics_t metrics;
} memory_extract_t;

enum {
    MEMORY_EXTRACT_OK           =  0,
    MEMORY_EXTRACT_NO_CANDIDATE =  1,
    MEMORY_EXTRACT_E_ARG        = -1,
    MEMORY_EXTRACT_E_SESSION    = -2,
    MEMORY_EXTRACT_E_INPUT      = -3,
    MEMORY_EXTRACT_E_LENGTH     = -4
};

/* Initialise a stateless extractor. It retains no text, capture id or candidate. */
void memory_extract_init(memory_extract_t *e);

/* Inspect one borrowed normalized-text input from an active capture. `capture` is
 * consulted only to verify that `capture_session_id` is currently authorised.
 * `asr_confidence_pct` is supplied by a future local adapter and must be 0..100.
 * On success the candidate has no textual payload. On NO_CANDIDATE or error, `out`
 * is cleared. The caller owns and must clear the source text/buffer. */
int memory_extract_text(memory_extract_t *e, const memory_capture_t *capture,
                        uint32_t capture_session_id, const char *text, size_t len,
                        uint8_t asr_confidence_pct, memory_candidate_t *out);

/* Compose the typed candidate with the already-proven relevance policy. This call
 * cannot persist, send or mutate the extractor/capture; it only returns an advisory
 * assessment. */
int memory_extract_assess(const memory_candidate_t *candidate,
                          memory_assessment_t *assessment);

const memory_extract_metrics_t *memory_extract_metrics(const memory_extract_t *e);

#endif /* HERUS_MEMORY_EXTRACT_H */
