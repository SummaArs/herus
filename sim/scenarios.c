/* scenarios.c — the questions worth asking a Herus that does not exist yet.
 *
 * Each scenario ends in invariant lines, not in prose, for the same reason
 * prove.sh does: a figure nobody can fail is decoration. The difference is that
 * prove.sh proves properties of the CODE, and this file proves properties of the
 * SYSTEM — the code plus distance, plus a duty cycle, plus other people talking
 * at the same time, plus somebody hostile.
 *
 * A scenario that always passes is worthless, so several of these are built to
 * fail when the design is wrong: drift fails without resync, range fails past
 * the link budget, crowd fails when the slot is oversubscribed. What they must
 * never do is deliver something false, and that is the one invariant every
 * scenario checks.
 */
#include "sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void sim_ok(sim_score *s, int cond, const char *what)
{
    printf("  %s  %s\n", cond ? "PASS" : "FAIL", what);
    if (cond) s->pass++; else s->fail++;
}

static void hdr(const char *t)
{
    printf("\n%s\n", t);
    for (size_t i = 0; i < strlen(t); i++) putchar('-');
    putchar('\n');
}

int opt_int(int argc, char **argv, const char *k, int def)
{
    for (int i = 0; i < argc - 1; i++) if (!strcmp(argv[i], k)) return atoi(argv[i + 1]);
    return def;
}

double opt_dbl(int argc, char **argv, const char *k, double def)
{
    for (int i = 0; i < argc - 1; i++) if (!strcmp(argv[i], k)) return atof(argv[i + 1]);
    return def;
}

/* ===================================================================== 0 == */
/* The simulator has to be trusted before its output means anything. This
 * re-derives, from the constants in sim.h, the figures that docs/ publishes from
 * tools/budget.py. If they have drifted apart, everything below is fiction. */
