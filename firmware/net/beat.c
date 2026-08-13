/* beat.c — see beat.h. */
#include "beat.h"
#include "region.h"

/* The claim in docs/02-PROTOCOL.md §6.2 that no long preamble is needed, turned
 * into a build failure if it ever stops being true. An 8-symbol preamble at SF9
 * is 32.8 ms and must cover the +-10 ms guard window, i.e. 2 x 10 ms. Change the
 * spreading factor and this line, not the comment, is what tells you. */
_Static_assert((49u * HZ_TSYM_US(HERUS_SF_MEANING)) / 4u >= 2u * BEAT_GUARD_MS * 1000u,
               "Beat: the preamble no longer covers the guard window — a long "
               "preamble mode or a smaller guard is now required");

/* Worst-case relative drift over one resync interval must stay inside the guard,
 * or two nodes miss each other before they get a chance to re-anchor. */
_Static_assert((uint64_t)BEAT_RESYNC_MS * BEAT_DRIFT_PPM / 1000u <= BEAT_GUARD_MS * 1000u,
               "Beat: drift over the resync interval exceeds the guard window");

void beat_init(beat_t *b, uint64_t now_ms, uint32_t period_ms, uint8_t clock_class)
{
    b->epoch_ms      = now_ms;
    b->period_ms     = period_ms ? period_ms : BEAT_PERIOD_MS;
    b->clock_class   = clock_class;
    b->last_resync_ms = now_ms;
}

static uint32_t phase_ms(const beat_t *b, uint64_t now_ms)
{
    uint64_t d = now_ms - b->epoch_ms;
    return (uint32_t)(d % b->period_ms);
}

int beat_slot_open(const beat_t *b, uint64_t now_ms)
{
    uint32_t p = phase_ms(b, now_ms);
    /* The window is [-guard, RX + guard) around the boundary, so a peer whose
     * clock has drifted either way is still heard. */
    return (p < BEAT_RX_MS + BEAT_GUARD_MS) ||
           (p >= b->period_ms - BEAT_GUARD_MS);
}

uint32_t beat_until_next_slot_ms(const beat_t *b, uint64_t now_ms)
{
    if (beat_slot_open(b, now_ms)) return 0;
    uint32_t p = phase_ms(b, now_ms);
    return b->period_ms - BEAT_GUARD_MS - p;
}

uint32_t beat_until_tx_ms(const beat_t *b, uint64_t now_ms)
{
    /* Aim the START of the preamble at the boundary minus the guard: the preamble
     * is 32.8 ms and the peer's window is 20 ms wide plus guards, so the peer
     * detects the preamble regardless of which end drifted. */
    uint32_t p = phase_ms(b, now_ms);
    uint32_t target = b->period_ms - BEAT_GUARD_MS;
    if (p <= target) return target - p;
    return b->period_ms - p + target;
}

int32_t beat_correction_ms(int32_t off_ms)
{
    /* De-bias first. The sender drew its offset uniformly on [0,
     * BEAT_TX_JITTER_MS), so the expected measurement is half of that and it is
     * systematic. Filtering without removing it leaves a standing error the loop
     * will faithfully track instead of reject. */
    off_ms -= (int32_t)(BEAT_TX_JITTER_MS / 2u);

    /* Acquisition: a large error means we are actually lost, and a slow crawl
     * back would cost more frames than the overshoot ever could. */
    if (off_ms > (int32_t)BEAT_GUARD_MS || off_ms < -(int32_t)BEAT_GUARD_MS)
        return off_ms;
    /* Tracking: most of a small error is the sender's own transmit offset, not
     * our crystal. Applying a fraction averages that away. See finding B1. */
    return off_ms / BEAT_RESYNC_GAIN;
}

int beat_resync(beat_t *b, uint64_t arrival_ms, uint8_t peer_clock_class)
{
    if (peer_clock_class > b->clock_class) return 0;   /* worse clock: ignore */

    /* The arrival instant of any decoded frame is an implicit time reference: the
     * sender aimed it at a boundary. It is a NOISY reference — see finding B1 in
     * beat.h — so it is filtered rather than adopted. No timestamp is
     * transmitted and no bytes are spent either way. */
    int32_t off = (int32_t)phase_ms(b, arrival_ms);
    if (off > (int32_t)(b->period_ms / 2)) off -= (int32_t)b->period_ms;
    off = beat_correction_ms(off);
    b->epoch_ms = (uint64_t)((int64_t)b->epoch_ms + off);
    b->last_resync_ms = arrival_ms;
    if (peer_clock_class < b->clock_class) b->clock_class = (uint8_t)(peer_clock_class + 1);
    return 1;
}

uint32_t beat_drift_us(const beat_t *b, uint64_t now_ms)
{
    uint64_t since = now_ms - b->last_resync_ms;
    return (uint32_t)(since * BEAT_DRIFT_PPM / 1000u);
}
