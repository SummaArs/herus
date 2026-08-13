/* intent_gate.h — fail-closed admission of local ASR command events.
 *
 * The gate classifies an observation. It has no audio, transcript, clock, radio,
 * key, Nucleus state or HCP transport. In particular, a context hint may only
 * disambiguate the same already-strong command; it cannot invent or promote one.
 */
#ifndef HERUS_INTENT_GATE_H
#define HERUS_INTENT_GATE_H

#include <stdint.h>
#include "voice.h"

#define INTENT_GATE_MIN_CONFIDENCE_PCT  80u
#define INTENT_GATE_MIN_MARGIN_PCT      15u
#define INTENT_GATE_CONTEXT_MIN_SUPPORT  3u
#define INTENT_GATE_CONTEXT_MIN_CONF_PCT 70u

typedef enum {
    INTENT_SOURCE_CORE = 0,
    INTENT_SOURCE_NUCLEUS = 1
} intent_source_t;

typedef struct {
    intent_source_t  source;          /* local Core or local Nucleus only */
    uint32_t         session_id;      /* must equal active physical PTT session */
    voice_command_t  command;         /* configured command class, never free text */
    uint8_t          minutes;         /* valid only for ARRIVE */
    uint8_t          confidence_pct;  /* primary candidate score, 0..100 */
    uint8_t          runner_up_pct;   /* strongest alternate score, 0..100 */
} intent_observation_t;

typedef struct {
    uint8_t         available;        /* 1 only after local authorized-message evidence */
    voice_command_t command;          /* predicted intent; must equal ASR primary */
    uint16_t        support;          /* retained observations behind the hint */
    uint8_t         confidence_pct;   /* support / all retained context observations */
} intent_context_hint_t;

typedef enum {
    INTENT_GATE_ACCEPT_DIRECT = 0,
    INTENT_GATE_ACCEPT_CONTEXT,
    INTENT_GATE_STALE,
    INTENT_GATE_LOW_CONFIDENCE,
    INTENT_GATE_AMBIGUOUS,
    INTENT_GATE_REJECTED
} intent_gate_status_t;

typedef struct {
    intent_gate_status_t status;
    uint8_t              context_used;
} intent_gate_result_t;

/* Decide whether an observation for active_session may become a local draft.
 * A stale result is intentionally ignored. The function does not mutate input,
 * request a confirmation, create an HCP message, or communicate with hardware. */
intent_gate_status_t intent_gate_evaluate(const intent_observation_t *obs,
                                          uint32_t active_session,
                                          const intent_context_hint_t *hint,
                                          intent_gate_result_t *out);

#endif /* HERUS_INTENT_GATE_H */