void scenario_selftest(sim_score *s)
{
    hdr("0. self-test — does the world agree with tools/budget.py?");

    sim_channel urban = { SIM_CLUTTER_URBAN, 0.0, 1, -1 };

    /* Agreement is checked at the parameters docs/ was written with — 14 dBm and
     * the bare chip — so that raising the design point cannot quietly break the
     * one link between this bench and the published figures. */
    double doc_caps = sim_range_at(&urban, HERUS_SF_MEANING, SIM_G_CAPSULE_DBI,
                                   SIM_TX_DBM_DOC, SIM_NF_CHIP);
    double doc_band = sim_range_at(&urban, HERUS_SF_MEANING, SIM_G_BAND_DBI,
                                   SIM_TX_DBM_DOC, SIM_NF_CHIP);
    double doc_voice = sim_range_at(&urban, HERUS_SF_VOICE, SIM_G_BAND_DBI,
                                    SIM_TX_DBM_DOC, SIM_NF_CHIP);
    printf("  at the documented 14 dBm / bare chip, the world still says:\n");
    printf("    Tier 0 glyph capsule %6.0f m (docs 365)   Band %6.0f m (docs 650)\n",
           doc_caps, doc_band);
    printf("    Tier 2 voice  Band   %6.0f m (docs 487)   meaning outranges speech by %.0f%%\n",
           doc_voice, 100.0 * (doc_band / doc_voice - 1.0));
    sim_ok(s, fabs(doc_caps - 365.0) < 5.0,  "capsule glyph range reproduces docs (365 m)");
    sim_ok(s, fabs(doc_band - 650.0) < 8.0,  "Band glyph range reproduces docs (650 m)");
    sim_ok(s, fabs(doc_voice - 487.0) < 8.0, "voice range reproduces docs (487 m)");

    for (int sf = 7; sf <= 12; sf++) {
        static const double PUB[] = {-124.5,-127.0,-129.5,-132.0,-134.5,-137.0};
        if (fabs(sim_sens_nf(sf, SIM_NF_CHIP) - PUB[sf-7]) > 0.01) { s->fail++; break; }
        if (sf == 12) sim_ok(s, 1, "the sensitivity model reproduces every SX1262 figure to 0.01 dB");
    }

    double r_glyph_caps = sim_range_m(&urban, HERUS_SF_MEANING, SIM_G_CAPSULE_DBI);
    double r_glyph_band = sim_range_m(&urban, HERUS_SF_MEANING, SIM_G_BAND_DBI);
    double r_voice_caps = sim_range_m(&urban, HERUS_SF_VOICE,   SIM_G_CAPSULE_DBI);
    printf("\n  at the DESIGN point (%.0f dBm, bare chip): capsule %.0f m, Band %.0f m\n",
           SIM_TX_DBM, r_glyph_caps, r_glyph_band);
    sim_ok(s, r_glyph_caps > r_voice_caps,       "meaning outranges speech, at equal antenna");

    /* The two shipping link profiles, and the wall behind the further one. */
    printf("\n  the dwell limit buys bytes or spreading factor, never both:\n");
    for (int k = 0; k < 2; k++) {
        const hz_link_profile_t *P = hz_link(k ? HZ_LINK_REACH : HZ_LINK_RICH);
        sim_channel u = { SIM_CLUTTER_URBAN, 0.0, 1, -1 };
        printf("    %-6s SF%-2u %2u B  %6.1f ms  %6.1f dBm  %u slots  ->  %3.0f m / %3.0f m urban\n",
               P->name, P->sf, P->frame_len, P->airtime_us / 1000.0,
               sim_sens_dbm(P->sf), P->max_slot,
               sim_range_m(&u, P->sf, SIM_G_CAPSULE_DBI),
               sim_range_m(&u, P->sf, SIM_G_BAND_DBI));
    }
    printf("    SF11 fits zero bytes in 400 ms and SF12's preamble alone is 663 ms,\n"
           "    so Reach is the last rung the regulation leaves open.\n\n");
    sim_ok(s, HERUS_AIRTIME_REACH_US < HZ_DWELL_LIMIT_US,
           "P2 Reach (SF10, 24 B) is legal");
    sim_ok(s, HZ_AIRTIME_US(HERUS_SF_REACH, HERUS_REACH_FRAME_LEN + 1u, 1u, 0u, HERUS_CR)
              > HZ_DWELL_LIMIT_US,
           "24 B is the CEILING at SF10 — one more byte is illegal");
    sim_ok(s, HZ_AIRTIME_US(11u, 1u, 1u, 0u, HERUS_CR) > HZ_DWELL_LIMIT_US,
           "SF11 cannot carry a single byte, so there is no rung above Reach");

    uint32_t a_mean = hz_airtime_us(HERUS_SF_MEANING, HERUS_FRAME_LEN, 1, 0, HERUS_CR);
    printf("  34 B at SF9 airtime                     %6.1f ms  (docs: 246.8 ms)\n", a_mean / 1000.0);
    sim_ok(s, a_mean == HERUS_AIRTIME_MEANING_US, "the world uses the firmware's own airtime");
    sim_ok(s, a_mean < HZ_DWELL_LIMIT_US,         "P2 the frame the world transmits is legal");

    /* Energy: the duty-cycle arithmetic must land on the three published
     * figures in weave.h, or every battery claim below is unanchored. */
    struct { const char *n; weave_role_t r; uint32_t per; double doc; } E[] = {
        { "leaf        20 ms / 2.0 s", WEAVE_LEAF,       BEAT_PERIOD_MS,      1.78   },
        { "responsive  20 ms / 0.5 s", WEAVE_RESPONSIVE, BEAT_PERIOD_RESP_MS, 5.60   },
        { "relay       continuous   ", WEAVE_RELAY,      BEAT_PERIOD_MS,      127.68 },
    };
    for (int i = 0; i < 3; i++) {
        sim_world w; sim_world_init(&w, 1, 1);
        w.n[0].period_ms = E[i].per;
        weave_init(&w.n[0].weave, E[i].r);
        w.end_us = 86400ull * 1000000ull;
        sim_settle_energy(&w);
        double mah = sim_mah_per_day(&w.n[0], w.end_us);
        char buf[96];
        snprintf(buf, sizeof buf, "%s -> %6.2f mAh/day (weave.h: %.2f)",
                 E[i].n, mah, E[i].doc);
        sim_ok(s, fabs(mah - E[i].doc) < 0.05, buf);
        sim_world_free(&w);
    }
}

/* ===================================================================== 1 == */
/* Where does the link actually stop? Not where the folklore says — where the
 * two-ray budget says, walked one step at a time with the real crypto running. */
