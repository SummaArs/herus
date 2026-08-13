/* beat.h — slotted rendezvous: how low duty cycle and low latency coexist.
 *
 * Duty-cycled listening trades latency for power: a 2 s window means up to 2 s of
 * delay. Beat recovers most of it by making the windows AGREE. Every node in a
 * group opens its receiver on a common 2 s boundary derived from a shared epoch,
 * so a transmitter that also aligns hits a listening receiver on the first try
 * instead of on average half a period later.
 *
 * The radio does this without host involvement (SX1262 SetRxDutyCycle), so the
 * MCU sleeps at ~15 uA through every empty window. Waking the MCU 43 200 times a
 * day just to run channel activity detection would cost ~1.8 mAh/day on its own —
 * more than the radio it was trying to help. That number is why this module
 * computes a schedule instead of running a loop.
 *
 * TIMING, DERIVED (docs/02-PROTOCOL.md §6.2)
 * ------------------------------------------
 *   crystal tolerance      +-20 ppm each end
 *   relative drift         40 us/s
 *   drift over 60 s resync 2.40 ms
 *   guard window           +-10 ms  (4x the drift)
 *   SF9 symbol             4.096 ms
 *   8-symbol preamble      32.8 ms
 *   RX window              20 ms
 *   preamble >= 2 x guard? 32.8 >= 20   yes
 *
 * That last line is the load-bearing one: the standard 8-symbol preamble already
 * covers the guard window, so no long-preamble mode is needed and no extra
 * airtime is spent. It is a convenient accident of SF9's symbol duration and it
 * MUST be re-derived if the spreading factor ever changes — hence the static
 * assert in beat.c rather than a comment.
 */
#ifndef HERUS_BEAT_H
#define HERUS_BEAT_H

#include <stdint.h>

#define BEAT_PERIOD_MS        2000u    /* leaf slot period */
#define BEAT_PERIOD_RESP_MS    500u    /* responsive slot period */
#define BEAT_RX_MS              20u    /* receiver open time per slot */
#define BEAT_GUARD_MS           10u    /* +- guard, 4x the 60 s drift */
#define BEAT_RESYNC_MS       60000u    /* re-anchor at least this often */
#define BEAT_DRIFT_PPM          40u    /* relative, both ends worst case */

/* ---------------- WHY RESYNC IS FILTERED (finding B1) ----------------
 *
 * Revision 1 of beat_resync adopted the arrival instant whole, on the reasoning
 * that "the sender aimed it at a boundary, so we adopt that boundary." That is
 * true of a sender that transmits exactly on its boundary — and no group of more
 * than two nodes can afford one, because identical transmit instants collide
 * deterministically. Every real implementation adds a small random offset inside
 * the window, and the moment it does, the arrival instant stops being a clean
 * reference and becomes a NOISY measurement of one.
 *
 * Adopting a noisy measurement in full is a phase-locked loop with unity gain,
 * which does not lock: each resync injects the sender's random offset into the
 * receiver's phase, and with both ends transmitting the two phases random-walk
 * apart. The bench measured the consequence — two leaves, a clean 200 m link,
 * nothing hostile, and delivery fell from 100% to 91% purely from phase noise
 * the resync itself was manufacturing.
 *
 * The fix is the standard one for disciplining any clock, and it is two lines:
 *
 *   step  |error| > guard    adopt in full     (acquisition — we are genuinely
 *                                               lost and speed matters)
 *   slew  |error| <= guard   apply 1/GAIN      (tracking — the residual is
 *                                               mostly the sender's jitter, and
 *                                               averaging it is free)
 *
 * Real drift is 40 us/s, so 2.4 ms accumulates between resyncs; a quarter-step
 * correction erases that in one exchange and still averages away a +-12 ms
 * transmit offset over a handful. The gain is not tuned to taste — it is the
 * largest value that leaves the loop over-damped against the jitter it must
 * reject, and scenario `day` is where a wrong choice shows up. */
#define BEAT_RESYNC_GAIN         4

