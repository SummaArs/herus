/*
 * authority_transition.h — Authority-Governed Semantic Continuity (AGSC).
 *
 * This is a host-side, deterministic contract for the personal semantic core.
 * It is deliberately not a language model, recognizer, identity engine or
 * hardware claim. A capsule carries provenance and authority through distinct
 * stages; promotion and action require separate evidence.
 */
#ifndef HERUS_AUTHORITY_TRANSITION_H
#define HERUS_AUTHORITY_TRANSITION_H

#include <stdint.h>

typedef enum {
    AT_SOURCE_LOCAL_OBSERVATION = 1u,
    AT_SOURCE_CORE_KNOWLEDGE = 2u
} at_source_t;

typedef enum {
    AT_STAGE_NONE = 0u,
    AT_STAGE_OBSERVATION,
    AT_STAGE_MEMORY,
    AT_STAGE_RETRIEVAL,
    AT_STAGE_OFFER,
    AT_STAGE_ACTION
} at_stage_t;

enum {
    AT_AUTH_OBSERVATION = 1u << 0,
    AT_AUTH_MEMORY      = 1u << 1,
    AT_AUTH_ACTION      = 1u << 2
};

enum {
    AT_SCOPE_LOCAL_HAPTIC   = 1u << 0,
    AT_SCOPE_LOCAL_DIALOGUE = 1u << 1,
    AT_SCOPE_LOCAL_RADIO    = 1u << 2,
    AT_SCOPE_CORE_EXECUTE   = 1u << 31
};

typedef enum {
    AT_OK = 0,
    AT_E_ARG = -1,
    AT_E_SOURCE = -2,
    AT_E_STAGE = -3,
    AT_E_AUTH = -4,
    AT_E_SCOPE = -5,
    AT_E_CONFLICT = -6,
    AT_E_EXPIRED = -7,
    AT_E_FORMAT = -8
} at_status_t;

typedef struct {
    at_stage_t stage;
    at_source_t source;
    uint32_t provenance_id;
    uint32_t authority;
    uint32_t scope;
    uint32_t generation;
    uint32_t epoch;
    uint32_t valid_until_generation;
    uint8_t conflict;
    uint8_t physically_confirmed;
} at_capsule_t;

typedef struct {
    uint32_t epoch;
    uint32_t observations;
    uint32_t memories;
    uint32_t retrievals;
    uint32_t offers;
    uint32_t actions;
    uint32_t rejected;
} at_machine_t;

void at_init(at_machine_t *machine);

int at_observe(at_machine_t *machine, at_source_t source,
               uint32_t provenance_id, uint32_t generation,
               uint32_t valid_until_generation, at_capsule_t *out);

int at_promote_memory(at_machine_t *machine, const at_capsule_t *candidate,
                      uint8_t physical_confirmation,
                      uint32_t generation, at_capsule_t *out);

int at_retrieve(at_machine_t *machine, const at_capsule_t *memory,
                uint32_t generation, at_capsule_t *out);

int at_offer(at_machine_t *machine, const at_capsule_t *retrieval,
             uint32_t generation, at_capsule_t *out);

int at_grant_local_action(at_machine_t *machine, const at_capsule_t *offer,
                          uint32_t local_scope, uint8_t physical_confirmation,
                          uint32_t generation, at_capsule_t *out);

int at_execute_local(const at_capsule_t *action, uint32_t requested_scope);

void at_mark_conflict(at_capsule_t *capsule);
void at_reboot(at_machine_t *machine);

#endif /* HERUS_AUTHORITY_TRANSITION_H */
