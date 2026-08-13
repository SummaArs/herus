/* stress.c — scenarios written to fail.
 *
 * scenarios.c asks whether the design works. This file assumes it does and goes
 * looking for the place it stops working. Every scenario here targets one
 * specific suspicion, states the number that settles it, and is built so that a
 * PASS means something. A stress test that passed on its first run either found
 * nothing or was too polite to the thing it was testing.
 *
 * The four suspicions, and where each came from:
 *
 *   deaf      session.h admits it: "If more than SESS_WINDOW consecutive frames
 *             are lost, the sender's address is no longer in our table ... The
 *             recovery is an application-level timeout followed by a re-key."
 *             There is no re-key in the tree. So: is the loss permanent?
 *
 *   drain     weave.h relays any frame from anyone — that is what a mesh IS.
 *             Nothing authenticates a frame as worth relaying, and relaying costs
 *             45 mA for 246.8 ms. So: what does a stranger with a transmitter
 *             cost a leaf's battery?
 *
 *   unmask    weave_offer(mine=1) returns without relaying: "ours: consume, do
 *             not relay". Every node that CANNOT read a frame relays it, and the
 *             one that can does not. So: does relay behaviour identify the
 *             recipient that P6 works so hard to hide?
 *
 *   birthday  E-P2 spent two address bits on ttl, taking the space from 65536 to
 *             16384, and called the extra collisions acceptable. So: at what peer
 *             count does "acceptable" stop being true?
 */
#include "sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static void hdr(const char *t)
{
    printf("\n%s\n", t);
    for (size_t i = 0; i < strlen(t); i++) putchar('-');
    putchar('\n');
}

/* ===================================================================== S1 = */
void scenario_deaf(sim_score *s, int argc, char **argv)
{
    hdr("S1. deaf — one bad tunnel and the link never comes back");

    int gone_s = opt_int(argc, argv, "--outage", 150);
    int every  = opt_int(argc, argv, "--every", 3);

    sim_world w; sim_world_init(&w, 2, 0xDEAF);
    w.n[0].band = w.n[1].band = 1;
    w.n[1].x = 100;
    weave_init(&w.n[0].weave, WEAVE_RESPONSIVE);
    weave_init(&w.n[1].weave, WEAVE_RESPONSIVE);
    w.n[0].period_ms = w.n[1].period_ms = BEAT_PERIOD_RESP_MS;
    sim_pair(&w, 0, 1, 0x7777);

    /* B walks into a basement at t=30 s and comes out at t=30+outage. Nothing
     * hostile happens. This is a lift, a tunnel, a concrete stairwell. */
    sim_push(&w, 30000000ull, EV_MOVE, 1, 100000, 0);
    sim_push(&w, (uint64_t)(30 + gone_s) * 1000000ull, EV_MOVE, 1, 100, 0);

    int total = 300 / every;
    for (int i = 0; i < total; i++) {
        uint8_t rl[1] = { 1 }; uint16_t fl[1] = { 2 };
        sim_queue_send(&w, (uint64_t)i * (uint64_t)every * 1000000ull,
                       0, 1, 1, 1, rl, fl, 1);
    }
    w.end_us = 300000000ull;

    /* Count deliveries before the outage and after B is back in range. */
    w.verbose = 0;
    sim_run(&w);

    int lost = gone_s / every;
    printf("  A transmits every %d s. B is out of range for %d s — %d frames lost,\n"
           "  against a receive window of %d.\n\n", every, gone_s, lost, SESS_WINDOW);
    printf("  delivered, whole run          %u / %d\n", w.g_delivered, total);
    printf("  frames B never recognised     %u   (SESS_E_ADDR, the cheap reject)\n",
           w.n[1].n_missed_addr);
    printf("  deep resyncs that landed      %u   (session_recover)\n", w.n[1].n_recovered);
    printf("  B's receive counter           %u\n", w.n[1].sess[0].recv.n);
    printf("  A's send counter              %u\n", w.n[0].sess[1].send.n);

    uint32_t gap = w.n[0].sess[1].send.n - w.n[1].sess[0].recv.n;
    printf("  counters apart by             %u\n", gap);

    /* The decisive number: after B is back in range and physically able to hear
     * everything, does anything at all get through? */
    int after_ok = (w.g_delivered > (uint32_t)(30 / every) + 2);
    printf("\n  after B returns to 100 m, with a perfect radio link:  %s\n",
           after_ok ? "the link recovers" : "NOTHING EVER OPENS AGAIN");

    sim_ok(s, lost > SESS_WINDOW, "the outage really did exceed the window, so the test is loaded");
    sim_ok(s, after_ok,
           "a link that loses more than SESS_WINDOW frames can still recover");
    sim_world_free(&w);
}

