/* memory_retrieval_present.h — bounded human presentation of typed retrieval state.
 *
 * This module turns a canonical memory_retrieval_result_t into local, symbolic
 * presentation effects. It is not a voice synthesizer, screen driver, haptic
 * driver, vault client, data store, model adapter, network client or action
 * dispatcher. It never receives a key, blob, card, candidate, text, audio,
 * transcript, embedding, identity, location, timestamp, radio frame or HCP.
 */
#ifndef HERUS_MEMORY_RETRIEVAL_PRESENT_H
#define HERUS_MEMORY_RETRIEVAL_PRESENT_H

#include <stdint.h>
#include "memory_retrieval.h"
#include "voice.h"

typedef enum {
    MEMORY_RETRIEVAL_PRESENT_IDLE = 0,
    MEMORY_RETRIEVAL_PRESENT_SHOWN,
    MEMORY_RETRIEVAL_PRESENT_BLOCKED
} memory_retrieval_present_state_t;

/* These are local phrase identifiers for a future controlled renderer, not free
 * Portuguese text, an answer, a summary or an instruction. The renderer must not
 * expand a code into an action or present data that is absent from this structure. */
typedef enum {
    MEMORY_RETRIEVAL_PHRASE_NONE = 0,
    MEMORY_RETRIEVAL_PHRASE_MATCH_AVAILABLE,
    MEMORY_RETRIEVAL_PHRASE_NO_MATCH,
    MEMORY_RETRIEVAL_PHRASE_AMBIGUOUS_REVIEW,
    MEMORY_RETRIEVAL_PHRASE_REJECTED
} memory_retrieval_phrase_t;

/* This is a status presentation, not a factual claim. `MATCH_AVAILABLE` may carry
 * only the winner's already-public typed category/origin and reason mask. No card
 * identifier is exported to a voice, haptic or screen adapter. For NO_MATCH and
 * AMBIGUOUS every typed detail is zero. */
typedef struct {
    memory_retrieval_phrase_t phrase;
    haptic_plan_t             haptic;
    memory_kind_t             kind;
    memory_extract_origin_t   origin;
    uint32_t                  reasons;
} memory_retrieval_presentation_t;

typedef struct {
    uint32_t shown_match;
    uint32_t shown_no_match;
    uint32_t shown_ambiguous;
    uint32_t dismissed;
    uint32_t rejected_access;
    uint32_t rejected_result;
    uint32_t rejected_state;
} memory_retrieval_present_metrics_t;

typedef struct {
    memory_retrieval_present_state_t state;
    uint32_t active_physical_session_id;
    memory_retrieval_presentation_t pending;
    memory_retrieval_present_metrics_t metrics;
} memory_retrieval_present_t;

enum {
    MEMORY_RETRIEVAL_PRESENT_OK        =  0,
    MEMORY_RETRIEVAL_PRESENT_E_ARG     = -1,
    MEMORY_RETRIEVAL_PRESENT_E_ACCESS  = -2,
    MEMORY_RETRIEVAL_PRESENT_E_RESULT  = -3,
    MEMORY_RETRIEVAL_PRESENT_E_STATE   = -4
};

void memory_retrieval_present_init(memory_retrieval_present_t *p);

/* Accepts only a canonical retrieval result and canonical physical access assertion.
 * It may be called once while IDLE. On success, `out` is a symbolic local status
 * and the presenter becomes SHOWN. A malformed result/access fails closed, zeroes
 * output, and never produces a local effect. */
int memory_retrieval_present_show(memory_retrieval_present_t *p,
                                  const memory_consolidation_access_t *access,
                                  const memory_retrieval_result_t *result,
                                  memory_retrieval_presentation_t *out);

/* Dismisses one already shown status under the same canonical physical session and
 * scrubs all transient presentation data. It does not re-run retrieval, change a
 * score, open/write/erase a vault, send anything or make a retention decision. */
int memory_retrieval_present_dismiss(memory_retrieval_present_t *p,
                                     const memory_consolidation_access_t *access);

const memory_retrieval_present_metrics_t *memory_retrieval_present_metrics(
    const memory_retrieval_present_t *p);

#endif /* HERUS_MEMORY_RETRIEVAL_PRESENT_H */
