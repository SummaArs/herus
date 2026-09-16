#include "sim.h"

#include <stddef.h>

static const int8_t HIDDEN[3][SIM_FEATURES] = {
    {3, 2, -2, -1}, {-2, -1, 3, 2}, {1, -3, -1, 3}
};
static const int8_t OUTPUT[SIM_LABELS][3] = {
    {3, -2, 1}, {-1, 3, -2}, {-2, 1, 3}, {0, 0, 0}
};

static int16_t dot_hidden(const int8_t *row, const int8_t *features) {
    int16_t total = 0;
    size_t i;
    for (i = 0; i < SIM_FEATURES; ++i) total += (int16_t)row[i] * features[i];
    return total > 0 ? total : 0;
}

void sim_infer(const sim_pattern_t *pattern, sim_neural_scores_t *out) {
    int16_t hidden[3] = {0, 0, 0};
    int16_t best = 0;
    int16_t second = 0;
    uint8_t i;
    if (pattern == NULL || out == NULL) return;
    for (i = 0; i < 3u; ++i) hidden[i] = dot_hidden(HIDDEN[i], pattern->features);
    for (i = 0; i < SIM_LABELS; ++i) {
        out->scores[i] = (int16_t)(OUTPUT[i][0] * hidden[0] + OUTPUT[i][1] * hidden[1] + OUTPUT[i][2] * hidden[2]);
        if (i == 0u || out->scores[i] > best) best = out->scores[i];
    }
    for (i = 0; i < SIM_LABELS; ++i) {
        if (out->scores[i] < best && (second == 0 || out->scores[i] > second)) second = out->scores[i];
    }
    out->label = SIM_LABEL_UNKNOWN;
    for (i = 0; i < SIM_LABELS; ++i) if (out->scores[i] == best) { out->label = (sim_label_t)i; break; }
    {
        int32_t margin = (int32_t)best - (int32_t)second;
        int32_t confidence = 500 + margin * 25;
        if (confidence < 0) confidence = 0;
        if (confidence > (int32_t)SIM_CONFIDENCE_MAX) confidence = SIM_CONFIDENCE_MAX;
        out->confidence_milli = (uint16_t)confidence;
    }
}

static sim_representation_t choose_representation(const sim_host_budget_t *host,
                                                    uint16_t required_bytes, uint8_t required_steps) {
    if (host->supports_int8 && host->budget_bytes >= 4096u && host->budget_steps >= 12u && required_bytes >= 4096u && required_steps >= 12u) return SIM_REP_INT8;
    if (host->supports_hdc8 && host->budget_bytes >= 2048u && host->budget_steps >= 8u && required_bytes >= 2048u && required_steps >= 8u) return SIM_REP_HDC8;
    if (host->supports_rules && host->budget_bytes >= 512u && host->budget_steps >= 4u && required_bytes >= 512u && required_steps >= 4u) return SIM_REP_RULES;
    return SIM_REP_NONE;
}

sim_decision_t sim_decide(const sim_host_budget_t *host, const sim_pattern_t *pattern,
                          uint16_t required_bytes, uint8_t required_steps,
                          uint16_t minimum_confidence) {
    sim_decision_t decision = {SIM_LABEL_UNKNOWN, SIM_REP_NONE, 0u, 0u, 0u};
    sim_neural_scores_t scores;
    if (host == NULL || pattern == NULL || !host->authority_none) return decision;
    decision.representation = choose_representation(host, required_bytes, required_steps);
    if (decision.representation == SIM_REP_NONE) return decision;
    sim_infer(pattern, &scores);
    decision.label = scores.label;
    decision.confidence_milli = scores.confidence_milli;
    if (!pattern->provenance_present) {
        decision.label = SIM_LABEL_UNKNOWN;
        return decision;
    }
    if (scores.label != SIM_LABEL_UNKNOWN && scores.confidence_milli >= minimum_confidence) decision.proposal = 1u;
    /* execution intentionally remains zero: SIM is never an authority gate. */
    return decision;
}