void scenario_range(sim_score *s, int argc, char **argv)
{
    hdr("1. range — how far does a meaning carry, in metres");

    int    band   = opt_int(argc, argv, "--band", 0);
    int    msgs   = opt_int(argc, argv, "--msgs", 12);
    double shadow = opt_dbl(argc, argv, "--shadow", 0.0);
    double clut   = opt_dbl(argc, argv, "--clutter", SIM_CLUTTER_URBAN);

    printf("  antenna %s, clutter %.0f dB, shadowing sigma %.0f dB, %d messages per step\n",
           band ? "Band (-3 dBi)" : "capsule (-8 dBi)", clut, shadow, msgs);
    printf("\n   distance   delivered   asleep   below sens   false\n");

    int last_good = 0, first_dead = 0;
    for (int d = 50; d <= 1400; d += 50) {
        sim_world w; sim_world_init(&w, 2, 0xBEEF + (uint64_t)d);
        w.ch.clutter_db = clut;
        w.ch.shadow_db  = shadow;
        w.n[0].band = w.n[1].band = band;
        w.n[1].x = (double)d;
        /* Both responsive, so the sweep measures the RADIO and not the duty
         * cycle. The duty cycle is what scenario 5 is for. */
        w.n[0].period_ms = w.n[1].period_ms = BEAT_PERIOD_RESP_MS;
        weave_init(&w.n[0].weave, WEAVE_RESPONSIVE);
        weave_init(&w.n[1].weave, WEAVE_RESPONSIVE);
        sim_pair(&w, 0, 1, 0xA5A5);

        for (int i = 0; i < msgs; i++) {
            uint8_t  rl[1] = { 2 };
            uint16_t fl[1] = { 5 };
            sim_queue_send(&w, (uint64_t)i * 3000000ull, 0, 1, 1, 1, rl, fl, 1);
        }
        w.end_us = (uint64_t)(msgs + 2) * 3000000ull;
        sim_run(&w);

        double pct = 100.0 * w.g_delivered / (double)msgs;
        if (d % 100 == 0 || (pct > 0 && pct < 100))
            printf("   %5d m      %5.0f %%   %6u   %10u   %5u\n",
                   d, pct, w.g_asleep, w.g_below_sens, w.g_false_deliveries);
        if (pct >= 50.0) last_good = d;
        else if (!first_dead) first_dead = d;
        if (w.g_false_deliveries) s->fail++;
        sim_world_free(&w);
    }

    double predicted = sim_range_m(&(sim_channel){ clut, 0, 1, -1 },
                                   HERUS_SF_MEANING,
                                   band ? SIM_G_BAND_DBI : SIM_G_CAPSULE_DBI);
    printf("\n  measured cliff  %d m .. %d m      link budget predicts  %.0f m\n",
           last_good, first_dead, predicted);
    sim_ok(s, last_good <= (int)predicted + 50 && first_dead >= (int)predicted - 50,
           "the measured range matches the link budget, +/- one step");
    sim_ok(s, last_good > 0, "the link works at all at short range");
}

/* ===================================================================== 2 == */
/* The mesh, and the confidentiality claim that makes it acceptable: a relay
 * carries a message it cannot read. */
void scenario_relay(sim_score *s, int argc, char **argv)
{
    hdr("2. relay — three hops, and what the relay is allowed to know");

    int msgs = opt_int(argc, argv, "--msgs", 20);
    int gap  = opt_int(argc, argv, "--gap", 300);      /* metres between hops */

    sim_world w; sim_world_init(&w, 3, 0xC0FFEE);
    w.verbose = opt_int(argc, argv, "--verbose", 0);
    for (int i = 0; i < 3; i++) { w.n[i].band = 0; w.n[i].x = (double)(i * gap); }

    /* B is the relay: continuous receive, which is what the solar Band buys. */
    weave_init(&w.n[0].weave, WEAVE_LEAF);
    weave_init(&w.n[1].weave, WEAVE_RELAY);
    weave_init(&w.n[2].weave, WEAVE_LEAF);

    sim_pair(&w, 0, 2, 0x1111);      /* A and C share a ratchet */
    /* B is deliberately NOT paired with anyone. It has no key, and it will still
     * carry the traffic — that is the whole point of the design. */

    printf("  A at 0 m, relay B at %d m, C at %d m   (direct A->C is %d m)\n",
           gap, 2 * gap, 2 * gap);
    printf("  A and C share a ratchet. B holds no key at all.\n\n");

    for (int i = 0; i < msgs; i++) {
        uint8_t  rl[2] = { 2, 3 };
        uint16_t fl[2] = { 5, 8 };
        sim_queue_send(&w, (uint64_t)i * 6000000ull, 0, 2, 3, 2, rl, fl, 3);
    }
    w.end_us = (uint64_t)(msgs + 3) * 6000000ull;
    sim_run(&w);

    double pct = 100.0 * w.g_delivered / (double)msgs;
    printf("  delivered            %u / %d  (%.0f %%)\n", w.g_delivered, msgs, pct);
    printf("  relayed by B         %u frames\n", w.n[1].n_relayed);
    printf("  B opened             %u frames\n", w.n[1].n_opened);
    printf("  mean latency         %.0f ms   (worst %.0f ms)\n",
           w.lat_n ? (double)w.lat_sum_us / w.lat_n / 1e3 : 0.0,
           (double)w.lat_max_us / 1e3);
    printf("  duplicates suppressed %u\n", w.g_dedup_drop);

    sim_ok(s, w.g_delivered > 0,               "a message crosses a gap no single hop covers");
    sim_ok(s, w.n[1].n_relayed > 0,            "the relay actually relayed");
    sim_ok(s, w.n[1].n_opened == 0,            "the relay never opened a frame it carried");
    sim_ok(s, w.g_false_deliveries == 0,       "nobody received anything addressed elsewhere");
    sim_ok(s, w.n[1].n_relayed <= (uint32_t)msgs * SESS_TTL_MAX,
           "flooding terminates — relays are bounded by ttl, not by luck");
    sim_world_free(&w);
}

/* ===================================================================== 3 == */
/* What happens when a group is in one room. This is the scenario most likely to
 * embarrass the design, which is why it is here. */
