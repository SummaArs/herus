#include "attribution_guard.h"
#include <string.h>

static const uint32_t AUTH_ALLOWED = AT_AUTH_OBSERVATION | AT_AUTH_MEMORY;
static const uint32_t SCOPE_ALLOWED = AT_SCOPE_LOCAL_HAPTIC |
                                      AT_SCOPE_LOCAL_DIALOGUE |
                                      AT_SCOPE_LOCAL_RADIO;

static int valid_source(at_source_t source)
{
    return source == AT_SOURCE_LOCAL_OBSERVATION ||
           source == AT_SOURCE_CORE_KNOWLEDGE ||
           source == AT_SOURCE_COMPOSITE;
}

static uint32_t source_mask_for(at_source_t source)
{
    if (source == AT_SOURCE_LOCAL_OBSERVATION) return AT_SOURCE_LOCAL_OBSERVATION;
    if (source == AT_SOURCE_CORE_KNOWLEDGE) return AT_SOURCE_CORE_KNOWLEDGE;
    return AT_SOURCE_LOCAL_OBSERVATION | AT_SOURCE_CORE_KNOWLEDGE;
}

static at_source_t source_for_mask(uint32_t mask)
{
    if (mask == AT_SOURCE_LOCAL_OBSERVATION) return AT_SOURCE_LOCAL_OBSERVATION;
    if (mask == AT_SOURCE_CORE_KNOWLEDGE) return AT_SOURCE_CORE_KNOWLEDGE;
    if (mask == (AT_SOURCE_LOCAL_OBSERVATION | AT_SOURCE_CORE_KNOWLEDGE))
        return AT_SOURCE_COMPOSITE;
    return 0;
}

static int valid_root_role(at_source_t source, ag_role_t role)
{
    if (source == AT_SOURCE_LOCAL_OBSERVATION)
        return role == AG_ROLE_OBSERVATION || role == AG_ROLE_PREFERENCE;
    if (source == AT_SOURCE_CORE_KNOWLEDGE)
        return role == AG_ROLE_KNOWLEDGE || role == AG_ROLE_POLICY;
    return 0;
}

static int valid_derived_role(const ag_node_t *parent, ag_edge_t edge,
                              ag_role_t role)
{
    if (!parent || (edge != AG_EDGE_DERIVED && edge != AG_EDGE_SUPPORTS &&
                    edge != AG_EDGE_CONTRADICTS))
        return 0;
    if (parent->role == AG_ROLE_OBSERVATION)
        return role == AG_ROLE_OBSERVATION || role == AG_ROLE_PREFERENCE;
    if (parent->role == AG_ROLE_KNOWLEDGE)
        return role == AG_ROLE_KNOWLEDGE || role == AG_ROLE_POLICY;
    if (parent->role == AG_ROLE_PREFERENCE)
        return role == AG_ROLE_PREFERENCE;
    if (parent->role == AG_ROLE_POLICY)
        return role == AG_ROLE_POLICY;
    return 0;
}

static int find_node(const ag_index_t *index, uint32_t node_id)
{
    uint16_t i;
    if (!index || node_id == 0u) return -1;
    for (i = 0u; i < index->node_count; i++)
        if (index->nodes[i].node_id == node_id) return (int)i;
    return -1;
}

static int valid_time(uint32_t generation, uint32_t born,
                      uint32_t valid_until)
{
    return generation >= born &&
           (valid_until == 0u || generation <= valid_until);
}

static int valid_record_shape(uint32_t node_id, uint32_t provenance_id,
                              uint32_t authority, uint32_t scope,
                              uint32_t epoch, uint32_t generation,
                              uint32_t valid_until)
{
    return node_id != 0u && provenance_id != 0u && authority != 0u &&
           (authority & ~AUTH_ALLOWED) == 0u &&
           (scope & ~SCOPE_ALLOWED) == 0u && epoch != 0u && generation != 0u &&
           (valid_until == 0u || valid_until >= generation);
}

