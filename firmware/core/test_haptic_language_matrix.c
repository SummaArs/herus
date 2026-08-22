#include "haptic_language.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned frames;
    unsigned round_trips;
    unsigned field_corruptions;
    unsigned profile_mismatches;
    unsigned failures;
} matrix_score_t;

static hl_profile_t profile_fixture(hl_actuator_t actuator)
{
    hl_profile_t profile;
    uint8_t base = actuator == HL_ACTUATOR_LRA ? 10u : 40u;
    memset(&profile, 0, sizeof(profile));
    profile.version = HL_VERSION_1;
    profile.actuator = actuator;
    profile.profile_version = 3u;
    profile.effect_sync = (uint8_t)(base - 9u);
    profile.effect_mark = (uint8_t)(base - 8u);
    profile.effect_end = (uint8_t)(base - 7u);
    for (uint8_t i = 0u; i < HL_CODEBOOK_SIZE; i++)
        profile.effect_code[i] = (uint8_t)(base + i);
    return profile;
}

static hl_event_t event_fixture(uint8_t scope, uint8_t class_code,
                                uint8_t state, uint8_t urgency)
{
    hl_event_t event;
    memset(&event, 0, sizeof(event));
    event.version = HL_VERSION_1;
    event.scope = scope;
    event.class_code = class_code;
    event.state = state;
    event.urgency = urgency;
    event.fragment_total = 1u;
    return event;
}

static int check(int condition, matrix_score_t *score, const char *label)
{
    if (!condition) {
        printf("  FAIL  %s\n", label);
        score->failures++;
        return 0;
    }
    return 1;
}

static void exercise_profile(const hl_profile_t *profile,
                             const hl_profile_t *other,
                             matrix_score_t *score)
{
    for (uint8_t scope = 0u; scope < HL_SCOPE_COUNT; scope++) {
        for (uint8_t class_code = 0u; class_code < HL_CLASS_COUNT; class_code++) {
            for (uint8_t state = 0u; state < HL_STATE_COUNT; state++) {
                for (uint8_t urgency = 0u; urgency < HL_URGENCY_COUNT; urgency++) {
                    hl_event_t event = event_fixture(scope, class_code, state,
                                                     urgency);
                    hl_event_t decoded;
                    hl_encoded_t encoded;
                    int result = hl_encode(&event, profile, &encoded);
                    score->frames++;
                    if (!check(result == HL_OK && encoded.slot_count == 6u,
                               score, "every permitted semantic tuple encodes"))
                        continue;
                    result = hl_decode_with_profile(&encoded, profile, &decoded);
                    if (check(result == HL_OK &&
                                  memcmp(&event, &decoded, sizeof(event)) == 0,
                              score, "every tuple survives profile-bound round-trip"))
                        score->round_trips++;
                    result = hl_decode_with_profile(&encoded, other, &decoded);
                    if (check(result == HL_E_PROFILE, score,
                              "an incompatible actuator profile cannot decode silently"))
                        score->profile_mismatches++;

                    for (uint8_t slot = 1u; slot <= 4u; slot++) {
                        hl_encoded_t corrupted = encoded;
                        corrupted.code[slot] =
                            (uint8_t)((corrupted.code[slot] + 1u) & 0x0fu);
                        if (check(hl_decode(&corrupted, &decoded) != HL_OK,
                                  score, "single semantic-field corruption abstains"))
                            score->field_corruptions++;
                    }
                    for (uint8_t slot = 0u; slot < encoded.slot_count; slot++) {
                        hl_encoded_t corrupted = encoded;
                        corrupted.effect_id[slot] ^= 0x80u;
                        if (check(hl_decode_with_profile(&corrupted, profile,
                                                         &decoded) == HL_E_PROFILE,
                                  score, "single physical-field corruption abstains"))
                            score->field_corruptions++;
                    }
                }
            }
        }
    }
}

int main(void)
{
    matrix_score_t score = { 0u, 0u, 0u, 0u, 0u };
    hl_profile_t lra = profile_fixture(HL_ACTUATOR_LRA);
    hl_profile_t erm = profile_fixture(HL_ACTUATOR_ERM);
    hl_event_t fragmented = event_fixture(HL_SCOPE_COM, HL_CLASS_NOTICE,
                                          HL_STATE_NEW, HL_URGENCY_U1);
    hl_encoded_t encoded;

    check(hl_profile_validate(&lra) == HL_OK &&
              hl_profile_validate(&erm) == HL_OK,
          &score, "both physical profiles are internally valid");
    exercise_profile(&lra, &erm, &score);
    exercise_profile(&erm, &lra, &score);

    fragmented.has_data = 1u;
    check(hl_encode(&fragmented, &lra, &encoded) == HL_E_FRAGMENT,
          &score, "payload outside the compact frame is rejected");
    fragmented = event_fixture(HL_SCOPE_COM, HL_CLASS_NOTICE,
                               HL_STATE_NEW, HL_URGENCY_U1);
    fragmented.fragment_total = 2u;
    check(hl_encode(&fragmented, &lra, &encoded) == HL_E_FRAGMENT,
          &score, "multi-fragment frames are rejected without truncation");

    printf("HAPTIC LANGUAGE MATRIX: frames %u/%u, round-trips %u/%u, "
           "field corruptions %u, profile mismatches %u, failures %u\n",
           score.round_trips, score.frames,
           score.round_trips, score.frames,
           score.field_corruptions, score.profile_mismatches,
           score.failures);
    return score.failures == 0u && score.frames == 1440u &&
                   score.round_trips == 1440u &&
                   score.profile_mismatches == 1440u &&
                   score.field_corruptions == 14400u ? 0 : 1;
}
