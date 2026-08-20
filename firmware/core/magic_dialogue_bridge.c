#include "magic_dialogue_bridge.h"
#include <string.h>

mdb_status_t mdb_propose(const sd_dialogue_t *dialogue,
                         const mse_index_t *memory,
                         uint32_t current_generation,
                         const magic_context_t *context,
                         const magic_policy_t *policy,
                         sr_reasoner_t *scratch,
                         mdb_reply_t *out)
{
    magic_status_t status;
    if (out) memset(out, 0, sizeof(*out));
    if (!dialogue || !memory || !context || !policy || !scratch || !out ||
        dialogue->active != 1u)
        return MDB_E_ARG;
    status = magic_propose(&dialogue->reasoner, memory, current_generation,
                           context, policy, scratch, &out->proposal);
    out->requires_confirmation = out->proposal.requires_confirmation;
    if (status == MAGIC_RECALL || status == MAGIC_CONNECTION ||
        status == MAGIC_KNOWN_GAP || status == MAGIC_CONTRADICTION) {
        out->status = status == MAGIC_CONTRADICTION ? MDB_ABSTAIN : MDB_OK;
        out->presentable = 1u;
        return out->status;
    }
    if (status == MAGIC_SILENT) {
        out->status = MDB_SILENT;
        return out->status;
    }
    if (status == MAGIC_SENSITIVE_BLOCK) {
        out->status = MDB_BLOCKED;
        return out->status;
    }
    if (status == MAGIC_ABSTAIN) {
        out->status = MDB_ABSTAIN;
        return out->status;
    }
    if (status == MAGIC_LIMIT) {
        out->status = MDB_LIMIT;
        return out->status;
    }
    out->status = MDB_NO_PROPOSAL;
    return out->status;
}
