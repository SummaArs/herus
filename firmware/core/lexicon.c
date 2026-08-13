#include "lexicon.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============================ codebook ============================ */

int lex_init(lex_t *L, uint64_t domain, uint16_t n)
{
    L->domain = domain;
    L->n = n;
    L->sk = (hv_sk_t *)malloc((size_t)n * sizeof(hv_sk_t));
    if (!L->sk) return -1;
    hv_sk_positions(L->pos, domain);
    hv_t v;
    for (uint16_t i = 0; i < n; i++) {
        hv_gen(&v, domain, i);
        hv_sk_make(&L->sk[i], &v, L->pos);
    }
    return 0;
}

void lex_free(lex_t *L) { free(L->sk); L->sk = NULL; L->n = 0; }

void lex_code(const lex_t *L, hv_t *out, uint32_t id)
{
    hv_gen(out, L->domain, id);
}

int lex_search_full(const lex_t *L, const hv_t *q, int *out_dist)
{
    int best = -1, bd = HV_BITS + 1;
    hv_t v;
    for (uint16_t i = 0; i < L->n; i++) {
        hv_gen(&v, L->domain, i);
        int d = hv_dist(q, &v);
        if (d < bd) { bd = d; best = i; }
    }
    if (out_dist) *out_dist = bd;
    return best;
}

int lex_search(const lex_t *L, const hv_t *q, int shortlist, int *out_dist)
{
    if (shortlist < 1) shortlist = 1;
    if (shortlist > L->n) shortlist = L->n;

    hv_sk_t qs;
    hv_sk_make(&qs, q, L->pos);

    /* Coarse pass: insertion into a small top-k. n * 32 bytes touched. */
    int  cid[64];
    int  cd[64];
    int  m = 0;
    if (shortlist > 64) shortlist = 64;
    for (uint16_t i = 0; i < L->n; i++) {
        int d = hv_sk_dist(&qs, &L->sk[i]);
        if (m < shortlist) {
            int j = m++;
            while (j > 0 && cd[j - 1] > d) { cd[j] = cd[j - 1]; cid[j] = cid[j - 1]; j--; }
            cd[j] = d; cid[j] = i;
        } else if (d < cd[m - 1]) {
            int j = m - 1;
            while (j > 0 && cd[j - 1] > d) { cd[j] = cd[j - 1]; cid[j] = cid[j - 1]; j--; }
            cd[j] = d; cid[j] = i;
        }
    }

    /* Fine pass: full codes for the shortlist only. */
    int best = -1, bd = HV_BITS + 1;
    hv_t v;
    for (int k = 0; k < m; k++) {
        hv_gen(&v, L->domain, (uint32_t)cid[k]);
        int d = hv_dist(q, &v);
        if (d < bd) { bd = d; best = cid[k]; }
    }
    if (out_dist) *out_dist = bd;
    return best;
}

int lex_search_sketch(const lex_t *L, const hv_sk_t *q, int *out_dist)
{
    int best = -1, bd = HV_SK_BITS + 1;
    for (uint16_t i = 0; i < L->n; i++) {
        int d = hv_sk_dist(q, &L->sk[i]);
        if (d < bd) { bd = d; best = i; }
    }
    if (out_dist) *out_dist = bd;
    return best;
}

/* ========================= learned prototypes ========================= */

int proto_init(proto_t *P, uint16_t cap, int min_sep)
{
    P->n = 0; P->cap = cap; P->min_sep = min_sep;
    P->acc   = (int16_t  *)calloc((size_t)cap * HV_BITS, sizeof(int16_t));
    P->proto = (hv_t     *)calloc(cap, sizeof(hv_t));
    P->votes = (uint16_t *)calloc(cap, sizeof(uint16_t));
    return (P->acc && P->proto && P->votes) ? 0 : -1;
}

void proto_free(proto_t *P)
{
    free(P->acc); free(P->proto); free(P->votes);
    P->acc = NULL; P->proto = NULL; P->votes = NULL; P->n = 0;
}

static void proto_rebuild(proto_t *P, uint16_t c)
{
    int16_t *a = &P->acc[(size_t)c * HV_BITS];
    uint64_t s = 0x51EDC0DEull ^ c;
    for (int i = 0; i < HV_WORDS; i++) {
        uint64_t w = 0, tb = 0;
        int need = 0;
        for (int b = 0; b < 64; b++) if (a[i * 64 + b] == 0) { need = 1; break; }
        if (need) tb = hv_mix(&s);
        for (int b = 0; b < 64; b++) {
            int16_t v = a[i * 64 + b];
            int bit = (v > 0) ? 1 : (v < 0) ? 0 : (int)((tb >> b) & 1);
            w |= (uint64_t)bit << b;
        }
        P->proto[c].w[i] = w;
    }
}

