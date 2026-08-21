/* main.c — the bench.
 *
 *   ./build/herus-sim              every scenario, and a verdict
 *   ./build/herus-sim relay -v     one scenario, with each message printed
 *   ./build/herus-sim crowd --nodes 24 --per-hour 60
 *
 * The exit code is the number of failed invariants, so this is usable from a
 * script and from CI without parsing the output.
 */
#include "sim.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *name;
    void (*fn)(sim_score *, int, char **);
    const char *what;
} entry_t;

static void run_selftest(sim_score *s, int argc, char **argv)
{ (void)argc; (void)argv; scenario_selftest(s); }

static const entry_t TABLE[] = {
    { "selftest",  run_selftest,        "does the world agree with tools/budget.py" },
    { "range",     scenario_range,      "how far a meaning carries, in metres"      },
    { "relay",     scenario_relay,      "three hops, and what a relay may know"     },
    { "crowd",     scenario_crowd,      "N units in one slot"                       },
    { "attack",    scenario_attack,     "replay, forgery and jamming"               },
    { "day",       scenario_day,        "24 h of battery and latency"               },
    { "babel",     scenario_babel,      "one frame, three languages"                },
    { "drift",     scenario_drift,      "what the crystal costs after silence"      },
    { "cognition", scenario_cognition,  "unbinding a role from a bundle"            },
    /* Written to fail. See stress.c. */
    { "deaf",      scenario_deaf,       "STRESS: recovery after a long outage"      },
    { "drain",     scenario_drain,      "STRESS: a stranger against a leaf battery" },
    { "unmask",    scenario_unmask,     "STRESS: does relay silence name the recipient" },
    { "birthday",  scenario_birthday,   "STRESS: what E-P2 cost in collisions"      },
    { "reliable",  scenario_reliable,   "STRESS: the retry single-use keys forbid"  },
    { "reach",     scenario_reach,      "RANGE: SF10 at 24 B, the regulatory ceiling" },
    { "hardware",  scenario_hardware,   "RANGE: what each part is worth, in dB and mA" },
    { "compose",   scenario_compose,    "MIND: how many button presses is a sentence" },
    { "study",     scenario_study,      "MIND: recall with honest refusal, for a concurseiro" },
    { "learn",     scenario_learn,      "MIND: the residue loop — it learns what it failed at" },
    { "virtual",   scenario_virtual,    "PRE-HARDWARE: Watch, Paper-Core, sensors, battery and LoRa" },
    { "personal",  scenario_personal,   "PRE-HARDWARE: personal semantic life, attention, energy and contact" },
    { "semantic-life", scenario_semantic_life, "PRE-HARDWARE: continuous preferences, goals, memory, reboot and abstention" },
    { "physical-faults", scenario_physical_faults, "PRE-HARDWARE: adapter loss, interruption, contact and energy failure" },
    { "authority", scenario_authority, "AGSC: provenance, memory, abstention and non-amplifying action authority" },
    { "authority-benchmark", scenario_authority_benchmark, "AGSC: baselines for false memory, conflict, stale facts, reboot and action" },
    { "adaptive-change", scenario_adaptive_change, "AGSC-D: supersession, revocation, expiry, drift and epoch continuity" },
};
#define NTABLE ((int)(sizeof TABLE / sizeof *TABLE))

static void usage(void)
{
    printf("herus-sim — a world for the Herus firmware to live in\n\n");
    printf("usage: herus-sim [scenario ...] [options]\n\n");
    printf("scenarios (default: all of them, in order)\n");
    for (int i = 0; i < NTABLE; i++)
        printf("  %-11s %s\n", TABLE[i].name, TABLE[i].what);
    printf("\ncommon options\n");
    printf("  --verbose 1        print every delivered message as it arrives\n");
    printf("  --msgs N           messages to offer (range, relay, attack)\n");
    printf("  --band 0|1         capsule antenna or Band antenna (range)\n");
    printf("  --clutter DB       22 urban, 12 suburban, 0 open (range)\n");
    printf("  --shadow DB        log-normal sigma; 0 reproduces the docs exactly\n");
    printf("  --gap M            metres between hops (relay)\n");
    printf("  --nodes N          units in the crowd\n");
    printf("  --minutes N        crowd duration\n");
    printf("  --per-hour N       messages per unit per hour (crowd, day)\n");
    printf("  --channels N       hop channels; a group hops together (crowd)\n");
    printf("\nthe exit code is the number of failed invariants.\n");
}

int main(int argc, char **argv)
{
    if (argc > 1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        usage();
        return 0;
    }

    printf("HERUS — simulated bench\n");
    printf("frame %u B, %.1f ms at SF9, dwell limit %u ms, D = %d bits\n",
           HERUS_FRAME_LEN, HERUS_AIRTIME_MEANING_US / 1000.0,
           HZ_DWELL_LIMIT_US / 1000, HV_BITS);
    printf("the firmware under test is firmware/core and firmware/net, unmodified.\n");

    /* Which scenarios? Any bare argument that names one. */
    int chosen[NTABLE]; int nchosen = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') { i++; continue; }      /* skip option + value */
        for (int k = 0; k < NTABLE; k++)
            if (!strcmp(argv[i], TABLE[k].name)) { chosen[nchosen++] = k; break; }
    }
    if (nchosen == 0) for (int k = 0; k < NTABLE; k++) chosen[nchosen++] = k;

    sim_score s = { 0, 0 };
    for (int i = 0; i < nchosen; i++)
        TABLE[chosen[i]].fn(&s, argc, argv);

    printf("\n==================================================\n");
    if (s.fail == 0)
        printf("ALL %d INVARIANTS HOLD IN SIMULATION\n", s.pass);
    else
        printf("%d PASS, %d FAIL — the world disagrees with the design\n", s.pass, s.fail);
    printf("==================================================\n");
    printf("\nWhat this does and does not prove: every byte on air was produced and\n"
           "opened by the shipping firmware, so a delivery here is a real decode.\n"
           "Propagation, sensitivity, capture, crystal drift and current draw are\n"
           "models, and models are where a simulator lies. The numbers that decide\n"
           "the project are still Phase 0 — a printed shell and a walked street.\n");
    return s.fail;
}
