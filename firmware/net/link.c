/* link.c — see link.h. */
#include "link.h"
#include <string.h>

const hz_link_profile_t *link_profile(const herus_link *l)
{
    return hz_link(l ? l->profile : HZ_LINK_RICH);
}

int link_send(herus_link *l, hcp_msg_t *m, uint8_t ttl, uint8_t *frame34)
{
    if (!l || !l->sess || !m || !frame34) return LINK_E_ARG;
    const hz_link_profile_t *P = link_profile(l);

    /* Voice and SOS keep their own spreading factors: they are not meaning tiers
     * and P1 never covered them. Everything else runs the group's profile. */
    uint8_t sf  = (m->tier == HCP_TIER_VOICE) ? HERUS_SF_VOICE
                : (m->tier == HCP_TIER_SOS)   ? HERUS_SF_SOS
                                              : P->sf;
    uint8_t len = (m->tier == HCP_TIER_VOICE || m->tier == HCP_TIER_SOS)
                ? (uint8_t)HERUS_FRAME_LEN : P->frame_len;

    if (!hz_tx_permitted(l->region, sf, len, 1, 0, HERUS_CR, l->tx_dbm))
        return LINK_E_ILLEGAL;

    uint8_t pt[LINK_PT_LEN];
    if (hcp_encode_n(pt, m, P->pt_len) != (int)P->pt_len) return LINK_E_ENCODE;

    int r = session_seal(l->sess, pt, P->pt_len, P->tag_len, ttl, frame34);
    secure_zero(pt, sizeof pt);
    return (r == SESS_OK) ? LINK_OK : LINK_E_SESSION;
}

/* Both receive paths share everything after the unsealing, so they share it in
 * code too: a second copy of the P4 filter is a second place for it to rot. */
/* Extend hcp's 16-bit seq to the monotonic 32-bit counter herus_replay wants.
 * Signed accumulation, so reordering moves it backwards and the replay window
 * refuses what has already been seen. */
static int dedup_accept(link_dedup *d, uint16_t seq)
{
    if (!d->started) {
        d->started = 1;
        d->last = seq;
        d->ext  = 1u << 20;            /* room to move backwards on reordering */
        herus_replay_init(&d->r, d->ext);
    } else {
        int16_t delta = (int16_t)(seq - d->last);
        d->ext = (uint32_t)((int32_t)d->ext + (int32_t)delta);
        d->last = seq;
    }
    return herus_replay_accept(&d->r, d->ext);
}

static int link_finish(herus_link *l, uint8_t *pt, hcp_msg_t *out)
{
    const hz_link_profile_t *P = link_profile(l);
    hcp_msg_t raw;
    if (hcp_decode_n(&raw, pt, P->pt_len) != 0) { secure_zero(pt, P->pt_len); return LINK_E_DECODE; }
    secure_zero(pt, P->pt_len);
    if (l->known_roles && l->nknown > 0)
        hcp_filter_known_roles(&raw, l->known_roles, l->nknown, out);
    else
        *out = raw;

    /* A retry is a different frame carrying the same message. Suppressing it here
     * — after the AEAD, on authenticated plaintext — is the only safe place: an
     * attacker cannot forge a seq it cannot encrypt, so this cannot be used to
     * make us drop a real message. */
    if (l->seen && !dedup_accept(l->seen, raw.seq)) return LINK_E_DUP;
    return LINK_OK;
}

int link_recover(herus_link *l, const uint8_t *frame34, uint64_t now_ms,
                 hcp_msg_t *out, uint32_t *counter_out, int *sess_err)
{
    if (!l || !l->sess || !frame34 || !out) return LINK_E_ARG;
    const hz_link_profile_t *P = link_profile(l);
    uint8_t pt[LINK_PT_LEN];
    int r = session_recover(l->sess, frame34, P->pt_len, P->tag_len, now_ms,
                            pt, counter_out);
    if (sess_err) *sess_err = r;
    if (r != SESS_OK) return LINK_E_SESSION;
    return link_finish(l, pt, out);
}

int link_recv(herus_link *l, const uint8_t *frame34, uint64_t now_ms,
              hcp_msg_t *out, uint32_t *counter_out, int *sess_err)
{
    if (!l || !l->sess || !frame34 || !out) return LINK_E_ARG;

    const hz_link_profile_t *P = link_profile(l);
    uint8_t pt[LINK_PT_LEN];
    int r = session_open(l->sess, frame34, P->pt_len, P->tag_len, now_ms,
                         pt, counter_out);
    if (sess_err) *sess_err = r;
    if (r != SESS_OK) return LINK_E_SESSION;

    /* P4 (unknown roles dropped, frame kept) and application-level duplicate
     * suppression both live in link_finish, so that both receive paths get them.
     * They used to be written out here instead, and the recovery path silently
     * did not have them — one of the two hazards of copy-paste, and the one that
     * only shows up as a wrong answer. */
    return link_finish(l, pt, out);
}