static uint32_t min_valid(uint32_t a, uint32_t b)
{
    if (a == 0u) return b;
    if (b == 0u) return a;
    return a < b ? a : b;
}

static int node_usable(const ag_node_t *node, uint32_t epoch,
                       uint32_t generation)
{
    return node && node->status == AG_NODE_ACTIVE && node->epoch == epoch &&
           valid_time(generation, node->generation,
                      node->valid_until_generation) && !node->conflict;
}

static int is_revoked(const ag_index_t *index, uint32_t node_id)
{
    int position = find_node(index, node_id);
    return position >= 0 && index->nodes[position].status == AG_NODE_REVOKED;
}

static int exact_duplicate(const ag_index_t *index, uint32_t node_id,
                           uint32_t provenance_id)
{
    uint16_t i;
    if (!index) return 0;
    for (i = 0u; i < index->node_count; i++)
        if (index->nodes[i].node_id == node_id ||
            index->nodes[i].provenance_id == provenance_id)
            return 1;
    return 0;
}

void ag_init(ag_index_t *index, uint32_t epoch)
{
    ag_init_principal(index, epoch, AG_PRINCIPAL_LOCAL);
}

void ag_init_principal(ag_index_t *index, uint32_t epoch,
                       uint32_t principal_id)
{
    if (index) {
        memset(index, 0, sizeof(*index));
        index->epoch = epoch == 0u ? 1u : epoch;
        index->principal_id = principal_id == 0u ? AG_PRINCIPAL_LOCAL : principal_id;
    }
}

int ag_add_root(ag_index_t *index, uint32_t node_id, uint32_t provenance_id,
                at_source_t source, ag_role_t role, uint32_t authority,
                uint32_t scope, uint32_t epoch, uint32_t generation,
                uint32_t valid_until_generation)
{
    ag_node_t *node;
    if (!index || !valid_source(source) || !valid_root_role(source, role))
        return AG_E_ROLE;
    if (!valid_record_shape(node_id, provenance_id, authority, scope, epoch,
                            generation, valid_until_generation))
        return AG_E_FORMAT;
    if (epoch != index->epoch || generation <= index->generation_floor)
        return epoch == index->epoch ? AG_E_REPLAY : AG_E_EPOCH;
    if (exact_duplicate(index, node_id, provenance_id)) return AG_E_FORMAT;
    if (index->node_count >= AG_MAX_NODES) return AG_E_FULL;

    node = &index->nodes[index->node_count++];
    memset(node, 0, sizeof(*node));
    node->node_id = node_id;
    node->provenance_id = provenance_id;
    node->parent_id = 0u;
    node->secondary_parent_id = 0u;
    node->source_root_id = node_id;
    node->source_mask = source_mask_for(source);
    node->owner_principal_id = index->principal_id;
    node->issuer_principal_id = index->principal_id;
    node->source_share_id = 0u;
    node->secondary_share_id = 0u;
    node->purpose_token = 0u;
    node->source = source;
    node->role = role;
    node->edge = AG_EDGE_ROOT;
    node->authority = authority;
    node->scope = scope;
    node->generation = generation;
    node->valid_until_generation = valid_until_generation;
    node->epoch = epoch;
    node->status = AG_NODE_ACTIVE;
    index->additions++;
    return AG_OK;
}

