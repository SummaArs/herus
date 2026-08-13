#include "hv.h"
#include <string.h>

#ifdef HV_LUT_POPCOUNT
const uint8_t hv_pc8[256] = {
#define B2(n) n, n + 1, n + 1, n + 2
#define B4(n) B2(n), B2(n + 1), B2(n + 1), B2(n + 2)
#define B6(n) B4(n), B4(n + 1), B4(n + 1), B4(n + 2)
    B6(0), B6(1), B6(1), B6(2)
#undef B6
#undef B4
#undef B2
};
#endif

void hv_gen(hv_t *out, uint64_t domain, uint32_t symbol_id)
{
    /* Mix the id into the domain so adjacent ids give uncorrelated codes. */
    uint64_t s = domain ^ (0x2545F4914F6CDD1Dull * (uint64_t)(symbol_id + 1));
    (void)hv_mix(&s);                    /* discard first draw (warm-up) */
    for (int i = 0; i < HV_WORDS; i++)
        out->w[i] = hv_mix(&s);
}

void hv_bind(hv_t *out, const hv_t *a, const hv_t *b)
{
    for (int i = 0; i < HV_WORDS; i++)
        out->w[i] = a->w[i] ^ b->w[i];
}

void hv_rot(hv_t *out, const hv_t *a, int k)
{
    k = ((k % HV_BITS) + HV_BITS) % HV_BITS;
    const int ws = k >> 6, bs = k & 63;
    hv_t tmp;                            /* allow out == a */
    for (int j = 0; j < HV_WORDS; j++) {
        const int lo  = (j - ws + HV_WORDS) % HV_WORDS;
        const int lo2 = (lo - 1 + HV_WORDS) % HV_WORDS;
        tmp.w[j] = bs ? ((a->w[lo] << bs) | (a->w[lo2] >> (64 - bs)))
                      : a->w[lo];
    }
    *out = tmp;
}

void hv_acc_zero(hv_acc_t *acc) { memset(acc->c, 0, sizeof(acc->c)); }

void hv_acc_add(hv_acc_t *acc, const hv_t *v, int weight)
{
    /* +weight for a 1 bit, -weight for a 0 bit. Signed accumulation makes
     * the majority a sign test and makes ties exactly acc == 0. */
    for (int i = 0; i < HV_WORDS; i++) {
        uint64_t w = v->w[i];
        int16_t *c = &acc->c[i * 64];
        for (int b = 0; b < 64; b++)
            c[b] += (int16_t)(((w >> b) & 1) ? weight : -weight);
    }
}

void hv_acc_majority(hv_t *out, const hv_acc_t *acc, uint64_t tiebreak_seed)
{
    uint64_t s = tiebreak_seed ^ 0xA5A5A5A5DEADBEEFull;
    for (int i = 0; i < HV_WORDS; i++) {
        uint64_t w = 0, tb = 0;
        int need_tb = 0;
        const int16_t *c = &acc->c[i * 64];
        for (int b = 0; b < 64; b++)
            if (c[b] == 0) { need_tb = 1; break; }
        if (need_tb) tb = hv_mix(&s);    /* R2: random, never toward zero */
        for (int b = 0; b < 64; b++) {
            int bit = (c[b] > 0) ? 1 : (c[b] < 0) ? 0 : (int)((tb >> b) & 1);
            w |= (uint64_t)bit << b;
        }
        out->w[i] = w;
    }
}

int hv_dist(const hv_t *a, const hv_t *b)
{
    int d = 0;
    for (int i = 0; i < HV_WORDS; i++)
        d += hv_pc64(a->w[i] ^ b->w[i]);
    return d;
}

double hv_sim(const hv_t *a, const hv_t *b)
{
    return 1.0 - 2.0 * (double)hv_dist(a, b) / (double)HV_BITS;
}

void hv_sk_positions(uint32_t *pos, uint64_t seed)
{
    /* Sample without replacement: duplicated positions would correlate the
     * sketch bits and inflate the apparent separation. */
    static uint8_t used[HV_BITS];
    memset(used, 0, sizeof(used));
    uint64_t s = seed ^ 0x0BADC0DE5EEDull;
    for (int i = 0; i < HV_SK_BITS; i++) {
        uint32_t p;
        do { p = (uint32_t)(hv_mix(&s) % HV_BITS); } while (used[p]);
        used[p] = 1;
        pos[i] = p;
    }
}

void hv_sk_make(hv_sk_t *out, const hv_t *v, const uint32_t *pos)
{
    memset(out->w, 0, sizeof(out->w));
    for (int i = 0; i < HV_SK_BITS; i++) {
        uint32_t p = pos[i];
        uint64_t bit = (v->w[p >> 6] >> (p & 63)) & 1;
        out->w[i >> 6] |= bit << (i & 63);
    }
}

int hv_sk_dist(const hv_sk_t *a, const hv_sk_t *b)
{
    int d = 0;
    for (int i = 0; i < HV_SK_WORDS; i++)
        d += hv_pc64(a->w[i] ^ b->w[i]);
    return d;
}

static void flip(uint64_t *w, int nbits, double ber, uint64_t *rng)
{
    /* Bernoulli(ber) per bit, drawn 64 bits at a time via threshold compare. */
    const uint64_t thr = (uint64_t)(ber * 18446744073709551615.0);
    for (int i = 0; i < nbits; i++)
        if (hv_mix(rng) < thr)
            w[i >> 6] ^= 1ull << (i & 63);
}

void hv_flip_bits(hv_t *v, double ber, uint64_t *rng)
{
    flip(v->w, HV_BITS, ber, rng);
}

void hv_sk_flip_bits(hv_sk_t *v, double ber, uint64_t *rng)
{
    flip(v->w, HV_SK_BITS, ber, rng);
}
