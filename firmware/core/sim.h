#ifndef HERUS_SIM_H
#define HERUS_SIM_H

#include <stdint.h>

#define SIM_FEATURES 4u
#define SIM_LABELS 4u
#define SIM_CONFIDENCE_MAX 1000u

typedef enum {
    SIM_LABEL_ARRIVE = 0,
    SIM_LABEL_HELP = 1,
    SIM_LABEL_CANCEL = 2,
    SIM_LABEL_UNKNOWN = 3
} sim_label_t;

typedef enum {
    SIM_REP_NONE = 0,
    SIM_REP_INT8,
    SIM_REP_HDC8,
    SIM_REP_RULES
} sim_representation_t;

typedef struct {
    int8_t features[SIM_FEATURES];
    uint8_t provenance_present;
} sim_pattern_t;

typedef struct {
    int16_t scores[SIM_LABELS];
    sim_label_t label;
    uint16_t confidence_milli;
} sim_neural_scores_t;

typedef struct {
    sim_label_t label;
    sim_representation_t representation;
    uint16_t confidence_milli;
    uint8_t proposal;
    uint8_t execution;
} sim_decision_t;

typedef struct {
    uint16_t budget_bytes;
    uint8_t budget_steps;
    uint8_t authority_none;
    uint8_t supports_int8;
    uint8_t supports_hdc8;
    uint8_t supports_rules;
} sim_host_budget_t;

void sim_infer(const sim_pattern_t *pattern, sim_neural_scores_t *out);
sim_decision_t sim_decide(const sim_host_budget_t *host, const sim_pattern_t *pattern,
                          uint16_t required_bytes, uint8_t required_steps,
                          uint16_t minimum_confidence);

#endif