/* ===================================================================== S2 = */
void scenario_drain(sim_score *s, int argc, char **argv)
{
    hdr("S2. drain — what a stranger with a transmitter costs a leaf");

    /* Long enough for the governor to reach steady state. The bucket starts full,
     * so a short run measures the boot allowance rather than the sustained rate —
     * the first version of this scenario ran 10 minutes and reported a number the
     * governor would never have let continue. */
    int minutes = opt_int(argc, argv, "--minutes", 240);
    int victims = opt_int(argc, argv, "--victims", 6);
    if (victims > 16) victims = 16;

    sim_world w; sim_world_init(&w, victims + 1, 0xD4A11);
    for (int i = 0; i < victims; i++) {
        w.n[i].x = 20.0 * i; w.n[i].band = 1;
        weave_init(&w.n[i].weave, WEAVE_LEAF);        /* the cheapest role there is */
        w.n[i].period_ms = BEAT_PERIOD_MS;
    }
    int J = victims;
    w.n[J].x = 50; w.n[J].band = 1; w.n[J].adversary = 1;
    weave_init(&w.n[J].weave, WEAVE_RELAY);

    /* The attacker does the only thing it can: transmit well-formed 34-byte
     * frames with ttl at maximum, addressed to nobody, as fast as the band
     * allows. It holds no key and needs none — a relay does not check keys,
     * because checking keys is exactly what a relay cannot do. */
    uint64_t dur = (uint64_t)minutes * 60ull * 1000000ull;
    uint64_t step = HERUS_AIRTIME_MEANING_US + 5000ull;
    int njunk = 0;
    for (uint64_t t = 0; t < dur; t += step) {
        sim_push(&w, t, EV_ADVERSARY, J, ADV_JAM, 0);
        njunk++;
    }
    w.end_us = dur;
    sim_run(&w);

    /* A leaf's whole daily allowance is 1.78 mAh. Relaying one frame costs
     * 45 mA for 246.8 ms. The question is how many days of budget the attacker
     * spends per minute of transmitting. */
    double worst = 0;
    uint32_t relays = 0;
    for (int i = 0; i < victims; i++) {
            if (w.n[i].uah_tx > worst) worst = w.n[i].uah_tx;
        relays += w.n[i].n_relayed;
    }
    /* The only framing that decides anything: extrapolate the attack to a full
     * day and compare with what the strap harvests. Below the harvest, the
     * attacker is an annoyance. Above it, the device is no longer off-grid. */
    double idle_mah    = 1.78;
    double harvest_mah = 19.46;
    double sustained   = worst / 1000.0 / ((double)minutes / 1440.0) + idle_mah;
    uint32_t governed  = 0;
    for (int i = 0; i < victims; i++) governed += w.n[i].weave.stat_governed;

    printf("  %d leaves in earshot, one attacker, %d minutes\n", victims, minutes);
    printf("  junk frames transmitted       %d\n", njunk);
    printf("  frames the leaves relayed     %u\n", relays);
    printf("  relays the governor refused   %u\n", governed);
    printf("  worst leaf spent              %.0f uAh forwarding frames nobody sent it\n", worst);
    printf("  ------------------------------------------------\n");
    printf("  sustained draw under attack   %6.2f mAh/day\n", sustained);
    printf("  the same leaf, left alone     %6.2f mAh/day\n", idle_mah);
    printf("  what a Band harvests in sun   %6.2f mAh/day\n", harvest_mah);
    printf("  400 mAh cell lasts            %6.1f days under attack, %.0f days idle\n",
           400.0 / sustained, 400.0 / idle_mah);

    sim_ok(s, njunk > 100, "the attack actually ran long enough to mean something");
    sim_ok(s, sustained < harvest_mah,
           "under sustained attack a leaf still draws less than its strap harvests");
    sim_world_free(&w);
}

