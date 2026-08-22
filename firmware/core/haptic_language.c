#include "haptic_language.h"

#include <string.h>

static int valid_event(const hl_event_t *event)
{
    if (!event || event->version != HL_VERSION_1 ||
        event->scope >= HL_SCOPE_COUNT || event->class_code >= HL_CLASS_COUNT ||
        event->state >= HL_STATE_COUNT || event->urgency >= HL_URGENCY_COUNT ||
        event->has_data > 1u || event->fragment_total == 0u ||
        event->fragment_index >= event->fragment_total)
        return 0;
    if (event->has_data && event->data_code >= HL_MAX_DATA_CODES) return 0;
    return 1;
}

int hl_profile_validate(const hl_profile_t *profile)
{
    if (!profile || profile->version != HL_VERSION_1 ||
        (profile->actuator != HL_ACTUATOR_ERM &&
         profile->actuator != HL_ACTUATOR_LRA) ||
        profile->profile_version == 0u || profile->effect_sync == 0u ||
        profile->effect_mark == 0u || profile->effect_end == 0u)
        return HL_E_PROFILE;
    for (uint8_t i = 0u; i < HL_CODEBOOK_SIZE; i++) {
        if (profile->effect_code[i] == 0u ||
            profile->effect_code[i] == profile->effect_sync ||
            profile->effect_code[i] == profile->effect_mark ||
            profile->effect_code[i] == profile->effect_end)
            return HL_E_PROFILE;
        for (uint8_t j = 0u; j < i; j++) {
            if (profile->effect_code[i] == profile->effect_code[j])
                return HL_E_PROFILE;
        }
    }
    if (profile->effect_sync == profile->effect_mark ||
        profile->effect_sync == profile->effect_end ||
        profile->effect_mark == profile->effect_end)
        return HL_E_PROFILE;
    return HL_OK;
}

uint8_t hl_checksum(const hl_event_t *event)
{
    uint8_t checksum = 0x5au;
    const uint8_t fields[] = {
        event ? event->version : 0u,
        event ? event->scope : 0u,
        event ? event->class_code : 0u,
        event ? event->state : 0u,
        event ? event->urgency : 0u,
        event ? event->has_data : 0u,
        event ? event->data_code : 0u,
        event ? event->fragment_index : 0u,
        event ? event->fragment_total : 0u
    };
    for (size_t i = 0u; i < sizeof(fields); i++) {
        checksum ^= fields[i];
        checksum = (uint8_t)((checksum << 1u) | (checksum >> 7u));
    }
    return checksum;
}

int hl_symbol_to_effect(const hl_profile_t *profile, hl_symbol_kind_t kind,
                        uint8_t code, uint8_t *effect_id)
{
    if (hl_profile_validate(profile) != HL_OK || !effect_id) return HL_E_PROFILE;
    if (kind == HL_SYM_SYNC) {
        *effect_id = profile->effect_sync;
        return HL_OK;
    }
    if (kind == HL_SYM_MARK) {
        *effect_id = profile->effect_mark;
        return HL_OK;
    }
    if (kind == HL_SYM_END) {
        *effect_id = profile->effect_end;
        return HL_OK;
    }
    if (kind == HL_SYM_CODE && code < HL_CODEBOOK_SIZE) {
        *effect_id = profile->effect_code[code];
        return *effect_id == 0u ? HL_E_PROFILE : HL_OK;
    }
    return HL_E_UNKNOWN;
}

static int append_symbol(hl_encoded_t *out, const hl_profile_t *profile,
                         hl_symbol_kind_t kind, uint8_t code)
{
    uint8_t effect;
    int result;
    if (!out || out->slot_count >= HL_MAX_SLOTS) return HL_E_LIMIT;
    result = hl_symbol_to_effect(profile, kind, code, &effect);
    if (result != HL_OK) return result;
    out->kind[out->slot_count] = kind;
    out->code[out->slot_count] = code;
    out->effect_id[out->slot_count] = effect;
    out->slot_count++;
    return HL_OK;
}

