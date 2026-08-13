/* nucleus.h — private, bounded local intelligence for the Herus Nucleus.
 *
 * This is deliberately NOT a language model and it never opens a radio path.
 * It observes an explicitly authorised stream of already-structured HCP meanings,
 * learns which template follows which preceding meaning, and returns explainable
 * local suggestions. The caller remains responsible for confirmation, sequence,
 * priority, TTL, encryption and transmission.
 *
 * Safety properties:
 *   - learning is opt-in and disabled after nucleus_init();
 *   - no allocation, I/O, clock, radio or global mutable state;
 *   - fixed storage and deterministic eviction;
 *   - templates exclude sequence, ttl, priority, position and header flags;
 *   - a prediction never mutates learned state;
 *   - predictions require repeated observations and may expire.
 */
#ifndef HERUS_NUCLEUS_H
#define HERUS_NUCLEUS_H

#include <stdint.h>
#include "hcp.h"

#define NUC_RULE_CAP          32u
#define NUC_SUGGESTION_CAP     3u
#define NUC_MIN_SUPPORT        3u

/* Result codes distinguish a privacy-respecting disabled learner from malformed
 * input. A disabled learner is not an error and must remain silent. */
enum { NUC_OK = 0, NUC_DISABLED = 1, NUC_REJECTED = -1 };

typedef struct {
    uint8_t  used;
    uint64_t context;          /* fingerprint of the preceding authorised meaning */
    hcp_msg_t next;            /* semantic template only; transport fields are zero */
    uint16_t support;          /* observed occurrences of this exact transition */
    uint32_t last_seen;        /* caller-supplied monotonic tick */
} nucleus_rule_t;

typedef struct {
    nucleus_rule_t rule[NUC_RULE_CAP];
    uint64_t       context;        /* previous meaning; 0 means no context is armed */
    uint32_t       context_seen;   /* monotonic tick for the pending context TTL */
    uint8_t        learning_enabled;
} nucleus_t;

typedef struct {
    hcp_msg_t template;        /* fill seq/ttl/prio and confirm before transmission */
    uint16_t  support;
    uint16_t  observations;    /* all retained observations for this context */
    uint8_t   confidence_pct;  /* floor(100 * support / observations), never certainty */
} nucleus_suggestion_t;

/* Initialise with learning disabled. Context and rules contain no data. */
void nucleus_init(nucleus_t *n);

/* Explicitly enable or disable learning. Disabling also clears the pending
 * predecessor so an unconsented message cannot become the context of a later
 * authorised observation. Stored rules remain until forgotten or expired. */
void nucleus_set_learning(nucleus_t *n, int enabled);

/* Remove only the transient predecessor. Useful on peer change, lock, timeout or
 * after a user rejects a suggested action. */
void nucleus_clear_context(nucleus_t *n);

/* Observe a user-authorised HCP Glyph or Composed message. The first accepted
 * message only arms context; each subsequent one records a transition. No input
 * is transmitted, decrypted, rendered or stored outside the caller-owned state.
 * `now` is a monotonic tick supplied by the platform. */
int nucleus_observe(nucleus_t *n, const hcp_msg_t *m, uint32_t now);

/* Return up to `cap` ranked templates predicted after `context`. A result requires
 * NUC_MIN_SUPPORT observations and a nonzero `max_age`; confidence is computed
 * against every non-expired retained transition from this context. This call does
 * not mutate the learner. */
unsigned nucleus_suggest(const nucleus_t *n, const hcp_msg_t *context,
                         uint32_t now, uint32_t max_age,
                         nucleus_suggestion_t *out, unsigned cap);

/* Erase every learned template and context. This is the local privacy control. */
void nucleus_forget(nucleus_t *n);

/* Remove rules older than max_age ticks. A max_age of zero expires every rule. */
unsigned nucleus_expire(nucleus_t *n, uint32_t now, uint32_t max_age);

/* Number of live rules, for diagnostics and tests only. */
unsigned nucleus_rule_count(const nucleus_t *n);

/* ---------------- mobile-base governor ----------------------------------
 *
 * Telemetry is intentionally coarse and supplied by a trusted hardware layer.
 * This function makes recommendations only: it cannot transmit, change a group's
 * Rich/Reach provisioning, enable charging or override a physical safety limit.
 * In particular, changing profile per message would violate HCP P1 by making
 * message importance visible through airtime. */
typedef struct {
    uint8_t  core_link_ok;       /* encrypted Core<->Nucleus link is presently usable */
    uint8_t  pdr_pct;            /* recent external link packet delivery ratio, 0..100 */
    int16_t  rssi_dbm_x2;        /* external link RSSI in half-dBm, diagnostic only */
    int16_t  snr_db_x4;          /* external link SNR in quarter-dB, diagnostic only */
    uint16_t battery_permille;   /* 0..1000; fuel-gauge value, not an estimate from voltage */
    uint8_t  cradle_present;     /* electrical/mechanical detection, not a blind charge command */
} nucleus_telemetry_t;

typedef enum {
    NUC_BASE_HEALTHY = 0,
    NUC_BASE_REPOSITION,
    NUC_BASE_SAVE_POWER,
    NUC_BASE_CORE_UNREACHABLE
} nucleus_base_state_t;

typedef struct {
    nucleus_base_state_t state;
    uint8_t relay_recommended;   /* caller still enforces radio law and user role */
    uint8_t charge_recommended;  /* charger state machine still enforces thermal/current limits */
    uint8_t owner_alert;         /* request haptic/audio/UI feedback, never an autonomous frame */
} nucleus_governance_t;

/* Produce a deterministic, conservative recommendation from local telemetry. */
void nucleus_govern(const nucleus_telemetry_t *in, nucleus_governance_t *out);

#endif /* HERUS_NUCLEUS_H */
