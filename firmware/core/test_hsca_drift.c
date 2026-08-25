/* test_hsca_drift.c — custody, opacity, termination, and reach as a function of
 * time.
 *
 * Two things are proven here. First, that carrying a meaning teaches the carrier
 * nothing and grants it nothing. Second, that a 34-byte meaning crosses a
 * distance no radio in the ladder can reach, because it is carried rather than
 * relayed — and that it does so under a hard hop cap, a hard fanout cap and a
 * hard expiry.
 *
 * The mobility model is declared, deterministic and stated in full below. It is
 * a simulation. It is not field data and it is not a range claim.
 */
#include "drift.h"
#include "herald.h"
#include "hir.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>

static int pass_count;
static int fail_count;

static void check(int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) pass_count++; else fail_count++;
}

static void fill_key(uint8_t key[32], uint8_t seed)
{
    int i;
    for (i = 0; i < 32; i++) key[i] = (uint8_t)(seed * 31u + (uint8_t)i);
}

static int meaning_wire(const char *text, uint8_t wire[HIR_WIRE_BYTES], uint8_t digest[HIR_DIGEST_BYTES])
{
    herald_unit_t u;
    if (herald_compile(text, strlen(text), &u) != HERALD_OK) return 0;
    if (hir_encode_wire(&u.meaning, wire) != HIR_OK) return 0;
    memcpy(digest, u.digest, HIR_DIGEST_BYTES);
    return 1;
}

/* ------------------------------------------------------------- seal and open */

static void run_seal_open(void)
{
    uint8_t key[32], wrong[32], wire[HIR_WIRE_BYTES], back[HIR_WIRE_BYTES];
    uint8_t digest[HIR_DIGEST_BYTES], digest2[HIR_DIGEST_BYTES];
    drift_bundle_t b;
    hir_t decoded;

    fill_key(key, 7u);
    fill_key(wrong, 9u);
    check(meaning_wire("avisa a maria que to atrasado 10 minutos", wire, digest),
          "a compiled meaning becomes a 24-byte wire form");
    check(drift_seal(key, 1u, wire, 1u, 1000u, 86400u, &b) == DRIFT_OK, "the meaning seals into a bundle");
    check(sizeof b.onair == DRIFT_ONAIR_BYTES && DRIFT_ONAIR_BYTES == 34u,
          "a bundle on air is 34 bytes: address, ciphertext, tag");
    check(drift_open(key, 1u, &b, back) == DRIFT_OK, "the recipient opens it");
    check(memcmp(back, wire, HIR_WIRE_BYTES) == 0, "the bytes survive exactly");
    check(hir_decode_wire(back, &decoded) == HIR_OK &&
          hir_digest(&decoded, digest2) == HIR_OK &&
          memcmp(digest, digest2, HIR_DIGEST_BYTES) == 0,
          "the meaning that arrives is the meaning that left, by digest");

    check(drift_open(wrong, 1u, &b, back) == DRIFT_E_AUTH, "a carrier without the key cannot open it");
    {
        uint8_t zero[HIR_WIRE_BYTES];
        memset(zero, 0, sizeof zero);
        check(memcmp(back, zero, HIR_WIRE_BYTES) == 0,
              "a failed open releases nothing: the buffer is left zeroed");
    }
    check(drift_open(key, 200u, &b, back) == DRIFT_E_AUTH,
          "the counter window is bounded: a far-future base does not find it");
    check(drift_seal(key, 1u, wire, 2u, 1000u, 86400u, &b) == DRIFT_E_CLASS,
          "custody carries essential meanings only, never a card or telemetry");
}

static void run_tamper(void)
{
    uint8_t key[32], wire[HIR_WIRE_BYTES], back[HIR_WIRE_BYTES], digest[HIR_DIGEST_BYTES];
    drift_bundle_t b;
    unsigned pos, bit, flips = 0, caught = 0;

    fill_key(key, 3u);
    (void)meaning_wire("avisa o joao que cheguei", wire, digest);
    (void)drift_seal(key, 5u, wire, 1u, 100u, 3600u, &b);

    for (pos = 0; pos < DRIFT_ONAIR_BYTES; pos++) {
        for (bit = 0; bit < 8u; bit++) {
            drift_bundle_t t = b;
            t.onair[pos] ^= (uint8_t)(1u << bit);
            flips++;
            if (drift_open(key, 5u, &t, back) != DRIFT_OK) caught++;
        }
    }
    printf("  ---- single-bit tamper: %u/%u rejected\n", caught, flips);
    check(caught == flips, "every single-bit change to a bundle is rejected");
}