/* ===================================================================== S3 = */
void scenario_unmask(sim_score *s, int argc, char **argv)
{
    hdr("S3. unmask — does relay silence say who the message was for?");

    int nn   = opt_int(argc, argv, "--nodes", 8);
    int msgs = opt_int(argc, argv, "--msgs", 40);
    if (nn > 32) nn = 32;

    sim_world w; sim_world_init(&w, nn, 0x11A5C);
    for (int i = 0; i < nn; i++) {
        w.n[i].x = 15.0 * i;        /* everyone hears everyone */
        w.n[i].band = 1;
        weave_init(&w.n[i].weave, WEAVE_RELAY);   /* all listening, all willing */
    }
    for (int i = 0; i < nn; i++)
        for (int j = i + 1; j < nn; j++) sim_pair(&w, i, j, 0x8888);

    /* Node 0 talks to node 3 and to nobody else. An observer with one receiver
     * writes down who retransmitted each frame. It never decrypts anything. */
    const int SRC = 0, DST = 3;
    for (int i = 0; i < msgs; i++) {
        uint8_t rl[1] = { 1 }; uint16_t fl[1] = { 2 };
        sim_queue_send(&w, (uint64_t)i * 4000000ull, SRC, DST, 1, 1, rl, fl, 2);
    }
    w.end_us = (uint64_t)(msgs + 4) * 4000000ull;
    sim_run(&w);

    /* The inference: among the nodes that heard the frame, the recipient is the
     * one that did not pass it on. */
    int guessed_right = 0, usable = 0;
    for (int i = 0; i < SIM_PROV_N; i++) {
        const sim_prov *p = &w.prov[i];
        if (!p->used || !p->born || p->orig != SRC) continue;
        int silent = -1, nsilent = 0;
        for (int k = 0; k < nn; k++) {
            if (k == SRC) continue;
            if (!(p->relayed_by & (1ull << k))) { silent = k; nsilent++; }
        }
        if (nsilent == 1) { usable++; if (silent == DST) guessed_right++; }
    }
    double acc = usable ? 100.0 * guessed_right / usable : 0.0;

    printf("  %d units, all relay-capable, all in range of each other.\n", nn);
    printf("  node %d sends %d messages to node %d. The observer holds no key.\n\n", SRC, msgs, DST);
    uint32_t decoys = 0, relays = 0;
    for (int k = 0; k < nn; k++) { decoys += w.n[k].weave.stat_decoy; relays += w.n[k].n_relayed; }

    printf("  frames relayed by somebody                    %u\n", relays);
    printf("  of those, relayed by their own recipient      %u   (decoys)\n", decoys);
    printf("  frames where exactly one node stayed silent   %d\n", usable);
    printf("  of those, the silent one WAS the recipient    %d\n", guessed_right);
    printf("  recipient identified with accuracy            %.0f %%\n", acc);
    printf("\n  P6 keeps every stable identifier off the air. It says nothing about\n"
           "  what a node DOES with a frame, and doing nothing is an observable.\n");

    sim_ok(s, relays > (uint32_t)msgs, "the mesh really was relaying, so the test is loaded");
    sim_ok(s, decoys > 0, "the recipient relayed its own traffic, as leak L1 requires");
    sim_ok(s, acc < 40.0,
           "relay behaviour does not identify the recipient better than guessing");
    sim_world_free(&w);
}

/* ===================================================================== S4 = */
void scenario_birthday(sim_score *s, int argc, char **argv)
{
    hdr("S4. birthday — what E-P2 cost, in collisions per peer");

    printf("  E-P2 spent two address bits on ttl: %d bits of address, %u live per\n",
           SESS_ADDR_BITS, (unsigned)SESS_WINDOW);
    printf("  session. A frame for peer X can land in the window of peer Y, and the\n"
           "  cost is one wasted AEAD over 24 bytes plus one rate-limiter token.\n\n");
    printf("   peers   live addrs   P(a random frame hits some window)   measured\n");

    double worst_err = 0;
    for (int peers = 2; peers <= 32; peers *= 2) {
        int live = peers * SESS_WINDOW;
        double space = (double)(1u << SESS_ADDR_BITS);
        double p_theory = 1.0 - pow(1.0 - 1.0 / space, (double)live);

        /* Measure it: build one node paired with `peers` others and count how
         * many random 14-bit addresses land in some window. */
        sim_world w; sim_world_init(&w, peers + 1, 0xB144 + (uint64_t)peers);
        for (int i = 1; i <= peers; i++) sim_pair(&w, 0, i, 0x9999 + (uint64_t)i);
        int hits = 0, trials = 200000;
        for (int t = 0; t < trials; t++) {
            uint8_t f[LINK_FRAME_LEN] = {0};
            uint16_t a = (uint16_t)(sim_rand(&w) & SESS_ADDR_MASK);
            f[0] = (uint8_t)(a & 0xff);
            f[1] = (uint8_t)((a >> 8) & 0x3f);
            for (int p = 1; p <= peers; p++)
                if (session_addr_in_window(&w.n[0].sess[p], f)) { hits++; break; }
        }
        double measured = (double)hits / trials;
        double err = fabs(measured - p_theory);
        if (err > worst_err) worst_err = err;
        printf("   %5d   %9d   %31.4f   %8.4f\n", peers, live, p_theory, measured);
        sim_world_free(&w);
    }

    /* What it costs in practice: a device hearing the whole group's traffic pays
     * one needless decryption at that rate. */
    double p32 = 1.0 - pow(1.0 - 1.0 / 16384.0, 32.0 * 32.0);
    printf("\n  At 32 peers that is one wasted decryption per %.0f foreign frames.\n"
           "  A 24-byte AEAD costs ~16 us, so even at the band's ceiling of %.1f\n"
           "  frames/s the wasted work is %.4f %% of one core. E-P2's bill is real\n"
           "  and it is small — the honest conclusion is that it was worth paying.\n",
           1.0 / p32, 1e6 / (double)HERUS_AIRTIME_MEANING_US,
           100.0 * p32 * 16e-6 * (1e6 / (double)HERUS_AIRTIME_MEANING_US));

    sim_ok(s, worst_err < 0.01, "the measured collision rate matches the birthday model");
    sim_ok(s, p32 < 0.10, "even at 32 peers, fewer than 1 in 10 foreign frames costs a decryption");
}