int ag_derive(ag_index_t *index, uint32_t node_id, uint32_t provenance_id,
              uint32_t parent_id, ag_edge_t edge, ag_role_t role,
              uint32_t authority, uint32_t scope, uint32_t epoch,
              uint32_t generation, uint32_t valid_until_generation)
{
    int parent_position;
    ag_node_t *parent;
    ag_node_t *node;
    uint32_t inherited_expiry;

    if (!index || parent_id == 0u) return AG_E_ARG;
    parent_position = find_node(index, parent_id);
    if (parent_position < 0) return AG_E_PARENT;
    parent = &index->nodes[parent_position];
    if (parent->status == AG_NODE_REVOKED) return AG_E_REVOKED;
    if (!node_usable(parent, epoch, generation))
        return parent->epoch != epoch ? AG_E_EPOCH : AG_E_EXPIRED;
    if (!valid_derived_role(parent, edge, role)) return AG_E_ROLE;
    if (edge == AG_EDGE_AUTHORIZED_BY || role == AG_ROLE_ACTION ||
        role == AG_ROLE_OFFER || (authority & AT_AUTH_ACTION) != 0u)
        return AG_E_AUTH;
    if (!valid_record_shape(node_id, provenance_id, authority, scope, epoch,
                            generation, valid_until_generation))
        return AG_E_FORMAT;
    if ((authority & ~parent->authority) != 0u ||
        (scope & ~parent->scope) != 0u)
        return (authority & ~parent->authority) != 0u ? AG_E_AUTH : AG_E_SCOPE;
    inherited_expiry = min_valid(parent->valid_until_generation,
                                 valid_until_generation);
    if (parent->valid_until_generation != 0u &&
        (valid_until_generation == 0u ||
         valid_until_generation > parent->valid_until_generation))
        return AG_E_EXPIRED;
    if (inherited_expiry != 0u && generation > inherited_expiry)
        return AG_E_EXPIRED;
    if (exact_duplicate(index, node_id, provenance_id)) return AG_E_FORMAT;
    if (index->node_count >= AG_MAX_NODES) return AG_E_FULL;

    node = &index->nodes[index->node_count++];
    memset(node, 0, sizeof(*node));
    node->node_id = node_id;
    node->provenance_id = provenance_id;
    node->parent_id = parent->node_id;
    node->secondary_parent_id = 0u;
    node->source_root_id = parent->source_root_id;
    node->source_mask = parent->source_mask;
    node->owner_principal_id = parent->owner_principal_id;
    node->issuer_principal_id = parent->issuer_principal_id;
    node->source_share_id = parent->source_share_id;
    node->secondary_share_id = parent->secondary_share_id;
    node->purpose_token = parent->purpose_token;
    node->source = parent->source;
    node->role = role;
    node->edge = edge;
    node->authority = authority;
    node->scope = scope;
    node->generation = generation;
    node->valid_until_generation = inherited_expiry;
    node->epoch = epoch;
    node->status = AG_NODE_ACTIVE;
    index->derivations++;
    return AG_OK;
}

int ag_set_purpose(ag_index_t *index, uint32_t node_id,
                    uint32_t purpose_token)
{
    int position;
    if (!index || node_id == 0u || purpose_token == 0u)
        return AG_E_PURPOSE;
    position = find_node(index, node_id);
    if (position < 0) return AG_E_PARENT;
    if (index->nodes[position].owner_principal_id != index->principal_id)
        return AG_E_PRINCIPAL;
    if (index->nodes[position].status != AG_NODE_ACTIVE)
        return AG_E_REVOKED;
    index->nodes[position].purpose_token = purpose_token;
    return AG_OK;
}

