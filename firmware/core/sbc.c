#include "sbc.h"
#include <string.h>

void sbc_gen(sbc_t *out, uint64_t domain, uint32_t symbol_id)
{
    uint64_t s = domain ^ (0x9E3779B97F4A7C15ull * (uint64_t)(symbol_id + 1));
    (void)hv_mix(&s);
    for (int i = 0; i < SBC_S; i++)
        out->b[i] = (uint8_t)(hv_mix(&s) % SBC_B);
}

void sbc_bind(sbc_t *out, const sbc_t *a, const sbc_t *b)
{
    for (int i = 0; i < SBC_S; i++)
        out->b[i] = (uint8_t)((a->b[i] + b->b[i]) % SBC_B);
}

void sbc_unbind(sbc_t *out, const sbc_t *a, const sbc_t *b)
{
    for (int i = 0; i < SBC_S; i++)
        out->b[i] = (uint8_t)((a->b[i] + SBC_B - b->b[i]) % SBC_B);
}

void sbc_rot(sbc_t *out, const sbc_t *a, int k)
{
    sbc_t tmp;                                  /* allow out == a */
    k = ((k % SBC_S) + SBC_S) % SBC_S;
    for (int i = 0; i < SBC_S; i++)
        tmp.b[i] = a->b[(i - k + SBC_S) % SBC_S];
    *out = tmp;
}

void sbc_acc_zero(sbc_acc_t *acc) { memset(acc->c, 0, sizeof(acc->c)); }

void sbc_acc_add(sbc_acc_t *acc, const sbc_t *v, int weight)
{
    for (int i = 0; i < SBC_S; i++)
        acc->c[i][v->b[i]] = (int16_t)(acc->c[i][v->b[i]] + weight);
}

void sbc_acc_decay(sbc_acc_t *acc, int alpha)
{
    if (alpha >= 256) return;
    int16_t *p = &acc->c[0][0];
    for (int i = 0; i < SBC_S * SBC_B; i++)
        p[i] = (int16_t)((p[i] * alpha) >> 8);
}

void sbc_acc_read(sbc_t *out, const sbc_acc_t *acc, uint64_t tiebreak)
{
    uint64_t s = tiebreak ^ 0xC0FFEE123456789ull;
    for (int i = 0; i < SBC_S; i++) {
        int best = 0, nties = 1;
        for (int j = 1; j < SBC_B; j++) {
            if (acc->c[i][j] > acc->c[i][best]) { best = j; nties = 1; }
            else if (acc->c[i][j] == acc->c[i][best]) {
                /* Reservoir-sample among ties so an empty or tied block is a
                 * uniform draw, never a fixed slot. A fixed slot would make all
                 * empty blocks agree across vectors and fake up similarity. */
                if (hv_mix(&s) % (uint64_t)(++nties) == 0) best = j;
            }
        }
        out->b[i] = (uint8_t)best;
    }
}

int sbc_matches(const sbc_t *a, const sbc_t *b)
{
    int m = 0;
    for (int i = 0; i < SBC_S; i++)
        m += (a->b[i] == b->b[i]);
    return m;
}

void sbc_sketch_make(uint8_t *out, const sbc_t *v, int n)
{
    for (int i = 0; i < n; i++) out[i] = v->b[i];
}

int sbc_sketch_matches(const uint8_t *sk, const sbc_t *v, int n)
{
    int m = 0;
    for (int i = 0; i < n; i++) m += (sk[i] == v->b[i]);
    return m;
}

void sbc_sketch_flip(uint8_t *sk, int n, double ber, uint64_t *rng)
{
    const uint64_t thr = (uint64_t)(ber * 18446744073709551615.0);
    for (int i = 0; i < n; i++)
        for (int b = 0; b < 6; b++)            /* 6 payload bits per block */
            if (hv_mix(rng) < thr)
                sk[i] = (uint8_t)((sk[i] ^ (1u << b)) % SBC_B);
}
