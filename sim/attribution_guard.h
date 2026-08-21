/*
 * attribution_guard.h — typed lineage and non-launderable admission.
 *
 * A derived record may support an offer, but it cannot change source, role,
 * authority or scope. Similarity is intentionally absent from this contract.
 * Local action still requires a fresh physical confirmation through AGSC.
 */
#ifndef HERUS_ATTRIBUTION_GUARD_H
#define HERUS_ATTRIBUTION_GUARD_H

#include "authority_transition.h"

#define AG_MAX_NODES 16u

typedef enum {
    AG_ROLE_NONE = 0u,
    AG_ROLE_OBSERVATION,
    AG_ROLE_KNOWLEDGE,
    AG_ROLE_PREFERENCE,
    AG_ROLE_POLICY,
    AG_ROLE_OFFER,
    AG_ROLE_ACTION
} ag_role_t;

typedef enum {
    AG_EDGE_NONE = 0u,
    AG_EDGE_ROOT,
    AG_EDGE_DERIVED,
    AG_EDGE_SUPPORTS,
    AG_EDGE_CONTRADICTS,
    AG_EDGE_AUTHORIZED_BY
} ag_edge_t;

typedef enum {
    AG_NODE_NONE = 0u,
    AG_NODE_ACTIVE,
    AG_NODE_REVOKED,
    AG_NODE_EXPIRED,
    AG_NODE_QUARANTINED
} ag_node_status_t;

typedef enum {
    AG_OK = 0,
    AG_NO_CHANGE = 1,
    AG_E_ARG = -1,
    AG_E_FULL = -2,
    AG_E_PARENT = -3,
    AG_E_ROLE = -4,
    AG_E_AUTH = -5,
    AG_E_SCOPE = -6,
    AG_E_EPOCH = -7,
    AG_E_EXPIRED = -8,
    AG_E_CONFLICT = -9,
    AG_E_REVOKED = -10,
    AG_E_PURPOSE = -11,
    AG_E_ACTION = -12,
    AG_E_REPLAY = -13,
    AG_E_FORMAT = -14
} ag_status_t;

typedef struct {
    uint32_t node_id;
    uint32_t provenance_id;
    uint32_t parent_id;
    uint32_t source_root_id;
    at_source_t source;
    ag_role_t role;
    ag_edge_t edge;
    uint32_t authority;
    uint32_t scope;
    uint32_t generation;
    uint32_t valid_until_generation;
    uint32_t epoch;
    uint8_t conflict;
    ag_node_status_t status;
} ag_node_t;

typedef struct {
    uint32_t node_id;
    uint32_t provenance_id;
    uint32_t source_root_id;
    at_source_t source;
    ag_role_t role;
    uint32_t authority;
    uint32_t scope;
    uint32_t generation;
    uint32_t valid_until_generation;
    uint32_t epoch;
    uint8_t physically_confirmed;
} ag_offer_t;

typedef struct {
    uint32_t epoch;
    uint32_t generation_floor;
    uint32_t additions;
    uint32_t derivations;
    uint32_t revocations;
    uint32_t expirations;
    uint32_t rejected;
    uint16_t node_count;
    ag_node_t nodes[AG_MAX_NODES];
} ag_index_t;

void ag_init(ag_index_t *index, uint32_t epoch);

int ag_add_root(ag_index_t *index, uint32_t node_id, uint32_t provenance_id,
                at_source_t source, ag_role_t role, uint32_t authority,
                uint32_t scope, uint32_t epoch, uint32_t generation,
                uint32_t valid_until_generation);

int ag_derive(ag_index_t *index, uint32_t node_id, uint32_t provenance_id,
              uint32_t parent_id, ag_edge_t edge, ag_role_t role,
              uint32_t authority, uint32_t scope, uint32_t epoch,
              uint32_t generation, uint32_t valid_until_generation);

int ag_admit(const ag_index_t *index, uint32_t node_id,
            ag_role_t required_role, uint32_t purpose_token,
            uint32_t expected_purpose_token, uint32_t current_epoch,
            uint32_t generation, ag_offer_t *out_offer);

int ag_grant_local_action(const ag_index_t *index, const ag_offer_t *offer,
                         uint32_t current_epoch, uint32_t local_scope,
                         uint8_t physical_confirmation, uint32_t generation,
                         at_capsule_t *out_action);

int ag_revoke(ag_index_t *index, uint32_t node_id,
              uint8_t physical_confirmation, uint32_t generation);

unsigned ag_expire(ag_index_t *index, uint32_t generation);
int ag_set_generation_floor(ag_index_t *index, uint32_t floor);
void ag_reboot(ag_index_t *index);

#endif /* HERUS_ATTRIBUTION_GUARD_H */
