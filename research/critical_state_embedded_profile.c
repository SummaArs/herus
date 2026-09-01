#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define STATE_COUNT 2u
#define INPUT_COUNT 2u
#define ACTION_COUNT 2u
#define MAX_STEPS 4u

enum state { NOMINAL = 0, SAFE_HOLD = 1 };
enum input { INPUT_OK = 0, INPUT_LOSS = 1 };
enum action { HOLD = 0, ENTER_SAFE_HOLD = 1 };

struct transition {
    unsigned char next_state;
    unsigned char valid;
};

static const struct transition transitions[STATE_COUNT][INPUT_COUNT][ACTION_COUNT] = {
    {{{NOMINAL, 1u}, {SAFE_HOLD, 0u}}, {{NOMINAL, 0u}, {SAFE_HOLD, 1u}}},
    {{{SAFE_HOLD, 0u}, {SAFE_HOLD, 1u}}, {{SAFE_HOLD, 0u}, {SAFE_HOLD, 1u}}}
};

static const unsigned char policy[STATE_COUNT][INPUT_COUNT] = {
    {HOLD, ENTER_SAFE_HOLD},
    {ENTER_SAFE_HOLD, ENTER_SAFE_HOLD}
};

static bool verify_fixed_policy(void) {
    unsigned char frontier[STATE_COUNT] = {NOMINAL, SAFE_HOLD};
    unsigned char next[STATE_COUNT] = {0u, 0u};
    unsigned char seen[STATE_COUNT] = {0u, 0u};
    size_t frontier_count = 1u;
    frontier[0] = NOMINAL;

    for (size_t depth = 0u; depth <= MAX_STEPS; ++depth) {
        size_t next_count = 0u;
        seen[0] = 0u;
        seen[1] = 0u;
        for (size_t i = 0u; i < frontier_count; ++i) {
            unsigned char state = frontier[i];
            for (size_t input = 0u; input < INPUT_COUNT; ++input) {
                unsigned char action = policy[state][input];
                const struct transition *edge = &transitions[state][input][action];
                if (edge->valid == 0u || edge->next_state >= STATE_COUNT) {
                    return false;
                }
                if (seen[edge->next_state] == 0u) {
                    if (next_count >= STATE_COUNT) {
                        return false;
                    }
                    seen[edge->next_state] = 1u;
                    next[next_count++] = edge->next_state;
                }
            }
        }
        frontier_count = next_count;
        for (size_t i = 0u; i < frontier_count; ++i) {
            frontier[i] = next[i];
        }
    }
    return true;
}

int main(void) {
    const bool verified = verify_fixed_policy();
    printf("verified=%u static_bytes=%zu steps=%u\n",
           verified ? 1u : 0u, sizeof(transitions) + sizeof(policy), MAX_STEPS);
    return verified ? 0 : 1;
}