void scenario_crowd(sim_score *s, int argc, char **argv)
{
    hdr("3. crowd — N units in one slot, and what the slot can hold");

    int nn    = opt_int(argc, argv, "--nodes", 12);
    int mins  = opt_int(argc, argv, "--minutes", 10);
    int rate  = opt_int(argc, argv, "--per-hour", 30);   /* messages per node per hour */
    if (nn > SIM_MAX_NODES) nn = SIM_MAX_NODES;

    sim_world w; sim_world_init(&w, nn, 0xD15EA5E);
    w.ch.nchan = opt_int(argc, argv, "--channels", 1);
    w.cad      = opt_int(argc, argv, "--cad", 1);
    w.retries  = opt_int(argc, argv, "--retries", 2);
    w.retries  = opt_int(argc, argv, "--retries", 1);
    for (int i = 0; i < nn; i++) {
        w.n[i].x = sim_rand01(&w) * 120.0;               /* all in earshot */
        w.n[i].y = sim_rand01(&w) * 120.0;
        w.n[i].band = 1;
        weave_init(&w.n[i].weave, WEAVE_RESPONSIVE);
        w.n[i].period_ms = BEAT_PERIOD_RESP_MS;
    }
    for (int i = 0; i < nn; i++)
        for (int j = i + 1; j < nn; j++) sim_pair(&w, i, j, 0x2222);

    uint64_t dur = (uint64_t)mins * 60ull * 1000000ull;
    for (int i = 0; i < nn; i++) {
        int k = rate * mins / 60;
        for (int m = 0; m < k; m++) {
            int dst = (int)(sim_rand(&w) % (uint64_t)nn);
            if (dst == i) dst = (dst + 1) % nn;
            uint64_t t = (uint64_t)(sim_rand01(&w) * (double)dur);
            uint8_t  rl[1] = { 1 };
            uint16_t fl[1] = { 1 };
            sim_queue_send(&w, t, i, dst, 1, 1, rl, fl, 2);
        }
    }
    w.end_us = dur + 10000000ull;
    sim_run(&w);

    double util = 100.0 * (double)(w.g_offered + w.g_relay_tx)
                * (double)HERUS_AIRTIME_MEANING_US / (double)dur;
    printf("  %d units, %d minutes, %d msg/unit/hour, %d channel(s)\n", nn, mins, rate, w.ch.nchan);
    printf("  offered              %u\n", w.g_offered);
    printf("  delivered            %u  (%.0f %%)\n", w.g_delivered,
           w.g_offered ? 100.0 * w.g_delivered / w.g_offered : 0.0);
    printf("  lost to collision    %u receptions\n", w.g_collisions);
    printf("  CAD deferrals        %u   (listen before talk: %s)\n",
           w.g_cad_defer, w.cad ? "on" : "off");
    printf("  lost to a closed rx  %u receptions\n", w.g_asleep);
    printf("  relay transmissions  %u   (%.1fx amplification of the offered load)\n",
           w.g_relay_tx, w.g_offered ? (double)w.g_relay_tx / w.g_offered : 0.0);
    printf("  band occupancy       %.1f %% of wall clock\n", util);
    printf("  mean latency         %.0f ms\n",
           w.lat_n ? (double)w.lat_sum_us / w.lat_n / 1e3 : 0.0);

    sim_ok(s, w.g_false_deliveries == 0, "no unit opened a frame addressed to another");
    sim_ok(s, w.g_relay_tx <= (uint32_t)(w.g_offered * nn),
           "flooding terminates — dedup bounds relays by node count");
    sim_ok(s, util < 100.0, "the band is not saturated at this offered load");
    sim_world_free(&w);
}

