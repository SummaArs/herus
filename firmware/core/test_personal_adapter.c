#include "personal_adapter.h"

#include <stdio.h>

static void check(int *pass, int *fail, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) (*pass)++; else (*fail)++;
}

static pa_sample_t sample(uint32_t feature, pa_style_t style)
{
    pa_sample_t value = { feature, style, 1u, 100u };
    return value;
}

int main(void)
{
    pa_profile_t profile;
    pa_prediction_t prediction;
    int pass = 0;
    int fail = 0;
    pa_sample_t concise = sample(7u, PA_STYLE_CONCISE);
    pa_sample_t detailed = sample(7u, PA_STYLE_DETAILED);

    pa_init(&profile);
    check(&pass, &fail, profile.epoch == 1u && profile.entry_count == 0u,
          "profile starts empty with a local epoch");
    check(&pass, &fail, pa_update(&profile, &concise, 0u) == PA_E_AUTH &&
                    profile.accepted_updates == 0u,
          "without explicit consent no preference update is accepted");
    concise.local_origin = 0u;
    check(&pass, &fail, pa_update(&profile, &concise, 1u) == PA_E_AUTH,
          "external-origin learning cannot self-promote into personal state");
    concise.local_origin = 1u;
    check(&pass, &fail, pa_update(&profile, &concise, 1u) == PA_OK &&
                    pa_update(&profile, &concise, 1u) == PA_OK,
          "two consented local samples create a bounded preference signal");
    check(&pass, &fail, pa_predict(&profile, 7u, &prediction) == PA_OK &&
                    prediction.style == PA_STYLE_CONCISE &&
                    prediction.confidence_pct == 100u && prediction.abstain == 0u,
          "a stable local preference becomes a typed prediction");

    check(&pass, &fail, pa_update(&profile, &detailed, 1u) == PA_OK,
          "a competing sample is accepted as evidence, not an overwrite");
    check(&pass, &fail, pa_predict(&profile, 7u, &prediction) == PA_OK &&
                    prediction.style == PA_STYLE_CONCISE &&
                    prediction.margin_votes == 1u,
          "a one-vote margin remains bounded and explicit");
    check(&pass, &fail, pa_update(&profile, &detailed, 1u) == PA_OK &&
                    pa_predict(&profile, 7u, &prediction) == PA_ABSTAIN &&
                    prediction.abstain == 1u,
          "a tied preference abstains instead of choosing silently");

    check(&pass, &fail, pa_forget(&profile, 7u) == PA_OK &&
                    pa_predict(&profile, 7u, &prediction) == PA_ABSTAIN &&
                    prediction.abstain == 1u,
          "forget tombstones the learned preference and blocks prediction");
    concise.preferred_style = PA_STYLE_CONCISE;
    check(&pass, &fail, pa_update(&profile, &concise, 1u) == PA_E_REVOKED,
          "a revoked feature cannot be reintroduced by a later sample");

    pa_init(&profile);
    check(&pass, &fail, pa_update(&profile, &concise, 1u) == PA_OK &&
                    pa_reboot_quarantine(&profile) == PA_OK &&
                    profile.epoch == 2u && pa_active_entries(&profile) == 0u &&
                    pa_predict(&profile, 7u, &prediction) == PA_ABSTAIN,
          "reboot quarantines active adaptation until a later policy restores it");

    pa_init(&profile);
    {
        pa_sample_t detailed = sample(9u, PA_STYLE_DETAILED);
        int capacity_ok = 1;
        uint32_t feature;
        for (feature = 1u; feature <= PA_MAX_FEATURES; ++feature) {
            detailed.feature_id = feature;
            if (pa_update(&profile, &detailed, 1u) != PA_OK) capacity_ok = 0;
        }
        detailed.feature_id = PA_MAX_FEATURES + 1u;
        check(&pass, &fail, capacity_ok &&
                        pa_update(&profile, &detailed, 1u) == PA_E_FULL,
              "bounded profile capacity rejects growth without eviction");
    }

    printf("PERSONAL ADAPTER: %d pass, %d fail\n", pass, fail);
    return fail == 0 ? 0 : 1;
}