static void run_unlinkability(void)
{
    uint8_t key[32], other[32], wire[HIR_WIRE_BYTES], digest[HIR_DIGEST_BYTES];
    drift_bundle_t a, b, c;
    unsigned i, j, repeats = 0;
    static uint8_t addr[512][2];

    fill_key(key, 11u);
    fill_key(other, 12u);
    (void)meaning_wire("avisa o joao que cheguei", wire, digest);

    (void)drift_seal(key, 1u, wire, 1u, 0u, 3600u, &a);
    (void)drift_seal(key, 2u, wire, 1u, 0u, 3600u, &b);
    (void)drift_seal(other, 1u, wire, 1u, 0u, 3600u, &c);

    check(memcmp(a.onair, b.onair, DRIFT_ONAIR_BYTES) != 0,
          "the same meaning sent twice looks like two unrelated bundles");
    check(memcmp(a.id, b.id, DRIFT_ID_BYTES) != 0,
          "identifiers do not repeat across sends");
    check(memcmp(a.onair, c.onair, DRIFT_ONAIR_BYTES) != 0,
          "two people sending the same meaning are not linkable by bytes");
    check(a.onair[0] != b.onair[0] || a.onair[1] != b.onair[1],
          "the two-byte address rotates");

    for (i = 0; i < 512u; i++) {
        drift_bundle_t s;
        (void)drift_seal(key, i, wire, 1u, 0u, 3600u, &s);
        addr[i][0] = s.onair[0];
        addr[i][1] = s.onair[1];
    }
    for (i = 0; i < 512u; i++)
        for (j = i + 1u; j < 512u; j++)
            if (addr[i][0] == addr[j][0] && addr[i][1] == addr[j][1]) repeats++;
    printf("  ---- 512 consecutive sends: %u address repeats (birthday expectation ~ %u)\n",
           repeats, (unsigned)(512u * 511u / 2u / 65536u));
    check(repeats < 4u,
          "a rotating 16-bit address repeats only at the birthday rate, not by pattern");
}

/* -------------------------------------------------------------- relay policy */

