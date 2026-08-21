#include "attribution_guard.h"
#include <string.h>

static const uint32_t AUTH_ALLOWED = AT_AUTH_OBSERVATION | AT_AUTH_MEMORY;
static const uint32_t SCOPE_ALLOWED = AT_SCOPE_LOCAL_HAPTIC |
                                      AT_SCOPE_LOCAL_DIALOGUE |
                                      AT_SCOPE_LOCAL_RADIO;

static int valid_source(at_source_t source)
{
    return source == AT_SOURCE_LOCAL_OBSERVATION ||
           source == AT_SOURCE_CORE_KNOWLEDGE;
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
    if (index) {
        memset(index, 0, sizeof(*index));
        index->epoch = epoch == 0u ? 1u : epoch;
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
    node->source_root_id = node_id;
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
    node->source_root_id = parent->source_root_id;
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
    out_offer->source = node->source;
    out_offer->role = node->role;
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
    if (offer->source != AT_SOURCE_LOCAL_OBSERVATION ||
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
