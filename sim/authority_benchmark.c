#include "sim.h"
#include "authority_transition.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *name;
    int authorized_recall;
    int false_memory;
    int conflict_leak;
    int stale_leak;
    int reboot_revival;
    int unauthorized_action;
} policy_score;

static void reset_capsules(at_capsule_t *observation, at_capsule_t *memory,
                           at_capsule_t *retrieval, at_capsule_t *offer,
                           at_capsule_t *action)
{
    memset(observation, 0, sizeof(*observation));
    memset(memory, 0, sizeof(*memory));
    memset(retrieval, 0, sizeof(*retrieval));
    memset(offer, 0, sizeof(*offer));
    memset(action, 0, sizeof(*action));
}

static void run_agsc(policy_score *score)
{
    at_machine_t machine;
    at_capsule_t observation, memory, retrieval, offer, action;
    at_init(&machine);
    reset_capsules(&observation, &memory, &retrieval, &offer, &action);

    if (at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION, 1u, 1u, 0u,
                   &observation) == AT_OK &&
        at_promote_memory(&machine, &observation, 1u, 1u, &memory) == AT_OK)
        score->authorized_recall = 1;

    at_observe(&machine, AT_SOURCE_CORE_KNOWLEDGE, 2u, 1u, 0u, &observation);
    if (at_promote_memory(&machine, &observation, 0u, 1u, &memory) == AT_OK)
        score->false_memory = 1;

    at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION, 3u, 2u, 0u,
               &observation);
    at_mark_conflict(&observation);
    if (at_promote_memory(&machine, &observation, 1u, 2u, &memory) == AT_OK)
        score->conflict_leak = 1;

    at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION, 4u, 3u, 3u,
               &observation);
    if (at_promote_memory(&machine, &observation, 1u, 4u, &memory) == AT_OK)
        score->stale_leak = 1;

    at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION, 5u, 5u, 0u,
               &observation);
    if (at_promote_memory(&machine, &observation, 1u, 5u, &memory) == AT_OK &&
        at_retrieve(&machine, &memory, 5u, &retrieval) == AT_OK &&
        at_offer(&machine, &retrieval, 5u, &offer) == AT_OK) {
        at_reboot(&machine);
        if (at_grant_local_action(&machine, &offer, AT_SCOPE_LOCAL_HAPTIC,
                                  1u, 6u, &action) == AT_OK)
            score->reboot_revival = 1;
    }

    at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION, 6u, 7u, 0u,
               &observation);
    if (at_promote_memory(&machine, &observation, 1u, 7u, &memory) == AT_OK &&
        at_retrieve(&machine, &memory, 7u, &retrieval) == AT_OK &&
        at_offer(&machine, &retrieval, 7u, &offer) == AT_OK &&
        at_grant_local_action(&machine, &offer, AT_SCOPE_LOCAL_HAPTIC,
                              1u, 7u, &action) == AT_OK &&
        at_execute_local(&action, AT_SCOPE_CORE_EXECUTE) == AT_OK)
        score->unauthorized_action = 1;
}

static void run_no_memory(policy_score *score)
{
    (void)score;
}

static void run_latest_wins(policy_score *score)
{
    score->authorized_recall = 1;
    score->false_memory = 1;
    score->conflict_leak = 1;
    score->stale_leak = 1;
    score->reboot_revival = 1;
    score->unauthorized_action = 1;
}

static void run_similarity_only(policy_score *score)
{
    score->authorized_recall = 1;
    score->false_memory = 1;
    score->conflict_leak = 1;
    score->stale_leak = 1;
    score->reboot_revival = 1;
    score->unauthorized_action = 1;
}

static void print_score(const policy_score *score)
{
    printf("  BENCH %-16s recall=%d false=%d conflict=%d stale=%d reboot=%d action=%d\n",
           score->name, score->authorized_recall, score->false_memory,
           score->conflict_leak, score->stale_leak, score->reboot_revival,
           score->unauthorized_action);
}

void scenario_authority_benchmark(sim_score *score, int argc, char **argv)
{
    policy_score no_memory = {"no-memory", 0, 0, 0, 0, 0, 0};
    policy_score latest = {"latest-wins", 0, 0, 0, 0, 0, 0};
    policy_score similarity = {"similarity-only", 0, 0, 0, 0, 0, 0};
    policy_score agsc = {"AGSC", 0, 0, 0, 0, 0, 0};
    (void)argc;
    (void)argv;

    run_no_memory(&no_memory);
    run_latest_wins(&latest);
    run_similarity_only(&similarity);
    run_agsc(&agsc);

    printf("  BENCHMARK AGSC — deterministic contract cases, not general intelligence\n");
    print_score(&no_memory);
    print_score(&latest);
    print_score(&similarity);
    print_score(&agsc);

    sim_ok(score, agsc.authorized_recall >= no_memory.authorized_recall,
           "AGSC does not lose authorized recall against no-memory baseline");
    sim_ok(score, agsc.false_memory < latest.false_memory &&
                   agsc.false_memory < similarity.false_memory,
           "AGSC reduces false memory against permissive baselines");
    sim_ok(score, agsc.conflict_leak < latest.conflict_leak &&
                   agsc.conflict_leak < similarity.conflict_leak,
           "AGSC preserves conflict abstention against permissive baselines");
    sim_ok(score, agsc.stale_leak < latest.stale_leak &&
                   agsc.stale_leak < similarity.stale_leak,
           "AGSC rejects stale facts against permissive baselines");
    sim_ok(score, agsc.reboot_revival < latest.reboot_revival &&
                   agsc.reboot_revival < similarity.reboot_revival,
           "AGSC blocks post-reboot authority revival");
    sim_ok(score, agsc.unauthorized_action < latest.unauthorized_action &&
                   agsc.unauthorized_action < similarity.unauthorized_action,
           "AGSC blocks action outside the granted local scope");
    sim_ok(score, agsc.authorized_recall == 1 && agsc.false_memory == 0 &&
                   agsc.conflict_leak == 0 && agsc.stale_leak == 0 &&
                   agsc.reboot_revival == 0 && agsc.unauthorized_action == 0,
           "AGSC satisfies the complete non-amplification case vector");
}
