#include "generative_core.h"

#include <string.h>

#define GC_FNV_OFFSET 2166136261u
#define GC_FNV_PRIME  16777619u

static void digest_byte(uint32_t *digest, uint8_t value)
{
    *digest ^= (uint32_t)value;
    *digest *= GC_FNV_PRIME;
}

static void digest_symbol(uint32_t *digest, sr_symbol_t symbol)
{
    digest_byte(digest, (uint8_t)(symbol & 0xffu));
    digest_byte(digest, (uint8_t)((symbol >> 8) & 0xffu));
    digest_byte(digest, (uint8_t)((symbol >> 16) & 0xffu));
    digest_byte(digest, (uint8_t)((symbol >> 24) & 0xffu));
}

static uint32_t fact_digest(const sr_fact_t *fact)
{
    uint32_t digest = GC_FNV_OFFSET;
    if (fact == NULL) return 0u;
    digest_symbol(&digest, fact->subject);
    digest_symbol(&digest, fact->predicate);
    digest_symbol(&digest, fact->object);
    digest_byte(&digest, fact->negated);
    return digest;
}

static int append_bytes(char *out, uint16_t *length, const char *text,
                        size_t text_length)
{
    if (out == NULL || length == NULL || text == NULL ||
        text_length > (size_t)(GC_MAX_RESPONSE_BYTES - 1u - *length)) {
        return 0;
    }
    memcpy(&out[*length], text, text_length);
    *length = (uint16_t)(*length + text_length);
    out[*length] = '\0';
    return 1;
}

static int append_u32(char *out, uint16_t *length, uint32_t value)
{
    char digits[10];
    uint8_t count = 0u;
    uint8_t i;
    if (value == 0u) {
        return append_bytes(out, length, "0", 1u);
    }
    while (value > 0u && count < sizeof(digits)) {
        digits[count++] = (char)('0' + (value % 10u));
        value /= 10u;
    }
    for (i = count; i > 0u; --i) {
        if (!append_bytes(out, length, &digits[i - 1u], 1u)) return 0;
    }
    return 1;
}

static const gc_lexeme_t *find_lexeme(const gc_lexicon_t *lexicon,
                                      sr_symbol_t symbol)
{
    size_t i;
    if (lexicon == NULL || lexicon->entries == NULL) return NULL;
    for (i = 0u; i < lexicon->count; ++i) {
        if (lexicon->entries[i].symbol == symbol) return &lexicon->entries[i];
    }
    return NULL;
}

static int append_symbol(const gc_lexicon_t *lexicon, sr_symbol_t symbol,
                         char *out, uint16_t *length, uint8_t *missing)
{
    const gc_lexeme_t *entry = find_lexeme(lexicon, symbol);
    if (entry == NULL || entry->text == NULL || entry->length == 0u ||
        entry->length > GC_MAX_LEXEM_BYTES) {
        if (missing != NULL) *missing = 1u;
        return 0;
    }
    return append_bytes(out, length, entry->text, entry->length);
}

static int append_fact(const gc_lexicon_t *lexicon, const sr_fact_t *fact,
                       char *out, uint16_t *length, uint8_t *missing)
{
    if (fact == NULL || out == NULL || length == NULL) return 0;
    if (fact->negated != 0u && !append_bytes(out, length, "nao ", 4u))
        return 0;
    if (!append_symbol(lexicon, fact->subject, out, length, missing)) return 0;
    if (!append_bytes(out, length, " ", 1u)) return 0;
    if (!append_symbol(lexicon, fact->predicate, out, length, missing)) return 0;
    if (!append_bytes(out, length, " ", 1u)) return 0;
    if (!append_symbol(lexicon, fact->object, out, length, missing)) return 0;
    return 1;
}

static int query_is_ground(const sr_pattern_t *query)
{
    return query != NULL && query->subject.kind == SR_TERM_CONSTANT &&
           query->predicate.kind == SR_TERM_CONSTANT &&
           query->object.kind == SR_TERM_CONSTANT;
}

static sr_fact_t pattern_to_fact(const sr_pattern_t *pattern)
{
    sr_fact_t fact = { 0u, 0u, 0u, 0u };
    if (pattern == NULL) return fact;
    fact.subject = pattern->subject.value;
    fact.predicate = pattern->predicate.value;
    fact.object = pattern->object.value;
    fact.negated = pattern->negated;
    return fact;
}

