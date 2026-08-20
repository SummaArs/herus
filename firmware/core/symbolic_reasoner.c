#include "symbolic_reasoner.h"
#include <string.h>

#define SR_RULE_NONE 0xffu

typedef struct {
    uint8_t used[SR_MAX_VARIABLES];
    uint16_t value[SR_MAX_VARIABLES];
} sr_binding_t;

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_term(sr_term_t term)
{
    if (term.kind == SR_TERM_CONSTANT) return 1;
    return term.kind == SR_TERM_VARIABLE && term.value < SR_MAX_VARIABLES;
}

static int valid_pattern(const sr_pattern_t *pattern)
{
    return pattern && canonical_bool(pattern->negated) &&
           valid_term(pattern->subject) && valid_term(pattern->predicate) &&
           valid_term(pattern->object);
}

static int valid_fact(sr_fact_t fact)
{
    return canonical_bool(fact.negated);
}

static int fact_equal(sr_fact_t a, sr_fact_t b)
{
    return a.subject == b.subject && a.predicate == b.predicate &&
           a.object == b.object && a.negated == b.negated;
}

static int fact_opposite(sr_fact_t a, sr_fact_t b)
{
    return a.subject == b.subject && a.predicate == b.predicate &&
           a.object == b.object && a.negated != b.negated;
}

static int find_fact(const sr_reasoner_t *r, sr_fact_t fact)
{
    if (!r) return -1;
    for (unsigned i = 0u; i < r->fact_count; i++) {
        if (fact_equal(r->facts[i], fact)) return (int)i;
    }
    return -1;
}

static int bind_term(sr_term_t pattern, uint16_t value, sr_binding_t *binding)
{
    if (pattern.kind == SR_TERM_CONSTANT) return pattern.value == value;
    if (!binding || pattern.value >= SR_MAX_VARIABLES) return 0;
    if (!binding->used[pattern.value]) {
        binding->used[pattern.value] = 1u;
        binding->value[pattern.value] = value;
        return 1;
    }
    return binding->value[pattern.value] == value;
}

static int match_pattern(const sr_pattern_t *pattern, sr_fact_t fact,
                         sr_binding_t *binding)
{
    if (!valid_pattern(pattern) || !valid_fact(fact) || !binding ||
        pattern->negated != fact.negated) return 0;
    return bind_term(pattern->subject, fact.subject, binding) &&
           bind_term(pattern->predicate, fact.predicate, binding) &&
           bind_term(pattern->object, fact.object, binding);
}

static uint16_t instantiate_term(sr_term_t term, const sr_binding_t *binding)
{
    if (term.kind == SR_TERM_CONSTANT) return term.value;
    return binding->value[term.value];
}

static int pattern_variables_bound(const sr_rule_t *rule)
{
    const sr_term_t *conclusion[3] = {
        &rule->conclusion.subject, &rule->conclusion.predicate,
        &rule->conclusion.object
    };
    for (unsigned c = 0u; c < 3u; c++) {
        if (conclusion[c]->kind != SR_TERM_VARIABLE) continue;
        int found = 0;
        for (unsigned p = 0u; p < rule->premise_count && !found; p++) {
            const sr_term_t *premise[3] = {
                &rule->premise[p].subject, &rule->premise[p].predicate,
                &rule->premise[p].object
            };
            for (unsigned t = 0u; t < 3u; t++) {
                if (premise[t]->kind == SR_TERM_VARIABLE &&
                    premise[t]->value == conclusion[c]->value) {
                    found = 1;
                    break;
                }
            }
        }
        if (!found) return 0;
    }
    return 1;
}

static int valid_rule(const sr_rule_t *rule)
{
    if (!rule || rule->premise_count == 0u ||
        rule->premise_count > SR_MAX_PREMISES || !valid_pattern(&rule->conclusion) ||
        !pattern_variables_bound(rule)) return 0;
    for (unsigned i = 0u; i < rule->premise_count; i++) {
        if (!valid_pattern(&rule->premise[i])) return 0;
    }
    return 1;
}

