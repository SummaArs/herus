/* hcp.c — Herus Composition Protocol rev 0.2. See hcp.h for the rationale.
 *
 * Byte order is little-endian, chosen once and stated here: both target cores
 * are little-endian, and a protocol that "inherits" host order is a protocol
 * that breaks the day someone ports it to a big-endian part.
 *
 * Encoding is CANONICAL: exactly one byte string represents a given message.
 * Two encodings of the same meaning would be a covert channel inside an
 * otherwise constant-length frame, which is the same class of leak P1 exists to
 * close. Hence: slots pack contiguously from index 0, the first zero slot ends
 * the list, reserved bits must be zero, and a decoder rejects anything else
 * instead of charitably repairing it.
 */
#include "hcp.h"
#include <string.h>
#include <math.h>

int hcp_encode(uint8_t *out24, hcp_msg_t *m)
{
    return hcp_encode_n(out24, m, HCP_PT_RICH);
}

int hcp_encode_n(uint8_t *out24, hcp_msg_t *m, unsigned ptlen)
{
    if (!out24 || !m) return -1;
    if (ptlen < HCP_FIXED_LEN + 2u || ptlen > HCP_PT_RICH) return -1;
    if (m->nslot > HCP_MAX_SLOT)            return -1;
    /* Refuse rather than truncate. A message that does not fit the group's
     * profile is the application's problem to solve, and silently dropping its
     * last five slots would surface as a wrong answer at the far end. */
    if ((unsigned)m->nslot > HCP_SLOTS_IN(ptlen)) return -1;
    if (m->intent >= HCP_INTENT_N)          return -1;
    if (m->tier  > 7 || m->flags > 7)       return -1;
    if (m->ttl   > 15 || m->prio  > 3)      return -1;

    memset(out24, 0, ptlen);
    out24[0] = (uint8_t)(((HCP_VERSION & 3) << 6) | ((m->tier & 7) << 3) | (m->flags & 7));
    out24[1] = (uint8_t)(((m->ttl & 15) << 4) | ((m->prio & 3) << 2));

    out24[2] = (uint8_t)(m->intent & 0xff);
    out24[3] = (uint8_t)(m->intent >> 8);
    out24[4] = (uint8_t)(m->seq & 0xff);
    out24[5] = (uint8_t)(m->seq >> 8);

    for (int i = 0; i < m->nslot; i++) {
        if (m->slot[i].role   >= HCP_ROLE_N)   return -1;
        if (m->slot[i].filler >= HCP_FILLER_N) return -1;
        /* 0x0000 means absent, so it cannot also mean role 0 / filler 0. */
        if (m->slot[i].role == 0 && m->slot[i].filler == 0) return -1;
        uint16_t p = (uint16_t)(((uint16_t)m->slot[i].role << HCP_FILLER_BITS)
                                | m->slot[i].filler);
        out24[6 + i * 2]     = (uint8_t)(p & 0xff);
        out24[6 + i * 2 + 1] = (uint8_t)(p >> 8);
        m->pos[i] = (uint8_t)i;     /* stamp it, so hcp_to_hv agrees with the wire */
    }
    /* Tail already zero from the memset: that IS the padding, and it happens
     * here — before encryption — on purpose (footgun #1). */
    return (int)ptlen;
}

int hcp_decode(hcp_msg_t *m, const uint8_t *in24)
{
    return hcp_decode_n(m, in24, HCP_PT_RICH);
}

int hcp_decode_n(hcp_msg_t *m, const uint8_t *in24, unsigned ptlen)
{
    if (!m || !in24) return -1;
    if (ptlen < HCP_FIXED_LEN + 2u || ptlen > HCP_PT_RICH) return -1;
    memset(m, 0, sizeof(*m));

    if ((in24[0] >> 6) != HCP_VERSION)  return -1;
    if ((in24[1] & 0x03) != 0)          return -1;   /* rsv must be zero */

    m->ver   = HCP_VERSION;
    m->tier  = (uint8_t)((in24[0] >> 3) & 7);
    m->flags = (uint8_t)(in24[0] & 7);
    m->ttl   = (uint8_t)(in24[1] >> 4);
    m->prio  = (uint8_t)((in24[1] >> 2) & 3);

    m->intent = (uint16_t)(in24[2] | (in24[3] << 8));
    if (m->intent >= HCP_INTENT_N)      return -1;    /* upper bits reserved */
    m->seq    = (uint16_t)(in24[4] | (in24[5] << 8));

    int ended = 0;
    for (unsigned i = 0; i < HCP_SLOTS_IN(ptlen); i++) {
        uint16_t p = (uint16_t)(in24[6 + i * 2] | (in24[6 + i * 2 + 1] << 8));
        if (p == 0) { ended = 1; continue; }
        if (ended) return -1;            /* non-canonical: gap in the slot list */
        m->slot[m->nslot].role   = (uint8_t)(p >> HCP_FILLER_BITS);
        m->slot[m->nslot].filler = (uint16_t)(p & (HCP_FILLER_N - 1));
        m->pos[m->nslot]         = (uint8_t)i;      /* on-air position */
        m->nslot++;
    }
    return 0;
}

