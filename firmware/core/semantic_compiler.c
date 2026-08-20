#include "semantic_compiler.h"
#include <string.h>

#define SC_HASH_OFFSET 2166136261u
#define SC_HASH_PRIME  16777619u
#define SC_ERR_SENSITIVE 100u
#define SC_ERR_COLLISION 101u

typedef struct {
    const char *start;
    uint8_t length;
} sc_token_t;

static int is_space(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int is_punctuation(unsigned char c)
{
    return c == '.' || c == ',' || c == '?' || c == ';' || c == ':';
}

static unsigned char ascii_lower(unsigned char c)
{
    return (c >= (unsigned char)'A' && c <= (unsigned char)'Z')
               ? (unsigned char)(c + ((unsigned char)'a' - (unsigned char)'A'))
               : c;
}

static int word_eq(const sc_token_t *token, const char *word)
{
    size_t length = strlen(word);
    if (!token || length != token->length) return 0;
    for (size_t i = 0u; i < length; i++) {
        if (ascii_lower((unsigned char)token->start[i]) !=
            ascii_lower((unsigned char)word[i])) return 0;
    }
    return 1;
}

static int word_any(const sc_token_t *token, const char *a, const char *b)
{
    return word_eq(token, a) || word_eq(token, b);
}

static uint16_t fold_id(uint32_t hash)
{
    uint16_t id = (uint16_t)((hash >> 16) ^ (hash & 0xffffu));
    return id == 0u ? 1u : id;
}

uint16_t sc_symbol_id(const char *text, size_t length)
{
    uint32_t hash = SC_HASH_OFFSET;
    if (!text || length == 0u) return 0u;
    for (size_t i = 0u; i < length; i++) {
        hash ^= ascii_lower((unsigned char)text[i]);
        hash *= SC_HASH_PRIME;
    }
    return fold_id(hash);
}

static uint16_t canonical_id(const char *word)
{
    return sc_symbol_id(word, strlen(word));
}

static int token_same_lexeme(const sc_token_t *a, const sc_token_t *b)
{
    if (!a || !b || a->length != b->length) return 0;
    for (uint8_t i = 0u; i < a->length; i++) {
        if (ascii_lower((unsigned char)a->start[i]) !=
            ascii_lower((unsigned char)b->start[i])) return 0;
    }
    return 1;
}

static int find_token_collision(const sc_token_t *tokens, uint8_t count,
                                uint8_t *error_token)
{
    if (!tokens || !error_token) return SC_E_ARG;
    for (uint8_t i = 0u; i < count; i++) {
        uint16_t first = sc_symbol_id(tokens[i].start, tokens[i].length);
        for (uint8_t j = (uint8_t)(i + 1u); j < count; j++) {
            uint16_t second = sc_symbol_id(tokens[j].start, tokens[j].length);
            if (first == second && !token_same_lexeme(&tokens[i], &tokens[j])) {
                *error_token = j;
                return 1;
            }
        }
    }
    return 0;
}

static int tokenize(const char *input, size_t length,
                    sc_token_t *tokens, uint8_t *count, uint8_t *question)
{
    size_t pos = 0u;
    uint8_t n = 0u;
    uint8_t has_question = 0u;
    if (!input || !tokens || !count || !question) return SC_E_ARG;
    while (pos < length) {
        size_t start;
        while (pos < length && is_space((unsigned char)input[pos])) pos++;
        if (pos >= length) break;
        if (input[pos] == '?') has_question = 1u;
        if (is_punctuation((unsigned char)input[pos])) {
            pos++;
            continue;
        }
        start = pos;
        while (pos < length && !is_space((unsigned char)input[pos]) &&
               !is_punctuation((unsigned char)input[pos])) {
            pos++;
        }
        if (pos - start == 0u || pos - start > SC_MAX_LEXEM)
            return SC_E_TOKEN;
        if (n >= SC_MAX_TOKENS) return SC_E_LIMIT;
        tokens[n].start = &input[start];
        tokens[n].length = (uint8_t)(pos - start);
        n++;
    }
    *count = n;
    *question = has_question;
    return n == 0u ? SC_E_EMPTY : SC_OK;
}

static int token_is_sensitive(const sc_token_t *token)
{
    return word_any(token, "audio", "áudio") ||
           word_any(token, "transcricao", "transcrição") ||
           word_any(token, "embedding", "embeddings") ||
           word_any(token, "localizacao", "localização") ||
           word_any(token, "senha", "chave");
}

static int parse_term(const sc_token_t *token, sr_term_t *out)
{
    if (!token || !out) return SC_E_ARG;
    if (word_any(token, "alguem", "alguém")) {
        *out = SC_VAR(1u);
        return SC_OK;
    }
    *out = SC_CONST(sc_symbol_id(token->start, token->length));
    return out->value == 0u ? SC_E_TOKEN : SC_OK;
}

static int parse_relation(const sc_token_t *tokens, uint8_t start, uint8_t count,
                          sr_pattern_t *out)
{
    uint8_t verb = start + 1u;
    uint8_t object = start + 2u;
    uint8_t negated = 0u;
    sr_term_t subject_term;
    sr_term_t object_term;
    if (!tokens || !out || count < (uint8_t)(start + 3u)) return SC_E_SYNTAX;
    if (word_any(&tokens[verb], "nao", "não")) {
        negated = 1u;
        verb++;
        object++;
    }
    if (object >= count) return SC_E_SYNTAX;
    if (parse_term(&tokens[start], &subject_term) != SC_OK ||
        parse_term(&tokens[object], &object_term) != SC_OK)
        return SC_E_TOKEN;
    memset(out, 0, sizeof(*out));
    out->subject = subject_term;
    out->object = object_term;
    out->negated = negated;
    if (word_any(&tokens[verb], "possui", "tem")) {
        if (count != (uint8_t)(object + 1u)) return SC_E_SYNTAX;
        out->predicate = SC_CONST(canonical_id("possui"));
    } else if (word_any(&tokens[verb], "esta", "está")) {
        object = (uint8_t)(verb + 2u);
        if ((uint8_t)(verb + 1u) >= count || object >= count ||
            !word_any(&tokens[verb + 1u], "em", "no")) return SC_E_SYNTAX;
        out->predicate = SC_CONST(canonical_id("estar_em"));
        if (parse_term(&tokens[object], &object_term) != SC_OK ||
            count != (uint8_t)(object + 1u)) return SC_E_SYNTAX;
        out->object = object_term;
    } else if (word_eq(&tokens[verb], "pode")) {
        if (count != (uint8_t)(object + 1u)) return SC_E_SYNTAX;
        out->predicate = SC_CONST(canonical_id("poder"));
    } else {
        return SC_E_UNSUPPORTED;
    }
    return SC_OK;
}

static int parse_fact(const sc_token_t *tokens, uint8_t count, sr_fact_t *out)
{
    sr_pattern_t pattern;
    int result;
    if (!tokens || !out) return SC_E_ARG;
    result = parse_relation(tokens, 0u, count, &pattern);
    if (result != SC_OK) return result;
    if (pattern.subject.kind != SC_TERM_CONSTANT ||
        pattern.object.kind != SC_TERM_CONSTANT)
        return SC_E_SYNTAX;
    out->subject = pattern.subject.value;
    out->predicate = pattern.predicate.value;
    out->object = pattern.object.value;
    out->negated = pattern.negated;
    return SC_OK;
}

static int parse_rule(const sc_token_t *tokens, uint8_t count, sr_rule_t *out)
{
    uint8_t split = 0u;
    sr_pattern_t premise;
    sr_pattern_t conclusion;
    if (!tokens || !out || count < 7u || !word_eq(&tokens[0], "se"))
        return SC_E_SYNTAX;
    for (uint8_t i = 1u; i < count; i++) {
        if (word_any(&tokens[i], "entao", "então")) {
            if (split != 0u) return SC_E_SYNTAX;
            split = i;
        }
    }
    if (split < 4u || split + 3u >= count) return SC_E_SYNTAX;
    if (parse_relation(tokens, 1u, split, &premise) != SC_OK ||
        parse_relation(tokens, (uint8_t)(split + 1u), count, &conclusion) != SC_OK)
        return SC_E_SYNTAX;
    memset(out, 0, sizeof(*out));
    out->id = (uint8_t)(canonical_id("regra") & 0xffu);
    out->premise_count = 1u;
    out->premise[0] = premise;
    out->conclusion = conclusion;
    out->cost = 1u;
    return SC_OK;
}

static int parse_query(const sc_token_t *tokens, uint8_t count,
                       sr_pattern_t *out)
{
    sr_term_t subject;
    if (!tokens || !out || count < 4u || !word_eq(&tokens[0], "o") ||
        !word_eq(&tokens[1], "que")) return SC_E_SYNTAX;
    if (parse_term(&tokens[2], &subject) != SC_OK ||
        subject.kind != SC_TERM_CONSTANT) return SC_E_SYNTAX;
    memset(out, 0, sizeof(*out));
    out->subject = subject;
    out->object = SC_VAR(2u);
    if (word_any(&tokens[3], "possui", "tem")) {
        if (count != 4u) return SC_E_SYNTAX;
        out->predicate = SC_CONST(canonical_id("possui"));
    } else if (word_eq(&tokens[3], "pode")) {
        if (count != 4u) return SC_E_SYNTAX;
        out->predicate = SC_CONST(canonical_id("poder"));
    } else {
        return SC_E_UNSUPPORTED;
    }
    return SC_OK;
}

static int parse_goal(const sc_token_t *tokens, uint8_t count, sr_fact_t *out)
{
    uint8_t object;
    if (!tokens || !out || count < 2u ||
        !word_any(&tokens[0], "planeje", "planejar")) return SC_E_SYNTAX;
    memset(out, 0, sizeof(*out));
    out->subject = canonical_id("eu");
    if (word_eq(&tokens[1], "chegar")) {
        if (count != 4u || !word_any(&tokens[2], "em", "no"))
            return SC_E_SYNTAX;
        object = 3u;
        out->predicate = canonical_id("chegar");
    } else if (word_eq(&tokens[1], "estudar")) {
        if (count != 2u) return SC_E_SYNTAX;
        object = 1u;
        out->predicate = canonical_id("estudar");
    } else {
        return SC_E_UNSUPPORTED;
    }
    if (object >= count) return SC_E_SYNTAX;
    out->object = sc_symbol_id(tokens[object].start, tokens[object].length);
    out->negated = 0u;
    return SC_OK;
}

static int is_reject(const sc_token_t *tokens, uint8_t count)
{
    if (count < 2u) return 0;
    return word_any(&tokens[0], "nao", "não") &&
           (word_eq(&tokens[1], "guardar") ||
            word_eq(&tokens[1], "memorizar"));
}

int sc_compile(const char *input, size_t length, sc_unit_t *out)
{
    sc_token_t tokens[SC_MAX_TOKENS];
    uint8_t count = 0u;
    uint8_t question = 0u;
    int result;
    if (!out || !input) return SC_E_ARG;
    memset(out, 0, sizeof(*out));
    out->status = SC_E_ARG;
    if (length == 0u) return out->status = SC_E_EMPTY;
    if (length > SC_MAX_INPUT_BYTES) return out->status = SC_E_TOO_LONG;
    for (size_t i = 0u; i < length; i++) {
        if ((unsigned char)input[i] == 0u) {
            out->error_token = 0xffu;
            return out->status = SC_E_TOKEN;
        }
    }
    result = tokenize(input, length, tokens, &count, &question);
    out->token_count = count;
    if (result != SC_OK) return out->status = result;
    for (uint8_t i = 0u; i < count; i++) {
        if (token_is_sensitive(&tokens[i])) {
            out->error_token = i;
            out->error_code = SC_ERR_SENSITIVE;
            return out->status = SC_E_SENSITIVE;
        }
    }
    if (find_token_collision(tokens, count, &out->error_token) == 1) {
        out->error_code = SC_ERR_COLLISION;
        return out->status = SC_E_TOKEN;
    }
    if (is_reject(tokens, count)) {
        out->kind = SC_UNIT_REJECT;
        out->exact_parse = 1u;
        out->status = SC_OK;
        return SC_OK;
    }
    if (question || (count >= 2u && word_eq(&tokens[0], "o") &&
                     word_eq(&tokens[1], "que"))) {
        result = parse_query(tokens, count, &out->meaning.query);
        out->kind = SC_UNIT_QUERY;
        out->requires_confirmation = 0u;
    } else if (word_eq(&tokens[0], "se")) {
        result = parse_rule(tokens, count, &out->meaning.rule);
        out->kind = SC_UNIT_RULE;
        out->requires_confirmation = 1u;
    } else if (word_any(&tokens[0], "planeje", "planejar")) {
        result = parse_goal(tokens, count, &out->meaning.goal);
        out->kind = SC_UNIT_GOAL;
        out->requires_confirmation = 1u;
    } else {
        result = parse_fact(tokens, count, &out->meaning.fact);
        out->kind = SC_UNIT_FACT;
        out->requires_confirmation = 1u;
    }
    out->status = result;
    out->exact_parse = result == SC_OK ? 1u : 0u;
    return result;
}

static void bridge_reset(sc_bridge_result_t *out)
{
    memset(out, 0, sizeof(*out));
    out->status = SC_BRIDGE_E_ARG;
}

static int map_dialogue_status(int status, sc_bridge_result_t *out)
{
    if (status == SD_OK) {
        out->status = SC_BRIDGE_OK;
        return SC_BRIDGE_OK;
    }
    if (status == SD_E_AUTH) {
        out->confirmation_required = 1u;
        out->abstained = 1u;
        out->status = SC_BRIDGE_E_AUTH;
        return SC_BRIDGE_E_AUTH;
    }
    if (status == SD_E_LIMIT) {
        out->abstained = 1u;
        out->status = SC_BRIDGE_E_LIMIT;
        return SC_BRIDGE_E_LIMIT;
    }
    out->abstained = 1u;
    out->status = SC_BRIDGE_E_ABSTAIN;
    return SC_BRIDGE_E_ABSTAIN;
}

int sc_apply_dialogue(sd_dialogue_t *dialogue, const sc_unit_t *unit,
                      uint8_t explicit_confirmation, uint32_t derivation_budget,
                      sc_bridge_result_t *out)
{
    int result;
    if (!dialogue || !unit || !out) return SC_BRIDGE_E_ARG;
    bridge_reset(out);
    out->derivation_budget = derivation_budget;
    if (unit->status != SC_OK || unit->exact_parse != 1u) {
        out->abstained = 1u;
        out->status = SC_BRIDGE_E_ABSTAIN;
        return out->status;
    }
    if (unit->kind == SC_UNIT_FACT || unit->kind == SC_UNIT_RULE) {
        if (unit->requires_confirmation != 0u &&
            explicit_confirmation != 1u) {
            out->confirmation_required = 1u;
            out->abstained = 1u;
            out->status = SC_BRIDGE_E_AUTH;
            return out->status;
        }
        if (unit->kind == SC_UNIT_FACT)
            result = sd_add_personal_fact(dialogue, unit->meaning.fact, 1u);
        else
            result = sd_add_rule(dialogue, &unit->meaning.rule);
        result = map_dialogue_status(result, out);
        if (result == SC_BRIDGE_OK) out->state_changed = 1u;
        return result;
    }
    if (unit->kind == SC_UNIT_QUERY) {
        if (derivation_budget == 0u) {
            out->abstained = 1u;
            out->status = SC_BRIDGE_E_LIMIT;
            return out->status;
        }
        result = sd_ask(dialogue, &unit->meaning.query,
                        derivation_budget, &out->reply);
        return map_dialogue_status(result, out);
    }
    if (unit->kind == SC_UNIT_REJECT) {
        out->status = SC_BRIDGE_OK;
        return SC_BRIDGE_OK;
    }
    out->abstained = 1u;
    out->status = SC_BRIDGE_E_KIND;
    return out->status;
}

int sc_plan_goal(const sc_unit_t *unit, const sp_problem_t *catalog,
                 uint16_t max_nodes, uint8_t max_depth,
                 sc_bridge_result_t *out)
{
    sp_problem_t problem;
    int result;
    if (!unit || !catalog || !out || max_nodes == 0u || max_depth == 0u)
        return SC_BRIDGE_E_ARG;
    bridge_reset(out);
    if (unit->status != SC_OK || unit->exact_parse != 1u ||
        unit->kind != SC_UNIT_GOAL) {
        out->abstained = 1u;
        out->status = SC_BRIDGE_E_ABSTAIN;
        return out->status;
    }
    problem = *catalog;
    problem.goal = unit->meaning.goal;
    out->confirmation_required = unit->requires_confirmation;
    result = sp_plan(&problem, max_nodes, max_depth, &out->plan);
    if (result == SP_OK) {
        out->status = SC_BRIDGE_OK;
        return SC_BRIDGE_OK;
    }
    out->abstained = 1u;
    if (result == SP_E_LIMIT) {
        out->status = SC_BRIDGE_E_LIMIT;
        return out->status;
    }
    if (result == SP_NO_PLAN) {
        out->status = SC_BRIDGE_E_NO_PLAN;
        return out->status;
    }
    out->status = SC_BRIDGE_E_ABSTAIN;
    return out->status;
}