int ag_compose(ag_index_t *index, uint32_t left_id, uint32_t right_id,
                uint32_t node_id, uint32_t provenance_id, ag_role_t role,
                uint32_t purpose_token, uint32_t epoch, uint32_t generation,
                uint32_t valid_until_generation)
{
    int left_position;
    int right_position;
    ag_node_t *left;
    ag_node_t *right;
    ag_node_t *node;
    uint32_t authority;
    uint32_t scope;
    uint32_t inherited_expiry;
    uint32_t source_mask;

    if (!index || left_id == 0u || right_id == 0u || left_id == right_id ||
        purpose_token == 0u)
        return AG_E_ARG;
    left_position = find_node(index, left_id);
    right_position = find_node(index, right_id);
    if (left_position < 0 || right_position < 0) return AG_E_PARENT;
    left = &index->nodes[left_position];
    right = &index->nodes[right_position];
    if (left->owner_principal_id != index->principal_id ||
        right->owner_principal_id != index->principal_id)
        return AG_E_PRINCIPAL;
    if (left->source_share_id != 0u || right->source_share_id != 0u)
        return AG_E_SHARE;
    if (left->status == AG_NODE_REVOKED || right->status == AG_NODE_REVOKED)
        return AG_E_REVOKED;
    if (left->status == AG_NODE_QUARANTINED || right->status == AG_NODE_QUARANTINED)
        return AG_E_EPOCH;
    if (!node_usable(left, epoch, generation) ||
        !node_usable(right, epoch, generation))
        return left->epoch != epoch || right->epoch != epoch ? AG_E_EPOCH : AG_E_EXPIRED;
    if (role != AG_ROLE_POLICY && role != AG_ROLE_KNOWLEDGE)
        return AG_E_ROLE;
    if (role == AG_ROLE_KNOWLEDGE &&
        (left->role == AG_ROLE_POLICY || right->role == AG_ROLE_POLICY))
        return AG_E_ROLE;
    authority = left->authority & right->authority;
    scope = left->scope & right->scope;
    if (authority == 0u) return AG_E_AUTH;
    if (scope == 0u) return AG_E_SCOPE;
    inherited_expiry = min_valid(left->valid_until_generation,
                                 right->valid_until_generation);
    if (valid_until_generation != 0u &&
        (inherited_expiry == 0u || valid_until_generation > inherited_expiry))
        return AG_E_EXPIRED;
    if (inherited_expiry != 0u && generation > inherited_expiry)
        return AG_E_EXPIRED;
    if (epoch != index->epoch || generation <= index->generation_floor)
        return epoch == index->epoch ? AG_E_REPLAY : AG_E_EPOCH;
    if (exact_duplicate(index, node_id, provenance_id)) return AG_E_FORMAT;
    if (index->node_count >= AG_MAX_NODES) return AG_E_FULL;
    source_mask = left->source_mask | right->source_mask;
    node = &index->nodes[index->node_count++];
    memset(node, 0, sizeof(*node));
    node->node_id = node_id;
    node->provenance_id = provenance_id;
    node->parent_id = left->node_id;
    node->secondary_parent_id = right->node_id;
    node->source_root_id = left->source_root_id == right->source_root_id ?
                           left->source_root_id : 0u;
    node->source_mask = source_mask;
    node->owner_principal_id = index->principal_id;
    node->issuer_principal_id = index->principal_id;
    node->source_share_id = left->source_share_id | right->source_share_id;
    node->secondary_share_id = left->secondary_share_id | right->secondary_share_id;
    node->purpose_token = purpose_token;
    node->source = source_for_mask(source_mask);
    node->role = role;
    node->edge = AG_EDGE_SUPPORTS;
    node->authority = authority;
    node->scope = scope;
    node->generation = generation;
    node->valid_until_generation = min_valid(inherited_expiry,
                                             valid_until_generation);
    node->epoch = epoch;
    node->status = AG_NODE_ACTIVE;
    index->derivations++;
    return AG_OK;
}

