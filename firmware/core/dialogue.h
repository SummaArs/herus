/* dialogue.h — bounded conversational intelligence for the HERUS Nucleus.
 *
 * A dialogue adapter may be a measured local model on a future Nucleus compute
 * target. It is NOT a cloud client: this module contains no socket, credential,
 * radio, storage, HCP, Core-link or send API. The ESP32-S3 target has not yet
 * demonstrated a generative LLM under HERUS latency/energy constraints, so the
 * production adapter is deliberately optional and fail-closed when unavailable.
 *
 * The adapter is untrusted. It sees exactly one current local-ASR utterance and
 * bounded, typed topic cards; it never receives raw conversation history,
 * identity, location, keys, learned Nucleus rules, radio state or send authority.
 * Its output is a reply for local UX only. It cannot create an intent observation,
 * draft, HCP message or transmit action. A user must begin a separate physical
 * push-to-talk command session and pass the existing intent/confirmation gates to
 * send a meaning.
 */
#ifndef HERUS_DIALOGUE_H
#define HERUS_DIALOGUE_H

#include <stddef.h>
#include <stdint.h>

#define DIALOGUE_UTTERANCE_MAX 160u
#define DIALOGUE_REPLY_MAX     192u
#define DIALOGUE_CONTEXT_CAP     4u
#define DIALOGUE_DEFAULT_TURN_MS 10000u
#define DIALOGUE_DEFAULT_CONTEXT_MS 900000u

typedef enum {
    DIALOGUE_TOPIC_NONE = 0,
    DIALOGUE_TOPIC_STATUS,
    DIALOGUE_TOPIC_GUIDANCE,
    DIALOGUE_TOPIC_SAFETY,
    DIALOGUE_TOPIC_DEVICE,
    DIALOGUE_TOPIC_COUNT
} dialogue_topic_t;

typedef struct {
    dialogue_topic_t topic;
    uint32_t         expires_ms;
} dialogue_card_t;

typedef struct {
    dialogue_card_t card[DIALOGUE_CONTEXT_CAP];
} dialogue_context_t;

/* The current utterance is borrowed for the synchronous call only. It is never
 * copied into dialogue_t, telemetry or persistent storage. Cards carry only an
 * enum topic and expiry, never text, embeddings or identity. */
typedef struct {
    const char                 *utterance;
    size_t                      utterance_len;
    const dialogue_context_t   *context;
} dialogue_request_t;

/* A model can only return display/speech text plus an allowed topic label. The
 * reply is treated as untrusted local UX; it is not parsed as a command. */
typedef struct {
    char             reply[DIALOGUE_REPLY_MAX];
    uint8_t          reply_len;
    dialogue_topic_t topic;
} dialogue_model_reply_t;

/* A target supplies this callback only for a measured local model or local
 * compute peripheral. The interface intentionally has no network endpoint,
 * token, function-calling callback or reference to HERUS state. Return zero on
 * a complete candidate reply; any nonzero result is unavailable/failure. */
typedef struct {
    void *ctx;
    int (*generate_local)(void *ctx, const dialogue_request_t *request,
                          dialogue_model_reply_t *out);
} dialogue_model_t;

typedef enum {
    DIALOGUE_IDLE = 0,
    DIALOGUE_LISTENING,
    DIALOGUE_REPLY_READY,
    DIALOGUE_TIMED_OUT,
    DIALOGUE_MODEL_FAILED,
    DIALOGUE_REJECTED
} dialogue_state_t;

enum {
    DIALOGUE_OK         =  0,
    DIALOGUE_E_ARG      = -1,
    DIALOGUE_E_STATE    = -2,
    DIALOGUE_E_PHYSICAL = -3,
    DIALOGUE_E_TIMEOUT  = -4,
    DIALOGUE_E_MODEL    = -5,
    DIALOGUE_E_REPLY    = -6
};

/* Numeric-only diagnostics. Raw utterances, replies, embeddings, identities,
 * locations, keys, HCP meanings and model prompts are deliberately absent. */
typedef struct {
    uint32_t turns_started;
    uint32_t replies_ready;
    uint32_t model_failed;
    uint32_t model_rejected;
    uint32_t timed_out;
    uint32_t context_expired;
    uint32_t privacy_clears;
    uint32_t last_latency_ms;
} dialogue_metrics_t;

typedef struct {
    uint32_t turn_timeout_ms;
    uint32_t context_timeout_ms;
} dialogue_config_t;

typedef struct {
    dialogue_config_t  cfg;
    dialogue_model_t   model;
    dialogue_state_t   state;
    uint32_t           physical_session_id;
    uint32_t           turn_started_ms;
    dialogue_context_t context;
    unsigned           next_card;
    dialogue_model_reply_t pending; /* local UX only; cleared on take/timeout */
    dialogue_metrics_t metrics;
} dialogue_t;

void dialogue_config_default(dialogue_config_t *out);
void dialogue_init(dialogue_t *d, const dialogue_config_t *cfg,
                   const dialogue_model_t *model);

/* A nonzero session must originate from a currently held physical PTT gesture in
 * the caller's runtime. The dialogue layer records it only for one live turn and
 * never forwards it to the model. */
int dialogue_begin_turn(dialogue_t *d, uint32_t physical_session_id, uint32_t now_ms);

/* Add a trusted, non-textual local topic card. Model output cannot call this API;
 * callers must not map a model reply into a topic card. */
int dialogue_note_topic(dialogue_t *d, dialogue_topic_t topic, uint32_t now_ms);

/* Synchronously submit one local-ASR utterance to the local model adapter.
 * Text is bounded and borrowed; the dialogue layer stores no transcript. */
int dialogue_submit_utterance(dialogue_t *d, const char *utterance, size_t utterance_len,
                              uint32_t now_ms);

/* Copy the local reply to caller-owned UX memory, then zero the internal copy and
 * return to IDLE. This function never returns a command, HCP message or action. */
int dialogue_take_reply(dialogue_t *d, char *out, size_t out_cap,
                        dialogue_topic_t *out_topic);

/* Enforce the turn timeout and expire typed context cards with caller monotonic ms. */
int dialogue_tick(dialogue_t *d, uint32_t now_ms);

/* Erase cards, pending UX reply and live session. It does not affect any existing
 * Nucleus semantic learner, trust binding or interaction-runtime state. */
void dialogue_forget(dialogue_t *d);

const dialogue_metrics_t *dialogue_metrics(const dialogue_t *d);
const dialogue_context_t *dialogue_context(const dialogue_t *d);

#endif /* HERUS_DIALOGUE_H */
