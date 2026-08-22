/* sim.h — the world that supplies the two things firmware/core and firmware/net
 * do not have: a radio and a clock.
 *
 * link.h states the split this file depends on: "everything below this line is
 * testable on a host with no radio ... the only thing the ESP32 adds is a radio
 * and a clock." The simulator takes that sentence literally. It links the REAL
 * hcp, session, weave, beat, region, crypto and link objects — not a model of
 * them — and wraps them in a virtual world of metres, milliseconds and mA.
 *
 * WHAT IS REAL AND WHAT IS MODELLED, STATED ONCE SO NOBODY HAS TO GUESS
 * --------------------------------------------------------------------
 *   REAL      Every byte on air. Framing, the ratchet, AEAD, dedup, ttl, slot
 *             maths, airtime, the regional legality check. If the simulator
 *             delivers a message, the shipping firmware produced and opened it.
 *
 *   MODELLED  Propagation (two-ray + clutter, constants shared with
 *             tools/budget.py), receiver sensitivity (SX1262 datasheet table),
 *             co-channel capture, crystal drift, and current draw.
 *
 *   BOOKKEEPING  tx_t carries dst/seq/born_us so the simulator can score
 *             delivery and latency. NONE of that is on air and none of it is
 *             visible to a node. A relay sees exactly 34 bytes, and the tests
 *             assert it stays that way.
 *
 * The value of the distinction: a bug the simulator finds is a bug in the
 * firmware, not in a re-implementation of it.
 */
#ifndef HERUS_SIM_H
#define HERUS_SIM_H

#include <stdint.h>
#include <stddef.h>

#include "link.h"
#include "beat.h"
#include "weave.h"
#include "lexicon.h"
#include "transport_selector.h"
#include "personal_telemetry.h"
#include "personal_sim.h"

#define SIM_MAX_NODES     48
#define SIM_MAX_INFLIGHT  64
#define SIM_FRAME_MAX     WEAVE_FRAME_MAX          /* 38 */

/* ------------------------------------------------------------------ physics
 * Every constant below is copied from tools/budget.py, which is the single
 * source for every hardware figure in docs/00-HERUS-MASTER.md. They are
 * duplicated rather than imported because C cannot import Python — so
 * scenario_selftest() re-derives budget.py's published ranges from them and
 * fails the run if they have drifted apart. Duplication with a check beats a
 * comment asking people to keep two files in sync.
 */
#define SIM_FREQ_HZ        915e6
#define SIM_C              299792458.0
/* The design point, and the baseline it replaced. SIM_TX_DBM_DOC exists so the
 * self-test can still prove the world reproduces the published 365 m / 650 m
 * figures — those were computed at 14 dBm, and a model that quietly stopped
 * agreeing with them would be a model nobody could check. */
#define SIM_TX_DBM         22.0
#define SIM_TX_DBM_DOC     14.0
#define SIM_NF_CHIP        6.03
#define SIM_NF_LNA         1.30
#define SIM_MARGIN_DB      3.0        /* implementation loss + slow fading */
#define SIM_G_CAPSULE_DBI  (-8.0)     /* PCB antenna, sealed, worn */
#define SIM_G_BAND_DBI     (-3.0)     /* antenna in the strap */
#define SIM_CLUTTER_URBAN  22.0
#define SIM_CLUTTER_SUBURB 12.0
#define SIM_CLUTTER_OPEN   0.0
#define SIM_H_WRIST_M      1.0
#define SIM_CAPTURE_DB     6.0        /* LoRa co-channel rejection */

/* Currents in mA at 3.3 V, datasheet typicals — same table as budget.py. */
#define SIM_I_TX14         45.0
#define SIM_I_TX22        118.0
#define SIM_I_LNA           5.0
#define SIM_I_RX           5.3
#define SIM_I_MCU_ACTIVE   45.0
#define SIM_I_BASELINE     0.0212     /* SX1262 retention + ATECC + S3 deep */

/* MCU time actually spent per frame, from test_net X1 measured on host and
 * projected in docs/05-FIRMWARE.md. Deliberately generous. */
#define SIM_MCU_MS_SEND    5.0
#define SIM_MCU_MS_RECV    2.0

/* ------------------------------------------------------------------- world */
typedef enum { EV_APP_SEND = 1, EV_TX_END, EV_RELAY_KICK, EV_ADVERSARY, EV_MOVE,
               EV_STOP }
        ev_type_t;

/* What a hostile transmitter is allowed to try. Everything it does, it does with
 * only what it heard: it has no key and never will. */
enum { ADV_REPLAY = 0, ADV_FORGE, ADV_JAM };

typedef struct {
    uint64_t t_us;
    uint64_t seq;        /* insertion order: makes ties deterministic */
    int      type;
    int      a, b, c;    /* meaning depends on type */
} sim_event;