int proto_min_separation(const proto_t *P)
{
    /* Only classes that have actually been taught. proto_learn grows P->n to
     * cover the highest class id it has seen, so learning ids out of order —
     * which is exactly what a residue queue does, since the student asks in no
     * particular order — leaves untaught classes in between as all-zero
     * prototypes. Two of those are zero bits apart, the drift guard sees a
     * separation of 0, and it rolls back every single update.
     *
     * The bench found this by watching a codex report 64 facts while answering
     * as if it had 16: 48 nights of learning silently undone by the invariant
     * monitor measuring prototypes that do not exist. The guard was right to
     * fire on what it was shown; it was being shown the wrong thing. */
    int mn = HV_BITS, seen = 0;
    for (uint16_t i = 0; i < P->n; i++) {
        if (!P->votes[i]) continue;
        seen++;
        for (uint16_t j = (uint16_t)(i + 1); j < P->n; j++) {
            if (!P->votes[j]) continue;
            int d = hv_dist(&P->proto[i], &P->proto[j]);
            if (d < mn) mn = d;
        }
    }
    return (seen < 2) ? HV_BITS : mn;
}

int proto_learn(proto_t *P, uint16_t c, const hv_t *sample)
{
    if (c >= P->cap) return -1;
    if (c >= P->n) P->n = (uint16_t)(c + 1);

    int16_t *a = &P->acc[(size_t)c * HV_BITS];
    hv_t saved = P->proto[c];

    for (int i = 0; i < HV_WORDS; i++) {
        uint64_t w = sample->w[i];
        for (int b = 0; b < 64; b++)
            a[i * 64 + b] = (int16_t)(a[i * 64 + b] + (((w >> b) & 1) ? 1 : -1));
    }
    P->votes[c]++;
    proto_rebuild(P, c);

    /* Drift guard. Quasi-orthogonality is the load-bearing assumption of the
     * whole architecture; here it is checked, not assumed. */
    if (P->n >= 2 && proto_min_separation(P) < P->min_sep) {
        for (int i = 0; i < HV_WORDS; i++) {
            uint64_t w = sample->w[i];
            for (int b = 0; b < 64; b++)
                a[i * 64 + b] = (int16_t)(a[i * 64 + b] - (((w >> b) & 1) ? 1 : -1));
        }
        P->votes[c]--;
        P->proto[c] = saved;
        return -1;
    }
    return 0;
}

int proto_classify(const proto_t *P, const hv_t *q, double thresh_sigma,
                   int *out_dist)
{
    int best = -1, bd = HV_BITS + 1;
    for (uint16_t i = 0; i < P->n; i++) {
        if (P->votes[i] == 0) continue;
        int d = hv_dist(q, &P->proto[i]);
        if (d < bd) { bd = d; best = i; }
    }
    if (out_dist) *out_dist = bd;
    if (best < 0) return -1;

    const double mu = HV_BITS / 2.0;
    const double sigma = sqrt((double)HV_BITS) / 2.0;
    if ((mu - (double)bd) < thresh_sigma * sigma) return -1;   /* unknown */
    return best;
}

/* =========================== resonator ============================ */

int resonator(const lex_t *L, const hv_t *S, int nf,
              const uint32_t *base, const uint32_t *size,
              uint32_t *out, int max_iter)
{
    if (nf < 2 || nf > 6) return -1;

    hv_t est[6];
    hv_acc_t *acc = (hv_acc_t *)malloc(sizeof(hv_acc_t));
    if (!acc) return -1;

    /* Start from the equal superposition of every candidate: the estimate
     * literally holds all hypotheses at once, and iteration collapses it. */
    for (int f = 0; f < nf; f++) {
        hv_acc_zero(acc);
        hv_t v;
        for (uint32_t k = 0; k < size[f]; k++) {
            lex_code(L, &v, base[f] + k);
            hv_acc_add(acc, &v, 1);
        }
        hv_acc_majority(&est[f], acc, 0x9E37u + (uint64_t)f);
    }

    uint32_t prev[6];
    for (int f = 0; f < nf; f++) prev[f] = 0xFFFFFFFFu;
    int stable = 0;

    for (int it = 1; it <= max_iter; it++) {
        for (int f = 0; f < nf; f++) {
            /* Unbind every other factor's current estimate out of S. */
            hv_t x = *S;
            for (int g = 0; g < nf; g++)
                if (g != f) hv_bind(&x, &x, &est[g]);

            /* Soft cleanup: bundle the codebook weighted by similarity.
             * Signed weights matter — dropping the negative half turns the
             * resonator into a plain argmax and it stops converging. */
            hv_acc_zero(acc);
            hv_t v;
            for (uint32_t k = 0; k < size[f]; k++) {
                lex_code(L, &v, base[f] + k);
                double s = hv_sim(&x, &v);
                int wgt = (int)lround(s * 64.0);
                if (wgt) hv_acc_add(acc, &v, wgt);
            }
            hv_acc_majority(&est[f], acc, (uint64_t)(it * 31 + f));
        }

        /* Hard read-out and reconstruction check. */
        uint32_t cur[6];
        for (int f = 0; f < nf; f++) {
            int bd = HV_BITS + 1; uint32_t bi = base[f];
            hv_t v;
            for (uint32_t k = 0; k < size[f]; k++) {
                lex_code(L, &v, base[f] + k);
                int d = hv_dist(&est[f], &v);
                if (d < bd) { bd = d; bi = base[f] + k; }
            }
            cur[f] = bi;
        }

        int same = 1;
        for (int f = 0; f < nf; f++) if (cur[f] != prev[f]) same = 0;
        memcpy(prev, cur, sizeof(prev));
        stable = same ? stable + 1 : 0;

        if (stable >= 1) {
            hv_t r, v;
            lex_code(L, &r, cur[0]);
            for (int f = 1; f < nf; f++) { lex_code(L, &v, cur[f]); hv_bind(&r, &r, &v); }
            if (hv_dist(&r, S) == 0) {
                memcpy(out, cur, sizeof(uint32_t) * (size_t)nf);
                free(acc);
                return it;
            }
            /* Stable but wrong: a limit cycle. Perturb and continue. */
            for (int f = 0; f < nf; f++) {
                uint64_t rng = (uint64_t)(it * 7919 + f);
                hv_flip_bits(&est[f], 0.05 + 0.02 * (it % 8), &rng);
            }
            stable = 0;
        }
    }
    free(acc);
    return -1;
}