/* ===================================================================== 4 == */
void scenario_attack(sim_score *s, int argc, char **argv)
{
    hdr("4. attack — a hostile transmitter in the same room");

    int msgs      = opt_int(argc, argv, "--msgs", 40);
    int replays   = opt_int(argc, argv, "--replays", 300);
    int forgeries = opt_int(argc, argv, "--forgeries", 600);
    int jams      = opt_int(argc, argv, "--jams", 60);

    sim_world w; sim_world_init(&w, 3, 0xBADC0DE);
    w.n[0].x = 0;   w.n[0].band = 1;
    w.n[1].x = 100; w.n[1].band = 1;
    w.n[2].x = 50;  w.n[2].band = 1; w.n[2].adversary = 1;   /* J, between them */
    weave_init(&w.n[0].weave, WEAVE_RESPONSIVE);
    weave_init(&w.n[1].weave, WEAVE_RESPONSIVE);
    weave_init(&w.n[2].weave, WEAVE_RELAY);                  /* always listening */
    w.n[0].period_ms = w.n[1].period_ms = BEAT_PERIOD_RESP_MS;
    sim_pair(&w, 0, 1, 0x3333);

    /* Phase 1: honest traffic, adversary listening. */
    uint64_t t = 0;
    for (int i = 0; i < msgs; i++, t += 2000000ull) {
        uint8_t  rl[1] = { 2 };
        uint16_t fl[1] = { 6 };
        sim_queue_send(&w, t, 0, 1, 4, 1, rl, fl, 1);
    }
    /* Phase 2: replay the captured frame, verbatim, over and over. */
    uint64_t t_replay = t + 5000000ull;
    for (int i = 0; i < replays; i++)
        sim_push(&w, t_replay + (uint64_t)i * 400000ull, EV_ADVERSARY, 2, ADV_REPLAY, 1);
    /* Phase 3: keep the address, randomise the body. 2^-64 per attempt is a
     * claim about the tag; the rate limiter is what bounds the attempts. */
    uint64_t t_forge = t_replay + (uint64_t)replays * 400000ull + 5000000ull;
    for (int i = 0; i < forgeries; i++)
        sim_push(&w, t_forge + (uint64_t)i * 300000ull, EV_ADVERSARY, 2, ADV_FORGE, 1);
    /* Phase 4: jam, while honest traffic continues. */
    uint64_t t_jam = t_forge + (uint64_t)forgeries * 300000ull + 5000000ull;
    for (int i = 0; i < jams; i++) {
        sim_push(&w, t_jam + (uint64_t)i * 500000ull, EV_ADVERSARY, 2, ADV_JAM, 1);
        uint8_t rl[1] = { 2 }; uint16_t fl[1] = { 6 };
        sim_queue_send(&w, t_jam + (uint64_t)i * 500000ull, 0, 1, 4, 1, rl, fl, 1);
    }

    w.end_us = t_jam + (uint64_t)jams * 500000ull + 10000000ull;
    sim_run(&w);

    /* With two nodes and no relay, every honest arrival is exactly one open. So
     * "opens beyond deliveries" is precisely the number of hostile frames that
     * got through, and it is the number that must be zero. */
    uint32_t hostile_opens = w.n[1].n_opened > w.g_delivered
                           ? w.n[1].n_opened - w.g_delivered : 0;

    printf("  honest messages offered           %d\n", msgs + jams);
    printf("  delivered                         %u\n", w.g_delivered);
    printf("  adversary captured a frame        %s\n", w.n[2].cap_len ? "yes" : "no");
    printf("  verbatim replays transmitted      %u\n", w.g_replay_tx);
    printf("  forgeries transmitted             %u\n", w.g_forge_tx);
    printf("  jamming bursts transmitted        %u\n", w.g_jam_tx);
    printf("  ------------------------------------------------\n");
    printf("  hostile frames the victim opened  %u\n", hostile_opens);
    printf("  reached the AEAD and failed       %u   (SESS_E_AUTH, 2^-64 each)\n",
           w.n[1].n_auth_fail);
    printf("  rejected on address alone         %u   (never cost a decryption)\n",
           w.n[1].n_missed_addr);
    printf("  receptions lost to the jammer     %u\n", w.g_collisions);

    /* Something the bench found that the unit tests could not, because it is a
     * statement about the radio and not about the code: at SF9 a 34-byte frame
     * occupies the band for 246.8 ms, so ONE transmitter cannot exceed ~4
     * forgery attempts per second. SESS_RATE_TOKENS is 20/s. The token bucket is
     * therefore never the binding constraint against a single attacker — the
     * airtime is, and it is 5x tighter. That does not make the rate limiter
     * useless (it bounds a multi-radio attacker and a buggy peer), but it does
     * mean the honest forgery figure is set by physics. */
    double max_attempts_s = 1e6 / (double)HERUS_AIRTIME_MEANING_US;
    double per_year = max_attempts_s * 365.0 * 86400.0;
    printf("\n  One SF9 transmitter can attempt at most %.1f forgeries per second,\n"
           "  against a %u/s token bucket — airtime is %.1fx tighter than the rate\n"
           "  limiter, so it is physics that bounds this attack. Sustained for a\n"
           "  year that is 2^%.0f attempts against a 2^-64 tag: 2^%.0f odds of one\n"
           "  success.\n",
           max_attempts_s, SESS_RATE_TOKENS, SESS_RATE_TOKENS / max_attempts_s,
           log2(per_year), log2(per_year) - 64.0);

    sim_ok(s, w.n[2].cap_len > 0,        "the adversary really was in range and did capture ciphertext");
    sim_ok(s, w.g_replay_tx > 0 && w.g_forge_tx > 0, "the attacks actually ran");
    sim_ok(s, w.n[1].n_auth_fail > 0,
           "the forgeries really did reach the tag check, so the test tested something");
    sim_ok(s, hostile_opens == 0,        "no replay and no forgery ever opened");
    sim_ok(s, w.n[2].n_opened == 0,      "the adversary opened nothing it captured");
    sim_ok(s, w.g_false_deliveries == 0, "nothing false was ever delivered as meaning");
    sim_ok(s, w.g_delivered > 0,         "jamming degrades the link, it does not corrupt it");
    sim_world_free(&w);
}

