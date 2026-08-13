/* memory_consolidation.h — human-controlled selective-memory consolidation.
 *
 * This module is a short-lived review workflow, not a database, text processor,
 * candidate extractor, search engine, LLM adapter, radio client or clock service.
 * It accepts a minimal typed proposal (never memory_candidate_t), holds it only
 * during a bounded physical review, and emits vault authority only after canonical
 * user confirmation. Conflict is never auto-resolved. Timeout/cancel/failure scrub
 * the pending proposal. Recall and erase require a separately canonical physical
 * access assertion supplied by a future UI adapter.
 */
#ifndef HERUS_MEMORY_CONSOLIDATION_H
#define HERUS_MEMORY_CONSOLIDATION_H

#include <stdint.h>
#include "memory_policy.h"
#include "memory_extract.h"
#include "memory_vault.h"

#define MEMORY_CONSOLIDATION_REVIEW_WINDOW_MS 30000u

typedef enum {
    MEMORY_CONSOLIDATION_IDLE = 0,
    MEMORY_CONSOLIDATION_REVIEWING,
    MEMORY_CONSOLIDATION_CONFLICTED,
    MEMORY_CONSOLIDATION_EXPIRED,
    MEMORY_CONSOLIDATION_FAILED
} memory_consolidation_state_t;

/* Intentionally not memory_candidate_t and deliberately contains no text, audio,
 * transcript, embedding, identity, location, timestamp, key or network field. */
typedef struct {
    uint32_t                card_id;
    memory_signal_t         signal;
    memory_extract_origin_t origin;
    uint32_t                extract_reasons;
} memory_consolidation_proposal_t;

/* A UI/hardware adapter is responsible for producing a true physical session. This
 * portable module validates only a nonzero identifier and canonical 1 confirmation;
 * it does not claim to prove a button, gesture, biometric or real-world identity. */
typedef struct {
    uint32_t physical_session_id;
    uint8_t  physical_confirmed; /* exactly 1 is true */
} memory_consolidation_access_t;

typedef struct {
    uint32_t review_window_ms; /* 1..2^31-1, default 30 seconds */
} memory_consolidation_config_t;

typedef struct {
    uint32_t begun;
    uint32_t persisted;
    uint32_t recalled;
    uint32_t erased;
    uint32_t cancelled;
    uint32_t expired;
    uint32_t conflicts;
    uint32_t rejected_proposals;
    uint32_t rejected_access;
    uint32_t vault_failures;
} memory_consolidation_metrics_t;

typedef struct {
    memory_consolidation_config_t cfg;
    memory_consolidation_state_t state;
    uint32_t active_physical_session_id;
    uint32_t started_at_ms;
    uint32_t next_review_receipt_id;
    memory_consolidation_proposal_t pending;
    memory_consolidation_metrics_t metrics;
} memory_consolidation_t;

enum {
    MEMORY_CONSOLIDATION_OK          =  0,
    MEMORY_CONSOLIDATION_E_ARG       = -1,
    MEMORY_CONSOLIDATION_E_CONFIG    = -2,
    MEMORY_CONSOLIDATION_E_STATE     = -3,
    MEMORY_CONSOLIDATION_E_PROPOSAL  = -4,
    MEMORY_CONSOLIDATION_E_ACCESS    = -5,
    MEMORY_CONSOLIDATION_E_EXPIRED   = -6,
    MEMORY_CONSOLIDATION_E_CONFLICT  = -7,
    MEMORY_CONSOLIDATION_E_VAULT     = -8
};

void memory_consolidation_config_default(memory_consolidation_config_t *cfg);
int memory_consolidation_init(memory_consolidation_t *c,
                              const memory_consolidation_config_t *cfg);

/* Starts a transient review. No record is written, no receipt is issued, and no
 * clock/timestamp is persisted. A proposal must already satisfy the selective
 * memory policy's AUTO_ELIGIBLE disposition. */
int memory_consolidation_begin(memory_consolidation_t *c,
                               const memory_consolidation_proposal_t *proposal,
                               uint32_t physical_session_id, uint32_t now_ms);

/* Expires a review whose bounded window elapsed. It is safe to call periodically;
 * the pending proposal is scrubbed and no write is attempted. */
int memory_consolidation_expire(memory_consolidation_t *c, uint32_t now_ms);

/* Records only that a caller identified an incompatible existing card. It does not
 * inspect content or decide which card is true; confirmation remains blocked until
 * the current proposal is cancelled and a new physical review begins. */
int memory_consolidation_mark_conflict(memory_consolidation_t *c,
                                      uint32_t competing_card_id, uint32_t now_ms);

/* A canonical physical confirmation under the same active session is the sole path
 * that creates a receipt and calls memory_vault_seal. Any vault failure scrubs the
 * proposal and moves the workflow to FAILED rather than retrying autonomously. */
int memory_consolidation_confirm_store(memory_consolidation_t *c,
                                       memory_vault_t *vault,
                                       const memory_consolidation_access_t *access,
                                       uint32_t now_ms);

/* Cancellation deliberately makes no retention decision and scrubs the proposal.
 * It can leave REVIEWING, CONFLICTED or EXPIRED; it never alters the coffer. */
int memory_consolidation_cancel(memory_consolidation_t *c);

/* Controlled recovery/erase of the single Step-4 vault record. Neither operation
 * parses free text, performs semantic search, uses a model, sends data nor creates
 * a record. On a vault failure the workflow enters FAILED and makes no success claim. */
int memory_consolidation_recall(memory_consolidation_t *c, memory_vault_t *vault,
                                uint32_t expected_card_id,
                                const memory_consolidation_access_t *access,
                                memory_vault_card_t *out);
int memory_consolidation_erase(memory_consolidation_t *c, memory_vault_t *vault,
                               const memory_consolidation_access_t *access);

const memory_consolidation_metrics_t *memory_consolidation_metrics(
    const memory_consolidation_t *c);

#endif /* HERUS_MEMORY_CONSOLIDATION_H */