/* ---------------- refusal with a reason: see the note in lexicon.h ---------- */
const char *lex_verdict_name(lex_verdict_t v)
{
    switch (v) {
        case LEX_OK:            return "ok";
        case LEX_V1_UNKNOWN:    return "desconhecido";
        case LEX_V2_AMBIGUOUS:  return "ambiguo";
        case LEX_V3_COLLISION:  return "colisao";
        case LEX_V4_LOCKED:     return "bloqueado";
        case LEX_V0_SUGGEST:    return "voce quis dizer";
        default:                return "?";
    }
}

lex_verdict_t proto_resolve(const proto_t *P, const hv_t *q,
                            double min_sigma, double suggest_sigma,
                            double margin_sigma,
                            const uint8_t *tier_of, int unlocked,
                            lex_resolution *out)
{
    lex_resolution r;
    r.verdict = LEX_V1_UNKNOWN; r.id = -1; r.runner_up = -1;
    r.sigma = 0.0; r.margin = 0.0;

    if (!P || !q || P->n == 0) { if (out) *out = r; return r.verdict; }

    /* Three best distances, in one pass. Three and not two because V3 needs to
     * know whether the runner-up is alone or is one of a crowd. */
    int d1 = HV_BITS + 1, d2 = HV_BITS + 1, d3 = HV_BITS + 1;
    int i1 = -1, i2 = -1;
    for (uint16_t i = 0; i < P->n; i++) {
        if (!P->votes[i]) continue;              /* never learned */
        int d = hv_dist(q, &P->proto[i]);
        if (d < d1)      { d3 = d2; d2 = d1; i2 = i1; d1 = d; i1 = (int)i; }
        else if (d < d2) { d3 = d2; d2 = d; i2 = (int)i; }
        else if (d < d3) { d3 = d; }
    }
    if (i1 < 0) { if (out) *out = r; return r.verdict; }

    const double sd = sqrt((double)HV_BITS) / 2.0;      /* sigma, in bits */
    const double mid = (double)HV_BITS / 2.0;
    double s1 = (mid - d1) / sd;
    double s2 = (i2 >= 0) ? (mid - d2) / sd : 0.0;
    double s3 = (d3 <= HV_BITS) ? (mid - d3) / sd : 0.0;

    r.id = i1; r.runner_up = i2; r.sigma = s1; r.margin = s1 - s2;

    /* Order matters and is not arbitrary: a match that is not evidence cannot be
     * ambiguous, and a crowd is a more specific complaint than a pair. */
    if (s1 < min_sigma) {
        /* Not enough to assert. Enough to ask? */
        r.verdict = (suggest_sigma > 0.0 && s1 >= suggest_sigma)
                  ? LEX_V0_SUGGEST : LEX_V1_UNKNOWN;
    }
    else if (i2 >= 0 && (s1 - s3) < margin_sigma / 2.0)
                                              r.verdict = LEX_V3_COLLISION;
    else if (i2 >= 0 && (s1 - s2) < margin_sigma)
                                              r.verdict = LEX_V2_AMBIGUOUS;
    else if (tier_of && tier_of[i1] > (uint8_t)unlocked)
                                              r.verdict = LEX_V4_LOCKED;
    else                                      r.verdict = LEX_OK;

    if (out) *out = r;
    return r.verdict;
}
