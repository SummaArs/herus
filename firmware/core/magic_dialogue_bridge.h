/*
 * HERUS magic_dialogue_bridge — read-only presentation of local anticipation.
 *
 * It adapts the symbolic dialogue's reasoner to the magic proposal engine. It
 * never advances a dialogue turn, writes a fact, sends a message or executes
 * the proposal; presentation and physical confirmation remain external.
 */
#ifndef HERUS_MAGIC_DIALOGUE_BRIDGE_H
#define HERUS_MAGIC_DIALOGUE_BRIDGE_H

#include "magic_anticipation.h"
#include "symbolic_dialogue.h"
#include <stdint.h>

typedef enum {
    MDB_OK = 0,
    MDB_NO_PROPOSAL = 1,
    MDB_SILENT = 2,
    MDB_ABSTAIN = 3,
    MDB_BLOCKED = 4,
    MDB_LIMIT = 5,
    MDB_E_ARG = -1
} mdb_status_t;

typedef struct {
    mdb_status_t status;
    uint8_t presentable;
    uint8_t requires_confirmation;
    magic_proposal_t proposal;
} mdb_reply_t;

/* Read-only bridge over an already-initialised symbolic dialogue. The dialogue,
 * memory index and scratch remain caller-owned; no Core or external language
 * service is consulted. */
mdb_status_t mdb_propose(const sd_dialogue_t *dialogue,
                         const mse_index_t *memory,
                         uint32_t current_generation,
                         const magic_context_t *context,
                         const magic_policy_t *policy,
                         sr_reasoner_t *scratch,
                         mdb_reply_t *out);

#endif /* HERUS_MAGIC_DIALOGUE_BRIDGE_H */
