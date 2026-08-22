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
#define AG_MAX_SHARES 8u

#define AG_PRINCIPAL_LOCAL   1u
#define AG_PRINCIPAL_CORE    2u
#define AG_PRINCIPAL_CONTACT 4u

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
    AG_E_FORMAT = -14,
    AG_E_PRINCIPAL = -15,
    AG_E_SHARE = -16,
    AG_E_COMPOSITION = -17
} ag_status_t;

typedef struct {
    uint32_t node_id;
    uint32_t provenance_id;
    uint32_t parent_id;
    uint32_t secondary_parent_id;
    uint32_t source_root_id;
    uint32_t source_mask;
    uint32_t owner_principal_id;
    uint32_t issuer_principal_id;
    uint32_t source_share_id;
    uint32_t secondary_share_id;
    uint32_t purpose_token;
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
    uint32_t source_mask;
    uint32_t owner_principal_id;
    uint32_t issuer_principal_id;
    at_source_t source;
    ag_role_t role;
    uint32_t purpose_token;
    uint32_t authority;
    uint32_t scope;
    uint32_t generation;
    uint32_t valid_until_generation;
    uint32_t epoch;
    uint8_t physically_confirmed;
} ag_offer_t;

typedef struct {
    uint32_t share_id;
    uint32_t node_id;
    uint32_t provenance_id;
    uint32_t source_root_id;
    uint32_t source_mask;
    uint32_t issuer_principal_id;
    uint32_t recipient_principal_id;
    at_source_t source;
    ag_role_t role;
    uint32_t purpose_token;
    uint32_t authority;
    uint32_t scope;
    uint32_t generation;
    uint32_t valid_until_generation;
    uint32_t issuer_epoch;
    uint8_t physically_confirmed;
} ag_share_t;

typedef struct {
    uint32_t share_id;
    uint32_t issuer_principal_id;
    uint32_t recipient_principal_id;
    uint32_t issuer_epoch;
    uint32_t revocation_generation;
    uint8_t physically_confirmed;
} ag_revocation_t;

typedef struct {
    uint32_t epoch;
    uint32_t principal_id;
    uint32_t generation_floor;
    uint32_t additions;
    uint32_t derivations;
    uint32_t revocations;
    uint32_t expirations;
    uint32_t rejected;
    uint16_t node_count;
    ag_node_t nodes[AG_MAX_NODES];
    uint32_t imported_share_ids[AG_MAX_SHARES];
    uint16_t imported_share_count;
    uint32_t exported_share_ids[AG_MAX_SHARES];
    uint32_t exported_share_node_ids[AG_MAX_SHARES];
    uint32_t exported_share_recipient_ids[AG_MAX_SHARES];
    uint16_t exported_share_count;
    uint32_t revoked_share_ids[AG_MAX_SHARES];
    uint16_t revoked_share_count;
} ag_index_t;

void ag_init(ag_index_t *index, uint32_t epoch);
void ag_init_principal(ag_index_t *index, uint32_t epoch,
                       uint32_t principal_id);

int ag_add_root(ag_index_t *index, uint32_t node_id, uint32_t provenance_id,
                at_source_t source, ag_role_t role, uint32_t authority,
                uint32_t scope, uint32_t epoch, uint32_t generation,
                uint32_t valid_until_generation);

int ag_derive(ag_index_t *index, uint32_t node_id, uint32_t provenance_id,
              uint32_t parent_id, ag_edge_t edge, ag_role_t role,
              uint32_t authority, uint32_t scope, uint32_t epoch,
              uint32_t generation, uint32_t valid_until_generation);

int ag_set_purpose(ag_index_t *index, uint32_t node_id,
                    uint32_t purpose_token);

int ag_compose(ag_index_t *index, uint32_t left_id, uint32_t right_id,
                uint32_t node_id, uint32_t provenance_id, ag_role_t role,
                uint32_t purpose_token, uint32_t epoch, uint32_t generation,
                uint32_t valid_until_generation);

int ag_admit(const ag_index_t *index, uint32_t node_id,
            ag_role_t required_role, uint32_t purpose_token,
            uint32_t expected_purpose_token, uint32_t current_epoch,
            uint32_t generation, ag_offer_t *out_offer);

int ag_grant_local_action(const ag_index_t *index, const ag_offer_t *offer,
                         uint32_t current_epoch, uint32_t local_scope,
                         uint8_t physical_confirmation, uint32_t generation,
                         at_capsule_t *out_action);

int ag_export_share(ag_index_t *index, uint32_t node_id,
                     uint32_t share_id, uint32_t recipient_principal_id,
                     uint8_t physical_confirmation, uint32_t generation,
                     ag_share_t *out_share);

int ag_import_share(ag_index_t *index, const ag_share_t *share,
                    uint8_t physical_confirmation, uint32_t generation);

int ag_revoke_share(ag_index_t *index, uint32_t share_id,
                    uint8_t physical_confirmation, uint32_t generation);

int ag_export_revocation(const ag_index_t *index, uint32_t share_id,
                         uint32_t recipient_principal_id,
                         uint8_t physical_confirmation, uint32_t generation,
                         ag_revocation_t *out_revocation);

int ag_apply_revocation(ag_index_t *index, const ag_revocation_t *revocation,
                        uint8_t physical_confirmation, uint32_t generation);

int ag_revoke(ag_index_t *index, uint32_t node_id,
              uint8_t physical_confirmation, uint32_t generation);

unsigned ag_expire(ag_index_t *index, uint32_t generation);
int ag_set_generation_floor(ag_index_t *index, uint32_t floor);
void ag_reboot(ag_index_t *index);

#endif /* HERUS_ATTRIBUTION_GUARD_H */
