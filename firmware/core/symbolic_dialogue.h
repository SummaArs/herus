/*
 * HERUS symbolic_dialogue — typed local conversation over verified inference.
 *
 * The dialogue surface is deliberately semantic. A future language adapter maps
 * speech, text or HCP meanings into patterns; this layer never stores raw text,
 * audio or embeddings. Personal facts require explicit authority before entering
 * the reasoner. Asking a question is read-only and cannot transmit or act.
 */
#ifndef HERUS_SYMBOLIC_DIALOGUE_H
#define HERUS_SYMBOLIC_DIALOGUE_H

#include "symbolic_reasoner.h"
#include "resonator_bridge.h"

#define SD_MAX_DERIVATION_STEPS 64u

enum {
    SD_OK = 0,
    SD_E_ARG = -1,
    SD_E_AUTH = -2,
    SD_E_LIMIT = -3,
    SD_E_FORMAT = -4,
    SD_E_ABSTAIN = -5
};

typedef struct {
    sr_reasoner_t reasoner;
    uint16_t turn;
    uint8_t active;
} sd_dialogue_t;

typedef struct {
    int status;
    uint16_t turn;
    sr_answer_t answer;
} sd_reply_t;

void sd_init(sd_dialogue_t *dialogue);

/* Rules are compiled knowledge, not personal memory. */
int sd_add_rule(sd_dialogue_t *dialogue, const sr_rule_t *rule);

/* Personal facts require a physical/explicit confirmation bit. */
int sd_add_personal_fact(sd_dialogue_t *dialogue, sr_fact_t fact,
                         uint8_t explicit_memory_confirmation);

int sd_ask(sd_dialogue_t *dialogue, const sr_pattern_t *query,
           uint32_t derivation_budget, sd_reply_t *out);

/* Propose one missing ground fact for a goal after bounded saturation. The
 * proposal is read-only and must never be inserted as personal knowledge. */
int sd_abduce(sd_dialogue_t *dialogue, const sr_pattern_t *ground_goal,
              uint32_t derivation_budget, sr_abduction_t *out);

/* VSA is an evidence producer only. The proposal remains transient until the
 * caller presents and physically accepts it. */
int sd_propose_vsa_relation(sd_dialogue_t *dialogue, const hv_t *product,
                            const rv_problem_t *problem, uint8_t negated,
                            rb_proposal_t *out);
int sd_accept_vsa_proposal(sd_dialogue_t *dialogue,
                           const rb_proposal_t *proposal,
                           uint8_t explicit_confirmation);

#endif /* HERUS_SYMBOLIC_DIALOGUE_H */
