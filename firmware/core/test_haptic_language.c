#include "haptic_language.h"

#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static hl_profile_t profile_fixture(hl_actuator_t actuator)
{
    hl_profile_t profile;
    memset(&profile, 0, sizeof(profile));
    profile.version = HL_VERSION_1;
    profile.actuator = actuator;
    profile.profile_version = 3u;
    profile.effect_sync = 1u;
    profile.effect_mark = 2u;
    profile.effect_end = 3u;
    for (uint8_t i = 0u; i < HL_CODEBOOK_SIZE; i++)
        profile.effect_code[i] = (uint8_t)(10u + i);
    return profile;
}

static hl_event_t event_fixture(void)
{
    hl_event_t event;
    memset(&event, 0, sizeof(event));
    event.version = HL_VERSION_1;
    event.scope = HL_SCOPE_COM;
    event.class_code = HL_CLASS_ALERT;
    event.state = HL_STATE_PENDING;
    event.urgency = HL_URGENCY_U2;
    event.fragment_index = 0u;
    event.fragment_total = 1u;
    return event;
}

int main(void)
{
    score_t score = { 0, 0 };
    hl_profile_t lra = profile_fixture(HL_ACTUATOR_LRA);
    hl_profile_t erm = profile_fixture(HL_ACTUATOR_ERM);
    hl_event_t event = event_fixture();
    hl_event_t decoded;
    hl_encoded_t encoded;
    hl_encoded_t corrupted;
    int result;

    check(&score, hl_profile_validate(&lra) == HL_OK &&
                    hl_profile_validate(&erm) == HL_OK,
          "valid LRA and ERM profiles are accepted");
    check(&score, hl_encode(&event, &lra, &encoded) == HL_OK &&
                    encoded.slot_count == 6u && encoded.effect_id[0] == 1u &&
                    encoded.effect_id[5] == 3u,
          "semantic event fits six slots with explicit sync/end");
    result = hl_decode(&encoded, &decoded);
    check(&score, result == HL_OK && memcmp(&event, &decoded, sizeof(event)) == 0,
          "encode/decode round-trip preserves semantic fields");
    check(&score, encoded.checksum == hl_checksum(&event),
          "checksum is deterministic and independent of actuator profile");

    corrupted = encoded;
    corrupted.checksum ^= 0x01u;
    check(&score, hl_decode(&corrupted, &decoded) == HL_E_CHECKSUM,
          "checksum corruption abstains instead of selecting a frame");
    corrupted = encoded;
    corrupted.kind[0] = HL_SYM_MARK;
    check(&score, hl_decode(&corrupted, &decoded) == HL_E_FORMAT,
          "missing sync delimiter rejects the frame");
    corrupted = encoded;
    corrupted.kind[2] = HL_SYM_ABSTAIN;
    check(&score, hl_decode(&corrupted, &decoded) == HL_E_UNKNOWN,
          "unknown physical token does not become semantic success");

    event.has_data = 1u;
    check(&score, hl_encode(&event, &lra, &encoded) == HL_E_FRAGMENT,
          "data exceeding v0.1 frame budget requires fragmentation");
    event = event_fixture();
    event.fragment_total = 2u;
    check(&score, hl_encode(&event, &lra, &encoded) == HL_E_FRAGMENT,
          "multi-fragment event is not silently truncated");
    event = event_fixture();
    check(&score, hl_encode(&event, NULL, &encoded) == HL_E_PROFILE,
          "missing actuator profile fails closed");
    lra.effect_code[HL_CODEBOOK_SIZE - 1u] = 0u;
    check(&score, hl_encode(&event, &lra, &encoded) == HL_E_PROFILE,
          "incomplete effect codebook is rejected");
    event.version = 2u;
    check(&score, hl_encode(&event, &erm, &encoded) == HL_E_FORMAT,
          "unknown semantic language version is rejected");

    printf("HAPTIC LANGUAGE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