/* ===================================================================== 5 == */
void scenario_day(sim_score *s, int argc, char **argv)
{
    hdr("5. a day — does the battery survive, and what does latency cost");

    int rate = opt_int(argc, argv, "--per-hour", 20);

    struct { const char *name; weave_role_t r; uint32_t per; } R[] = {
        { "leaf       ", WEAVE_LEAF,       BEAT_PERIOD_MS      },
        { "responsive ", WEAVE_RESPONSIVE, BEAT_PERIOD_RESP_MS },
        { "relay      ", WEAVE_RELAY,      BEAT_PERIOD_MS      },
    };

    printf("  two paired units 200 m apart, %d messages per hour each way, 24 h\n\n", rate);
    printf("   role         delivered   mean lat   worst lat    mAh/day   400 mAh lasts\n");

    double leaf_mah = 0;
    for (int k = 0; k < 3; k++) {
        sim_world w; sim_world_init(&w, 2, 0x5A5A + (uint64_t)k);
        w.n[0].band = w.n[1].band = 1;
        w.n[1].x = 200;
        for (int i = 0; i < 2; i++) {
            weave_init(&w.n[i].weave, R[k].r);
            w.n[i].period_ms = R[k].per;
        }
        sim_pair(&w, 0, 1, 0x4444);

        int total = rate * 24;
        for (int i = 0; i < total; i++) {
            uint64_t t = (uint64_t)((double)i / total * 86400.0 * 1e6);
            uint8_t  rl[1] = { 3 };
            uint16_t fl[1] = { 9 };
            /* ttl 0: a direct link does not ask for the mesh, so nothing
             * relays and there is no relay silence to observe (leak L1). */
            sim_queue_send(&w, t, i & 1, 1 - (i & 1), 5, 1, rl, fl, 0);
        }
        w.end_us = 86400ull * 1000000ull;
        sim_run(&w);

        double mah = sim_mah_per_day(&w.n[0], w.end_us);
        if (k == 0) leaf_mah = mah;
        printf("   %s   %4u/%-4d   %6.0f ms   %6.0f ms   %8.2f   %6.1f days\n",
               R[k].name, w.g_delivered, total,
               w.lat_n ? (double)w.lat_sum_us / w.lat_n / 1e3 : 0.0,
               (double)w.lat_max_us / 1e3, mah, 400.0 / mah);
        if (w.g_false_deliveries) s->fail++;
        sim_world_free(&w);
    }

    printf("\n  A leaf harvests 19.46 mAh/day in 1.5 h of sun (docs). A relay wants\n"
           "  127.68 mAh/day and gets 26 cm2 of cell, which is why relaying is a\n"
           "  property of the strap and not a setting.\n");

    sim_ok(s, leaf_mah < 19.46, "a leaf's daily draw is under what the Band harvests");
    sim_ok(s, 400.0 / leaf_mah > 30.0, "a 400 mAh cell carries a leaf for over a month");
}