int hcp_filter_known_roles(const hcp_msg_t *m,
                           const uint8_t *known_roles, int nknown,
                           hcp_msg_t *out)
{
    *out = *m;
    out->nslot = 0;
    for (int i = 0; i < m->nslot; i++) {
        for (int k = 0; k < nknown; k++) {
            if (known_roles[k] == m->slot[i].role) {
                /* Kept slots pack densely for the caller's convenience, but their
                 * ORIGINAL on-air position travels with them in pos[]. The sender
                 * bound this filler under rho^(pos+2), so anything that unbinds
                 * must use pos[] — renumbering densely and then unbinding at the
                 * new index reads noise. Forward compatibility is about ignoring
                 * fields, not renumbering them. */
                out->slot[out->nslot] = m->slot[i];
                out->pos[out->nslot]  = m->pos[i];
                out->nslot++;
                break;
            }
        }
    }
    return out->nslot;
}

/* ------------------------------------------------------------------ HV form */

void hcp_to_hv(hv_t *out, hv_acc_t *acc, const lex_t *L, const hcp_msg_t *m)
{
    hv_t r, f, b, t;

    hv_acc_zero(acc);

    /* The intent gets rho^1 so that it occupies its own slot position and can
     * never be confused with a slot binding at position k. */
    lex_code(L, &t, HCP_ID_INTENT_BASE + (m->intent % HCP_INTENT_N));
    hv_rot(&b, &t, 1);
    hv_acc_add(acc, &b, 1);
    int k_votes = 1;

    for (int i = 0; i < m->nslot && i < HCP_MAX_SLOT; i++) {
        lex_code(L, &r, HCP_ID_ROLE_BASE   + (m->slot[i].role   % HCP_ROLE_N));
        lex_code(L, &f, HCP_ID_FILLER_BASE + (m->slot[i].filler % HCP_FILLER_N));
        hv_bind(&t, &r, &f);
        /* pos[i], not i: a filtered message must bind at the position the SENDER
         * used, or the two ends build different vectors from the same message. */
        hv_rot(&b, &t, m->pos[i] + 2);         /* rho^(k+2): order is meaning */
        hv_acc_add(acc, &b, 1);
        k_votes++;
    }

    /* R2: majority over an even count is undefined. Rather than break ties
     * toward zero (which sparsifies the bundle and silently destroys
     * similarity), add one parity vector so the arity is always odd. */
    if ((k_votes & 1) == 0) {
        lex_code(L, &t, HCP_ID_PAD);
        hv_acc_add(acc, &t, 1);
    }
    hv_acc_majority(out, acc, (uint64_t)m->seq);
}

int hcp_query_role(const hv_t *H, const lex_t *L, uint8_t role, int slot_index,
                   uint16_t filler_lo, uint16_t filler_n, double sigma,
                   uint16_t *filler)
{
    hv_t r, x, v;

    /* Undo the slot permutation first, then unbind the role. Both are exact and
     * they commute with nothing else, so the order is fixed. */
    hv_rot(&x, H, -(slot_index + 2));
    lex_code(L, &r, HCP_ID_ROLE_BASE + (role % HCP_ROLE_N));
    hv_bind(&x, &x, &r);

    int best = -1, bd = HV_BITS + 1;
    for (uint16_t k = 0; k < filler_n; k++) {
        lex_code(L, &v, HCP_ID_FILLER_BASE + ((filler_lo + k) % HCP_FILLER_N));
        int d = hv_dist(&x, &v);
        if (d < bd) { bd = d; best = (int)(filler_lo + k); }
    }
    if (best < 0) return 0;

    const double mu = HV_BITS / 2.0, sg = sqrt((double)HV_BITS) / 2.0;
    if ((mu - bd) < sigma * sg) return 0;          /* indistinguishable from noise */
    if (filler) *filler = (uint16_t)best;
    return 1;
}