static void set_input_meta(sr_fact_meta_t *meta)
{
    memset(meta, 0, sizeof(*meta));
    meta->origin = SR_ORIGIN_INPUT;
    meta->rule_id = SR_RULE_NONE;
}

void sr_init(sr_reasoner_t *r)
{
    if (!r) return;
    memset(r, 0, sizeof(*r));
}

int sr_add_fact(sr_reasoner_t *r, sr_fact_t fact)
{
    if (!r || !valid_fact(fact)) return SR_E_ARG;
    if (find_fact(r, fact) >= 0) return SR_NO_CHANGE;
    if (r->fact_count >= SR_MAX_FACTS) return SR_E_FULL;
    for (unsigned i = 0u; i < r->fact_count; i++) {
        if (fact_opposite(r->facts[i], fact)) r->contradiction_count++;
    }
    r->facts[r->fact_count] = fact;
    set_input_meta(&r->meta[r->fact_count]);
    r->fact_count++;
    return SR_OK;
}

int sr_add_rule(sr_reasoner_t *r, const sr_rule_t *rule)
{
    if (!r || !valid_rule(rule)) return SR_E_FORMAT;
    for (unsigned i = 0u; i < r->rule_count; i++) {
        if (r->rules[i].id == rule->id) return SR_NO_CHANGE;
    }
    if (r->rule_count >= SR_MAX_RULES) return SR_E_FULL;
    r->rules[r->rule_count++] = *rule;
    return SR_OK;
}

static int add_derived(sr_reasoner_t *r, sr_fact_t fact,
                       uint8_t rule_id, const uint8_t *parents,
                       uint8_t parent_count, uint8_t depth,
                       uint16_t derivation_cost)
{
    int existing;
    if (!r || !valid_fact(fact) || parent_count > SR_MAX_PARENTS) return SR_E_ARG;
    existing = find_fact(r, fact);
    if (existing >= 0) return SR_NO_CHANGE;
    if (r->fact_count >= SR_MAX_FACTS) return SR_E_FULL;
    for (unsigned i = 0u; i < r->fact_count; i++) {
        if (fact_opposite(r->facts[i], fact)) r->contradiction_count++;
    }
    r->facts[r->fact_count] = fact;
    memset(&r->meta[r->fact_count], 0, sizeof(r->meta[r->fact_count]));
    r->meta[r->fact_count].origin = SR_ORIGIN_RULE;
    r->meta[r->fact_count].depth = depth;
    r->meta[r->fact_count].rule_id = rule_id;
    r->meta[r->fact_count].parent_count = parent_count;
    r->meta[r->fact_count].derivation_cost = derivation_cost;
    for (unsigned i = 0u; i < parent_count; i++)
        r->meta[r->fact_count].parent[i] = parents[i];
    r->fact_count++;
    return SR_OK;
}

