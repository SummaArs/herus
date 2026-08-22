/* intent_router.c — bounded, local and fail-closed text intent routing. */
#include "intent_router.h"

#include <ctype.h>
#include <string.h>

static void clear_result(intent_router_result_t *out)
{
    (void)memset(out, 0, sizeof(*out));
    out->intent = INTENT_ROUTER_UNKNOWN;
    out->abstain = 1u;
}

static int contains(const char *text, const char *needle)
{
    return strstr(text, needle) != NULL;
}

static void lower_ascii(const char *input, size_t length, char *output)
{
    size_t i;
    for (i = 0u; i < length; ++i) {
        unsigned char c = (unsigned char)input[i];
        output[i] = (char)tolower(c);
    }
    output[length] = '\0';
}

static void set_intent(intent_router_result_t *out,
                       intent_router_kind_t intent,
                       uint8_t confidence,
                       uint8_t margin,
                       uint8_t abstain)
{
    out->intent = intent;
    out->confidence_pct = confidence;
    out->margin_pct = margin;
    out->abstain = abstain;
    out->requires_confirmation =
        (intent == INTENT_ROUTER_ACTION_REQUEST ||
         intent == INTENT_ROUTER_FORGET_MEMORY ||
         intent == INTENT_ROUTER_SHARE_MEMORY) ? 1u : 0u;
}

static int contains_any(const char *text, const char *const *needles, size_t count)
{
    size_t i;
    for (i = 0u; i < count; ++i) {
        if (contains(text, needles[i])) {
            return 1;
        }
    }
    return 0;
}

static intent_router_kind_t classify(const char *text)
{
    static const char *const conflict[] = {
        "afinal", "diverg", "duas mem", "duas anot", "contradit"
    };
    static const char *const capture[] = {
        "salvar este ponto", "salve este ponto", "registre",
        "anote esta", "guarde que"
    };
    static const char *const recall[] = {
        "me diga", "o que ficou registrado", "lembra", "qual horario",
        "qual horário", "qual foi", "consultar depois"
    };
    static const char *const chat[] = {
        "fique comigo", "quero conversar", "como voce esta",
        "como você está", "me acompanhe"
    };
    static const char *const unknown[] = {
        "senha", "codigo secreto", "código secreto", "nunca te contei",
        "nao esta na memoria", "não está na memória", "não te contei"
    };
    static const char *const forget[] = {
        "esqueça", "esqueca", "apague", "revogue", "retire da memória",
        "retire da memoria"
    };
    static const char *const share[] = {
        "compartilhe", "compartilhar", "outro dispositivo", "meu contato",
        "ao contato"
    };
    static const char *const action[] = {
        "faça a compra", "faca a compra", "abra o portão", "abra o portao",
        "ligue para", "envie uma mensagem", "execute", "aja agora"
    };
    static const char *const preference[] = {
        "prefiro", "passe a", "gosto de", "aprenda que"
    };

    if (contains_any(text, conflict, sizeof(conflict) / sizeof(conflict[0]))) {
        return INTENT_ROUTER_CONFLICT_QUERY;
    }
    if (contains_any(text, capture, sizeof(capture) / sizeof(capture[0]))) {
        return INTENT_ROUTER_CAPTURE_MEMORY;
    }
    if (contains_any(text, recall, sizeof(recall) / sizeof(recall[0]))) {
        return INTENT_ROUTER_RECALL_MEMORY;
    }
    if (contains_any(text, chat, sizeof(chat) / sizeof(chat[0]))) {
        return INTENT_ROUTER_CHITCHAT;
    }
    if (contains_any(text, unknown, sizeof(unknown) / sizeof(unknown[0]))) {
        return INTENT_ROUTER_UNKNOWN;
    }
    if (contains_any(text, forget, sizeof(forget) / sizeof(forget[0]))) {
        return INTENT_ROUTER_FORGET_MEMORY;
    }
    if (contains_any(text, share, sizeof(share) / sizeof(share[0]))) {
        return INTENT_ROUTER_SHARE_MEMORY;
    }
    if (contains_any(text, action, sizeof(action) / sizeof(action[0]))) {
        return INTENT_ROUTER_ACTION_REQUEST;
    }
    if (contains_any(text, preference, sizeof(preference) / sizeof(preference[0]))) {
        return INTENT_ROUTER_UPDATE_PREFERENCE;
    }
    return INTENT_ROUTER_UNKNOWN;
}

static int has_word(const char *text, const char *word)
{
    return contains(text, word);
}

static void add_evidence(intent_router_result_t *out,
                         const intent_router_memory_t *memory)
{
    if (out->evidence_count >= INTENT_ROUTER_MAX_EVIDENCE) {
        return;
    }
    out->evidence[out->evidence_count].memory_id = memory->memory_id;
    out->evidence[out->evidence_count].generation = memory->generation;
    out->evidence[out->evidence_count].origin_local = memory->origin_local;
    out->evidence_count = (uint8_t)(out->evidence_count + 1u);
}

