/* world.c — virtual time, deterministic randomness, and the node-local clock.
 *
 * Two properties are non-negotiable and everything here exists to hold them:
 *
 *   DETERMINISM.  Same seed, same result, byte for byte. A simulator whose
 *                 output moves between runs cannot be used to decide whether a
 *                 change helped. Ties in the event queue are broken by insertion
 *                 order, not by whatever the sort happened to do.
 *
 *   VIRTUAL TIME. A day of battery life runs in milliseconds of wall clock,
 *                 because nothing is ever waited for — the clock jumps to the
 *                 next event. This is why the "day" scenario is affordable.
 */
#include "sim.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* splitmix64 — the same generator the codebook uses, for the same reason:
 * it is deterministic, seedable and has no state to get out of sync. */
uint64_t sim_rand(sim_world *w)
{
    uint64_t z = (w->rng += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

double sim_rand01(sim_world *w)
{
    return (double)(sim_rand(w) >> 11) / 9007199254740992.0;   /* 2^53 */
}

/* Box-Muller. Used only for log-normal shadowing, which is off by default. */
double sim_gauss(sim_world *w)
{
    double u1 = sim_rand01(w), u2 = sim_rand01(w);
    if (u1 < 1e-12) u1 = 1e-12;
    return sqrt(-2.0 * log(u1)) * cos(6.283185307179586 * u2);
}

/* ------------------------------------------------------------ event queue */
static int ev_before(const sim_event *a, const sim_event *b)
{
    if (a->t_us != b->t_us) return a->t_us < b->t_us;
    return a->seq < b->seq;
}

void sim_push(sim_world *w, uint64_t t_us, int type, int a, int b, int c)
{
    if (w->nheap == w->cheap) {
        w->cheap = w->cheap ? w->cheap * 2 : 256;
        w->heap = realloc(w->heap, (size_t)w->cheap * sizeof *w->heap);
    }
    sim_event e = { t_us, w->ev_seq++, type, a, b, c };
    int i = w->nheap++;
    w->heap[i] = e;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (!ev_before(&w->heap[i], &w->heap[p])) break;
        sim_event t = w->heap[p]; w->heap[p] = w->heap[i]; w->heap[i] = t;
        i = p;
    }
}

int sim_pop(sim_world *w, sim_event *out)
{
    if (w->nheap == 0) return 0;
    *out = w->heap[0];
    w->heap[0] = w->heap[--w->nheap];
    int i = 0;
    for (;;) {
        int l = 2 * i + 1, r = l + 1, m = i;
        if (l < w->nheap && ev_before(&w->heap[l], &w->heap[m])) m = l;
        if (r < w->nheap && ev_before(&w->heap[r], &w->heap[m])) m = r;
        if (m == i) break;
        sim_event t = w->heap[m]; w->heap[m] = w->heap[i]; w->heap[i] = t;
        i = m;
    }
    return 1;
}

/* ------------------------------------------------------------------ setup */
void sim_world_init(sim_world *w, int nn, uint64_t seed)
{
    memset(w, 0, sizeof *w);
    w->rng  = seed ? seed : 0x1234567890abcdefull;
    w->nn   = nn;
    w->n    = calloc((size_t)nn, sizeof *w->n);
    w->prov = calloc(SIM_PROV_N, sizeof *w->prov);
    sim_pending_reset();
    w->retries       = 1;
    w->ch.clutter_db = SIM_CLUTTER_URBAN;
    w->ch.shadow_db  = 0.0;
    w->ch.nchan      = 1;
    w->ch.jam_chan   = -1;

    for (int i = 0; i < nn; i++) {
        sim_node *n = &w->n[i];
        n->id        = i;
        n->band      = 0;
        n->lang      = LANG_PT;
        n->group     = 0;
        n->period_ms = BEAT_PERIOD_MS;
        n->sess      = calloc(SIM_MAX_NODES, sizeof *n->sess);
        n->seen      = calloc(SIM_MAX_NODES, sizeof *n->seen);
        /* +-20 ppm, uniformly drawn. beat.h budgets for exactly this. */
        n->ppm       = (sim_rand01(w) * 2.0 - 1.0) * 20.0;
        n->resync_us = 0;
        n->phase0_us = 0;
        weave_init(&n->weave, WEAVE_LEAF);
        beat_init(&n->beat, 0, n->period_ms, 3);
    }
}

void sim_world_free(sim_world *w)
{
    for (int i = 0; i < w->nn; i++) { free(w->n[i].sess); free(w->n[i].seen); }
    free(w->n);
    free(w->heap);
    free(w->prov);
    memset(w, 0, sizeof *w);
}

/* ------------------------------------------------------------ local clock */
uint64_t sim_local_us(const sim_node *n, uint64_t t_us)
{
    if (t_us < n->resync_us) return n->phase0_us;
    double d = (double)(t_us - n->resync_us) * (1.0 + n->ppm * 1e-6);
    return n->phase0_us + (uint64_t)(d + 0.5);
}

uint64_t sim_true_us_of_local(const sim_node *n, uint64_t local_us)
{
    if (local_us <= n->phase0_us) return n->resync_us;
    double d = (double)(local_us - n->phase0_us) / (1.0 + n->ppm * 1e-6);
    return n->resync_us + (uint64_t)(d + 0.5);
}

/* Can this receiver hear a frame whose preamble starts at t_us?
 *
 * A relay listens continuously. Everyone else opens the radio for BEAT_RX_MS at
 * every boundary of its OWN clock — and BEAT_RX_MS is all the energy it spends,
 * which is why the leaf figure comes out at exactly the published 1.78 mAh/day.
 *
 * Detection is NOT "was the window open at t_us". It is "did the window overlap
 * the PREAMBLE", and that distinction is the entire content of beat.h's
 * load-bearing line:
 *
 *     8-symbol preamble 32.8 ms >= 2 x guard 20 ms   -> no long-preamble mode
 *
 * A receiver whose window opened 5 ms after the sender started still hears it,
 * because the preamble is still going. Modelling detection as an instant test
 * instead of an overlap test made the relay deliver nothing at all in the first
 * run of this simulator — the sender was 80 microseconds early. That failure was
 * the simulator's, not the design's, and the fix is to model what the radio
 * actually does.
 *
 * The tolerance this yields is deliberately ASYMMETRIC: the receiver's boundary
 * may sit anywhere from BEAT_RX_MS before the sender's to one preamble after it.
 * Scenario 7 walks the crystal until it leaves that band.
 */
int sim_can_hear(const sim_node *n, uint64_t t_us, uint8_t sf)
{
    if (n->weave.role == WEAVE_RELAY) return 1;

    uint64_t pre = 8ull * HZ_TSYM_US(sf);          /* 8 symbols; 32.768 ms at SF9 */
    uint64_t l0  = sim_local_us(n, t_us);
    uint64_t l1  = sim_local_us(n, t_us + pre);
    uint64_t p   = (uint64_t)n->period_ms * 1000u;
    uint64_t rx  = (uint64_t)BEAT_RX_MS * 1000u;

    uint64_t k = l0 / p;
    if (k * p + rx > l0) return 1;                 /* window open when it starts */
    if ((k + 1) * p < l1) return 1;                /* window opens mid-preamble  */
    return 0;
}

/* Discipline this node's phase from the instant a peer's preamble arrived.
 *
 * The correction rule is NOT written here — beat_correction_ms() in the firmware
 * decides how much of the observed error to apply, and this function only has to
 * measure the error and obey. Two copies of a control law is two control laws,
 * and only one of them ever gets flashed.
 *
 * Adopting the arrival instant whole was what the first version did, and it made
 * two leaves on a clean link lose 9% of their traffic to phase noise the resync
 * itself was creating (finding B1). */
void sim_discipline(sim_node *n, uint64_t arrival_us)
{
    uint64_t p = (uint64_t)n->period_ms * 1000u;
    uint64_t l = sim_local_us(n, arrival_us);

    /* Signed phase error, in the node's own timeline: how far our boundary sits
     * from where the peer says it should be. */
    int64_t off = (int64_t)(l % p);
    if (off > (int64_t)(p / 2)) off -= (int64_t)p;

    int64_t applied = 1000ll * beat_correction_ms((int32_t)(off / 1000));

    int64_t target = (int64_t)l - applied;
    target %= (int64_t)p;
    if (target < 0) target += (int64_t)p;

    n->resync_us = arrival_us;
    n->phase0_us = (uint64_t)target;
}

/* True time at which this node's own clock next reaches a slot boundary. */
uint64_t sim_next_slot_us(const sim_node *n, uint64_t t_us)
{
    uint64_t l = sim_local_us(n, t_us);
    uint64_t p = (uint64_t)n->period_ms * 1000u;
    uint64_t nb = ((l / p) + 1) * p;
    return sim_true_us_of_local(n, nb);
}