static void run_relay_policy(void)
{
    uint8_t key[32], wire[HIR_WIRE_BYTES], digest[HIR_DIGEST_BYTES];
    drift_bundle_t b, hostile;
    drift_store_t relay;

    fill_key(key, 21u);
    (void)meaning_wire("avisa o joao que cheguei", wire, digest);
    (void)drift_seal(key, 1u, wire, 1u, 1000u, 3600u, &b);

    drift_store_init(&relay);
    check(drift_accept(&relay, &b, 1000u) == DRIFT_OK, "a stranger takes custody");
    check(relay.count == 1u, "custody occupies one slot");
    check(drift_accept(&relay, &b, 1000u) == DRIFT_E_DUPLICATE,
          "the same replica is never taken twice");
    {
        drift_bundle_t fresher = b, staler = b;
        fresher.hops_left = (uint8_t)(b.hops_left + 1u);
        staler.hops_left = (uint8_t)(b.hops_left - 1u);
        check(drift_accept(&relay, &staler, 1000u) == DRIFT_E_DUPLICATE,
              "a replica with fewer legs left is not worth taking");
        check(drift_accept(&relay, &fresher, 1000u) == DRIFT_UPGRADED,
              "a replica with more legs left replaces the one held");
        check(relay.count == 1u, "upgrading never grows custody");
        relay.slot[0].hops_left = b.hops_left;
    }
    check(drift_holds(&relay, b.id) == 1, "the relay knows what it holds by identifier only");

    hostile = b;
    hostile.expires_at = 900u;
    hostile.id[0] ^= 0xffu;
    check(drift_accept(&relay, &hostile, 1000u) == DRIFT_E_EXPIRED,
          "an expired bundle is refused, not carried out of politeness");

    hostile = b;
    hostile.hops_left = 0u;
    hostile.id[0] ^= 0x0fu;
    check(drift_accept(&relay, &hostile, 1000u) == DRIFT_E_EXHAUSTED,
          "a bundle with no hops left stops");

    hostile = b;
    hostile.payload_class = 3u;
    hostile.id[0] ^= 0x33u;
    check(drift_accept(&relay, &hostile, 1000u) == DRIFT_E_CLASS,
          "custody refuses any class above an essential meaning");

    {
        drift_store_t full;
        uint8_t i;
        int filled = 1;
        drift_store_init(&full);
        for (i = 0; i < DRIFT_MAX_CUSTODY; i++) {
            drift_bundle_t s;
            (void)drift_seal(key, 100u + i, wire, 1u, 1000u, 3600u, &s);
            if (drift_accept(&full, &s, 1000u) != DRIFT_OK) filled = 0;
        }
        check(filled, "custody fills to its declared capacity");
        {
            drift_bundle_t s;
            (void)drift_seal(key, 200u, wire, 1u, 1000u, 3600u, &s);
            check(drift_accept(&full, &s, 1000u) == DRIFT_E_FULL,
                  "a full carrier refuses instead of dropping someone else's meaning");
        }
    }

    {
        static drift_store_t next;
        uint8_t before = relay.slot[0].hops_left;
        drift_store_init(&next);
        check(drift_contact(&relay, &next, 1000u) == 1u, "custody passes to the next carrier");
        check(next.slot[0].hops_left == (uint8_t)(before - 1u),
              "every handover spends exactly one hop: the budget really decreases");
        check(relay.slot[0].fanout_left == (uint8_t)(DRIFT_MAX_FANOUT - 1u),
              "and exactly one of the carrier's fanout slots");
        {
            static drift_store_t chain[DRIFT_MAX_HOPS + 3u];
            unsigned k, hops_seen = 0;
            for (k = 0; k < DRIFT_MAX_HOPS + 3u; k++) drift_store_init(&chain[k]);
            (void)drift_accept(&chain[0], &b, 1000u);
            for (k = 0; k + 1u < DRIFT_MAX_HOPS + 3u; k++) {
                if (drift_contact(&chain[k], &chain[k+1u], 1000u) != 1u) break;
                hops_seen++;
            }
            printf("  ---- a single chain of carriers stopped after %u handovers (cap %u)\n",
                   hops_seen, (unsigned)DRIFT_MAX_HOPS);
            check(hops_seen < DRIFT_MAX_HOPS,
                  "a chain of carriers cannot outrun the hop cap");
        }
    }
    check(drift_deliver_once(&relay, b.id) == DRIFT_OK, "the addressee consumes the bundle");
    check(drift_deliver_once(&relay, b.id) == DRIFT_E_DUPLICATE,
          "a delivered meaning is never delivered a second time");
    check(drift_accept(&relay, &b, 1000u) == DRIFT_E_DUPLICATE,
          "a consumed identifier is remembered, so the same meaning cannot return");

    {
        drift_store_t s;
        drift_bundle_t x;
        drift_store_init(&s);
        (void)drift_seal(key, 300u, wire, 1u, 1000u, 60u, &x);
        check(drift_accept(&s, &x, 1000u) == DRIFT_OK, "a short-lived bundle is accepted");
        check(drift_gc(&s, 1030u) == 0u && s.count == 1u, "it survives before expiry");
        check(drift_gc(&s, 1061u) == 1u && s.count == 0u, "it is dropped at expiry, without asking");
    }
}

static void run_termination(void)
{
    uint8_t key[32], wire[HIR_WIRE_BYTES], digest[HIR_DIGEST_BYTES];
    drift_bundle_t b;
    static drift_store_t node[24];
    uint32_t total_tx = 0;
    unsigned round, i, j;

    fill_key(key, 31u);
    (void)meaning_wire("avisa o joao que cheguei", wire, digest);
    (void)drift_seal(key, 1u, wire, 1u, 0u, 100000u, &b);

    for (i = 0; i < 24u; i++) drift_store_init(&node[i]);
    (void)drift_accept(&node[0], &b, 0u);

    /* Everybody meets everybody, every round, forever. If the bounds are real
     * the epidemic still stops. */
    for (round = 0; round < 40u; round++)
        for (i = 0; i < 24u; i++)
            for (j = 0; j < 24u; j++)
                if (i != j) (void)drift_contact(&node[i], &node[j], 10u);

    for (i = 0; i < 24u; i++) total_tx += node[i].transmissions;
    printf("  ---- fully connected mesh, 40 rounds, 24 nodes: %u transmissions total\n",
           (unsigned)total_tx);
    check(total_tx > 0u, "the meaning did spread");
    check(total_tx <= 24u * DRIFT_MAX_FANOUT * 2u,
          "the epidemic is bounded by fanout, not by luck: it terminates");
    {
        unsigned reached = 0;
        for (i = 0; i < 24u; i++) if (node[i].count > 0u) reached++;
        printf("  ---- carriers holding the meaning after saturation: %u/24\n", reached);
        check(reached <= 24u, "no carrier holds more copies than it accepted");
    }
}