typedef struct {
    int      active;
    int      src;
    int      chan;
    uint8_t  sf;
    uint64_t t0_us, t1_us;
    uint8_t  frame[SIM_FRAME_MAX];
    int      len;
    /* simulator bookkeeping — never on air, never readable by a node */
    int      dst, seq_app, hops;
    uint64_t born_us;
} sim_tx;

typedef struct {
    double clutter_db;      /* urban / suburban / open */
    double shadow_db;       /* log-normal sigma; 0 reproduces the docs exactly */
    int    nchan;           /* hop channels; a group hops together, so this
                             * separates GROUPS, not members of one group */
    int    jam_chan;        /* -1 = no jammer */
} sim_channel;

typedef struct {
    int    id;
    double x, y;            /* metres */
    double vx, vy;          /* m/s */
    int    band;            /* 0 = capsule antenna, 1 = Band antenna */
    int    lang;
    int    group;
    int    adversary;
    hz_link_t profile;      /* Rich (SF9, 34 B) or Reach (SF10, 24 B, +2.5 dB) */

    weave_t   weave;
    beat_t    beat;
    uint32_t  period_ms;

    /* Crystal and phase.
     *   local_us(t) = phase0_us + (t - resync_us) * (1 + ppm*1e-6)
     * and a slot boundary is any t where local_us(t) is a multiple of the
     * period. Two fields, not one, because re-anchoring to a peer means moving
     * the PHASE — setting the clock to absolute simulator time instead would
     * align every node to the simulator and hide drift completely. */
    double   ppm;
    uint64_t resync_us;
    uint64_t phase0_us;

    /* one ratchet per peer. Full-mesh pairing is a simulator convenience;
     * the device pairs deliberately. */
    herus_session *sess;            /* SIM_MAX_NODES entries */
    link_dedup    *seen;            /* application-level retry suppression */
    uint8_t        paired[SIM_MAX_NODES];
    uint16_t       app_seq[SIM_MAX_NODES];

    /* energy, in uAh, kept apart so the report can say WHERE it went */
    double uah_tx, uah_rx, uah_mcu;

    /* counters */
    uint32_t n_sent, n_opened, n_relayed, n_missed_addr, n_auth_fail;
    uint32_t n_corrupt_seen, n_plaintext_leaks, n_recovered, n_dup;
    uint64_t rx_extra_us;           /* radio held open past the scan window */

    /* what an adversary in range managed to capture off the air */
    uint8_t  cap[SIM_FRAME_MAX];
    int      cap_len;
} sim_node;

/* Provenance: who originally sent this frame, when, and for whom. Keyed by the
 * SAME fingerprint weave uses for dedup (FNV-1a with the ttl bits masked), so a
 * relayed copy resolves to its origin. This table is the simulator's scoreboard
 * and is not readable by any node — a relay still sees 34 opaque bytes. */
#define SIM_PROV_N 8192
typedef struct {
    uint32_t fp;
    int      orig, dst, seq, hops;
    uint64_t born;
    /* Which nodes retransmitted this frame. The scoreboard needs it to answer a
     * question no node can answer: does relay BEHAVIOUR identify the recipient? */
    uint64_t relayed_by;
    uint8_t  used;
} sim_prov;

typedef struct {
    uint64_t   now_us;
    uint64_t   end_us;
    uint64_t   rng;
    uint64_t   ev_seq;

    sim_event *heap;
    int        nheap, cheap;

    sim_node  *n;
    int        nn;

    sim_tx      air[SIM_MAX_INFLIGHT];
    sim_channel ch;
    sim_prov   *prov;

    int        verbose;
    int        retries;         /* copies per message; 1 = send once, as before */
    int        cad;             /* 1 = listen before talk (SX1262 SetCad) */
    uint32_t   g_cad_defer;
    uint32_t   g_dup_suppressed;

    /* global ledger */
    uint32_t g_offered, g_delivered, g_collisions, g_below_sens, g_asleep;
    uint32_t g_relay_tx, g_dedup_drop, g_ttl_drop;
    uint32_t g_replay_tx, g_forge_tx, g_jam_tx;
    uint32_t g_false_deliveries;    /* must stay 0, forever */
    uint64_t lat_sum_us, lat_max_us;
    uint32_t lat_n;
} sim_world;

/* ---------------------------------------------------------------- world.c */
uint64_t sim_rand(sim_world *w);
double   sim_rand01(sim_world *w);
double   sim_gauss(sim_world *w);
void     sim_push(sim_world *w, uint64_t t_us, int type, int a, int b, int c);
int      sim_pop(sim_world *w, sim_event *out);
void     sim_world_init(sim_world *w, int nn, uint64_t seed);
void     sim_world_free(sim_world *w);

/* Node-local clock. The whole point of modelling it: beat.h's guard window is
 * a claim about crystal drift, and a claim needs something that can break it. */
