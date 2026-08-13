/* voice.h — controlled Portuguese intent parsing and bounded haptic feedback.
 *
 * This layer is deliberately downstream of local ASR and upstream of confirmation.
 * It has no microphone, network, crypto, radio, GPIO, clock or allocation code.
 * A recognized phrase creates at most a local HCP DRAFT. The application must
 * collect a physical confirmation and then set transport state before link_send().
 */
#ifndef HERUS_VOICE_H
#define HERUS_VOICE_H

#include <stdint.h>
#include "hcp.h"

#define VOICE_TRANSCRIPT_MAX   96u
#define HAPTIC_MAX_PULSE        5u
#define HAPTIC_MAX_ON_MS      200u
#define HAPTIC_MAX_TOTAL_MS  1500u

typedef struct {
    uint16_t intent_arrive;       /* 0..2047, configured by the domain lexicon */
    uint16_t intent_help;         /* private HELP, never a public Tier SOS */
    uint8_t  role_time;           /* 0..31 */
    uint16_t minute_filler_base;  /* filler(base + minutes), must leave room through 60 */
} voice_lexicon_t;

typedef enum {
    VOICE_DRAFT = 0,
    VOICE_CANCEL_LOCAL,
    VOICE_UNKNOWN,
    VOICE_REJECTED
} voice_status_t;

/* Typed local-ASR output. It carries only a configured command class; it is not
 * a transcript, a wire value or permission to send. */
typedef enum {
    VOICE_COMMAND_NONE = 0,
    VOICE_COMMAND_ARRIVE = 1,
    VOICE_COMMAND_HELP = 2,
    VOICE_COMMAND_CANCEL = 3
} voice_command_t;

typedef enum {
    VOICE_EVENT_DRAFT = 0,
    VOICE_EVENT_CRITICAL_DRAFT,
    VOICE_EVENT_CANCEL,
    VOICE_EVENT_UNKNOWN,
    VOICE_EVENT_REJECTED
} voice_event_t;

typedef struct {
    uint16_t on_ms;
    uint16_t off_ms;
} haptic_pulse_t;

typedef struct {
    uint8_t        n;
    haptic_pulse_t pulse[HAPTIC_MAX_PULSE];
} haptic_plan_t;

typedef struct {
    voice_status_t status;
    voice_event_t  event;
    hcp_msg_t      draft;                 /* semantic fields only; transport is zero */
    uint8_t        requires_confirmation; /* 1 for every VOICE_DRAFT */
    uint8_t        minutes;               /* parsed duration, 0 when absent */
} voice_result_t;

/* A small usable default for host tests and early prototypes. Product domains
 * should provision their own ids, never rely on these universal-looking values. */
void voice_lexicon_default(voice_lexicon_t *out);

/* Parse a local ASR transcript in Brazilian Portuguese. Accepted speech is a
 * deliberately bounded grammar: arrival, private help, cancellation and minutes
 * 1..60. Unknown or malformed text fails closed and never creates a draft. */
voice_status_t voice_parse_pt(const char *transcript, const voice_lexicon_t *lexicon,
                              voice_result_t *out);

/* Convert a bounded command id emitted by a local ASR adapter into the same
 * semantic-only result used by the text parser. `minutes` is allowed only for
 * ARRIVE and must be 0 or 1..60. This function never trusts a score, session or
 * transport field; those belong to intent_gate and interaction respectively. */
voice_status_t voice_from_command(voice_command_t command, uint8_t minutes,
                                  const voice_lexicon_t *lexicon,
                                  voice_result_t *out);

/* Create an abstract vibration plan. The driver/HAL must still enforce electrical
 * and thermal safety. This function never actuates hardware. */
void voice_haptic_plan(voice_event_t event, haptic_plan_t *out);

/* Validate a plan against the portable human-feedback limits. */
int haptic_plan_safe(const haptic_plan_t *plan);

#endif /* HERUS_VOICE_H */