int hl_encode(const hl_event_t *event, const hl_profile_t *profile,
              hl_encoded_t *out)
{
    int result;
    if (!event || !out) return HL_E_ARG;
    if (!valid_event(event)) return HL_E_FORMAT;
    if (hl_profile_validate(profile) != HL_OK) return HL_E_PROFILE;
    memset(out, 0, sizeof(*out));
    out->version = event->version;
    out->actuator = profile->actuator;
    result = append_symbol(out, profile, HL_SYM_SYNC, 0u);
    if (result != HL_OK) return result;
    /* Four base-16 positional fields carry the semantic header. The fixed
     * positions remove delimiter ambiguity and keep the complete frame within
     * six slots: SYNC + 4 fields + END. MARK remains reserved for fragments. */
    {
        const uint8_t fields[] = {
            event->scope, event->class_code, event->state, event->urgency
        };
        for (size_t i = 0u; i < sizeof(fields); i++) {
            result = append_symbol(out, profile, HL_SYM_CODE, fields[i]);
            if (result != HL_OK) return HL_E_FRAGMENT;
        }
    }
    /* Data and fragment metadata are deliberately omitted when the slot budget
     * cannot hold them; omission is never silently accepted as a full event. */
    if (event->has_data || event->fragment_total > 1u) return HL_E_FRAGMENT;
    result = append_symbol(out, profile, HL_SYM_END, 0u);
    if (result != HL_OK) return HL_E_FRAGMENT;
    out->checksum = hl_checksum(event);
    return HL_OK;
}

int hl_decode(const hl_encoded_t *encoded, hl_event_t *out)
{
    uint8_t fields[4];
    uint8_t field_count = 0u;
    uint8_t i;
    if (!encoded || !out) return HL_E_ARG;
    if (encoded->version != HL_VERSION_1 ||
        (encoded->actuator != HL_ACTUATOR_ERM &&
         encoded->actuator != HL_ACTUATOR_LRA) ||
        encoded->slot_count < 2u || encoded->slot_count > HL_MAX_SLOTS)
        return HL_E_FORMAT;
    if (encoded->kind[0] != HL_SYM_SYNC) return HL_E_FORMAT;
    if (encoded->kind[encoded->slot_count - 1u] != HL_SYM_END)
        return HL_E_FORMAT;
    for (i = 1u; i + 1u < encoded->slot_count; i++) {
        if (encoded->kind[i] == HL_SYM_CODE) {
            if (field_count >= 4u || encoded->code[i] >= 16u)
                return HL_E_FORMAT;
            fields[field_count++] = encoded->code[i];
        } else if (encoded->kind[i] != HL_SYM_MARK) {
            return HL_E_UNKNOWN;
        }
    }
    if (field_count != 4u) return HL_E_FORMAT;
    memset(out, 0, sizeof(*out));
    out->version = encoded->version;
    out->scope = fields[0];
    out->class_code = fields[1];
    out->state = fields[2];
    out->urgency = fields[3];
    out->fragment_total = 1u;
    if (!valid_event(out)) return HL_E_FORMAT;
    if (encoded->checksum != hl_checksum(out)) return HL_E_CHECKSUM;
    return HL_OK;
}

int hl_decode_with_profile(const hl_encoded_t *encoded,
                           const hl_profile_t *profile, hl_event_t *out)
{
    uint8_t expected;
    uint8_t i;
    int result;
    if (!encoded || !profile || !out) return HL_E_ARG;
    result = hl_profile_validate(profile);
    if (result != HL_OK) return result;
    if (encoded->actuator != profile->actuator) return HL_E_PROFILE;
    result = hl_decode(encoded, out);
    if (result != HL_OK) return result;
    for (i = 0u; i < encoded->slot_count; i++) {
        result = hl_symbol_to_effect(profile, encoded->kind[i],
                                     encoded->code[i], &expected);
        if (result != HL_OK || encoded->effect_id[i] != expected)
            return HL_E_PROFILE;
    }
    return HL_OK;
}
