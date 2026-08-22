#include "intent_router.h"

#include <stdio.h>
#include <string.h>

static const intent_router_memory_t memories[] = {
    { INTENT_ROUTER_MEMORY_ID_MEETING_OLD, INTENT_ROUTER_MEMORY_SCHEDULE, 1u, 1u, 1u, 1u },
    { INTENT_ROUTER_MEMORY_ID_MEETING_NEW, INTENT_ROUTER_MEMORY_SCHEDULE, 2u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PREF_CONCISE, INTENT_ROUTER_MEMORY_PREFERENCE, 1u, 1u, 0u, 1u },
    { 4u, INTENT_ROUTER_MEMORY_PREFERENCE, 1u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PROJECT, INTENT_ROUTER_MEMORY_PROJECT, 1u, 1u, 0u, 1u }
};

static int expect(const char *name,
                  const char *text,
                  intent_router_kind_t intent,
                  uint8_t abstain,
                  uint8_t confirmation,
                  uint8_t evidence_count,
                  uint32_t first_memory_id)
{
    intent_router_result_t result;
    intent_router_status_t status = intent_router_route(
        text, strlen(text), memories,
        sizeof(memories) / sizeof(memories[0]), &result);
    if (status != INTENT_ROUTER_OK ||
        result.intent != intent ||
        result.abstain != abstain ||
        result.requires_confirmation != confirmation ||
        result.evidence_count != evidence_count ||
        (evidence_count > 0u && result.evidence[0].memory_id != first_memory_id)) {
        printf("FAIL %s status=%d intent=%d abstain=%u confirm=%u evidence=%u first=%u\n",
               name, (int)status, (int)result.intent, result.abstain,
               result.requires_confirmation, result.evidence_count,
               evidence_count > 0u ? result.evidence[0].memory_id : 0u);
        return 0;
    }
    printf("PASS %s\n", name);
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 0;
    intent_router_result_t result;
    char too_long[INTENT_ROUTER_TEXT_MAX + 2u];

    ++total; passed += expect("recall-current-generation",
        "Me diga o que ficou registrado sobre a reunião.",
        INTENT_ROUTER_RECALL_MEMORY, 0u, 0u, 1u,
        INTENT_ROUTER_MEMORY_ID_MEETING_NEW);
    ++total; passed += expect("capture-is-not-action",
        "Pode salvar este ponto para eu consultar depois?",
        INTENT_ROUTER_CAPTURE_MEMORY, 0u, 0u, 0u, 0u);
    ++total; passed += expect("unknown-abstains",
        "Você conhece o código secreto que nunca te contei?",
        INTENT_ROUTER_UNKNOWN, 1u, 0u, 0u, 0u);
    ++total; passed += expect("action-requires-confirmation",
        "Ligue para essa pessoa imediatamente.",
        INTENT_ROUTER_ACTION_REQUEST, 0u, 1u, 0u, 0u);
    ++total; passed += expect("forget-targets-predecessor",
        "Retire da memória o compromisso antigo.",
        INTENT_ROUTER_FORGET_MEMORY, 0u, 1u, 1u,
        INTENT_ROUTER_MEMORY_ID_MEETING_OLD);
    ++total; passed += expect("preference-recovers-typed-evidence",
        "Passe a responder sem rodeios.",
        INTENT_ROUTER_UPDATE_PREFERENCE, 0u, 0u, 1u,
        INTENT_ROUTER_MEMORY_ID_PREF_CONCISE);
    ++total; passed += expect("share-requires-confirmation",
        "Dê ao meu contato acesso a esse registro.",
        INTENT_ROUTER_SHARE_MEMORY, 0u, 1u, 1u,
        INTENT_ROUTER_MEMORY_ID_PROJECT);
    ++total; passed += expect("conflict-exposes-both-causes",
        "Qual horário devo considerar quando as anotações divergem?",
        INTENT_ROUTER_CONFLICT_QUERY, 1u, 0u, 2u,
        INTENT_ROUTER_MEMORY_ID_MEETING_OLD);
    ++total; passed += expect("chat-is-not-memory-or-action",
        "Fique comigo por um instante.",
        INTENT_ROUTER_CHITCHAT, 0u, 0u, 0u, 0u);

    ++total;
    if (intent_router_route(NULL, 0u, memories,
                            sizeof(memories) / sizeof(memories[0]), &result) == INTENT_ROUTER_E_INPUT) {
        printf("PASS null-input-rejected\n");
        ++passed;
    } else {
        printf("FAIL null-input-rejected\n");
    }

    (void)memset(too_long, 'x', sizeof(too_long));
    ++total;
    if (intent_router_route(too_long, sizeof(too_long), memories,
                            sizeof(memories) / sizeof(memories[0]), &result) == INTENT_ROUTER_E_BOUNDS) {
        printf("PASS oversized-input-rejected\n");
        ++passed;
    } else {
        printf("FAIL oversized-input-rejected\n");
    }

    printf("INTENT ROUTER C11: %d/%d cases pass\n", passed, total);
    return passed == total ? 0 : 1;
}
