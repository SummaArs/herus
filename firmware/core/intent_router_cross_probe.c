#include "intent_router.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const char *id;
    const char *text;
} probe_case_t;

static const intent_router_memory_t memories[] = {
    { INTENT_ROUTER_MEMORY_ID_MEETING_OLD, INTENT_ROUTER_MEMORY_SCHEDULE, 1u, 1u, 1u, 1u },
    { INTENT_ROUTER_MEMORY_ID_MEETING_NEW, INTENT_ROUTER_MEMORY_SCHEDULE, 2u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PREF_CONCISE, INTENT_ROUTER_MEMORY_PREFERENCE, 1u, 1u, 0u, 1u },
    { 4u, INTENT_ROUTER_MEMORY_PREFERENCE, 1u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PROJECT, INTENT_ROUTER_MEMORY_PROJECT, 1u, 1u, 0u, 1u }
};

static const probe_case_t cases[] = {
    { "test_recall_04", "Me diga o que ficou registrado sobre a reunião." },
    { "test_capture_04", "Pode salvar este ponto para eu consultar depois?" },
    { "test_unknown_04", "Você conhece o código secreto que nunca te contei?" },
    { "test_action_04", "Ligue para essa pessoa imediatamente." },
    { "test_forget_04", "Retire da memória o compromisso antigo." },
    { "test_preference_04", "Passe a responder sem rodeios." },
    { "test_share_04", "Dê ao meu contato acesso a esse registro." },
    { "test_conflict_04", "Qual horário devo considerar quando as anotações divergem?" },
    { "test_chitchat_04", "Fique comigo por um instante." }
};

int main(void)
{
    size_t i;
    for (i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        intent_router_result_t result;
        intent_router_status_t status = intent_router_route(
            cases[i].text, strlen(cases[i].text), memories,
            sizeof(memories) / sizeof(memories[0]), &result);
        if (status != INTENT_ROUTER_OK) {
            return 1;
        }
        printf("CASE %s %d %u %u %u", cases[i].id, (int)result.intent,
               result.abstain, result.requires_confirmation,
               result.evidence_count);
        for (uint8_t j = 0u; j < result.evidence_count; ++j) {
            printf(" %u", result.evidence[j].memory_id);
        }
        printf("\n");
    }
    return 0;
}