/* ===================================================================== S5 = */
/* The one that took a protocol change to fix. A lost frame is lost, and the
 * obvious repair — transmit it again — is forbidden by the same single-use-key
 * rule that gives replay defence and forward secrecy. See the note in link.h. */
void scenario_reliable(sim_score *s, int argc, char **argv)
{
    hdr("S5. reliable — the retry that single-use keys forbid");

    int total = opt_int(argc, argv, "--msgs", 480);

    printf("  Two leaves, 200 m, %d messages over 24 h. `copies` is how many\n", total);
    printf("  DISTINCT sealed frames carry each message — a byte-identical\n");
    printf("  retransmission cannot open at the far end, by design.\n\n");
    printf("   copies   delivered   delivered twice   suppressed   leaf mAh/day\n");

    double base_mah = 0; uint32_t base_deliv = 0;
    int dup_leak = 0;
    for (int copies = 1; copies <= 3; copies++) {
        sim_world w; sim_world_init(&w, 2, 0xE111);
        w.retries = copies;
        w.n[0].band = w.n[1].band = 1;
        w.n[1].x = 200;
        for (int i = 0; i < 2; i++) { weave_init(&w.n[i].weave, WEAVE_LEAF);
                                      w.n[i].period_ms = BEAT_PERIOD_MS; }
        sim_pair(&w, 0, 1, 0x5151);
        for (int i = 0; i < total; i++) {
            uint64_t t = (uint64_t)((double)i / total * 86400.0 * 1e6);
            uint8_t rl[1] = { 3 }; uint16_t fl[1] = { 9 };
            sim_queue_send(&w, t, i & 1, 1 - (i & 1), 5, 1, rl, fl, 0);
        }
        w.end_us = 86400ull * 1000000ull;
        sim_run(&w);

        double mah = sim_mah_per_day(&w.n[0], w.end_us);
        uint32_t twice = w.g_delivered > (uint32_t)total ? w.g_delivered - total : 0;
        if (twice) dup_leak = 1;
        printf("   %5d    %4u/%-4d   %13u   %10u   %10.2f\n",
               copies, w.g_delivered, total, twice, w.g_dup_suppressed, mah);
        if (copies == 1) { base_mah = mah; base_deliv = w.g_delivered; }
        if (copies == 2) {
            printf("\n  one copy  %5.2f %% delivered\n", 100.0 * base_deliv / total);
            printf("  two copies %5.2f %% delivered, for %+.2f mAh/day\n",
                   100.0 * w.g_delivered / total, mah - base_mah);
            printf("  duplicates suppressed at the far end: %u\n", w.g_dup_suppressed);
            sim_ok(s, w.g_delivered > base_deliv,
                   "a second distinct frame recovers messages the first one lost");
            sim_ok(s, w.g_dup_suppressed > 0,
                   "the far end really did receive and discard duplicates");
        }
        sim_world_free(&w);
    }
    sim_ok(s, !dup_leak, "no message is ever delivered to the user twice");
}

