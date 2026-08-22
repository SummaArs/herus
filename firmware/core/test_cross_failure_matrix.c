#include "interaction.h"
#include "assurance.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static assurance_snapshot_t safe_core(void)
{
    assurance_snapshot_t snapshot;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.source = ASSURANCE_SOURCE_CORE;
    snapshot.physical_session_current = 1u;
    snapshot.intent_accepted = 1u;
    snapshot.physical_confirmation = 1u;
    snapshot.handoff_unused = 1u;
    return snapshot;
}

static assurance_snapshot_t safe_nucleus(void)
{
    assurance_snapshot_t snapshot = safe_core();
    snapshot.source = ASSURANCE_SOURCE_NUCLEUS;
    snapshot.trust_active = 1u;
    snapshot.control_link_authenticated = 1u;
    snapshot.control_link_fresh = 1u;
    return snapshot;
}

int main(void)
{
    score_t score = { 0, 0 };
    assurance_decision_t decision;

    {
        assurance_snapshot_t snapshot = safe_core();
        const uint32_t masks[] = {
            ASSURANCE_FAIL_PHYSICAL,
            ASSURANCE_FAIL_INTENT,
            ASSURANCE_FAIL_CONFIRM,
            ASSURANCE_FAIL_HANDOFF
        };
        const uint8_t *fields[] = {
            &snapshot.physical_session_current,
            &snapshot.intent_accepted,
            &snapshot.physical_confirmation,
            &snapshot.handoff_unused
        };
        for (unsigned i = 0u; i < sizeof(masks) / sizeof(masks[0]); i++) {
            assurance_snapshot_t hostile = snapshot;
            uint8_t *field = (uint8_t *)fields[i];
            (void)field;
            if (i == 0u) hostile.physical_session_current = 0u;
            if (i == 1u) hostile.intent_accepted = 0u;
            if (i == 2u) hostile.physical_confirmation = 0u;
            if (i == 3u) hostile.handoff_unused = 0u;
            check(&score, assurance_decide(&hostile, &decision) == ASSURANCE_E_BLOCKED &&
                            decision.handoff_permitted == 0u &&
                            (decision.failures & masks[i]) != 0u,
                  "each missing core authority precondition blocks the combined handoff");
        }
    }

    {
        const uint32_t masks[] = {
            ASSURANCE_FAIL_TRUST,
            ASSURANCE_FAIL_LINK_AUTH,
            ASSURANCE_FAIL_LINK_FRESH
        };
        for (unsigned i = 0u; i < sizeof(masks) / sizeof(masks[0]); i++) {
            assurance_snapshot_t hostile = safe_nucleus();
            assurance_snapshot_t *s = &hostile;
            if (i == 0u) s->trust_active = 0u;
            if (i == 1u) s->control_link_authenticated = 0u;
            if (i == 2u) s->control_link_fresh = 0u;
            check(&score, assurance_decide(s, &decision) == ASSURANCE_E_BLOCKED &&
                            decision.handoff_permitted == 0u &&
                            (decision.failures & masks[i]) != 0u,
                  "each missing Nucleus trust/link condition blocks cross-device handoff");
        }
        {
            assurance_snapshot_t revoked = safe_nucleus();
            revoked.trust_revoked = 1u;
            check(&score, assurance_decide(&revoked, &decision) == ASSURANCE_E_BLOCKED &&
                            (decision.failures & ASSURANCE_FAIL_REVOKED) != 0u &&
                            decision.handoff_permitted == 0u,
                  "revocation dominates a previously valid Nucleus transport state");
        }
    }

    {
        assurance_snapshot_t model = safe_core();
        model.local_model_enabled = 1u;
        check(&score, assurance_decide(&model, &decision) == ASSURANCE_E_BLOCKED &&
                        (decision.failures & ASSURANCE_FAIL_MODEL) != 0u &&
                        (decision.failures & ASSURANCE_FAIL_AGENCY) != 0u,
              "a local model without accepted display-only evidence cannot add handoff authority");
        model.local_model_accepted = 1u;
        model.model_reply_display_only = 1u;
        check(&score, assurance_decide(&model, &decision) == ASSURANCE_OK &&
                        decision.handoff_permitted == 1u,
              "accepted local-model evidence preserves, but does not enlarge, existing authority");
    }

    {
        interaction_t interaction;
        interaction_config_t config;
        const interaction_actions_t *actions;
        hcp_msg_t out;
        assurance_snapshot_t snapshot = safe_core();
        interaction_config_default(&config);
        config.allow_test_transcript = 1u;
        interaction_init(&interaction, &config);
        check(&score, interaction_push_to_talk(&interaction, 100u) == INTERACTION_OK &&
                        interaction_transcript(&interaction, "chego em vinte e cinco minutos", 600u) == INTERACTION_OK &&
                        interaction.state == INTERACTION_AWAIT_CONFIRM,
              "a real local draft enters the pending-confirmation state");
        interaction_set_asr_available(&interaction, 0, 650u);
        actions = interaction_actions(&interaction);
        check(&score, interaction.state == INTERACTION_LINK_LOST &&
                        actions->clear_draft == 1u &&
                        interaction.pending.intent == 0u &&
                        interaction_confirm(&interaction, 1, 700u) == INTERACTION_E_STATE,
              "source loss while confirmation is pending scrubs the draft and cannot confirm it later");
        check(&score, interaction_take_send_assured(&interaction, &snapshot, &out) == INTERACTION_E_STATE &&
                        interaction_metrics(&interaction)->sent == 0u,
              "a safe assurance snapshot cannot revive a runtime handoff after source loss");
    }

    {
        interaction_t interaction;
        interaction_config_t config;
        assurance_snapshot_t revoked = safe_core();
        hcp_msg_t out;
        interaction_config_default(&config);
        config.allow_test_transcript = 1u;
        interaction_init(&interaction, &config);
        interaction_push_to_talk(&interaction, 1000u);
        interaction_transcript(&interaction, "chego em vinte e cinco minutos", 1100u);
        interaction_confirm(&interaction, 1, 1150u);
        revoked.trust_revoked = 1u;
        check(&score, interaction_take_send_assured(&interaction, &revoked, &out) == INTERACTION_E_UNTRUSTED &&
                        interaction.state == INTERACTION_READY_SEND &&
                        interaction_metrics(&interaction)->sent == 0u,
              "revoked assurance blocks a ready handoff without consuming the one-shot draft");
        check(&score, interaction_take_send(&interaction, &out) == INTERACTION_OK &&
                        interaction.state == INTERACTION_IDLE,
              "only the ordinary local path can consume the already-confirmed draft after the failed assured attempt");
    }

    printf("CROSS FAILURE MATRIX: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
