#include "symbolic_dialogue.h"
#include <string.h>

void sd_init(sd_dialogue_t *dialogue)
{
    if (!dialogue) return;
    memset(dialogue, 0, sizeof(*dialogue));
    sr_init(&dialogue->reasoner);
    dialogue->active = 1u;
}

int sd_add_rule(sd_dialogue_t *dialogue, const sr_rule_t *rule)
{
    if (!dialogue || !rule || dialogue->active != 1u) return SD_E_ARG;
    switch (sr_add_rule(&dialogue->reasoner, rule)) {
    case SR_OK:
    case SR_NO_CHANGE:
        return SD_OK;
    case SR_E_FORMAT:
        return SD_E_FORMAT;
    default:
        return SD_E_ARG;
    }
}

int sd_add_personal_fact(sd_dialogue_t *dialogue, sr_fact_t fact,
                         uint8_t explicit_memory_confirmation)
{
    int result;
    if (!dialogue || dialogue->active != 1u) return SD_E_ARG;
    if (explicit_memory_confirmation != 1u) return SD_E_AUTH;
    result = sr_add_fact(&dialogue->reasoner, fact);
    if (result == SR_OK || result == SR_NO_CHANGE) return SD_OK;
    if (result == SR_E_FORMAT) return SD_E_FORMAT;
    return SD_E_ARG;
}

int sd_ask(sd_dialogue_t *dialogue, const sr_pattern_t *query,
           uint32_t derivation_budget, sd_reply_t *out)
{
    int result;
    if (!dialogue || !query || !out || dialogue->active != 1u ||
        derivation_budget == 0u) return SD_E_ARG;
    memset(out, 0, sizeof(*out));
    dialogue->turn++;
    out->turn = dialogue->turn;
    result = sr_saturate(&dialogue->reasoner, derivation_budget);
    if (result == SR_E_LIMIT) {
        out->status = SD_E_LIMIT;
        out->answer.kind = SR_ANSWER_LIMIT;
        return SD_E_LIMIT;
    }
    if (result != SR_OK) {
        out->status = SD_E_FORMAT;
        out->answer.kind = SR_ANSWER_NONE;
        return SD_E_FORMAT;
    }
    result = sr_query(&dialogue->reasoner, query, &out->answer);
    out->status = result == SR_OK ? SD_OK : result;
    return out->status;
}
