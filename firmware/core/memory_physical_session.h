/* memory_physical_session.h — bounded physical-session gate for collection actions.
 *
 * This is a portable verifier of adapter-supplied evidence, not a button driver,
 * biometric verifier, identity service, authenticator, secure element, clock,
 * RNG, storage backend or UI. It receives no card, query, key, text, audio,
 * transcript, embedding, identity, location, model output or callback.
 */
#ifndef HERUS_MEMORY_PHYSICAL_SESSION_H
#define HERUS_MEMORY_PHYSICAL_SESSION_H

#include <stdint.h>

#define MEMORY_PHYSICAL_SESSION_DEFAULT_WINDOW_MS 30000u
#define MEMORY_PHYSICAL_SESSION_MAX_WINDOW_MS 300000u
#define MEMORY_PHYSICAL_SESSION_DEFAULT_QUERY_USES 3u
#define MEMORY_PHYSICAL_SESSION_MAX_QUERY_USES 8u

typedef enum {
    MEMORY_PHYSICAL_PURPOSE_NONE = 0,
    MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
    MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN,
    MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE,
    MEMORY_PHYSICAL_PURPOSE_COLLECTION_COMPACT,
    MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY
} memory_physical_purpose_t;

typedef enum {
    MEMORY_PHYSICAL_SESSION_IDLE = 0,
    MEMORY_PHYSICAL_SESSION_ACTIVE,
    MEMORY_PHYSICAL_SESSION_CONSUMED,
    MEMORY_PHYSICAL_SESSION_CANCELLED,
    MEMORY_PHYSICAL_SESSION_EXPIRED,
    MEMORY_PHYSICAL_SESSION_BLOCKED
} memory_physical_session_state_t;

/* `event_nonce` is transient adapter evidence. The C11 gate never generates,
 * persists, logs or exports it. The selected platform must later demonstrate its
 * source, uniqueness, timing and behavior across reboot/power loss. */
typedef struct {
    uint32_t window_ms;      /* 1..MEMORY_PHYSICAL_SESSION_MAX_WINDOW_MS */
    uint8_t  max_query_uses; /* 1..MEMORY_PHYSICAL_SESSION_MAX_QUERY_USES */
} memory_physical_session_config_t;

/* Numeric-only aggregate metrics. No session ID, nonce, purpose history, event,
 * person, card or collection property is retained as product telemetry. */
typedef struct {
    uint32_t begun;
    uint32_t consumed;
    uint32_t cancelled;
    uint32_t expired;
    uint32_t rejected_format;
    uint32_t rejected_state;
    uint32_t rejected_purpose;
    uint32_t rejected_assertion;
    uint32_t rejected_time;
} memory_physical_session_metrics_t;

typedef struct {
    memory_physical_session_config_t cfg;
    memory_physical_session_state_t state;
    uint32_t session_floor; /* RAM-only; resets on reboot and is not a durable counter */
    uint32_t active_session_id;
    uint32_t active_event_nonce;
    memory_physical_purpose_t active_purpose;
    uint32_t started_at_ms;
    uint32_t expires_at_ms;
    uint8_t uses_remaining;
    memory_physical_session_metrics_t metrics;
} memory_physical_session_t;

enum {
    MEMORY_PHYSICAL_SESSION_OK          =  0,
    MEMORY_PHYSICAL_SESSION_E_ARG       = -1,
    MEMORY_PHYSICAL_SESSION_E_CONFIG    = -2,
    MEMORY_PHYSICAL_SESSION_E_FORMAT    = -3,
    MEMORY_PHYSICAL_SESSION_E_STATE     = -4,
    MEMORY_PHYSICAL_SESSION_E_PURPOSE   = -5,
    MEMORY_PHYSICAL_SESSION_E_ASSERTION = -6,
    MEMORY_PHYSICAL_SESSION_E_TIME      = -7,
    MEMORY_PHYSICAL_SESSION_E_CANCELLED = -8
};

void memory_physical_session_config_default(memory_physical_session_config_t *cfg);
int memory_physical_session_init(memory_physical_session_t *gate,
                                 const memory_physical_session_config_t *cfg);

/* Begins one purpose-bound session after the future platform adapter observes a
 * physical event. `physical_event_confirmed` must be exactly 1; it is evidence
 * supplied by that adapter, not proof of gesture, liveness, identity or consent.
 * Only query purpose may carry more than one use, and never above the configured
 * local bound. A new begin is refused while a session remains ACTIVE and its ID
 * must be strictly above the RAM-only session floor. */
int memory_physical_session_begin(memory_physical_session_t *gate,
                                  memory_physical_purpose_t purpose,
                                  uint32_t physical_session_id,
                                  uint32_t event_nonce,
                                  uint8_t physical_event_confirmed,
                                  uint8_t requested_uses,
                                  uint32_t now_ms);

/* Validates an active purpose-bound session without consuming a use. It is for a
 * caller that must reject invalid access before advancing an independent budget.
 * It never creates authority and may only expire/scrub a late session. */
int memory_physical_session_validate(memory_physical_session_t *gate,
                                     memory_physical_purpose_t expected_purpose,
                                     uint32_t physical_session_id,
                                     uint32_t now_ms);

/* Consumes exactly one permitted use. The caller supplies a transient session ID
 * and observed monotonic time; the hidden event nonce remains inside the gate.
 * Wrong purpose/ID, expiry, time rollback and replay never produce authority.
 * A successful last use moves the gate to CONSUMED and scrubs active evidence. */
int memory_physical_session_consume(memory_physical_session_t *gate,
                                    memory_physical_purpose_t expected_purpose,
                                    uint32_t physical_session_id,
                                    uint32_t now_ms);

/* Cancels only the current active session and scrubs its transient evidence. */
int memory_physical_session_cancel(memory_physical_session_t *gate);
const memory_physical_session_metrics_t *memory_physical_session_metrics(
    const memory_physical_session_t *gate);

#endif /* HERUS_MEMORY_PHYSICAL_SESSION_H */