int ag_admit(const ag_index_t *index, uint32_t node_id,
            ag_role_t required_role, uint32_t purpose_token,
            uint32_t expected_purpose_token, uint32_t current_epoch,
            uint32_t generation, ag_offer_t *out_offer)
{
    int position;
    const ag_node_t *node;
    if (!index || !out_offer || required_role == AG_ROLE_NONE)
        return AG_E_ARG;
    if (purpose_token == 0u || purpose_token != expected_purpose_token)
        return AG_E_PURPOSE;
    position = find_node(index, node_id);
    if (position < 0) return AG_E_PARENT;
    node = &index->nodes[position];
    if (node->status == AG_NODE_REVOKED) return AG_E_REVOKED;
    if (node->status == AG_NODE_EXPIRED) return AG_E_EXPIRED;
    if (node->status == AG_NODE_QUARANTINED) return AG_E_EPOCH;
    if (node->status != AG_NODE_ACTIVE) return AG_E_REPLAY;
    if (node->owner_principal_id != index->principal_id)
        return AG_E_PRINCIPAL;
    if (node->epoch != current_epoch) return AG_E_EPOCH;
    if (!valid_time(generation, node->generation,
                    node->valid_until_generation)) return AG_E_EXPIRED;
    if (node->conflict) return AG_E_CONFLICT;
    if (node->role != required_role) return AG_E_ROLE;
    if ((node->authority & AT_AUTH_ACTION) != 0u) return AG_E_AUTH;

    memset(out_offer, 0, sizeof(*out_offer));
    out_offer->node_id = node->node_id;
    out_offer->provenance_id = node->provenance_id;
    out_offer->source_root_id = node->source_root_id;
    out_offer->source_mask = node->source_mask;
    out_offer->owner_principal_id = node->owner_principal_id;
    out_offer->issuer_principal_id = node->issuer_principal_id;
    out_offer->source = node->source;
    out_offer->role = node->role;
    out_offer->purpose_token = node->purpose_token;
    out_offer->authority = node->authority;
    out_offer->scope = node->scope;
    out_offer->generation = generation;
    out_offer->valid_until_generation = node->valid_until_generation;
    out_offer->epoch = node->epoch;
    out_offer->physically_confirmed = 0u;
    return AG_OK;
}

int ag_grant_local_action(const ag_index_t *index, const ag_offer_t *offer,
                         uint32_t current_epoch, uint32_t local_scope,
                         uint8_t physical_confirmation, uint32_t generation,
                         at_capsule_t *out_action)
{
    at_machine_t machine;
    at_capsule_t capsule;
    int status;
    if (!index || !offer || !out_action) return AG_E_ARG;
    if (offer->epoch != current_epoch || offer->epoch != index->epoch)
        return AG_E_EPOCH;
    if (index->principal_id != AG_PRINCIPAL_LOCAL ||
        offer->owner_principal_id != AG_PRINCIPAL_LOCAL)
        return AG_E_PRINCIPAL;
    if (offer->source != AT_SOURCE_LOCAL_OBSERVATION ||
        offer->source_mask != AT_SOURCE_LOCAL_OBSERVATION ||
        offer->issuer_principal_id != AG_PRINCIPAL_LOCAL ||
        offer->role != AG_ROLE_PREFERENCE)
        return AG_E_ACTION;
    if (local_scope == 0u || (local_scope & ~offer->scope) != 0u ||
        (local_scope & ~SCOPE_ALLOWED) != 0u)
        return AG_E_SCOPE;
    if (physical_confirmation != 1u) return AG_E_AUTH;
    if (!valid_time(generation, offer->generation,
                    offer->valid_until_generation)) return AG_E_EXPIRED;

    memset(&capsule, 0, sizeof(capsule));
    capsule.stage = AT_STAGE_OFFER;
    capsule.source = offer->source;
    capsule.provenance_id = offer->provenance_id;
    capsule.authority = offer->authority;
    capsule.scope = offer->scope;
    capsule.generation = offer->generation;
    capsule.epoch = offer->epoch;
    capsule.valid_until_generation = offer->valid_until_generation;
    at_init(&machine);
    machine.epoch = offer->epoch;
    status = at_grant_local_action(&machine, &capsule, local_scope,
                                   physical_confirmation, generation,
                                   out_action);
    return status == AT_OK ? AG_OK : AG_E_AUTH;
}

static int share_id_is_revoked(const ag_index_t *index, uint32_t share_id)
{
    uint16_t i;
    if (!index || share_id == 0u) return 0;
    for (i = 0u; i < index->revoked_share_count; i++)
        if (index->revoked_share_ids[i] == share_id) return 1;
    return 0;
}