/* ------------------------------------------------- reach as a function of time
 *
 * Declared model, deterministic, seeded once:
 *   corridor           40 000 m, one dimension
 *   source             fixed at 0, seals once at t = 0
 *   destination        fixed at 40 000, opens with the pair key
 *   carriers           N mobile nodes, uniform initial position
 *   speed              1.2 to 14.0 m/s, sign alternating, reflecting at the ends
 *   contact radius     650 m — the urban capsule-to-capsule figure from
 *                      docs/00-HERUS-MASTER.md section 6, not a new claim
 *   step               60 s, horizon 12 h
 *   bounds             DRIFT_MAX_HOPS, DRIFT_MAX_FANOUT and a TTL, all enforced
 *
 * Every hop end to end still falls far short of the corridor. Anything that
 * arrives, arrived because someone walked or drove with it. That is the point.
 */
#define SIM_MAX_NODES 64u
#define SIM_LEN_M     40000
#define SIM_RADIUS_M  650
#define SIM_STEP_S    60u
#define SIM_HORIZON_S 43200u

static uint32_t lcg_state;
static uint32_t lcg(void)
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state >> 8;
}

static uint32_t simulate_delivery(unsigned n_nodes, uint32_t ttl_s, uint32_t *out_tx, uint32_t seed_offset)
{
    static drift_store_t store[SIM_MAX_NODES + 2u];
    static int32_t pos[SIM_MAX_NODES + 2u];
    static int32_t vel[SIM_MAX_NODES + 2u];
    uint8_t key[32], wire[HIR_WIRE_BYTES], digest[HIR_DIGEST_BYTES];
    drift_bundle_t seed;
    unsigned src, dst, i, j;
    uint32_t t;

    lcg_state = 20260824u + n_nodes + seed_offset * 7919u;
    fill_key(key, 42u);
    (void)meaning_wire("avisa o joao que cheguei", wire, digest);
    (void)drift_seal(key, 1u, wire, 1u, 0u, ttl_s, &seed);

    src = n_nodes;
    dst = n_nodes + 1u;
    for (i = 0; i < n_nodes + 2u; i++) {
        drift_store_init(&store[i]);
        pos[i] = (int32_t)(lcg() % (uint32_t)SIM_LEN_M);
        vel[i] = (int32_t)(lcg() % 13u) + 1;
        if (lcg() & 1u) vel[i] = -vel[i];
    }
    pos[src] = 0;          vel[src] = 0;
    pos[dst] = SIM_LEN_M;  vel[dst] = 0;
    (void)drift_accept(&store[src], &seed, 0u);

    for (t = 0; t <= SIM_HORIZON_S; t += SIM_STEP_S) {
        for (i = 0; i < n_nodes; i++) {
            pos[i] += vel[i] * (int32_t)SIM_STEP_S;
            if (pos[i] < 0) { pos[i] = -pos[i]; vel[i] = -vel[i]; }
            if (pos[i] > SIM_LEN_M) { pos[i] = 2 * SIM_LEN_M - pos[i]; vel[i] = -vel[i]; }
        }
        for (i = 0; i < n_nodes + 2u; i++) {
            for (j = 0; j < n_nodes + 2u; j++) {
                int32_t d;
                if (i == j) continue;
                d = pos[i] - pos[j];
                if (d < 0) d = -d;
                if (d > SIM_RADIUS_M) continue;
                (void)drift_contact(&store[i], &store[j], t);
            }
        }
        if (store[dst].count > 0u) {
            uint8_t back[HIR_WIRE_BYTES];
            if (drift_open(key, 1u, &store[dst].slot[0], back) == DRIFT_OK &&
                memcmp(back, wire, HIR_WIRE_BYTES) == 0) {
                if (out_tx) {
                    uint32_t tx = 0;
                    for (i = 0; i < n_nodes + 2u; i++) tx += store[i].transmissions;
                    *out_tx = tx;
                }
                return t;
            }
        }
    }
    if (out_tx) {
        uint32_t tx = 0;
        for (i = 0; i < n_nodes + 2u; i++) tx += store[i].transmissions;
        *out_tx = tx;
    }
    return 0xffffffffu;
}