static void recover_evidence(const char *text,
                             intent_router_kind_t intent,
                             const intent_router_memory_t *memories,
                             size_t memory_count,
                             intent_router_result_t *out)
{
    size_t i;
    const intent_router_memory_t *best = NULL;
    const intent_router_memory_t *second = NULL;
    int old_requested = has_word(text, "antigo") ||
                        has_word(text, "antiga") ||
                        has_word(text, "anterior") ||
                        has_word(text, "ontem");

    if (intent == INTENT_ROUTER_CONFLICT_QUERY) {
        for (i = 0u; i < memory_count; ++i) {
            if (!memories[i].active ||
                memories[i].purpose != INTENT_ROUTER_MEMORY_SCHEDULE) {
                continue;
            }
            if (best == NULL) {
                best = &memories[i];
            } else if (second == NULL) {
                second = &memories[i];
            }
        }
        if (best != NULL) {
            add_evidence(out, best);
        }
        if (second != NULL) {
            add_evidence(out, second);
        }
        out->abstain = 1u;
        return;
    }

    for (i = 0u; i < memory_count; ++i) {
        const intent_router_memory_t *candidate = &memories[i];
        int matches = 0;
        if (!candidate->active) {
            continue;
        }
        if (intent == INTENT_ROUTER_RECALL_MEMORY ||
            intent == INTENT_ROUTER_FORGET_MEMORY) {
            matches = candidate->purpose == INTENT_ROUTER_MEMORY_SCHEDULE;
            if (intent == INTENT_ROUTER_FORGET_MEMORY && old_requested) {
                matches = matches && candidate->superseded;
            }
        } else if (intent == INTENT_ROUTER_UPDATE_PREFERENCE) {
            matches = candidate->purpose == INTENT_ROUTER_MEMORY_PREFERENCE;
            if (matches && has_word(text, "discret")) {
                matches = candidate->memory_id != INTENT_ROUTER_MEMORY_ID_PREF_CONCISE;
            }
        } else if (intent == INTENT_ROUTER_SHARE_MEMORY) {
            matches = candidate->purpose == INTENT_ROUTER_MEMORY_PROJECT;
        }
        if (!matches) {
            continue;
        }
        if (best == NULL) {
            best = candidate;
        } else if (intent == INTENT_ROUTER_RECALL_MEMORY &&
                   candidate->generation > best->generation) {
            best = candidate;
        } else if (intent == INTENT_ROUTER_FORGET_MEMORY &&
                   candidate->superseded && !best->superseded) {
            best = candidate;
        }
    }

    if (best == NULL) {
        if (intent == INTENT_ROUTER_RECALL_MEMORY ||
            intent == INTENT_ROUTER_FORGET_MEMORY ||
            intent == INTENT_ROUTER_UPDATE_PREFERENCE ||
            intent == INTENT_ROUTER_SHARE_MEMORY) {
            out->abstain = 1u;
        }
        return;
    }
    add_evidence(out, best);
}

intent_router_status_t intent_router_route(
    const char *text,
    size_t length,
    const intent_router_memory_t *memories,
    size_t memory_count,
    intent_router_result_t *out)
{
    char lower[INTENT_ROUTER_TEXT_MAX + 1u];
    intent_router_kind_t intent;

    if (text == NULL || out == NULL) {
        return INTENT_ROUTER_E_INPUT;
    }
    if (length > INTENT_ROUTER_TEXT_MAX ||
        (memory_count > 0u && memories == NULL)) {
        return INTENT_ROUTER_E_BOUNDS;
    }
    clear_result(out);
    lower_ascii(text, length, lower);
    intent = classify(lower);

    if (intent == INTENT_ROUTER_UNKNOWN || intent == INTENT_ROUTER_CONFLICT_QUERY) {
        set_intent(out, intent, INTENT_ROUTER_CONF_STRONG, INTENT_ROUTER_CONF_STRONG, 1u);
    } else if (intent == INTENT_ROUTER_CHITCHAT) {
        set_intent(out, intent, INTENT_ROUTER_CONF_CHAT, 70u, 0u);
    } else {
        set_intent(out, intent, INTENT_ROUTER_CONF_STRONG, INTENT_ROUTER_CONF_STRONG, 0u);
    }

    if (intent == INTENT_ROUTER_ACTION_REQUEST ||
        intent == INTENT_ROUTER_CAPTURE_MEMORY ||
        intent == INTENT_ROUTER_CHITCHAT ||
        intent == INTENT_ROUTER_UNKNOWN) {
        return INTENT_ROUTER_OK;
    }

    recover_evidence(lower, intent, memories, memory_count, out);
    if (out->evidence_count == 0u &&
        (intent == INTENT_ROUTER_RECALL_MEMORY ||
         intent == INTENT_ROUTER_FORGET_MEMORY ||
         intent == INTENT_ROUTER_UPDATE_PREFERENCE ||
         intent == INTENT_ROUTER_SHARE_MEMORY)) {
        out->abstain = 1u;
    }
    return INTENT_ROUTER_OK;
}