/* ===================================================================== 6 == */
/* The thesis, in one screen: identical bytes, three languages, no translator. */
void scenario_babel(sim_score *s, int argc, char **argv)
{
    (void)argc; (void)argv;
    hdr("6. babel — one frame, three languages, no translation step");

    sim_world w; sim_world_init(&w, 4, 0x8ABE1);
    for (int i = 1; i < 4; i++) { w.n[i].x = 40.0 * i; w.n[i].band = 1; }
    w.n[0].band = 1;
    for (int i = 0; i < 4; i++) {
        weave_init(&w.n[i].weave, WEAVE_RESPONSIVE);
        w.n[i].period_ms = BEAT_PERIOD_RESP_MS;
    }
    w.n[1].lang = LANG_PT; w.n[2].lang = LANG_EN; w.n[3].lang = LANG_ES;

    /* Build the frame once, by hand, so the bytes can be shown. */
    uint8_t root[32]; memset(root, 0x5A, sizeof root);
    herus_session a, b; session_init(&a, root, 1, 0); session_init(&b, root, 0, 0);
    static const uint8_t roles[] = { 1, 2, 3, 4 };
    herus_link tx = { &a, HZ_REGION_BR915, 14, roles, 4 };

    hcp_msg_t m = {0};
    m.tier = HCP_TIER_COMPOSED;
    m.intent = 3;                       /* socorro / help / socorro */
    m.nslot = 2;
    m.slot[0].role = 2; m.slot[0].filler = 5;   /* where: the river */
    m.slot[1].role = 3; m.slot[1].filler = 8;   /* when : now */

    uint8_t frame[LINK_FRAME_LEN];
    int rc = link_send(&tx, &m, 2, frame);

    printf("  on air, %d bytes:  ", LINK_FRAME_LEN);
    for (unsigned i = 0; i < LINK_FRAME_LEN; i++) printf("%02x", frame[i]);
    printf("\n  airtime %.1f ms at SF9. Nothing in those bytes is a word.\n\n",
           HERUS_AIRTIME_MEANING_US / 1000.0);

    int all_ok = (rc == LINK_OK);
    for (int lang = 0; lang < LANG_N; lang++) {
        herus_session r; session_init(&r, root, 0, 0);
        herus_link rxl = { &r, HZ_REGION_BR915, 14, roles, 4 };
        hcp_msg_t out; uint32_t ctr; int se;
        if (link_recv(&rxl, frame, 0, &out, &ctr, &se) != LINK_OK) { all_ok = 0; continue; }
        char line[192];
        render_msg(line, sizeof line, &out, lang);
        printf("    receiver set to %-6s ->  %s\n", lang_name(lang), line);
    }

    /* P4: a sender from a later firmware adds a fifth role. An old receiver must
     * keep the message and drop the field, not reject the frame. */
    hcp_msg_t m2 = {0};
    m2.tier = HCP_TIER_COMPOSED; m2.intent = 6; m2.nslot = 3;
    m2.slot[0].role = 2;  m2.slot[0].filler = 7;
    m2.slot[1].role = 30; m2.slot[1].filler = 99;    /* a role from the future */
    m2.slot[2].role = 4;  m2.slot[2].filler = 11;
    uint8_t f2[LINK_FRAME_LEN];
    herus_session a2; session_init(&a2, root, 1, 0);
    herus_link tx2 = { &a2, HZ_REGION_BR915, 14, roles, 4 };
    int rc2 = link_send(&tx2, &m2, 2, f2);
    herus_session b2; session_init(&b2, root, 0, 0);
    herus_link rx2 = { &b2, HZ_REGION_BR915, 14, roles, 4 };
    hcp_msg_t o2; uint32_t c2; int e2;
    int rc3 = link_recv(&rx2, f2, 0, &o2, &c2, &e2);
    char l2[192]; render_msg(l2, sizeof l2, &o2, LANG_PT);
    printf("\n  a v3 sender adds role 30, which this firmware has never heard of:\n");
    printf("    v1 receiver ->  %s\n", l2);

    sim_ok(s, all_ok, "the same 34 bytes render in three languages");
    sim_ok(s, rc2 == LINK_OK && rc3 == LINK_OK, "P4 an unknown role does not reject the frame");
    sim_ok(s, o2.nslot == 2, "P4 the unknown field is dropped and the rest survives");
    sim_world_free(&w);
}

/* ===================================================================== 7 == */
/* beat.h claims a +-10 ms guard covers +-20 ppm crystals if the group resyncs
 * within 60 s. A claim needs something able to break it. */
void scenario_drift(sim_score *s, int argc, char **argv)
{
    (void)argc; (void)argv;
    hdr("7. drift — what the crystal costs when nobody talks for a while");

    printf("   silence before the next message   delivered   worst-case drift\n");
    int broke_at = 0;
    for (int quiet_s = 60; quiet_s <= 3840; quiet_s *= 2) {
        sim_world w; sim_world_init(&w, 2, 0x9E9E);
        w.n[0].band = w.n[1].band = 1;
        w.n[1].x = 100;
        /* Worst case rather than random: the two crystals at opposite corners of
         * the tolerance, which is the only case worth budgeting for. */
        w.n[0].ppm = +20.0; w.n[1].ppm = -20.0;
        weave_init(&w.n[0].weave, WEAVE_RESPONSIVE);
        weave_init(&w.n[1].weave, WEAVE_RESPONSIVE);
        w.n[0].period_ms = w.n[1].period_ms = BEAT_PERIOD_RESP_MS;
        sim_pair(&w, 0, 1, 0x6666);

        int msgs = 10;
        for (int i = 0; i < msgs; i++) {
            uint8_t rl[1] = { 1 }; uint16_t fl[1] = { 2 };
            sim_queue_send(&w, (uint64_t)quiet_s * 1000000ull * (uint64_t)(i + 1),
                           0, 1, 1, 1, rl, fl, 1);
        }
        w.end_us = (uint64_t)quiet_s * 1000000ull * (uint64_t)(msgs + 2);
        sim_run(&w);

        double drift_ms = 40.0 * quiet_s / 1000.0;   /* 40 us/s relative */
        printf("   %6d s                          %2u/%-2d       %7.1f ms\n",
               quiet_s, w.g_delivered, msgs, drift_ms);
        if (w.g_delivered < (uint32_t)msgs && !broke_at) broke_at = quiet_s;
        sim_world_free(&w);
    }
    printf("\n  The tolerance is asymmetric, and that is a property of the radio rather\n"
           "  than of the schedule: the receiver may open its %u ms window anywhere\n"
           "  from %u ms BEFORE the sender starts to %.1f ms after it, because a frame\n"
           "  whose preamble is still running is still detectable. At 40 us/s of\n"
           "  relative drift that band is worth about %.0f s of silence in the worst\n"
           "  direction — so BEAT_RESYNC_MS = %u s is not a comfort setting, it is the\n"
           "  deadline past which a silent group stops being able to hear each other,\n"
           "  with roughly 8x of headroom.\n",
           BEAT_RX_MS, BEAT_RX_MS, 8.0 * HZ_TSYM_US(HERUS_SF_MEANING) / 1000.0,
           (double)BEAT_RX_MS * 1000.0 / 40.0, BEAT_RESYNC_MS / 1000);

    sim_ok(s, broke_at == 0 || broke_at > (int)(BEAT_RESYNC_MS / 1000),
           "the link survives at least the resync interval of silence");
    sim_ok(s, broke_at == 0 || broke_at >= 8 * (int)(BEAT_RESYNC_MS / 1000),
           "the resync interval has at least 8x of margin against the crystal");
}

