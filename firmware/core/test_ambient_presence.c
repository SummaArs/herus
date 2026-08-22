#include "ambient_presence.h"
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s\n", (message)); \
        failures++; \
    } else { \
        printf("PASS %s\n", (message)); \
    } \
} while (0)

static ap_observation_t good_observation(uint32_t generation)
{
    ap_observation_t observation;
    memset(&observation, 0, sizeof(observation));
    observation.opportunity_class = AP_CLASS_RECALL;
    observation.privacy_class = AP_PRIVACY_PERSONAL;
    observation.attention_available = 1u;
    observation.proactive_consent = 1u;
    observation.confidence_pct = 95u;
    observation.relevance_pct = 90u;
    observation.novelty_pct = 80u;
    observation.risk_pct = 5u;
    observation.now_generation = generation;
    observation.valid_until_generation = generation + 3u;
    observation.cooldown_generations = 3u;
    return observation;
}

static void test_quiet_and_abstention(void)
{
    ap_presence_t presence;
    ap_observation_t observation = good_observation(1u);

    ap_init(&presence);
    observation.attention_available = 0u;
    CHECK(ap_observe(&presence, &observation) == AP_NO_OFFER &&
              presence.status == AP_QUIET && !presence.candidate_valid,
          "sem atenção disponível, a presença permanece silenciosa");

    observation = good_observation(2u);
    observation.proactive_consent = 0u;
    CHECK(ap_observe(&presence, &observation) == AP_NO_OFFER &&
              presence.status == AP_QUIET &&
              (presence.reason & AP_REASON_NO_CONSENT) != 0u,
          "sem consentimento proativo, não há oferta contextual");

    observation = good_observation(3u);
    observation.confidence_pct = 79u;
    CHECK(ap_observe(&presence, &observation) == AP_NO_OFFER &&
              presence.status == AP_ABSTAIN &&
              (presence.reason & AP_REASON_LOW_CONFIDENCE) != 0u,
          "baixa confiança produz abstention, não iniciativa");
}

static void test_privacy_and_format(void)
{
    ap_presence_t presence;
    ap_observation_t observation = good_observation(1u);

    ap_init(&presence);
    observation.privacy_class = AP_PRIVACY_SENSITIVE;
    CHECK(ap_observe(&presence, &observation) == AP_NO_OFFER &&
              presence.status == AP_ABSTAIN && !presence.candidate_valid &&
              (presence.reason & AP_REASON_SENSITIVE) != 0u,
          "contexto sensível é bloqueado antes de virar candidato");

    observation = good_observation(2u);
    observation.valid_until_generation = 1u;
    CHECK(ap_observe(&presence, &observation) == AP_NO_OFFER &&
              presence.status == AP_ABSTAIN &&
              (presence.reason & AP_REASON_BAD_FORMAT) != 0u,
          "janela temporal inválida é rejeitada");
}

static void test_one_offer_and_contact(void)
{
    ap_presence_t presence;
    ap_observation_t observation = good_observation(10u);
    ap_offer_t offer;

    ap_init(&presence);
    CHECK(ap_observe(&presence, &observation) == AP_OK &&
              presence.status == AP_HOLD && presence.candidate_valid,
          "observação qualificada fica latente em HOLD");
    CHECK(ap_offer(&presence, 10u, &offer) == AP_OK &&
              presence.status == AP_OFFER && presence.offered == 1u &&
              offer.requires_physical_contact == 1u,
          "o candidato produz no máximo uma microoferta local");
    CHECK(ap_offer(&presence, 10u, &offer) == AP_NO_OFFER &&
              presence.status == AP_OFFER &&
              (presence.reason & AP_REASON_BUDGET) != 0u,
          "a mesma oportunidade não é repetida nem perde seu estado");
    CHECK(ap_acknowledge(&presence, 0u) == AP_E_CONTACT &&
              presence.status == AP_OFFER,
          "ausência de contato não confirma nem remove a oferta");
    CHECK(ap_acknowledge(&presence, 1u) == AP_OK &&
              presence.status == AP_ACKNOWLEDGED && !presence.candidate_valid,
          "contato físico posterior reconhece somente o recebimento");
}

static void test_expiry_and_cooldown(void)
{
    ap_presence_t presence;
    ap_observation_t observation = good_observation(20u);
    ap_offer_t offer;

    ap_init(&presence);
    CHECK(ap_observe(&presence, &observation) == AP_OK,
          "cenário válido entra no pipeline transitório");
    CHECK(ap_tick(&presence, 24u) == AP_NO_OFFER &&
              presence.status == AP_EXPIRED && !presence.candidate_valid,
          "candidato expirado é apagado sem nova tentativa");

    observation = good_observation(30u);
    CHECK(ap_observe(&presence, &observation) == AP_OK &&
              ap_offer(&presence, 30u, &offer) == AP_OK &&
              ap_acknowledge(&presence, 1u) == AP_OK &&
              presence.status == AP_ACKNOWLEDGED,
          "contato físico reconhece recebimento, não uma ação");

    observation = good_observation(31u);
    CHECK(ap_observe(&presence, &observation) == AP_NO_OFFER &&
              presence.status == AP_QUIET &&
              (presence.reason & AP_REASON_COOLDOWN) != 0u,
          "cooldown impede reoferta imediata");
    observation = good_observation(32u);
    CHECK(ap_observe(&presence, &observation) == AP_NO_OFFER &&
              presence.status == AP_QUIET,
          "cooldown permanece fechado antes do limite");
    observation = good_observation(33u);
    CHECK(ap_observe(&presence, &observation) == AP_OK &&
              ap_offer(&presence, 33u, &offer) == AP_OK,
          "nova oportunidade nasce no limite seguro do cooldown");
}

static void test_monotonicity_and_forget(void)
{
    ap_presence_t presence;
    ap_observation_t observation = good_observation(40u);
    ap_offer_t offer;

    ap_init(&presence);
    CHECK(ap_observe(&presence, &observation) == AP_OK &&
              ap_offer(&presence, 40u, &offer) == AP_OK,
          "cenário monotônico oferece uma oportunidade válida");
    CHECK(ap_tick(&presence, 39u) == AP_E_FORMAT,
          "tempo regressivo não reabre nem reescreve o candidato");
    ap_forget(&presence);
    CHECK(presence.status == AP_QUIET && !presence.candidate_valid &&
              presence.offer_budget == 0u && presence.opportunity_class == 0u,
          "forget limpa o estado transitório da presença");
}

int main(void)
{
    test_quiet_and_abstention();
    test_privacy_and_format();
    test_one_offer_and_contact();
    test_expiry_and_cooldown();
    test_monotonicity_and_forget();
    if (failures != 0) {
        fprintf(stderr, "AMBIENT PRESENCE FAILED: %d failure(s)\n", failures);
        return 1;
    }
    puts("AMBIENT PRESENCE: properties hold");
    return 0;
}