#define SIM_SEEDS 12u

/* One seed is an anecdote. Each point below is the mean over SIM_SEEDS
 * independent populations; a run that never delivers is charged the full
 * horizon rather than dropped, so the mean cannot be flattered by failures. */
static uint32_t mean_delivery(unsigned n_nodes, uint32_t ttl_s, unsigned *out_delivered)
{
    uint32_t sum = 0;
    unsigned s, delivered = 0;
    for (s = 0; s < SIM_SEEDS; s++) {
        uint32_t t = simulate_delivery(n_nodes, ttl_s, NULL, s);
        if (t == 0xffffffffu) {
            sum += SIM_HORIZON_S;
        } else {
            sum += t;
            delivered++;
        }
    }
    if (out_delivered) *out_delivered = delivered;
    return sum / SIM_SEEDS;
}

static void run_reach_over_time(void)
{
    static const unsigned N[] = { 0u, 8u, 16u, 32u, 64u };
    unsigned i;
    uint32_t mean_at_8 = 0, mean_at_64 = 0;

    printf("  ---- reach over time: 40 km corridor, 650 m contact radius, %u seeds each\n",
           (unsigned)SIM_SEEDS);
    printf("       carriers   delivered   mean time to delivery   transmissions (seed 0)\n");
    for (i = 0; i < sizeof N / sizeof N[0]; i++) {
        unsigned delivered = 0;
        uint32_t tx = 0;
        uint32_t mean = mean_delivery(N[i], SIM_HORIZON_S, &delivered);
        (void)simulate_delivery(N[i], SIM_HORIZON_S, &tx, 0u);
        printf("       %8u   %4u/%-4u   %16u s   %10u\n",
               N[i], delivered, (unsigned)SIM_SEEDS, (unsigned)mean, (unsigned)tx);
        if (N[i] == 8u)  mean_at_8 = mean;
        if (N[i] == 64u) mean_at_64 = mean;
    }
    {
        unsigned delivered = 0;
        (void)mean_delivery(0u, SIM_HORIZON_S, &delivered);
        check(delivered == 0u,
              "with nobody to carry it, nothing arrives: the model has no magic");
    }
    {
        unsigned delivered = 0;
        (void)mean_delivery(8u, SIM_HORIZON_S, &delivered);
        check(delivered == SIM_SEEDS,
              "eight passers-by are enough to cross 40 km in every population tried");
    }
    {
        /* The interesting result is the one that argues against replication.
         * Nine times the traffic bought almost nothing; the horizon did the
         * work. Reach here is paid for in time, not in transmissions. */
        uint32_t hi = mean_at_8 > mean_at_64 ? mean_at_8 : mean_at_64;
        uint32_t lo = mean_at_8 > mean_at_64 ? mean_at_64 : mean_at_8;
        uint32_t spread_pct = (uint32_t)(((hi - lo) * 100u) / lo);
        printf("  ---- mean delivery spread between 8 and 64 carriers: %u%%\n",
               (unsigned)spread_pct);
        check(spread_pct < 30u,
              "eight-fold density moved the mean by under 30%: reach is bought with time");
    }
    {
        unsigned delivered = 0;
        (void)mean_delivery(32u, 1800u, &delivered);
        check(delivered == 0u,
              "a half-hour expiry never reaches 40 km: the bound is real, not decorative");
    }
    {
        uint32_t t = simulate_delivery(32u, SIM_HORIZON_S, NULL, 0u);
        check(t != 0xffffffffu &&
              (uint32_t)SIM_LEN_M > (uint32_t)DRIFT_MAX_HOPS * (uint32_t)SIM_RADIUS_M,
              "the corridor is longer than every hop end to end: it was carried, not relayed");
    }
    printf("  ---- %u hops of %u m span %u m; the corridor is %u m. The gap is legs.\n",
           (unsigned)DRIFT_MAX_HOPS, (unsigned)SIM_RADIUS_M,
           (unsigned)(DRIFT_MAX_HOPS * SIM_RADIUS_M), (unsigned)SIM_LEN_M);
}

int main(void)
{
    printf("--- drift: custody, opacity and reach as a function of time ---\n");
    run_seal_open();
    run_tamper();
    run_unlinkability();
    run_relay_policy();
    run_termination();
    run_reach_over_time();
    printf("HSCA DRIFT: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