static int derive_rule(const sr_reasoner_t *source, sr_reasoner_t *target,
                       const sr_rule_t *rule, unsigned premise_index,
                       sr_binding_t *binding, uint8_t *parents,
                       uint8_t parent_count, uint8_t depth,
                       uint16_t derivation_cost, uint32_t max_steps)
{
    if (target->derivation_steps >= max_steps) {
        target->saturation_truncated = 1u;
        return SR_E_LIMIT;
    }
    if (premise_index == rule->premise_count) {
        sr_fact_t conclusion;
        int result;
        conclusion.subject = instantiate_term(rule->conclusion.subject, binding);
        conclusion.predicate = instantiate_term(rule->conclusion.predicate, binding);
        conclusion.object = instantiate_term(rule->conclusion.object, binding);
        conclusion.negated = rule->conclusion.negated;
        target->derivation_steps++;
        result = add_derived(target, conclusion, rule->id, parents,
                             parent_count, depth, derivation_cost);
        return result == SR_E_FULL ? SR_E_FULL : SR_OK;
    }

    /* `source` is the target itself so facts generated earlier in the same pass
     * are available to later premise matches. The pointer name documents that
     * matching never mutates a fact. */
    for (unsigned i = 0u; i < source->fact_count; i++) {
        sr_binding_t next = *binding;
        if (!match_pattern(&rule->premise[premise_index], source->facts[i], &next))
            continue;
        if (parent_count >= SR_MAX_PARENTS) return SR_E_FORMAT;
        parents[parent_count] = (uint8_t)i;
        {
            uint8_t next_depth = depth;
            uint16_t next_cost = derivation_cost +
                                 source->meta[i].derivation_cost;
            if (source->meta[i].depth >= next_depth)
                next_depth = (uint8_t)(source->meta[i].depth + 1u);
            if (next_depth > SR_MAX_DEPTH ||
                next_cost < derivation_cost) {
                target->saturation_truncated = 1u;
                return SR_E_LIMIT;
            }
            {
                int recurse_result = derive_rule(
                    source, target, rule, premise_index + 1u, &next,
                    parents, (uint8_t)(parent_count + 1u), next_depth,
                    next_cost, max_steps);
                if (recurse_result == SR_E_LIMIT ||
                    recurse_result == SR_E_FULL)
                    return recurse_result;
            }
        }
    }
    return SR_OK;
}

int sr_saturate(sr_reasoner_t *r, uint32_t max_steps)
{
    if (!r || max_steps == 0u) return SR_E_ARG;
    r->saturation_truncated = 0u;
    r->derivation_steps = 0u;
    for (unsigned pass = 0u; pass < SR_MAX_DEPTH; pass++) {
        uint8_t before = r->fact_count;
        for (unsigned i = 0u; i < r->rule_count; i++) {
            sr_binding_t binding;
            uint8_t parents[SR_MAX_PARENTS] = { 0u };
            memset(&binding, 0, sizeof(binding));
            {
                int derive_result = derive_rule(r, r, &r->rules[i], 0u,
                                                &binding, parents, 0u, 0u,
                                                r->rules[i].cost, max_steps);
                if (derive_result == SR_E_LIMIT) return SR_E_LIMIT;
                if (derive_result == SR_E_FULL) {
                    r->saturation_truncated = 1u;
                    return SR_E_FULL;
                }
            }
            if (r->fact_count >= SR_MAX_FACTS && r->derivation_steps >= max_steps)
                return SR_E_LIMIT;
        }
        if (r->fact_count == before) return SR_OK;
    }
    r->saturation_truncated = 1u;
    return SR_E_LIMIT;
}

static int query_matches(const sr_pattern_t *query, sr_fact_t fact)
{
    sr_binding_t binding;
    memset(&binding, 0, sizeof(binding));
    return match_pattern(query, fact, &binding);
}

static void fill_answer(const sr_reasoner_t *r, int index, sr_answer_t *out,
                        sr_answer_kind_t kind, uint8_t hit_count)
{
    out->kind = kind;
    out->fact = r->facts[index];
    out->fact_index = (uint8_t)index;
    out->hit_count = hit_count;
    out->depth = r->meta[index].depth;
    out->rule_id = r->meta[index].rule_id;
    out->derivation_cost = r->meta[index].derivation_cost;
    out->evidence_count = r->meta[index].parent_count;
    for (unsigned i = 0u; i < out->evidence_count; i++)
        out->evidence[i] = r->meta[index].parent[i];
}