static int share_id_is_exported(const ag_index_t *index, uint32_t share_id)
{
    uint16_t i;
    if (!index || share_id == 0u) return 0;
    for (i = 0u; i < index->exported_share_count; i++)
        if (index->exported_share_ids[i] == share_id) return 1;
    return 0;
}

static int imported_share_seen(const ag_index_t *index, uint32_t share_id)
{
    uint16_t i;
    if (!index || share_id == 0u) return 0;
    for (i = 0u; i < index->imported_share_count; i++)
        if (index->imported_share_ids[i] == share_id) return 1;
    return 0;
}

int ag_export_share(ag_index_t *index, uint32_t node_id,
                   uint32_t share_id, uint32_t recipient_principal_id,
                   uint8_t physical_confirmation, uint32_t generation,
                   ag_share_t *out_share)
{
    int position;
    const ag_node_t *node;
    if (!index || !out_share || share_id == 0u ||
        recipient_principal_id == 0u || physical_confirmation != 1u)
        return AG_E_SHARE;
    if (recipient_principal_id == index->principal_id)
        return AG_E_PRINCIPAL;
    if (share_id_is_exported(index, share_id) ||
        share_id_is_revoked(index, share_id))
        return AG_E_REPLAY;
    if (index->exported_share_count >= AG_MAX_SHARES)
        return AG_E_FULL;
    position = find_node(index, node_id);
    if (position < 0) return AG_E_PARENT;
    node = &index->nodes[position];
    if (!node_usable(node, index->epoch, generation))
        return node->epoch != index->epoch ? AG_E_EPOCH : AG_E_EXPIRED;
    if (node->role == AG_ROLE_ACTION || node->role == AG_ROLE_OFFER ||
        (node->authority & AT_AUTH_ACTION) != 0u)
        return AG_E_SHARE;
    if (node->source_share_id != 0u || node->secondary_share_id != 0u)
        return AG_E_SHARE;
    /* share_id lives in the envelope namespace, not the node namespace. */
    memset(out_share, 0, sizeof(*out_share));
    out_share->share_id = share_id;
    out_share->node_id = node->node_id;
    out_share->provenance_id = node->provenance_id;
    out_share->source_root_id = node->source_root_id;
    out_share->source_mask = node->source_mask;
    out_share->issuer_principal_id = index->principal_id;
    out_share->recipient_principal_id = recipient_principal_id;
    out_share->source = node->source;
    out_share->role = node->role;
    out_share->purpose_token = node->purpose_token;
    out_share->authority = node->authority;
    out_share->scope = node->scope;
    out_share->generation = generation;
    out_share->valid_until_generation = node->valid_until_generation;
    out_share->issuer_epoch = index->epoch;
    out_share->physically_confirmed = 1u;
    index->exported_share_ids[index->exported_share_count] = share_id;
    index->exported_share_node_ids[index->exported_share_count] = node->node_id;
    index->exported_share_recipient_ids[index->exported_share_count] =
        recipient_principal_id;
    index->exported_share_count++;
    return AG_OK;
}

