/* memory_extract.c — transient, controlled candidate extraction. */
#include "memory_extract.h"
#include <string.h>

static int ascii_lower(int c)
{
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

static int contains_ascii_ci(const char *text, size_t len, const char *needle)
{
    size_t i;
    size_t j;
    size_t n = strlen(needle);
    if (n == 0u || n > len) return 0;
    for (i = 0u; i + n <= len; ++i) {
        for (j = 0u; j < n; ++j) {
            if (ascii_lower((unsigned char)text[i + j]) !=
                ascii_lower((unsigned char)needle[j])) break;
        }
        if (j == n) return 1;
    }
    return 0;
}

static int has_any(const char *text, size_t len, const char *const *terms, size_t count)
{
    size_t i;
    for (i = 0u; i < count; ++i)
        if (contains_ascii_ci(text, len, terms[i])) return 1;
    return 0;
}

static void clear_candidate(memory_candidate_t *out)
{
    memset(out, 0, sizeof(*out));
}

static void set_kind_defaults(memory_signal_t *s, memory_kind_t kind)
{
    s->kind = kind;
    switch (kind) {
    case MEMORY_KIND_IDEA:
        s->novelty_pct = 75u; s->future_value_pct = 90u; s->consequence_pct = 45u;
        break;
    case MEMORY_KIND_DECISION:
        s->novelty_pct = 70u; s->future_value_pct = 90u; s->consequence_pct = 70u;
        break;
    case MEMORY_KIND_COMMITMENT:
        s->novelty_pct = 60u; s->future_value_pct = 85u; s->consequence_pct = 85u;
        break;
    case MEMORY_KIND_PREFERENCE:
        s->novelty_pct = 50u; s->future_value_pct = 65u; s->consequence_pct = 35u;
        break;
    case MEMORY_KIND_PROJECT_FACT:
        s->novelty_pct = 50u; s->future_value_pct = 75u; s->consequence_pct = 50u;
        break;
    case MEMORY_KIND_ROUTINE:
        s->novelty_pct = 55u; s->future_value_pct = 70u; s->consequence_pct = 30u;
        break;
    default:
        break;
    }
}

static memory_kind_t infer_kind(const char *text, size_t len, uint32_t *reasons,
                                memory_extract_origin_t *origin)
{
    static const char *const explicit_terms[] = { "lembre", "guarde", "anote" };
    static const char *const idea_terms[] = { "ideia", "pensei", "imagino" };
    static const char *const decision_terms[] = { "decidimos", "decidi", "vamos" };
    static const char *const commitment_terms[] = { "eu vou", "me comprometo", "combino" };
    static const char *const preference_terms[] = { "prefiro", "nao gosto", "gosto de" };
    static const char *const project_terms[] = { "herus", "nucleo", "projeto" };
    static const char *const routine_terms[] = { "costumo", "sempre levo", "minha rotina" };

    if (has_any(text, len, explicit_terms, sizeof(explicit_terms) / sizeof(explicit_terms[0]))) {
        *reasons |= MEMORY_EXTRACT_REASON_EXPLICIT;
        *origin = MEMORY_EXTRACT_EXPLICIT;
    }
    if (has_any(text, len, decision_terms, sizeof(decision_terms) / sizeof(decision_terms[0]))) {
        *reasons |= MEMORY_EXTRACT_REASON_DECISION;
        if (*origin == MEMORY_EXTRACT_ORIGIN_NONE)
            *origin = MEMORY_EXTRACT_CONTROLLED_INFERENCE;
        return MEMORY_KIND_DECISION;
    }
    if (has_any(text, len, commitment_terms, sizeof(commitment_terms) / sizeof(commitment_terms[0]))) {
        *reasons |= MEMORY_EXTRACT_REASON_COMMITMENT;
        if (*origin == MEMORY_EXTRACT_ORIGIN_NONE)
            *origin = MEMORY_EXTRACT_CONTROLLED_INFERENCE;
        return MEMORY_KIND_COMMITMENT;
    }
    if (has_any(text, len, idea_terms, sizeof(idea_terms) / sizeof(idea_terms[0]))) {
        *reasons |= MEMORY_EXTRACT_REASON_IDEA;
        if (*origin == MEMORY_EXTRACT_ORIGIN_NONE)
            *origin = MEMORY_EXTRACT_CONTROLLED_INFERENCE;
        return MEMORY_KIND_IDEA;
    }
    if (has_any(text, len, preference_terms, sizeof(preference_terms) / sizeof(preference_terms[0]))) {
        *reasons |= MEMORY_EXTRACT_REASON_PREFERENCE;
        if (*origin == MEMORY_EXTRACT_ORIGIN_NONE)
            *origin = MEMORY_EXTRACT_CONTROLLED_INFERENCE;
        return MEMORY_KIND_PREFERENCE;
    }
    if (has_any(text, len, project_terms, sizeof(project_terms) / sizeof(project_terms[0]))) {
        *reasons |= MEMORY_EXTRACT_REASON_PROJECT;
        if (*origin == MEMORY_EXTRACT_ORIGIN_NONE)
            *origin = MEMORY_EXTRACT_CONTROLLED_INFERENCE;
        return MEMORY_KIND_PROJECT_FACT;
    }
    if (has_any(text, len, routine_terms, sizeof(routine_terms) / sizeof(routine_terms[0]))) {
        *reasons |= MEMORY_EXTRACT_REASON_ROUTINE;
        if (*origin == MEMORY_EXTRACT_ORIGIN_NONE)
            *origin = MEMORY_EXTRACT_CONTROLLED_INFERENCE;
        return MEMORY_KIND_ROUTINE;
    }
    *reasons |= MEMORY_EXTRACT_REASON_UNRECOGNIZED;
    return MEMORY_KIND_NONE;
}

static void classify_scope_and_sensitivity(memory_signal_t *s, const char *text,
                                           size_t len, uint32_t *reasons)
{
    static const char *const other_terms[] = {
        "ele ", "ela ", "outra pessoa", "meu amigo", "minha amiga", "minha mae", "meu pai"
    };
    static const char *const self_terms[] = { " eu ", " meu ", " minha ", " comigo" };
    static const char *const sensitive_terms[] = {
        "medico", "saude", "financeiro", "juridico", "senha", "endereco", "localizacao", "intimo"
    };
    int other = has_any(text, len, other_terms, sizeof(other_terms) / sizeof(other_terms[0]));
    int self = has_any(text, len, self_terms, sizeof(self_terms) / sizeof(self_terms[0]));

    s->scope = MEMORY_SCOPE_SELF;
    s->sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    if (other) {
        s->scope = self ? MEMORY_SCOPE_MIXED : MEMORY_SCOPE_THIRD_PARTY;
        *reasons |= MEMORY_EXTRACT_REASON_THIRD_PARTY;
    }
    if (has_any(text, len, sensitive_terms,
                sizeof(sensitive_terms) / sizeof(sensitive_terms[0]))) {
        s->sensitivity = MEMORY_SENSITIVITY_SENSITIVE;
        *reasons |= MEMORY_EXTRACT_REASON_SENSITIVE;
    }
}

void memory_extract_init(memory_extract_t *e)
{
    if (e) memset(e, 0, sizeof(*e));
}

int memory_extract_text(memory_extract_t *e, const memory_capture_t *capture,
                        uint32_t capture_session_id, const char *text, size_t len,
                        uint8_t asr_confidence_pct, memory_candidate_t *out)
{
    memory_kind_t kind;
    uint32_t reasons = MEMORY_EXTRACT_REASON_NONE;
    memory_extract_origin_t origin = MEMORY_EXTRACT_ORIGIN_NONE;

    if (!e || !capture || !out) return MEMORY_EXTRACT_E_ARG;
    clear_candidate(out);
    e->metrics.calls++;
    if (!text || len == 0u || asr_confidence_pct > 100u) {
        e->metrics.rejected_input++;
        return MEMORY_EXTRACT_E_INPUT;
    }
    if (len > MEMORY_EXTRACT_TEXT_MAX) {
        e->metrics.rejected_input++;
        return MEMORY_EXTRACT_E_LENGTH;
    }
    if (capture_session_id == 0u ||
        capture_session_id != memory_capture_session_id(capture)) {
        e->metrics.rejected_session++;
        return MEMORY_EXTRACT_E_SESSION;
    }

    kind = infer_kind(text, len, &reasons, &origin);
    if (kind == MEMORY_KIND_NONE) {
        e->metrics.no_candidate++;
        return MEMORY_EXTRACT_NO_CANDIDATE;
    }

    out->origin = origin;
    out->reasons = reasons;
    out->signal.session_authorized = 1u;
    out->signal.explicit_remember = origin == MEMORY_EXTRACT_EXPLICIT ? 1u : 0u;
    out->signal.confidence_pct = asr_confidence_pct;
    set_kind_defaults(&out->signal, kind);
    classify_scope_and_sensitivity(&out->signal, text, len, &out->reasons);

    if (asr_confidence_pct < MEMORY_POLICY_MIN_CONFIDENCE_PCT) {
        out->reasons |= MEMORY_EXTRACT_REASON_AMBIGUOUS;
        e->metrics.low_confidence++;
    }
    if (out->signal.scope != MEMORY_SCOPE_SELF ||
        out->signal.sensitivity != MEMORY_SENSITIVITY_ORDINARY)
        e->metrics.sensitive_or_other++;
    e->metrics.candidates++;
    return MEMORY_EXTRACT_OK;
}

int memory_extract_assess(const memory_candidate_t *candidate,
                          memory_assessment_t *assessment)
{
    if (!candidate || !assessment) return MEMORY_EXTRACT_E_ARG;
    if (candidate->origin <= MEMORY_EXTRACT_ORIGIN_NONE ||
        candidate->origin >= MEMORY_EXTRACT_ORIGIN_COUNT ||
        candidate->signal.kind <= MEMORY_KIND_NONE ||
        candidate->signal.kind >= MEMORY_KIND_COUNT)
        return MEMORY_EXTRACT_E_INPUT;
    return memory_policy_assess(&candidate->signal, assessment);
}

const memory_extract_metrics_t *memory_extract_metrics(const memory_extract_t *e)
{
    return e ? &e->metrics : 0;
}