int sr_query(const sr_reasoner_t *r, const sr_pattern_t *query,
             sr_answer_t *out)
{
    sr_pattern_t opposite;
    int first = -1;
    int opposite_first = -1;
    unsigned hits = 0u;
    unsigned opposite_hits = 0u;
    if (!r || !out || !valid_pattern(query)) return SR_E_ARG;
    memset(out, 0, sizeof(*out));
    out->kind = SR_ANSWER_NONE;
    opposite = *query;
    opposite.negated = (uint8_t)!opposite.negated;

    for (unsigned i = 0u; i < r->fact_count; i++) {
        if (query_matches(query, r->facts[i])) {
            if (first < 0) first = (int)i;
            if (hits < SR_MAX_QUERY_HITS) hits++;
        }
        if (query_matches(&opposite, r->facts[i])) {
            if (opposite_first < 0) opposite_first = (int)i;
            if (opposite_hits < SR_MAX_QUERY_HITS) opposite_hits++;
        }
    }
    if (hits > 0u && opposite_hits > 0u) {
        fill_answer(r, first, out, SR_ANSWER_CONTRADICTED,
                    (uint8_t)(hits > 255u ? 255u : hits));
        out->hit_count = (uint8_t)(hits + opposite_hits > 255u
                                        ? 255u : hits + opposite_hits);
        return SR_E_CONTRADICTION;
    }
    if (hits == 0u) {
        out->kind = SR_ANSWER_ABSENT;
        return SR_E_NO_EVIDENCE;
    }
    if (hits > 1u) {
        fill_answer(r, first, out, SR_ANSWER_AMBIGUOUS,
                    (uint8_t)(hits > 255u ? 255u : hits));
        return SR_E_AMBIGUOUS;
    }
    fill_answer(r, first, out,
                r->meta[first].origin == SR_ORIGIN_INPUT
                    ? SR_ANSWER_DIRECT : SR_ANSWER_DERIVED,
                1u);
    return SR_OK;
}

static int instantiate_ground_pattern(const sr_pattern_t *pattern,
                                       const sr_binding_t *binding,
                                       sr_fact_t *out)
{
    const sr_term_t *terms[3];
    uint16_t *values[3];
    if (!pattern || !binding || !out || !valid_pattern(pattern)) return 0;
    terms[0] = &pattern->subject;
    terms[1] = &pattern->predicate;
    terms[2] = &pattern->object;
    values[0] = &out->subject;
    values[1] = &out->predicate;
    values[2] = &out->object;
    for (unsigned i = 0u; i < 3u; i++) {
        if (terms[i]->kind == SR_TERM_CONSTANT) {
            *values[i] = terms[i]->value;
        } else if (terms[i]->value < SR_MAX_VARIABLES &&
                   binding->used[terms[i]->value]) {
            *values[i] = binding->value[terms[i]->value];
        } else {
            return 0;
        }
    }
    out->negated = pattern->negated;
    return valid_fact(*out);
}

typedef struct {
    const sr_reasoner_t *reasoner;
    const sr_rule_t *rule;
    uint8_t missing_premise;
    uint32_t max_candidates;
    sr_abduction_t *out;
    uint8_t found;
    uint8_t stop;
} sr_abduction_walk_t;