uint64_t sim_local_us(const sim_node *n, uint64_t t_us);
uint64_t sim_true_us_of_local(const sim_node *n, uint64_t local_us);
int      sim_can_hear(const sim_node *n, uint64_t t_us, uint8_t sf);
uint64_t sim_next_slot_us(const sim_node *n, uint64_t t_us);
void     sim_discipline(sim_node *n, uint64_t arrival_us);
int      sim_medium_busy(sim_world *w, int id);

/* -------------------------------------------------------------- channel.c */
double sim_sens_dbm(int sf);
double sim_sens_nf(int sf, double nf_db);
double sim_i_tx_ma(double dbm);
double sim_range_at(const sim_channel *c, int sf, double g_dbi,
                    double tx_dbm, double nf_db);
double sim_path_loss_db(const sim_channel *c, double d_m);
double sim_rssi_dbm(const sim_world *w, int src, int dst, double extra_shadow);
double sim_dist_m(const sim_node *a, const sim_node *b);
double sim_range_m(const sim_channel *c, int sf, double g_dbi);

/* ----------------------------------------------------------------- node.c */
void sim_pair(sim_world *w, int a, int b, uint64_t seed);
void sim_pending_reset(void);
uint32_t sim_fingerprint(const uint8_t *frame, size_t len);

/* Raw transmit, starting now. Used by the relay path and by adversaries; the
 * application path goes through sim_queue_send so that it waits for a slot the
 * way the firmware does. */
int  sim_start_tx(sim_world *w, int src, const uint8_t *frame, int len,
                  uint8_t sf, int dst, int seq_app, int hops, uint64_t born_us);

/* Ask node `src` to say something to `dst` at t_us. The world defers the actual
 * transmission to the sender's next Beat boundary, which is where the latency
 * of a duty-cycled link comes from; born_us stays at t_us so the reported
 * latency is what the user experiences, not what the radio measures. */
void sim_queue_send(sim_world *w, uint64_t t_us, int src, int dst,
                    uint16_t intent, int nslot, const uint8_t *roles,
                    const uint16_t *fillers, uint8_t ttl);

void sim_settle_energy(sim_world *w);
void sim_run(sim_world *w);
double sim_mah_per_day(const sim_node *n, uint64_t duration_us);

/* --------------------------------------------------------------- render.c */
enum { LANG_PT = 0, LANG_EN, LANG_ES, LANG_N };
const char *lang_name(int lang);
void render_msg(char *out, size_t n, const hcp_msg_t *m, int lang);

/* ------------------------------------------------------------ scenarios.c */
typedef struct { int pass, fail; } sim_score;
void sim_ok(sim_score *s, int cond, const char *what);

void scenario_selftest(sim_score *s);
void scenario_range   (sim_score *s, int argc, char **argv);
void scenario_relay   (sim_score *s, int argc, char **argv);
void scenario_crowd   (sim_score *s, int argc, char **argv);
void scenario_attack  (sim_score *s, int argc, char **argv);
void scenario_day     (sim_score *s, int argc, char **argv);
void scenario_babel   (sim_score *s, int argc, char **argv);
void scenario_drift   (sim_score *s, int argc, char **argv);
void scenario_cognition(sim_score *s, int argc, char **argv);

/* ------------------------------------------------------------- stress.c ---
 * Scenarios written to FAIL. Each one targets a specific place the design is
 * suspected of yielding, and each states the number that decides it. A stress
 * scenario that passes on the first run either found nothing or was too polite. */
void scenario_deaf   (sim_score *s, int argc, char **argv);
void scenario_drain  (sim_score *s, int argc, char **argv);
void scenario_unmask (sim_score *s, int argc, char **argv);
void scenario_birthday(sim_score *s, int argc, char **argv);
void scenario_reliable(sim_score *s, int argc, char **argv);
void scenario_reach   (sim_score *s, int argc, char **argv);
void scenario_hardware(sim_score *s, int argc, char **argv);
void scenario_compose (sim_score *s, int argc, char **argv);
void scenario_study   (sim_score *s, int argc, char **argv);
void scenario_learn   (sim_score *s, int argc, char **argv);
void scenario_virtual (sim_score *s, int argc, char **argv);
void scenario_personal(sim_score *s, int argc, char **argv);
void scenario_semantic_life(sim_score *s, int argc, char **argv);
void scenario_physical_faults(sim_score *s, int argc, char **argv);
void scenario_authority(sim_score *s, int argc, char **argv);
void scenario_authority_benchmark(sim_score *s, int argc, char **argv);
void scenario_adaptive_change(sim_score *s, int argc, char **argv);
void scenario_poisoning(sim_score *s, int argc, char **argv);
void scenario_attribution(sim_score *s, int argc, char **argv);
void scenario_attribution_benchmark(sim_score *s, int argc, char **argv);
void scenario_attribution_composition(sim_score *s, int argc, char **argv);
void scenario_attribution_transitive(sim_score *s, int argc, char **argv);

int  opt_int   (int argc, char **argv, const char *k, int def);
double opt_dbl (int argc, char **argv, const char *k, double def);

#endif /* HERUS_SIM_H */
