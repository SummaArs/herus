/* Fixed-memory profile for the discrete symbolic optimizer.
 *
 * This is a resource probe, not the trusted proof kernel. It deliberately uses
 * integer polynomial coefficients for the small benchmark and never executes
 * generated text or crosses into firmware authority.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define EPISODES 512u
#define STEPS 6u
#define ACTIONS 3u
#define MAX_SIZE 13u

typedef struct {
    int32_t c0;
    int32_t c1;
    int32_t c2;
    uint8_t size;
} term_t;

typedef struct {
    int32_t q[ACTIONS];
    uint32_t rng;
} policy_t;

static uint32_t next_random(policy_t *p) {
    p->rng = p->rng * 1664525u + 1013904223u;
    return p->rng;
}

static int64_t value_at(const term_t *t, int32_t x) {
    return (int64_t)t->c0 + (int64_t)t->c1 * x +
           (int64_t)t->c2 * x * x;
}

static uint64_t error_for(const term_t *t) {
    static const int32_t xs[] = {-2, 0, 3};
    static const int32_t ys[] = {4, 0, 9};
    uint64_t error = 0;
    for (size_t i = 0; i < sizeof(xs) / sizeof(xs[0]); ++i) {
        int64_t delta = value_at(t, xs[i]) - ys[i];
        error += (uint64_t)(delta < 0 ? -delta : delta);
    }
    return error;
}

static unsigned choose_action(policy_t *p) {
    if ((next_random(p) % 5u) == 0u) {
        return next_random(p) % ACTIONS;
    }
    unsigned best = 0u;
    for (unsigned action = 1u; action < ACTIONS; ++action) {
        if (p->q[action] > p->q[best]) {
            best = action;
        }
    }
    return best;
}

static term_t apply_action(term_t t, unsigned action) {
    if (action == 0u) {
        t.c1 += 1;
    } else if (action == 1u) {
        int32_t old_c0 = t.c0;
        int32_t old_c1 = t.c1;
        t.c0 = 0;
        t.c1 = old_c0;
        t.c2 = old_c1;
    } else {
        t.c0 = -t.c0;
        t.c1 = -t.c1;
        t.c2 = -t.c2;
    }
    t.size = (uint8_t)(t.size + 1u);
    return t;
}

int main(void) {
    policy_t policy = {{0, 0, 0}, 7u};
    term_t best = {0, 1, 0, 1};
    uint64_t best_error = error_for(&best);
    uint32_t evaluations = 0;
    uint32_t episode;

    for (episode = 1; episode <= EPISODES; ++episode) {
        term_t current = {0, 1, 0, 1};
        unsigned chosen[STEPS] = {0};
        unsigned chosen_count = 0;
        int64_t reward = 0;
        for (unsigned step = 0; step < STEPS; ++step) {
            unsigned action = choose_action(&policy);
            term_t candidate = apply_action(current, action);
            if (candidate.size > MAX_SIZE) {
                continue;
            }
            current = candidate;
            chosen[chosen_count++] = action;
            uint64_t error = error_for(&current);
            evaluations++;
            if (error < best_error) {
                best = current;
                best_error = error;
            }
            reward = -(int64_t)error - (int64_t)current.size;
            if (error == 0u) {
                break;
            }
        }
        for (unsigned i = 0; i < chosen_count; ++i) {
            unsigned action = chosen[i];
            policy.q[action] += (int32_t)((reward - policy.q[action]) / 5);
        }
        if (best_error == 0u) {
            break;
        }
    }

    printf("episodes=%u evaluations=%u best_error=%llu term=(%d,%d,%d)\n",
           episode, evaluations, (unsigned long long)best_error,
           best.c0, best.c1, best.c2);
    printf("sizeof_term=%zu sizeof_policy=%zu static_bytes=%zu\n",
           sizeof(term_t), sizeof(policy_t), sizeof(term_t) + sizeof(policy_t));
    return best_error == 0u ? EXIT_SUCCESS : EXIT_FAILURE;
}
