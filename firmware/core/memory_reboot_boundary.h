/* memory_reboot_boundary.h — durable floor import with semantic-index quarantine.
 *
 * A reboot may restore anti-replay floors, but it must not restore active physical
 * sessions or volatile semantic evidence. Durable cards remain in the vault and
 * require a later explicit, authorised reindexing path.
 */
#ifndef HERUS_MEMORY_REBOOT_BOUNDARY_H
#define HERUS_MEMORY_REBOOT_BOUNDARY_H

#include "memory_physical_session_bootstrap.h"
#include "memory_semantic_evidence.h"

#include <stdint.h>

typedef struct {
    uint32_t recovered_session_floor;
    memory_physical_session_recovery_action_t recovery_action;
    uint8_t active_session_scrubbed;
    uint8_t semantic_index_scrubbed;
} memory_reboot_boundary_result_t;

enum {
    MEMORY_REBOOT_BOUNDARY_OK = 0,
    MEMORY_REBOOT_BOUNDARY_E_ARG = -1,
    MEMORY_REBOOT_BOUNDARY_E_RECOVERY = -2
};

/* Rebuilds an idle physical-session gate from an authenticated recovery snapshot
 * and clears all volatile semantic evidence. On any invalid input or recovery
 * contradiction, the gate is BLOCKED, the index is scrubbed and `out` is zeroed.
 * No card, fact, query, session capability, key, event or model result is imported.
 */
int memory_reboot_boundary_bootstrap(
    memory_physical_session_t *gate,
    mse_index_t *index,
    const memory_physical_session_config_t *session_cfg,
    const memory_physical_session_recovery_snapshot_t *snapshot,
    memory_reboot_boundary_result_t *out);

#endif /* HERUS_MEMORY_REBOOT_BOUNDARY_H */
