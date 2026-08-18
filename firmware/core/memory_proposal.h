/* memory_proposal.h — fail-closed compiler for typed local-model proposals.
 *
 * A future local model may propose a bounded semantic interpretation, but it must
 * not write a memory signal directly. This module validates the proposal's schema,
 * session binding and enums, then compiles it into the existing transient
 * memory_candidate_t contract. The proposal has no text, embedding, identity,
 * location, key, prompt, HCP or action field. It cannot make a candidate explicit:
 * explicit_remember remains a separate human-controlled fact and is always zero
 * here.
 *
 * This is analogous to a spec -> compiler -> gate pipeline, but adapted to HERUS:
 * model proposal -> deterministic compiler -> existing memory policy. Persistence,
 * communication and HCP creation remain outside this module.
 */
#ifndef HERUS_MEMORY_PROPOSAL_H
#define HERUS_MEMORY_PROPOSAL_H

#include <stdint.h>
#include "memory_capture.h"
#include "memory_extract.h"

#define MEMORY_PROPOSAL_SCHEMA_VERSION 1u

typedef struct {
    uint16_t schema_version;
    uint8_t  abstain;           /* canonical boolean; 1 means no candidate */
    memory_kind_t kind;
    memory_scope_t scope;
    memory_sensitivity_t sensitivity;
    uint8_t confidence_pct;
    uint8_t novelty_pct;
    uint8_t future_value_pct;
    uint8_t consequence_pct;
} memory_model_proposal_t;

typedef struct {
    uint32_t calls;
    uint32_t compiled;
    uint32_t abstained;
    uint32_t rejected_session;
    uint32_t rejected_schema;
    uint32_t rejected_value;
} memory_proposal_metrics_t;

enum {
    MEMORY_PROPOSAL_OK = 0,
    MEMORY_PROPOSAL_NO_CANDIDATE = 1,
    MEMORY_PROPOSAL_E_ARG = -1,
    MEMORY_PROPOSAL_E_SESSION = -2,
    MEMORY_PROPOSAL_E_SCHEMA = -3,
    MEMORY_PROPOSAL_E_VALUE = -4
};

typedef struct {
    memory_proposal_metrics_t metrics;
} memory_proposal_t;

/* Initialise a stateless compiler. It retains no proposal or candidate. */
void memory_proposal_init(memory_proposal_t *compiler);

/* Compile exactly one proposal from an active authorised capture session.
 * `proposal` is untrusted model output. `out` is cleared on every non-success,
 * including ABSTAIN. A successful output is still only a candidate: it has
 * explicit_remember == 0 and must pass memory_policy plus the existing human-gated
 * lifecycle before persistence or communication. */
int memory_proposal_compile(memory_proposal_t *compiler,
                            const memory_capture_t *capture,
                            uint32_t capture_session_id,
                            const memory_model_proposal_t *proposal,
                            memory_candidate_t *out);

const memory_proposal_metrics_t *memory_proposal_metrics(const memory_proposal_t *compiler);

#endif /* HERUS_MEMORY_PROPOSAL_H */