/* ===================================================================== S6 = */
/* Range, walked. Not "we improved it" — the distance at which each profile stops
 * working, measured with the real crypto running, against a link budget computed
 * independently from the SX1262 sensitivity table. */
void scenario_reach(sim_score *s, int argc, char **argv)
{
    hdr("S6. reach — SF10 at 24 bytes, and the wall behind it");

    int msgs = opt_int(argc, argv, "--msgs", 12);

    printf("  The 400 ms dwell limit is a currency: it buys bytes or spreading\n");
    printf("  factor, never both. What it buys, exactly:\n\n");
    printf("   profile   SF   frame   airtime    sens      slots   dwell headroom\n");
    for (int k = 0; k < 2; k++) {
        const hz_link_profile_t *P = hz_link(k ? HZ_LINK_REACH : HZ_LINK_RICH);
        printf("   %-8s  %2u   %3u B   %6.1f ms   %6.1f    %2u      %6.1f ms\n",
               P->name, P->sf, P->frame_len, P->airtime_us / 1000.0,
               sim_sens_dbm(P->sf), P->max_slot,
               (HZ_DWELL_LIMIT_US - P->airtime_us) / 1000.0);
    }
    printf("\n  SF11 carries zero bytes inside the limit and SF12's preamble alone\n");
    printf("  is 663 ms. region.h asserts both, so this is the last rung.\n");

    printf("\n   antenna    profile   measured cliff   link budget   gain\n");
    int cliff[2][2] = {{0,0},{0,0}};
    for (int band = 0; band < 2; band++) {
        for (int k = 0; k < 2; k++) {
            int last_good = 0;
            for (int d = 50; d <= 1400; d += 25) {
                sim_world w; sim_world_init(&w, 2, 0x2EAC4 + (uint64_t)d);
                w.n[0].band = w.n[1].band = band;
                w.n[0].profile = w.n[1].profile = k ? HZ_LINK_REACH : HZ_LINK_RICH;
                w.n[1].x = (double)d;
                for (int i = 0; i < 2; i++) {
                    weave_init(&w.n[i].weave, WEAVE_RESPONSIVE);
                    w.n[i].period_ms = BEAT_PERIOD_RESP_MS;
                }
                sim_pair(&w, 0, 1, 0x4EAC4);
                for (int i = 0; i < msgs; i++) {
                    uint8_t rl[1] = { 2 }; uint16_t fl[1] = { 5 };
                    sim_queue_send(&w, (uint64_t)i * 3000000ull, 0, 1, 1, 1, rl, fl, 0);
                }
                w.end_us = (uint64_t)(msgs + 2) * 3000000ull;
                sim_run(&w);
                if (w.g_delivered * 2 >= (uint32_t)msgs) last_good = d;
                if (w.g_false_deliveries) s->fail++;
                sim_world_free(&w);
            }
            cliff[band][k] = last_good;
            const hz_link_profile_t *P = hz_link(k ? HZ_LINK_REACH : HZ_LINK_RICH);
            sim_channel urban = { SIM_CLUTTER_URBAN, 0.0, 1, -1 };
            double pred = sim_range_m(&urban, P->sf,
                                      band ? SIM_G_BAND_DBI : SIM_G_CAPSULE_DBI);
            printf("   %-8s   %-7s   %8d m       %6.0f m    %s\n",
                   band ? "Band" : "capsule", P->name, last_good, pred,
                   k ? "" : "(baseline)");
        }
        printf("   %-8s   %-7s   %+8.1f %%\n", "", "gain",
               100.0 * ((double)cliff[band][1] / (double)cliff[band][0] - 1.0));
    }

    sim_ok(s, cliff[0][1] > cliff[0][0], "Reach carries further than Rich, capsule antenna");
    sim_ok(s, cliff[1][1] > cliff[1][0], "Reach carries further than Rich, Band antenna");
    sim_ok(s, HERUS_AIRTIME_REACH_US < HZ_DWELL_LIMIT_US, "P2 the Reach frame is legal");
}

/* ===================================================================== S7 = */
/* The ladder. Every step is a different part, priced in dB and in milliamps,
 * because "better hardware" is not an argument until it is a column. */
