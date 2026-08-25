/* test_hsca_ladder.c — one meaning, every channel.
 *
 * The claim under test is the whole reason HSCA exists: at 34 bytes a meaning
 * fits every rung of the ladder, and four seconds of compressed speech does not.
 * If that stops being true the paradigm is gone, so it is asserted over the
 * table rather than written in a document.
 */
#include "ladder.h"
#include "hir.h"
#include <stdio.h>
#include <string.h>

static int pass_count;
static int fail_count;

static void check(int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) pass_count++; else fail_count++;
}

#define VOICE_4S_BYTES 178u   /* Codec2 700C, 4 s, 2 fragments (docs sec.5) */

static void run_paradigm(void)
{
    uint8_t i, meaning_rungs = 0, voice_rungs = 0;
    int all_fit = 1;

    for (i = 0; i < LDR_COUNT; i++) {
        const ldr_profile_t *p = ldr_profile(i);
        if (p->max_payload_bytes < HIR_ONAIR_BYTES) all_fit = 0;
        if (ldr_carries(i, HIR_ONAIR_BYTES, 0u)) meaning_rungs++;
        if (ldr_carries(i, VOICE_4S_BYTES, 0u)) voice_rungs++;
    }
    printf("  ---- a 34-byte meaning rides %u/%u rungs; 4 s of speech rides %u/%u\n",
           meaning_rungs, (unsigned)LDR_COUNT, voice_rungs, (unsigned)LDR_COUNT);
    check(all_fit, "every carrier in the table accepts a full 34-byte meaning");
    check(meaning_rungs == LDR_COUNT, "the meaning ladder has no missing rung");
    check(voice_rungs < LDR_COUNT, "speech does not fit every rung: this is the difference");
    check(!ldr_carries(LDR_GLYPH, VOICE_4S_BYTES, 0u) &&
          !ldr_carries(LDR_SOUND, VOICE_4S_BYTES, 0u) &&
          !ldr_carries(LDR_DRIFT, VOICE_4S_BYTES, 0u),
          "optical, acoustic and custody carry meaning but cannot carry speech");
    check(ldr_carries(LDR_LORA, HIR_ONAIR_BYTES, 400u),
          "a meaning crosses LoRa inside the 400 ms dwell ceiling");
    check(!ldr_carries(LDR_SAT, HIR_ONAIR_BYTES, 1000u),
          "a satellite burst is not a low-latency rung and does not pretend to be");
}

static void run_honesty(void)
{
    check(ldr_unmeasured_count() == LDR_COUNT,
          "no rung claims a measured number: every figure is declared, none is field data");
    check(ldr_profile(LDR_COUNT) == NULL, "an out-of-range carrier has no profile");
    check(strcmp(ldr_name(LDR_COUNT), "?") == 0, "an out-of-range carrier has no name");
}

static void base_facts(ldr_facts_t *f)
{
    uint8_t i;
    memset(f, 0, sizeof *f);
    f->payload_class = LDR_CLASS_ESSENTIAL;
    f->payload_bytes = HIR_ONAIR_BYTES;
    f->urgency = HIR_URG_ROTINA;
    f->peer_paired = 1u;
    f->core_present = 1u;
    f->line_of_sight = 1u;
    f->peer_distance_m = 0u;
    for (i = 0; i < LDR_COUNT; i++) f->available[i] = 1u;
}

static int plan_has(const ldr_plan_t *p, uint8_t carrier)
{
    uint8_t i;
    for (i = 0; i < p->count; i++) if (p->carrier[i] == carrier) return 1;
    return 0;
}

static void run_eligibility(void)
{
    ldr_facts_t f;
    ldr_plan_t p;

    base_facts(&f);
    f.core_present = 0u;
    check(ldr_plan(&f, &p) == LDR_OK, "a plan exists with the Core switched off");
    check(!plan_has(&p, LDR_SAT), "the satellite rung disappears without the Core antenna");
    check(plan_has(&p, LDR_LORA) && plan_has(&p, LDR_DRIFT),
          "the wrist keeps its own long rungs when the Core is absent");

    base_facts(&f);
    f.peer_paired = 0u;
    check(ldr_plan(&f, &p) == LDR_OK, "a plan exists with no paired peer");
    check(!plan_has(&p, LDR_LORA) && !plan_has(&p, LDR_BLE) && !plan_has(&p, LDR_ESPNOW),
          "rungs that need a provisioned peer drop out when there is none");
    check(plan_has(&p, LDR_TOUCH) && plan_has(&p, LDR_GLYPH) && plan_has(&p, LDR_SOUND),
          "touch, glyph and sound still work with a stranger");

    base_facts(&f);
    f.line_of_sight = 0u;
    check(ldr_plan(&f, &p) == LDR_OK && !plan_has(&p, LDR_GLYPH) && !plan_has(&p, LDR_SAT),
          "rungs that need line of sight drop out in a pocket");

    base_facts(&f);
    f.payload_class = LDR_CLASS_TELEMETRY;
    f.payload_bytes = 64u;
    check(ldr_plan(&f, &p) == LDR_OK, "consented telemetry can be planned");
    check(!plan_has(&p, LDR_LORA) && !plan_has(&p, LDR_DRIFT) && !plan_has(&p, LDR_SAT),
          "a carrier never carries a class above the one it declares");

    base_facts(&f);
    f.payload_bytes = 900u;
    f.payload_class = LDR_CLASS_BULK;
    check(ldr_plan(&f, &p) == LDR_OK && p.count == 1u && p.carrier[0] == LDR_WIFI,
          "a bulk package has exactly one rung, and it is the local one");
    check(p.widest_reach_dropped == 0u,
          "when nothing wider was possible the plan does not cry wolf");

    base_facts(&f);
    f.line_of_sight = 0u;
    check(ldr_plan(&f, &p) == LDR_OK && p.widest_reach_dropped == 1u,
          "the plan admits when a wider rung existed but was not eligible");

    base_facts(&f);
    {
        uint8_t i;
        for (i = 0; i < LDR_COUNT; i++) f.available[i] = 0u;
    }
    check(ldr_plan(&f, &p) == LDR_E_NO_ROUTE && p.count == 0u,
          "no available carrier is an explicit refusal, not a default");
}