/* ===================================================================== 8 == */
/* The layer above the wire: a composed meaning becomes a hypervector, and a role
 * is recovered from it by unbinding. This is the cognition path, not the receive
 * path — hcp.h is explicit that the wire carries ids and unbinding is for
 * matching a heard phrase against memory. */
void scenario_cognition(sim_score *s, int argc, char **argv)
{
    (void)argc; (void)argv;
    hdr("8. cognition — recovering a role from a composed hypervector");

    /* Two units that share a domain seed generate bit-identical codebooks
     * without ever exchanging one. That is what makes a 2-byte id mean the same
     * thing at both ends, and it is why nothing linguistic has to travel. */
    const uint64_t DOM_SEED = 0x48455255530000ull;
    lex_t A, B;
    if (lex_init(&A, DOM_SEED, 1024) != 0 || lex_init(&B, DOM_SEED, 1024) != 0) {
        sim_ok(s, 0, "codebook allocation");
        return;
    }
    hv_t ca, cb;
    lex_code(&A, &ca, HCP_ID_FILLER_BASE + 5);
    lex_code(&B, &cb, HCP_ID_FILLER_BASE + 5);
    int same = memcmp(&ca, &cb, sizeof ca) == 0;

    hv_t cz;
    lex_code(&A, &cz, HCP_ID_FILLER_BASE + 6);
    int d_self  = hv_dist(&ca, &cb);
    int d_other = hv_dist(&ca, &cz);
    printf("  two units, same domain seed, never exchanged a codebook:\n");
    printf("    distance between their code for symbol 5      %5d bits\n", d_self);
    printf("    distance to a DIFFERENT symbol                %5d bits  (D/2 = %d)\n",
           d_other, HV_BITS / 2);

    /* Compose a message into one hypervector, then interrogate it. */
    hcp_msg_t m = {0};
    m.tier = HCP_TIER_COMPOSED;
    m.intent = 3;
    m.nslot = 4;
    const uint8_t  R[4] = { 1, 2, 3, 4 };
    const uint16_t F[4] = { 1, 5, 8, 11 };
    for (int i = 0; i < 4; i++) { m.slot[i].role = R[i]; m.slot[i].filler = F[i]; }

    uint8_t pt[HCP_PLAINTEXT_LEN];
    int n = hcp_encode(pt, &m);        /* also stamps m.pos[] — hcp.h is explicit */

    hv_acc_t *acc = malloc(sizeof *acc);
    hv_t H;
    hcp_to_hv(&H, acc, &A, &m);

    int hits = 0;
    printf("\n  a 4-slot meaning, bundled into one %d-bit vector, then unbound:\n", HV_BITS);
    for (int i = 0; i < m.nslot; i++) {
        uint16_t got = 0;
        int found = hcp_query_role(&H, &B, m.slot[i].role, m.pos[i], 0, 512, 12.0, &got);
        printf("    role %u  ->  %s  (sent %u)\n", m.slot[i].role,
               found ? (got == F[i] ? "recovered exactly" : "WRONG") : "not found", F[i]);
        if (found && got == F[i]) hits++;
    }
    uint16_t ghost = 0;
    int absent = hcp_query_role(&H, &B, 20, 0, 0, 512, 12.0, &ghost);
    printf("    role 20 (never sent) -> %s\n", absent ? "FALSELY PRESENT" : "correctly absent");

    free(acc);
    lex_free(&A); lex_free(&B);

    sim_ok(s, same,             "two units derive identical codes from a shared seed alone");
    sim_ok(s, d_other > HV_BITS / 2 - 400 && d_other < HV_BITS / 2 + 400,
           "distinct symbols are quasi-orthogonal, as the whole design assumes");
    sim_ok(s, n == HCP_PLAINTEXT_LEN, "the composed meaning still encodes to 24 bytes");
    sim_ok(s, hits == 4,        "every role is recovered from the bundle by unbinding");
    sim_ok(s, !absent,          "a role that was never sent is reported absent, not guessed");
}
