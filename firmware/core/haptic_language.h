/*
 * HERUS HAP-SEM v0.1 — language-independent haptic event frames.
 *
 * This module models the semantic channel only. Waveform/effect IDs are
 * supplied by an actuator profile; no TI library number is treated as a
 * universal meaning. The module never stores or emits text, audio, identity,
 * location or authority.
 */
#ifndef HERUS_HAPTIC_LANGUAGE_H
#define HERUS_HAPTIC_LANGUAGE_H

#include <stddef.h>
#include <stdint.h>

#define HL_VERSION_1       1u
#define HL_MAX_SLOTS       8u
#define HL_MAX_DATA_CODES  1u
#define HL_CODEBOOK_SIZE   16u

#define HL_SCOPE_SYS       0u
#define HL_SCOPE_COM       1u
#define HL_SCOPE_MEM       2u
#define HL_SCOPE_PLAN      3u
#define HL_SCOPE_SFTY      4u
#define HL_SCOPE_COUNT     5u

#define HL_CLASS_NOTICE    0u
#define HL_CLASS_ALERT     1u
#define HL_CLASS_QUERY     2u
#define HL_CLASS_ACK       3u
#define HL_CLASS_ERROR     4u
#define HL_CLASS_PRIVACY   5u
#define HL_CLASS_COUNT    6u

#define HL_STATE_NEW       0u
#define HL_STATE_PENDING   1u
#define HL_STATE_CONFIRMED 2u
#define HL_STATE_DENIED    3u
#define HL_STATE_UNKNOWN   4u
#define HL_STATE_EXPIRED   5u
#define HL_STATE_COUNT     6u

#define HL_URGENCY_U0      0u
#define HL_URGENCY_U1      1u
#define HL_URGENCY_U2      2u
#define HL_URGENCY_U3      3u
#define HL_URGENCY_COUNT   4u

typedef enum {
    HL_ACTUATOR_ERM = 1,
    HL_ACTUATOR_LRA = 2
} hl_actuator_t;

typedef enum {
    HL_SYM_SYNC = 1,
    HL_SYM_MARK = 2,
    HL_SYM_CODE = 3,
    HL_SYM_END = 4,
    HL_SYM_ABSTAIN = 5
} hl_symbol_kind_t;

typedef enum {
    HL_OK = 0,
    HL_E_ARG = -1,
    HL_E_FORMAT = -2,
    HL_E_PROFILE = -3,
    HL_E_LIMIT = -4,
    HL_E_FRAGMENT = -5,
    HL_E_UNKNOWN = -6,
    HL_E_CHECKSUM = -7
} hl_status_t;

typedef struct {
    uint8_t version;
    uint8_t scope;
    uint8_t class_code;
    uint8_t state;
    uint8_t urgency;
    uint8_t has_data;
    uint8_t data_code;
    uint8_t fragment_index;
    uint8_t fragment_total;
} hl_event_t;

typedef struct {
    uint8_t version;
    hl_actuator_t actuator;
    uint8_t profile_version;
    uint8_t effect_sync;
    uint8_t effect_mark;
    uint8_t effect_end;
    uint8_t effect_code[HL_CODEBOOK_SIZE];
} hl_profile_t;

typedef struct {
    hl_symbol_kind_t kind[HL_MAX_SLOTS];
    uint8_t code[HL_MAX_SLOTS];
    uint8_t effect_id[HL_MAX_SLOTS];
    uint8_t slot_count;
    uint8_t checksum;
    uint8_t version;
    hl_actuator_t actuator;
} hl_encoded_t;

int hl_profile_validate(const hl_profile_t *profile);
uint8_t hl_checksum(const hl_event_t *event);
int hl_encode(const hl_event_t *event, const hl_profile_t *profile,
              hl_encoded_t *out);
int hl_decode(const hl_encoded_t *encoded, hl_event_t *out);
int hl_symbol_to_effect(const hl_profile_t *profile, hl_symbol_kind_t kind,
                        uint8_t code, uint8_t *effect_id);

#endif /* HERUS_HAPTIC_LANGUAGE_H */