int ag_import_share(ag_index_t *index, const ag_share_t *share,
                    uint8_t physical_confirmation, uint32_t generation)
{
    ag_node_t *node;
    if (!index || !share || physical_confirmation != 1u)
        return AG_E_SHARE;
    if (share->share_id == 0u || share->node_id == 0u ||
        share->provenance_id == 0u || share->issuer_principal_id == 0u ||
        share->recipient_principal_id != index->principal_id ||
        share->issuer_principal_id == index->principal_id)
        return AG_E_PRINCIPAL;
    if (share->physically_confirmed != 1u || share->issuer_epoch == 0u)
        return AG_E_SHARE;
    if (imported_share_seen(index, share->share_id) ||
        exact_duplicate(index, share->node_id, share->provenance_id))
        return AG_E_REPLAY;
    if (share_id_is_revoked(index, share->share_id)) return AG_E_REVOKED;
    if (!valid_source(share->source) ||
        source_mask_for(share->source) != share->source_mask ||
        !valid_record_shape(share->node_id, share->provenance_id,
                            share->authority, share->scope, index->epoch,
                            generation, share->valid_until_generation))
        return AG_E_FORMAT;
    if (share->role == AG_ROLE_ACTION || share->role == AG_ROLE_OFFER ||
        (share->authority & AT_AUTH_ACTION) != 0u)
        return AG_E_AUTH;
    if (share->valid_until_generation != 0u &&
        generation > share->valid_until_generation)
        return AG_E_EXPIRED;
    if (index->node_count >= AG_MAX_NODES ||
        index->imported_share_count >= AG_MAX_SHARES)
        return AG_E_FULL;
    node = &index->nodes[index->node_count++];
    memset(node, 0, sizeof(*node));
    node->node_id = share->node_id;
    node->provenance_id = share->provenance_id;
    node->parent_id = 0u;
    node->secondary_parent_id = 0u;
    node->source_root_id = share->source_root_id;
    node->source_mask = share->source_mask;
    node->owner_principal_id = index->principal_id;
    node->issuer_principal_id = share->issuer_principal_id;
    node->source_share_id = share->share_id;
    node->secondary_share_id = 0u;
    node->purpose_token = share->purpose_token;
    node->source = share->source;
    node->role = share->role;
    node->edge = AG_EDGE_SUPPORTS;
    node->authority = share->authority;
    node->scope = share->scope;
    node->generation = generation;
    node->valid_until_generation = share->valid_until_generation;
    node->epoch = index->epoch;
    node->status = AG_NODE_ACTIVE;
    index->imported_share_ids[index->imported_share_count++] = share->share_id;
    index->additions++;
    return AG_OK;
}

static int exported_share_slot(const ag_index_t *index, uint32_t share_id)
{
    uint16_t i;
    if (!index || share_id == 0u) return -1;
    for (i = 0u; i < index->exported_share_count; i++)
        if (index->exported_share_ids[i] == share_id) return (int)i;
    return -1;
}

static int imported_share_position(const ag_index_t *index, uint32_t share_id)
{
    uint16_t i;
    if (!index || share_id == 0u) return -1;
    for (i = 0u; i < index->node_count; i++)
        if (index->nodes[i].source_share_id == share_id) return (int)i;
    return -1;
}

static int record_revoked_share(ag_index_t *index, uint32_t share_id)
{
    if (!index || share_id == 0u) return AG_E_SHARE;
    if (share_id_is_revoked(index, share_id)) return AG_NO_CHANGE;
    if (index->revoked_share_count >= AG_MAX_SHARES) return AG_E_FULL;
    index->revoked_share_ids[index->revoked_share_count++] = share_id;
    return AG_OK;
}

int ag_revoke_share(ag_index_t *index, uint32_t share_id,
                    uint8_t physical_confirmation, uint32_t generation)
{
    int slot;
    int status;
    if (!index || share_id == 0u || physical_confirmation != 1u)
        return AG_E_AUTH;
    slot = exported_share_slot(index, share_id);
    if (slot < 0) return AG_E_PARENT;
    status = ag_revoke(index, index->exported_share_node_ids[slot],
                       physical_confirmation, generation);
    if (status != AG_OK && status != AG_NO_CHANGE) return status;
    if (record_revoked_share(index, share_id) == AG_E_FULL)
        return AG_E_FULL;
    return status;
}

int ag_export_revocation(const ag_index_t *index, uint32_t share_id,
                         uint32_t recipient_principal_id,
                         uint8_t physical_confirmation, uint32_t generation,
                         ag_revocation_t *out_revocation)
{
    int slot;
    if (!index || !out_revocation || share_id == 0u ||
        recipient_principal_id == 0u || physical_confirmation != 1u)
        return AG_E_SHARE;
    slot = exported_share_slot(index, share_id);
    if (slot < 0) return AG_E_PARENT;
    if (index->exported_share_recipient_ids[slot] != recipient_principal_id)
        return AG_E_PRINCIPAL;
    if (!share_id_is_revoked(index, share_id)) return AG_E_REVOKED;
    memset(out_revocation, 0, sizeof(*out_revocation));
    out_revocation->share_id = share_id;
    out_revocation->issuer_principal_id = index->principal_id;
    out_revocation->recipient_principal_id = recipient_principal_id;
    out_revocation->issuer_epoch = index->epoch;
    out_revocation->revocation_generation = generation;
    out_revocation->physically_confirmed = 1u;
    return AG_OK;
}