void scenario_hardware(sim_score *s, int argc, char **argv)
{
    (void)argc; (void)argv;
    hdr("S7. hardware — what each part is worth, in metres and milliamps");

    sim_channel urban = { SIM_CLUTTER_URBAN, 0.0, 1, -1 };
    struct { const char *what; int sf; double tx, nf, gc, gb; } L[] = {
        { "as specified: SX1262 14 dBm, bare chip, Rich",
          HERUS_SF_MEANING, SIM_TX_DBM_DOC, SIM_NF_CHIP, SIM_G_CAPSULE_DBI, SIM_G_BAND_DBI },
        { "+ Reach (SF10, 24 B) — modulation, free",
          HERUS_SF_REACH,   SIM_TX_DBM_DOC, SIM_NF_CHIP, SIM_G_CAPSULE_DBI, SIM_G_BAND_DBI },
        { "+ the SX1262's own PA at 22 dBm — no new part",
          HERUS_SF_REACH,   22.0,           SIM_NF_CHIP, SIM_G_CAPSULE_DBI, SIM_G_BAND_DBI },
        { "+ receive LNA, NF 6.03 -> 1.30 dB — ~$1, 5 mA",
          HERUS_SF_REACH,   22.0,           SIM_NF_LNA,  SIM_G_CAPSULE_DBI, SIM_G_BAND_DBI },
        { "+ engineered strap antenna (PROJECTED, bench)",
          HERUS_SF_REACH,   22.0,           SIM_NF_LNA,  -5.0,              -1.0 },
    };
    int n = (int)(sizeof L / sizeof *L);

    printf("   step                                             budget   capsule    Band     gain\n");
    double prev = 0;
    for (int i = 0; i < n; i++) {
        double sens = sim_sens_nf(L[i].sf, L[i].nf);
        double budget = L[i].tx + L[i].gb + L[i].gb - sens - SIM_MARGIN_DB;
        double rc = sim_range_at(&urban, L[i].sf, L[i].gc, L[i].tx, L[i].nf);
        double rb = sim_range_at(&urban, L[i].sf, L[i].gb, L[i].tx, L[i].nf);
        printf("   %-46s %6.1f  %6.0f m %7.0f m", L[i].what, budget, rc, rb);
        if (i) printf("  %+5.1f dB", budget - prev);
        printf("\n");
        prev = budget;
    }

    /* What the two costed steps actually take out of the battery. */
    double air = HERUS_AIRTIME_REACH_US / 1e6;
    double e14 = sim_i_tx_ma(14.0) * air / 3.6;      /* uAh per frame */
    double e22 = sim_i_tx_ma(22.0) * air / 3.6;
    double leaf_rx = SIM_I_RX * 0.01 * 24.0;                 /* mAh/day, 1% duty */
    double leaf_rx_lna = (SIM_I_RX + SIM_I_LNA) * 0.01 * 24.0;
    printf("\n   what it costs, at 50 messages a day and a leaf's 1%% receive duty:\n");
    printf("     transmit  14 dBm  %5.2f uAh/frame  ->  %5.2f mAh/day\n", e14, e14 * 50 / 1000);
    printf("     transmit  22 dBm  %5.2f uAh/frame  ->  %5.2f mAh/day   (+%.2f)\n",
           e22, e22 * 50 / 1000, (e22 - e14) * 50 / 1000);
    printf("     receive   no LNA                     %5.2f mAh/day\n", leaf_rx);
    printf("     receive   with LNA                   %5.2f mAh/day   (+%.2f)\n",
           leaf_rx_lna, leaf_rx_lna - leaf_rx);
    printf("     a Band harvests                      %5.2f mAh/day\n", 19.46);
    double total = e22 * 50 / 1000 + leaf_rx_lna + SIM_I_BASELINE * 24.0;
    printf("     full ladder, leaf total              %5.2f mAh/day  -> %.0f days on 400 mAh\n",
           total, 400.0 / total);

    /* A relay listens continuously, so the LNA it most wants is the one it can
     * least afford. Stated, because it decides a product SKU. */
    printf("\n   a relay listens continuously, so the same LNA costs it\n"
           "   %.0f mAh/day instead of %.0f — the part that helps sensitivity most\n"
           "   is the one a relay can least afford. That is a SKU decision, not a\n"
           "   firmware one.\n", (SIM_I_RX + SIM_I_LNA) * 24.0, SIM_I_RX * 24.0);

    sim_ok(s, total < 19.46, "the whole ladder still fits inside what a Band harvests");
    sim_ok(s, sim_range_at(&urban, HERUS_SF_REACH, SIM_G_BAND_DBI, 22.0, SIM_NF_LNA)
              > 2.0 * sim_range_at(&urban, HERUS_SF_MEANING, SIM_G_BAND_DBI,
                                   SIM_TX_DBM_DOC, SIM_NF_CHIP),
           "the buildable ladder more than doubles the published range");
}