static void run_distance(void)
{
    ldr_facts_t f;
    ldr_plan_t p;

    base_facts(&f);
    f.peer_distance_m = 0u;
    check(ldr_plan(&f, &p) == LDR_OK && plan_has(&p, LDR_TOUCH),
          "with the other person in front of you, a tap is a route");

    base_facts(&f);
    f.peer_distance_m = 4000u;
    check(ldr_plan(&f, &p) == LDR_OK, "a route still exists at four kilometres");
    check(!plan_has(&p, LDR_TOUCH) && !plan_has(&p, LDR_GLYPH) && !plan_has(&p, LDR_SOUND) &&
          !plan_has(&p, LDR_BLE) && !plan_has(&p, LDR_LORA) && !plan_has(&p, LDR_LORA_MESH),
          "no rung shorter than the distance is offered: reach is checked, not hoped for");
    check(plan_has(&p, LDR_DRIFT) && plan_has(&p, LDR_SAT),
          "what is left at four kilometres is custody and, with a Core, the sky");

    base_facts(&f);
    f.peer_distance_m = 500u;
    check(ldr_plan(&f, &p) == LDR_OK && plan_has(&p, LDR_LORA) && !plan_has(&p, LDR_BLE),
          "at five hundred metres LoRa is in and Bluetooth is out");

    base_facts(&f);
    f.peer_distance_m = 40000000u;
    f.core_present = 0u;
    check(ldr_plan(&f, &p) == LDR_E_NO_ROUTE,
          "beyond every declared reach the ladder refuses instead of guessing");
}

static void run_ordering(void)
{
    ldr_facts_t f;
    ldr_plan_t routine, urgent;

    base_facts(&f);
    f.urgency = HIR_URG_ROTINA;
    check(ldr_plan(&f, &routine) == LDR_OK, "routine plan builds");
    base_facts(&f);
    f.urgency = HIR_URG_URGENTE;
    check(ldr_plan(&f, &urgent) == LDR_OK, "urgent plan builds");

    check(ldr_profile(routine.carrier[0])->energy_cuah <=
          ldr_profile(urgent.carrier[0])->energy_cuah,
          "routine traffic is ordered by energy");
    check(ldr_profile(urgent.carrier[0])->latency_ms <=
          ldr_profile(routine.carrier[0])->latency_ms,
          "urgent traffic is ordered by latency");
    check(routine.carrier[routine.count - 1u] == LDR_DRIFT &&
          urgent.carrier[urgent.count - 1u] == LDR_DRIFT,
          "custody is always the last rung: it costs nothing and never expires early");
    {
        ldr_plan_t again;
        base_facts(&f);
        check(ldr_plan(&f, &again) == LDR_OK &&
              memcmp(again.carrier, routine.carrier, LDR_COUNT) == 0,
              "the same facts always produce the same ladder");
    }
}

static void run_authority(void)
{
    ldr_facts_t f;
    ldr_plan_t p;
    uint8_t chosen = 0xffu;

    base_facts(&f);
    check(ldr_plan(&f, &p) == LDR_OK, "a plan is advice and needs no confirmation");
    check(ldr_commit(&p, 0u, LDR_CLASS_ESSENTIAL, &chosen) == LDR_E_UNCONFIRMED,
          "nothing commits without physical confirmation");
    check(chosen == LDR_COUNT, "a refused commit names no carrier");
    check(ldr_commit(&p, 1u, LDR_CLASS_CARD, &chosen) == LDR_E_CLASS_DRIFT,
          "committing a different class than the one confirmed is refused");
    check(ldr_commit(&p, 1u, LDR_CLASS_ESSENTIAL, &chosen) == LDR_OK,
          "a confirmed commit of the confirmed class succeeds");
    check(chosen == p.carrier[0], "commit picks the top rung and nothing else");
    {
        ldr_plan_t empty;
        memset(&empty, 0, sizeof empty);
        check(ldr_commit(&empty, 1u, LDR_CLASS_ESSENTIAL, &chosen) == LDR_E_EMPTY_PLAN,
              "an empty plan cannot be committed by confirming harder");
    }
    {
        /* changing rung must never change what may be sent */
        ldr_facts_t g;
        ldr_plan_t q;
        base_facts(&g);
        g.available[LDR_LORA] = 0u;
        check(ldr_plan(&g, &q) == LDR_OK && q.payload_class == p.payload_class,
              "losing a carrier does not change the payload class the plan carries");
    }
}

int main(void)
{
    printf("--- ladder: one meaning, every channel ---\n");
    run_paradigm();
    run_honesty();
    run_eligibility();
    run_distance();
    run_ordering();
    run_authority();
    printf("HSCA LADDER: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
