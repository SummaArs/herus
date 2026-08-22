/* intent_router.h — bounded local text-intent routing.
 *
 * This module classifies a bounded local text observation. It returns a typed
 * proposal with bounded evidence; it never stores transcript bytes, allocates,
 * logs, sends, actuates or authorizes an action. A confirmation flag is a
 * requirement for a later physical gate, not confirmation itself.
 */
#ifndef HERUS_INTENT_ROUTER_H
#define HERUS_INTENT_ROUTER_H

#include <stddef.h>
#include <stdint.h>

#define INTENT_ROUTER_TEXT_MAX       160u
#define INTENT_ROUTER_MAX_EVIDENCE     2u
#define INTENT_ROUTER_CONF_STRONG    100u
#define INTENT_ROUTER_CONF_CHAT       85u

#define INTENT_ROUTER_MEMORY_ID_MEETING_OLD  1u
#define INTENT_ROUTER_MEMORY_ID_MEETING_NEW  2u
#define INTENT_ROUTER_MEMORY_ID_PREF_CONCISE 3u
#define INTENT_ROUTER_MEMORY_ID_PROJECT      5u

typedef enum {
    INTENT_ROUTER_UNKNOWN = 0,
    INTENT_ROUTER_RECALL_MEMORY,
    INTENT_ROUTER_CAPTURE_MEMORY,
    INTENT_ROUTER_ACTION_REQUEST,
    INTENT_ROUTER_FORGET_MEMORY,
    INTENT_ROUTER_UPDATE_PREFERENCE,
    INTENT_ROUTER_SHARE_MEMORY,
    INTENT_ROUTER_CONFLICT_QUERY,
    INTENT_ROUTER_CHITCHAT
} intent_router_kind_t;

typedef enum {
    INTENT_ROUTER_MEMORY_SCHEDULE = 0,
    INTENT_ROUTER_MEMORY_PREFERENCE,
    INTENT_ROUTER_MEMORY_PROJECT
} intent_router_memory_purpose_t;

typedef struct {
    uint32_t memory_id;
    intent_router_memory_purpose_t purpose;
    uint16_t generation;
    uint8_t  active;
    uint8_t  superseded;
    uint8_t  origin_local;
} intent_router_memory_t;

typedef struct {
    uint32_t memory_id;
    uint16_t generation;
    uint8_t  origin_local;
} intent_router_evidence_t;

typedef struct {
    intent_router_kind_t intent;
    uint8_t confidence_pct;
    uint8_t margin_pct;
    uint8_t abstain;
    uint8_t requires_confirmation;
    uint8_t evidence_count;
    intent_router_evidence_t evidence[INTENT_ROUTER_MAX_EVIDENCE];
} intent_router_result_t;

typedef enum {
    INTENT_ROUTER_OK = 0,
    INTENT_ROUTER_E_INPUT,
    INTENT_ROUTER_E_BOUNDS,
    INTENT_ROUTER_E_OUTPUT
} intent_router_status_t;

/* Route a bounded local observation against a bounded typed memory catalog.
 * `length` excludes any terminator. No input bytes are copied to `out`. */
intent_router_status_t intent_router_route(
    const char *text,
    size_t length,
    const intent_router_memory_t *memories,
    size_t memory_count,
    intent_router_result_t *out);

#endif /* HERUS_INTENT_ROUTER_H */