static void mark_abstention(gc_result_t *out, gc_kind_t kind,
                            gc_abstain_reason_t reason, gc_status_t status)
{
    out->status = status;
    out->kind = kind;
    out->abstain_reason = reason;
    out->authority = GC_AUTH_NONE;
    out->requires_confirmation = 0u;
}

static void copy_evidence(const sr_reasoner_t *reasoner,
                          const sr_answer_t *answer, gc_result_t *out)
{
    uint8_t i;
    if (reasoner == NULL || answer == NULL || out == NULL) return;
    for (i = 0u; i < answer->evidence_count &&
                out->evidence_count < GC_MAX_EVIDENCE_ROOTS; ++i) {
        uint8_t index = answer->evidence[i];
        if (index < reasoner->fact_count) {
            out->evidence[out->evidence_count++] = index;
            out->derivation_digest ^= fact_digest(&reasoner->facts[index]);
        }
    }
}

static gc_status_t render_answer(const gc_lexicon_t *lexicon,
                                 const sr_answer_t *answer,
                                 const gc_request_t *request,
                                 gc_result_t *out)
{
    uint8_t missing = 0u;
    uint16_t length = 0u;
    const char *prefix = request->mode == GC_MODE_EXPLAIN ? "porque " : "";
    if (request->personal_profile != NULL) {
        if (pa_predict(request->personal_profile,
                       GC_PERSONAL_FEATURE_RESPONSE_STYLE,
                       &out->adaptation) == PA_OK) {
            out->adapted = 1u;
            if (request->mode == GC_MODE_EXPLAIN &&
                out->adaptation.style == PA_STYLE_CONCISE) {
                prefix = "";
            } else if (request->mode == GC_MODE_EXPLAIN &&
                       out->adaptation.style == PA_STYLE_TECHNICAL) {
                prefix = "derivacao ";
            }
        }
    }
    if (!append_bytes(out->response, &length, prefix,
                      strlen(prefix)) ||
        !append_fact(lexicon, &answer->fact, out->response, &length,
                     &missing)) {
        missing = 1u;
    }
    if (missing != 0u) {
        out->response[0] = '\0';
        out->response_length = 0u;
        out->lexeme_missing = 1u;
        mark_abstention(out, GC_KIND_UNSUPPORTED,
                        GC_ABSTAIN_UNSUPPORTED, GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    out->response_length = length;
    out->response[length] = '\0';
    out->authority = GC_AUTH_PRESENTATION_ONLY;
    out->kind = answer->kind == SR_ANSWER_DIRECT ? GC_KIND_DIRECT :
                (request->mode == GC_MODE_EXPLAIN ? GC_KIND_COMPOSED : GC_KIND_DERIVED);
    out->status = GC_STATUS_OK;
    out->abstain_reason = GC_ABSTAIN_NONE;
    return GC_STATUS_OK;
}

static gc_status_t generate_answer(const sr_reasoner_t *base,
                                   const gc_lexicon_t *lexicon,
                                   const gc_request_t *request,
                                   sr_reasoner_t *scratch,
                                   gc_result_t *out)
{
    int saturated;
    int queried;
    mrb_status_t memory_status;
    if (request->derivation_budget == 0u) {
        mark_abstention(out, GC_KIND_LIMIT, GC_ABSTAIN_BUDGET,
                        GC_STATUS_LIMIT);
        return GC_STATUS_LIMIT;
    }
    if (request->memory != NULL) {
        if (!query_is_ground(&request->query)) {
            mark_abstention(out, GC_KIND_UNSUPPORTED,
                            GC_ABSTAIN_UNSUPPORTED, GC_STATUS_ABSTAIN);
            return GC_STATUS_ABSTAIN;
        }
        memory_status = mrb_query(base, request->memory, request->current_generation,
                                  &request->query, request->derivation_budget,
                                  scratch, &out->answer, &out->composition);
        if (memory_status == MRB_NO_EVIDENCE) {
            mark_abstention(out, GC_KIND_UNKNOWN, GC_ABSTAIN_NO_EVIDENCE,
                            GC_STATUS_ABSTAIN);
            return GC_STATUS_ABSTAIN;
        }
        if (memory_status == MRB_AMBIGUOUS) {
            mark_abstention(out, GC_KIND_AMBIGUOUS, GC_ABSTAIN_AMBIGUITY,
                            GC_STATUS_ABSTAIN);
            return GC_STATUS_ABSTAIN;
        }
        if (memory_status == MRB_CONTRADICTED) {
            mark_abstention(out, GC_KIND_CONTRADICTED, GC_ABSTAIN_CONFLICT,
                            GC_STATUS_ABSTAIN);
            return GC_STATUS_ABSTAIN;
        }
        if (memory_status == MRB_LIMIT) {
            mark_abstention(out, GC_KIND_LIMIT, GC_ABSTAIN_BUDGET,
                            GC_STATUS_LIMIT);
            return GC_STATUS_LIMIT;
        }
        if (memory_status != MRB_OK) {
            mark_abstention(out, GC_KIND_UNSUPPORTED, GC_ABSTAIN_UNSUPPORTED,
                            GC_STATUS_ABSTAIN);
            return GC_STATUS_ABSTAIN;
        }
        out->grounded = 1u;
        out->derivation_digest = fact_digest(&out->answer.fact) ^
                                 out->composition.selected_card_id ^
                                 out->composition.selected_generation;
        copy_evidence(scratch, &out->answer, out);
        return render_answer(lexicon, &out->answer, request, out);
    }
    *scratch = *base;
    saturated = sr_saturate(scratch, request->derivation_budget);
    if (saturated == SR_E_LIMIT || saturated == SR_E_FULL) {
        mark_abstention(out, GC_KIND_LIMIT, GC_ABSTAIN_BUDGET,
                        GC_STATUS_LIMIT);
        return GC_STATUS_LIMIT;
    }
    if (saturated != SR_OK) {
        mark_abstention(out, GC_KIND_UNSUPPORTED, GC_ABSTAIN_UNSUPPORTED,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    queried = sr_query(scratch, &request->query, &out->answer);
    if (queried == SR_E_NO_EVIDENCE) {
        mark_abstention(out, GC_KIND_UNKNOWN, GC_ABSTAIN_NO_EVIDENCE,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    if (queried == SR_E_AMBIGUOUS) {
        mark_abstention(out, GC_KIND_AMBIGUOUS, GC_ABSTAIN_AMBIGUITY,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    if (queried == SR_E_CONTRADICTION) {
        mark_abstention(out, GC_KIND_CONTRADICTED, GC_ABSTAIN_CONFLICT,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    if (queried != SR_OK) {
        mark_abstention(out, GC_KIND_UNSUPPORTED, GC_ABSTAIN_UNSUPPORTED,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    out->derivation_digest = fact_digest(&out->answer.fact);
    copy_evidence(scratch, &out->answer, out);
    return render_answer(lexicon, &out->answer, request, out);
}

static gc_status_t generate_counterfactual(const gc_lexicon_t *lexicon,
                                           const gc_request_t *request,
                                           sr_reasoner_t *scratch,
                                           gc_result_t *out)
{
    sr_abduction_status_t status;
    sr_fact_t goal_fact;
    uint16_t length = 0u;
    uint8_t missing = 0u;
    if (!query_is_ground(&request->query) || request->derivation_budget == 0u) {
        mark_abstention(out, GC_KIND_UNSUPPORTED,
                        request->derivation_budget == 0u ?
                            GC_ABSTAIN_BUDGET : GC_ABSTAIN_UNSUPPORTED,
                        request->derivation_budget == 0u ?
                            GC_STATUS_LIMIT : GC_STATUS_ABSTAIN);
        return out->status;
    }
    goal_fact = pattern_to_fact(&request->query);
    if (sr_saturate(scratch, request->derivation_budget) != SR_OK) {
        mark_abstention(out, GC_KIND_LIMIT, GC_ABSTAIN_BUDGET,
                        GC_STATUS_LIMIT);
        return GC_STATUS_LIMIT;
    }
    status = sr_abduce(scratch, &request->query, request->derivation_budget,
                       &out->abduction);
    if (status == SR_ABDUCTION_LIMIT) {
        mark_abstention(out, GC_KIND_LIMIT, GC_ABSTAIN_BUDGET,
                        GC_STATUS_LIMIT);
        return GC_STATUS_LIMIT;
    }
    if (status != SR_ABDUCTION_FOUND) {
        mark_abstention(out,
                        status == SR_ABDUCTION_AMBIGUOUS ? GC_KIND_AMBIGUOUS :
                                                           GC_KIND_UNKNOWN,
                        status == SR_ABDUCTION_AMBIGUOUS ?
                            GC_ABSTAIN_AMBIGUITY : GC_ABSTAIN_NO_EVIDENCE,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    if (!append_bytes(out->response, &length, "se ", 3u) ||
        !append_fact(lexicon, &out->abduction.missing_fact, out->response,
                     &length, &missing) ||
        !append_bytes(out->response, &length, ", entao ", 8u) ||
        !append_fact(lexicon, &goal_fact, out->response, &length, &missing)) {
        missing = 1u;
    }
    if (missing != 0u) {
        out->response[0] = '\0';
        out->lexeme_missing = 1u;
        mark_abstention(out, GC_KIND_UNSUPPORTED,
                        GC_ABSTAIN_UNSUPPORTED, GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    out->response_length = length;
    out->counterfactual = 1u;
    out->kind = GC_KIND_COUNTERFACTUAL;
    out->status = GC_STATUS_OK;
    out->authority = GC_AUTH_PRESENTATION_ONLY;
    out->abstain_reason = GC_ABSTAIN_NONE;
    out->derivation_digest = fact_digest(&out->abduction.missing_fact) ^
                             fact_digest(&goal_fact);
    return GC_STATUS_OK;
}

static gc_status_t generate_plan(const gc_request_t *request, gc_result_t *out)
{
    uint8_t i;
    uint16_t length = 0u;
    if (request->plan_problem == NULL || request->max_plan_nodes == 0u ||
        request->max_plan_depth == 0u) {
        mark_abstention(out, GC_KIND_UNSUPPORTED,
                        GC_ABSTAIN_UNSUPPORTED, GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    out->plan.status = sp_plan(request->plan_problem,
                               request->max_plan_nodes,
                               request->max_plan_depth, &out->plan);
    if (out->plan.status == SP_E_LIMIT) {
        mark_abstention(out, GC_KIND_LIMIT, GC_ABSTAIN_BUDGET,
                        GC_STATUS_LIMIT);
        return GC_STATUS_LIMIT;
    }
    if (out->plan.status == SP_NO_PLAN) {
        mark_abstention(out, GC_KIND_UNKNOWN, GC_ABSTAIN_NO_PLAN,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    if (out->plan.status != SP_OK) {
        mark_abstention(out, GC_KIND_UNSUPPORTED, GC_ABSTAIN_UNSUPPORTED,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    if (!append_bytes(out->response, &length, "plano:", 6u)) return GC_E_OUTPUT;
    for (i = 0u; i < out->plan.plan_length; ++i) {
        if (!append_bytes(out->response, &length, i == 0u ? " " : " -> ",
                          i == 0u ? 1u : 4u) ||
            !append_u32(out->response, &length, out->plan.action_id[i])) {
            return GC_E_OUTPUT;
        }
    }
    out->response_length = length;
    out->kind = GC_KIND_PLAN;
    out->status = GC_STATUS_OK;
    out->authority = out->plan.confirmation_count > 0u ?
                     GC_AUTH_CONFIRMATION_REQUIRED : GC_AUTH_PRESENTATION_ONLY;
    out->requires_confirmation = out->plan.confirmation_count > 0u ? 1u : 0u;
    out->derivation_digest = GC_FNV_OFFSET ^ out->plan.cost ^
                             ((uint32_t)out->plan.explored_nodes << 16);
    return GC_STATUS_OK;
}

gc_status_t gc_generate(const sr_reasoner_t *base,
                        const gc_lexicon_t *lexicon,
                        const gc_request_t *request,
                        sr_reasoner_t *scratch,
                        gc_result_t *out)
{
    if (out == NULL || request == NULL || scratch == NULL) return GC_E_ARG;
    if (request->mode != GC_MODE_PLAN && base == scratch) return GC_E_ARG;
    memset(out, 0, sizeof(*out));
    if (request->policy_blocked != 0u) {
        mark_abstention(out, GC_KIND_POLICY_BLOCKED, GC_ABSTAIN_POLICY,
                        GC_STATUS_ABSTAIN);
        return GC_STATUS_ABSTAIN;
    }
    if (request->mode == GC_MODE_PLAN) return generate_plan(request, out);
    if (base == NULL || lexicon == NULL) return GC_E_ARG;
    if (request->mode == GC_MODE_COUNTERFACTUAL) {
        *scratch = *base;
        return generate_counterfactual(lexicon, request, scratch, out);
    }
    if (request->mode != GC_MODE_ANSWER && request->mode != GC_MODE_EXPLAIN)
        return GC_E_FORMAT;
    return generate_answer(base, lexicon, request, scratch, out);
}