int ag_apply_revocation(ag_index_t *index, const ag_revocation_t *revocation,
                        uint8_t physical_confirmation, uint32_t generation)
{
    int position;
    int status;
    if (!index || !revocation || physical_confirmation != 1u)
        return AG_E_AUTH;
    if (revocation->share_id == 0u ||
        revocation->recipient_principal_id != index->principal_id ||
        revocation->issuer_principal_id == index->principal_id ||
        revocation->issuer_principal_id == 0u)
        return AG_E_PRINCIPAL;
    if (revocation->physically_confirmed != 1u ||
        revocation->issuer_epoch == 0u ||
        revocation->revocation_generation == 0u)
        return AG_E_SHARE;
    position = imported_share_position(index, revocation->share_id);
    if (position < 0) {
        if (share_id_is_revoked(index, revocation->share_id)) return AG_NO_CHANGE;
        return record_revoked_share(index, revocation->share_id);
    }
    if (share_id_is_revoked(index, revocation->share_id)) return AG_NO_CHANGE;
    status = ag_revoke(index, index->nodes[position].node_id, 1u, generation);
    if (status != AG_OK && status != AG_NO_CHANGE) return status;
    return record_revoked_share(index, revocation->share_id);
}

int ag_revoke(ag_index_t *index, uint32_t node_id,
              uint8_t physical_confirmation, uint32_t generation)
{
    uint16_t i;
    int position;
    int changed = 0;
    int grew;
    if (!index || node_id == 0u || physical_confirmation != 1u)
        return AG_E_AUTH;
    position = find_node(index, node_id);
    if (position < 0) return AG_E_PARENT;
    if (generation < index->nodes[position].generation) return AG_E_REPLAY;
    do {
        grew = 0;
        for (i = 0u; i < index->node_count; i++) {
            ag_node_t *node = &index->nodes[i];
            if (node->status != AG_NODE_REVOKED &&
                (node->node_id == node_id || is_revoked(index, node->parent_id))) {
                node->status = AG_NODE_REVOKED;
                changed++;
                grew = 1;
            }
        }
    } while (grew);
    if (changed == 0) return AG_NO_CHANGE;
    index->revocations += (uint32_t)changed;
    return AG_OK;
}

unsigned ag_expire(ag_index_t *index, uint32_t generation)
{
    uint16_t i;
    unsigned changed = 0u;
    if (!index) return 0u;
    for (i = 0u; i < index->node_count; i++) {
        ag_node_t *node = &index->nodes[i];
        if (node->status == AG_NODE_ACTIVE &&
            node->valid_until_generation != 0u &&
            generation > node->valid_until_generation) {
            node->status = AG_NODE_EXPIRED;
            changed++;
        }
    }
    index->expirations += changed;
    return changed;
}

int ag_set_generation_floor(ag_index_t *index, uint32_t floor)
{
    if (!index || floor == 0u) return AG_E_ARG;
    if (index->node_count != 0u) return AG_E_FORMAT;
    if (floor < index->generation_floor) return AG_E_REPLAY;
    index->generation_floor = floor;
    return AG_OK;
}

void ag_reboot(ag_index_t *index)
{
    uint16_t i;
    if (!index) return;
    if (++index->epoch == 0u) index->epoch = 1u;
    for (i = 0u; i < index->node_count; i++)
        if (index->nodes[i].status == AG_NODE_ACTIVE)
            index->nodes[i].status = AG_NODE_QUARANTINED;
}