static void walk_abduction_support(sr_abduction_walk_t *walk,
                                   uint8_t premise_index,
                                   const sr_binding_t *binding,
                                   uint8_t supporting_count)
{
    if (!walk || !binding || walk->stop) return;
    if (premise_index == walk->rule->premise_count) {
        sr_fact_t candidate;
        if (!instantiate_ground_pattern(&walk->rule->premise[
                                            walk->missing_premise],
                                        binding, &candidate)) return;
        if (find_fact(walk->reasoner, candidate) >= 0) return;
        if (walk->out->candidates_examined >= walk->max_candidates) {
            walk->out->status = SR_ABDUCTION_LIMIT;
            walk->stop = 1u;
            return;
        }
        walk->out->candidates_examined++;
        if (walk->out->status == SR_ABDUCTION_NONE) {
            walk->found = 1u;
            walk->out->status = SR_ABDUCTION_FOUND;
            walk->out->missing_fact = candidate;
            walk->out->rule_id = walk->rule->id;
            walk->out->missing_premise = walk->missing_premise;
            walk->out->supporting_count = supporting_count;
            walk->out->derivation_cost = walk->rule->cost;
        } else if (walk->out->status == SR_ABDUCTION_FOUND &&
                   !fact_equal(walk->out->missing_fact, candidate)) {
            walk->out->status = SR_ABDUCTION_AMBIGUOUS;
            memset(&walk->out->missing_fact, 0, sizeof(walk->out->missing_fact));
            walk->stop = 1u;
        }
        return;
    }
    if (premise_index == walk->missing_premise) {
        walk_abduction_support(walk, (uint8_t)(premise_index + 1u),
                               binding, supporting_count);
        return;
    }
    for (unsigned i = 0u; i < walk->reasoner->fact_count; i++) {
        sr_binding_t next = *binding;
        if (match_pattern(&walk->rule->premise[premise_index],
                          walk->reasoner->facts[i], &next)) {
            walk_abduction_support(walk, (uint8_t)(premise_index + 1u),
                                   &next, (uint8_t)(supporting_count + 1u));
            if (walk->stop) return;
        }
    }
}

sr_abduction_status_t sr_abduce(const sr_reasoner_t *r,
                                const sr_pattern_t *ground_goal,
                                uint32_t max_candidates,
                                sr_abduction_t *out)
{
    sr_fact_t goal;
    sr_pattern_t opposite;
    if (!r || !ground_goal || !out) return SR_ABDUCTION_E_ARG;
    memset(out, 0, sizeof(*out));
    out->status = SR_ABDUCTION_NONE;
    if (max_candidates == 0u || !valid_pattern(ground_goal)) {
        out->status = max_candidates == 0u ? SR_ABDUCTION_LIMIT
                                           : SR_ABDUCTION_E_ARG;
        return out->status;
    }
    if (ground_goal->subject.kind != SR_TERM_CONSTANT ||
        ground_goal->predicate.kind != SR_TERM_CONSTANT ||
        ground_goal->object.kind != SR_TERM_CONSTANT) {
        out->status = SR_ABDUCTION_E_ARG;
        return out->status;
    }
    goal.subject = ground_goal->subject.value;
    goal.predicate = ground_goal->predicate.value;
    goal.object = ground_goal->object.value;
    goal.negated = ground_goal->negated;
    opposite = *ground_goal;
    opposite.negated = (uint8_t)!opposite.negated;
    if (find_fact(r, goal) >= 0) return out->status;
    goal.negated = opposite.negated;
    if (find_fact(r, goal) >= 0) return out->status;
    goal.negated = ground_goal->negated;

    for (unsigned rule_index = 0u; rule_index < r->rule_count; rule_index++) {
        const sr_rule_t *rule = &r->rules[rule_index];
        sr_binding_t binding;
        memset(&binding, 0, sizeof(binding));
        if (!match_pattern(&rule->conclusion, goal, &binding)) continue;
        for (uint8_t missing = 0u; missing < rule->premise_count; missing++) {
            sr_abduction_walk_t walk;
            memset(&walk, 0, sizeof(walk));
            walk.reasoner = r;
            walk.rule = rule;
            walk.missing_premise = missing;
            walk.max_candidates = max_candidates;
            walk.out = out;
            walk_abduction_support(&walk, 0u, &binding, 0u);
            if (out->status == SR_ABDUCTION_AMBIGUOUS ||
                out->status == SR_ABDUCTION_LIMIT)
                return out->status;
        }
    }
    return out->status;
}

unsigned sr_fact_count(const sr_reasoner_t *r)
{
    return r ? r->fact_count : 0u;
}

unsigned sr_rule_count(const sr_reasoner_t *r)
{
    return r ? r->rule_count : 0u;
}

unsigned sr_contradiction_count(const sr_reasoner_t *r)
{
    return r ? r->contradiction_count : 0u;
}