/* ---------------- THE TRANSMIT OFFSET IS PROTOCOL, NOT POLICY ----------------
 *
 * A sender must not transmit exactly on its boundary: in a group, every member
 * would pick the same instant and collide, every time, forever. So it picks a
 * random offset inside the window. That much is obvious and every implementation
 * does it.
 *
 * What is not obvious is that the offset then belongs in this header. A uniform
 * draw on [0, J) has mean J/2, and that mean is a SYSTEMATIC error in every phase
 * measurement the receiver makes — not noise. A loop filter averages noise away
 * and tracks a bias forever. Two ends that each pull the other later by J/2 per
 * exchange walk the whole group's phase forward at a rate comparable to the
 * crystal drift the guard window was sized for.
 *
 * The fix costs one subtraction, and it only works if BOTH ends agree on J. So J
 * is defined once, here, next to the guard it interacts with — and beat_resync
 * removes the known mean before filtering the remainder.
 *
 * J must stay well inside BEAT_RX_MS or the offset alone would miss the window
 * it was supposed to land in. */
#define BEAT_TX_JITTER_MS       12u
_Static_assert(BEAT_TX_JITTER_MS < BEAT_RX_MS,
               "the transmit offset must fit inside the receive window");

/* ---------------- LISTEN BEFORE TALK (finding C1) ----------------
 *
 * Beat makes every node in a group open its receiver on the same boundary. That
 * is the point — and it means every node also wants to TRANSMIT on the same
 * boundary. A random offset inside the window keeps two senders from landing on
 * the same microsecond, but the window is 20 ms and a frame is 247 ms, so two
 * senders in one slot period overlap for 92% of their airtime no matter what
 * offset they draw. Jitter cannot fix a collision it is 12x too small to avoid.
 *
 * The bench measured the consequence: twelve units, sixty messages, 6642 lost
 * receptions and 27.7% band occupancy. Delivery survived only because flooding
 * retried it for free, which is not a design, it is luck being paid for in
 * other people's batteries.
 *
 * The SX1262 answers this in hardware: SetCad listens for LoRa energy in a few
 * symbol times and reports back, for far less than the cost of a collision. A
 * sender that finds the channel busy skips to a later slot instead of talking
 * over it. The skip is RANDOM over BEAT_CAD_SPREAD slots, because a deterministic
 * "try again next slot" turns one collision into a permanent lockstep between
 * the same two nodes.
 *
 * What it costs: one CAD per transmission (~2 symbol times, under 1% of a frame)
 * and added latency exactly when the band is busy — which is when a collision
 * would have cost the whole frame anyway. */
#define BEAT_CAD_SPREAD          4u    /* random slot skip when the band is busy */
#define BEAT_CAD_MAX_DEFER       6u    /* give up deferring and take the risk */

typedef struct {
    uint64_t epoch_ms;        /* shared phase origin */
    uint32_t period_ms;
    uint8_t  clock_class;     /* 0 = best (GPS/NTP), 3 = free-running */
    uint64_t last_resync_ms;
} beat_t;

void beat_init(beat_t *b, uint64_t now_ms, uint32_t period_ms, uint8_t clock_class);

/* Milliseconds until the next slot opens (0 if a slot is open now). The radio is
 * armed with this, then the MCU sleeps. */
uint32_t beat_until_next_slot_ms(const beat_t *b, uint64_t now_ms);

/* 1 if a receive slot is open at now_ms, guard included. */
int  beat_slot_open(const beat_t *b, uint64_t now_ms);

/* When to start transmitting so the frame's preamble lands inside the peer's
 * window. Returns milliseconds to wait. */
uint32_t beat_until_tx_ms(const beat_t *b, uint64_t now_ms);

/* Re-anchor the local phase from a frame's arrival instant. Only a peer with a
 * clock class at least as good as ours may move our phase, or a free-running node
 * with a bad crystal drags the whole group off the boundary. Returns 1 if the
 * phase moved. */
int  beat_resync(beat_t *b, uint64_t arrival_ms, uint8_t peer_clock_class);

/* How much of an observed phase error this node should actually apply, after
 * removing the sender's known mean transmit offset. Exposed
 * rather than kept private so that a host simulation disciplines its clock by
 * exactly the rule the firmware uses — two copies of a control law is two
 * control laws, and only one of them ships. */
int32_t beat_correction_ms(int32_t observed_off_ms);

/* Accumulated worst-case drift since the last resync, in microseconds. When this
 * approaches the guard window the node must resync or widen its window — that
 * decision is the caller's, and this is the number it needs. */
uint32_t beat_drift_us(const beat_t *b, uint64_t now_ms);

#endif /* HERUS_BEAT_H */
