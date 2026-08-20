/*
 * HERUS autonomy_policy — explicit initiative and confirmation envelope.
 *
 * This module is policy only. It does not execute an action, access a clock,
 * write memory, transmit, or grant authority to the Core.
 */
#ifndef HERUS_AUTONOMY_POLICY_H
#define HERUS_AUTONOMY_POLICY_H

#include <stdint.h>

typedef enum {
    HERUS_A0_SILENT = 0u,
    HERUS_A1_REACTIVE = 1u,
    HERUS_A2_CONTEXTUAL = 2u,
    HERUS_A3_PREPARATORY = 3u,
    HERUS_A4_CONFIRMED = 4u
} herus_autonomy_level_t;

typedef enum {
    HERUS_SCOPE_NONE = 0u,
    HERUS_SCOPE_PRESENT = 1u,
    HERUS_SCOPE_PREPARE = 2u,
    HERUS_SCOPE_REMEMBER = 3u,
    HERUS_SCOPE_TRANSMIT = 4u,
    HERUS_SCOPE_ACTUATE = 5u
} herus_action_scope_t;

typedef enum {
    HERUS_POLICY_OK = 0,
    HERUS_POLICY_SILENT = 1,
    HERUS_POLICY_NEEDS_CONFIRMATION = 2,
    HERUS_POLICY_REJECTED = -1,
    HERUS_POLICY_FORMAT = -2,
    HERUS_POLICY_SCOPE = -3,
    HERUS_POLICY_REVOKED = -4
} herus_policy_status_t;

typedef struct {
    herus_autonomy_level_t level;
    herus_action_scope_t scope;
    uint8_t proactive;
    uint8_t attention_window;
    uint8_t proactive_consent;
    uint8_t explicit_confirmation;
    uint8_t confirmation_consumed;
    uint8_t sensitive_context;
    uint8_t third_party_context;
    uint32_t proposal_id;
    uint32_t confirmation_id;
} herus_autonomy_envelope_t;

/* Validates the envelope without granting authority or modifying it. */
herus_policy_status_t herus_policy_validate(
    const herus_autonomy_envelope_t *envelope);

/* Classifies whether a proposal may be presented or needs one exact physical
 * confirmation. This function never executes or persists anything. */
herus_policy_status_t herus_policy_classify(
    const herus_autonomy_envelope_t *envelope);

/* Consumes exactly one matching confirmation for a bounded proposal. The
 * envelope is modified only to mark that exact confirmation as consumed. */
herus_policy_status_t herus_policy_consume_confirmation(
    herus_autonomy_envelope_t *envelope,
    uint32_t proposal_id,
    uint32_t confirmation_id);

#endif /* HERUS_AUTONOMY_POLICY_H */
