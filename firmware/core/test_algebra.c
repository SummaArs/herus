/* test_algebra.c — the fire test.
 *
 * Every quantitative claim in docs/02-algebra.md and docs/03-cognition.md is
 * either produced or falsified here. Doctrine: an asserted number is a rumour;
 * a measured number next to its closed form is a result. Where theory and
 * measurement disagree, the measurement wins and the doc gets rewritten.
 *
 *   T1  quasi-orthogonality, both algebras, vs closed form
 *   T2  binding is exactly invertible
 *   T3  bundle capacity law  p(K) = 1/2 + C(K-1,(K-1)/2)/2^K
 *   T4  retrieval capacity: largest K recoverable from an L-entry lexicon
 *   T5  the role-symmetry defect in the draft's relation encoding, and its fix
 *   T6  reject threshold: derived-from-sigma vs the draft's flat 60%
 *   T7  typo robustness: what n-grams actually tolerate, and what they do not
 *   T8  Tier 0.5 — graceful decode under channel errors, dense vs SBC
 *   T9  search cost with MCU-realistic popcount
 *   T10 two-stage search returns the exhaustive answer
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "hv.h"
#include "sbc.h"

#define DOM_SYM  0x4845525553ull      /* "HERUS" */
#define DOM_CHAR 0x43484152ull        /* "CHAR"  */

static uint64_t rng = 0x1234567890ABCDEFull;

static double agr(const hv_t *a, const hv_t *b)
{ return 1.0 - (double)hv_dist(a, b) / (double)HV_BITS; }

/* closed form: probability a majority bit equals a given member's bit */
static double bundle_theory(int K)
{
    int n = K - 1;                    /* K odd */
    double logC = lgamma(n + 1.0) - 2.0 * lgamma(n / 2.0 + 1.0);
    double ratio = exp(logC - n * log(2.0));
    return 0.5 + ratio / 2.0;
}

static void hdr(const char *s) { printf("\n=== %s ===\n", s); }

/* ---------------------------------------------------------------- T1 ---- */
static void t1_orthogonality(void)
{
    hdr("T1  quasi-orthogonality");
    const int N = 4000;
    double s = 0, s2 = 0;
    hv_t a, b;
    for (int i = 0; i < N; i++) {
        hv_gen(&a, DOM_SYM, (uint32_t)(2 * i));
        hv_gen(&b, DOM_SYM, (uint32_t)(2 * i + 1));
        double d = hv_dist(&a, &b);
        s += d; s2 += d * d;
    }
    double mean = s / N, sd = sqrt(s2 / N - mean * mean);
    printf("dense  D=%d   mean d = %.1f (theory %.1f)   sigma = %.2f (theory %.2f)\n",
           HV_BITS, mean, HV_BITS / 2.0, sd, sqrt((double)HV_BITS) / 2.0);
    printf("       normalised sigma = %.4f  -> 1 sigma is %.2f%% of similarity\n",
           sd / HV_BITS, 100.0 * sd / HV_BITS);

    /* The draft claimed P(similarity > 55%) < 1e-6. It is far smaller. */
    double z = (0.55 - 0.5) * HV_BITS / (sqrt((double)HV_BITS) / 2.0);
    printf("       P(sim > 55%%) = P(Z > %.1f) ~ %.2e   [draft said '< 1e-6']\n",
           z, 0.5 * erfc(z / sqrt(2.0)));

    double ms = 0, ms2 = 0;
    sbc_t x, y;
    for (int i = 0; i < N; i++) {
        sbc_gen(&x, DOM_SYM, (uint32_t)(2 * i));
        sbc_gen(&y, DOM_SYM, (uint32_t)(2 * i + 1));
        double m = sbc_matches(&x, &y);
        ms += m; ms2 += m * m;
    }
    double mm = ms / N, msd = sqrt(ms2 / N - mm * mm);
    double p = 1.0 / SBC_B;
    printf("sbc    S=%d B=%d mean match = %.2f (theory %.2f)  sigma = %.2f (theory %.2f)\n",
           SBC_S, SBC_B, mm, SBC_S * p, msd, sqrt(SBC_S * p * (1 - p)));
    printf("       random similarity = %.2f%% vs dense 50%% -> %.0fx more headroom\n",
           100.0 * mm / SBC_S, 0.5 / (mm / SBC_S));
}

/* ---------------------------------------------------------------- T2 ---- */
static void t2_invertible(void)
{
    hdr("T2  binding is exactly invertible");
    int bad = 0;
    for (int i = 0; i < 2000; i++) {
        hv_t a, b, c, r;
        hv_gen(&a, DOM_SYM, (uint32_t)i);
        hv_gen(&b, DOM_SYM, (uint32_t)(i + 100000));
        hv_bind(&c, &a, &b);
        hv_bind(&r, &c, &b);
        if (hv_dist(&r, &a) != 0) bad++;
        sbc_t x, y, z, w;
        sbc_gen(&x, DOM_SYM, (uint32_t)i);
        sbc_gen(&y, DOM_SYM, (uint32_t)(i + 100000));
        sbc_bind(&z, &x, &y);
        sbc_unbind(&w, &z, &y);
        if (sbc_matches(&w, &x) != SBC_S) bad++;
        /* and the bound vector must be orthogonal to its operands */
        if (i == 0)
            printf("sim(a^b, a) = %.4f   sbc sim(bind, a) = %.4f   (both ~random)\n",
                   agr(&c, &a), (double)sbc_matches(&z, &x) / SBC_S);
    }
    printf("round-trip failures: %d / 4000  -> %s\n", bad, bad ? "FAIL" : "PASS");
}

/* ---------------------------------------------------------------- T3 ---- */
static void t3_bundle_law(void)
{
    hdr("T3  bundle capacity law");
    printf(" K   dense measured   closed form   0.5+0.399/sqrt(K)   sbc measured\n");
    int Ks[] = { 1, 3, 5, 7, 9, 15, 31, 63, 127 };
    for (unsigned t = 0; t < sizeof(Ks) / sizeof(Ks[0]); t++) {
        int K = Ks[t];
        const int TR = 200;
        double sd = 0, ss = 0;
        for (int tr = 0; tr < TR; tr++) {
            hv_acc_t acc; hv_acc_zero(&acc);
            sbc_acc_t sacc; sbc_acc_zero(&sacc);
            hv_t v, bundle; sbc_t sv, sbundle;
            uint32_t base = (uint32_t)(tr * 1000 + t * 200000);
            for (int i = 0; i < K; i++) {
                hv_gen(&v, DOM_SYM, base + i);   hv_acc_add(&acc, &v, 1);
                sbc_gen(&sv, DOM_SYM, base + i); sbc_acc_add(&sacc, &sv, 1);
            }
            hv_acc_majority(&bundle, &acc, base);
            sbc_acc_read(&sbundle, &sacc, base);
            hv_gen(&v, DOM_SYM, base);   sd += agr(&bundle, &v);
            sbc_gen(&sv, DOM_SYM, base); ss += (double)sbc_matches(&sbundle, &sv) / SBC_S;
        }
        printf("%4d   %.4f           %.4f        %.4f              %.4f\n",
               K, sd / TR, bundle_theory(K), 0.5 + 0.399 / sqrt((double)K), ss / TR);
    }
}

/* ---------------------------------------------------------------- T4 ---- */
/* Largest K whose members are all still retrievable from an L-entry lexicon. */
static void t4_retrieval_capacity(void)
{
    hdr("T4  retrieval capacity (lexicon L=512, 200 trials, success = ALL K members win)");
    const int L = 512, TR = 200;
    printf("  K    dense success   sbc success\n");
    int Ks[] = { 3, 5, 9, 15, 21, 31, 63, 95, 127, 191, 255 };
    for (unsigned t = 0; t < sizeof(Ks) / sizeof(Ks[0]); t++) {
        int K = Ks[t];
        int dok = 0, sok = 0;
        for (int tr = 0; tr < TR; tr++) {
            uint32_t base = (uint32_t)(tr * 4096);
            hv_acc_t acc; hv_acc_zero(&acc);
            sbc_acc_t sacc; sbc_acc_zero(&sacc);
            hv_t v, bundle; sbc_t sv, sbundle;
            for (int i = 0; i < K; i++) {
                hv_gen(&v, DOM_SYM, base + i);   hv_acc_add(&acc, &v, 1);
                sbc_gen(&sv, DOM_SYM, base + i); sbc_acc_add(&sacc, &sv, 1);
            }
            hv_acc_majority(&bundle, &acc, base);
            sbc_acc_read(&sbundle, &sacc, base);
            /* worst member must still beat the best non-member */
            double dmin = 1e9, dmax = -1e9, smin = 1e9, smax = -1e9;
            for (int i = 0; i < K; i++) {
                hv_gen(&v, DOM_SYM, base + i);
                double a = agr(&bundle, &v); if (a < dmin) dmin = a;
                sbc_gen(&sv, DOM_SYM, base + i);
                double b = (double)sbc_matches(&sbundle, &sv); if (b < smin) smin = b;
            }
            for (int i = K; i < L; i++) {
                hv_gen(&v, DOM_SYM, base + i);
                double a = agr(&bundle, &v); if (a > dmax) dmax = a;
                sbc_gen(&sv, DOM_SYM, base + i);
                double b = (double)sbc_matches(&sbundle, &sv); if (b > smax) smax = b;
            }
            dok += (dmin > dmax);
            sok += (smin > smax);
        }
        printf("%4d    %6.1f%%        %6.1f%%\n",
               K, 100.0 * dok / TR, 100.0 * sok / TR);
    }
    printf("closed-form dense limit K < 0.637*D/z^2, z=4.6 (union bound, L=512, eps=1e-3):"
           " K_max ~ %.0f\n", 0.637 * HV_BITS / (4.6 * 4.6));
}

/* ---------------------------------------------------------------- T5 ---- */
/* The draft encoded a relation as R = phi(rel) ^ phi(arg) ^ phi(val).
 * XOR is commutative, so that vector cannot tell capital(Brazil)=Brasilia
 * apart from capital(Brasilia)=Brazil. Permutation restores role order. */
static void t5_role_symmetry(void)
{
    hdr("T5  role symmetry: the draft's relation encoding answers backwards");
    enum { REL = 1, ARG = 2, VAL = 3, DECOY = 4 };
    hv_t rel, arg, val, R, q, tmp, cand;
    hv_gen(&rel, DOM_SYM, REL);
    hv_gen(&arg, DOM_SYM, ARG);
    hv_gen(&val, DOM_SYM, VAL);

    /* --- draft: fully symmetric --- */
    hv_bind(&R, &rel, &arg); hv_bind(&R, &R, &val);
    hv_bind(&q, &R, &rel); hv_bind(&q, &q, &arg);          /* ask forwards */
    double fwd = agr(&q, &val);
    hv_bind(&q, &R, &rel); hv_bind(&q, &q, &val);          /* ask backwards */
    double bwd = agr(&q, &arg);
    printf("symmetric  forwards -> value  %.4f    backwards -> arg  %.4f\n", fwd, bwd);
    printf("           both resolve perfectly, so the roles are indistinguishable: BUG\n");

    /* --- fix: break symmetry with permutation --- */
    hv_rot(&tmp, &arg, 1);
    hv_bind(&R, &rel, &tmp);
    hv_rot(&tmp, &val, 2);
    hv_bind(&R, &R, &tmp);

    hv_rot(&tmp, &arg, 1);                                  /* forwards */
    hv_bind(&q, &R, &rel); hv_bind(&q, &q, &tmp);
    hv_rot(&q, &q, -2);
    double fwd2 = agr(&q, &val);

    hv_rot(&tmp, &val, 1);                                  /* backwards */
    hv_bind(&cand, &R, &rel); hv_bind(&cand, &cand, &tmp);
    hv_rot(&cand, &cand, -2);
    double bwd2 = agr(&cand, &arg);

    printf("ordered    forwards -> value  %.4f    backwards -> arg  %.4f\n", fwd2, bwd2);
    printf("           forwards exact, backwards at chance: roles now carry direction\n");
}

/* ---------------------------------------------------------------- T6 ---- */
static void t6_threshold(void)
{
    hdr("T6  reject threshold");
    const int L = 512;
    double sigma_n = (sqrt((double)HV_BITS) / 2.0) / HV_BITS;
    /* union bound over L non-members at total error eps */
    double eps = 1e-3;
    /* z such that erfc(z/sqrt2)/2 = eps/L */
    double lo = 0, hi = 10, z = 0;
    for (int i = 0; i < 200; i++) {
        z = 0.5 * (lo + hi);
        if (0.5 * erfc(z / sqrt(2.0)) > eps / L) lo = z; else hi = z;
    }
    printf("dense: sigma_norm = %.5f, union-bound z = %.2f\n", sigma_n, z);
    printf("       derived threshold = %.4f  (%.2f%%)\n", 0.5 + z * sigma_n,
           100.0 * (0.5 + z * sigma_n));
    printf("       draft threshold   = 0.6000 (60%%) = %.1f sigma\n", 0.1 / sigma_n);

    /* Consequence: how often does a genuine K=9 bundle member clear each bar? */
    int passd = 0, passdraft = 0;
    const int TR = 2000;
    for (int tr = 0; tr < TR; tr++) {
        hv_acc_t acc; hv_acc_zero(&acc);
        hv_t v, bundle;
        uint32_t base = (uint32_t)(tr * 64 + 7000000);
        for (int i = 0; i < 9; i++) { hv_gen(&v, DOM_SYM, base + i); hv_acc_add(&acc, &v, 1); }
        hv_acc_majority(&bundle, &acc, base);
        hv_gen(&v, DOM_SYM, base);
        double a = agr(&bundle, &v);
        passd += (a > 0.5 + z * sigma_n);
        passdraft += (a > 0.60);
    }
    printf("genuine K=9 member accepted: derived %.1f%%   draft-60%% %.1f%%\n",
           100.0 * passd / TR, 100.0 * passdraft / TR);
    /* and false accepts at the derived bar */
    int fa = 0;
    for (int tr = 0; tr < TR; tr++) {
        hv_t a, b; hv_gen(&a, DOM_SYM, (uint32_t)(tr * 2 + 9000000));
        hv_gen(&b, DOM_SYM, (uint32_t)(tr * 2 + 9000001));
        fa += (agr(&a, &b) > 0.5 + z * sigma_n);
    }
    printf("false accepts at derived bar: %d / %d\n", fa, TR);
}

/* ---------------------------------------------------------------- T7 ---- */
static void enc_word(hv_t *out, const char *w, int bag_weight)
{
    hv_acc_t acc; hv_acc_zero(&acc);
    int n = (int)strlen(w);
    hv_t a, b, g;
    for (int i = 0; i + 1 < n; i++) {                 /* positional bigrams */
        hv_gen(&a, DOM_CHAR, (uint32_t)(unsigned char)w[i]);
        hv_gen(&b, DOM_CHAR, (uint32_t)(unsigned char)w[i + 1]);
        hv_rot(&b, &b, 1);
        hv_bind(&g, &a, &b);                          /* bind INSIDE the n-gram */
        hv_acc_add(&acc, &g, 2);                      /* bundle ACROSS n-grams  */
    }
    if (bag_weight)                                   /* order-free channel */
        for (int i = 0; i < n; i++) {
            hv_gen(&a, DOM_CHAR, (uint32_t)(unsigned char)w[i]);
            hv_acc_add(&acc, &a, bag_weight);
        }
    hv_acc_majority(out, &acc, 0x5EED);
}

static void t7_typos(void)
{
    hdr("T7  what n-gram encoding actually tolerates");
    const char *base = "status";
    const char *var[] = { "status", "statvs", "estatus", "statsu", "silence" };
    const char *lab[] = { "identical", "substitution", "prefix insert",
                          "transposition", "unrelated word" };
    hv_t b1, b2, v1, v2;
    enc_word(&b1, base, 0);
    enc_word(&b2, base, 1);
    printf("               positional only   +order-free channel\n");
    for (int i = 0; i < 5; i++) {
        enc_word(&v1, var[i], 0);
        enc_word(&v2, var[i], 1);
        printf("%-14s %.4f            %.4f    %s\n", lab[i],
               agr(&b1, &v1), agr(&b2, &v2), var[i]);
    }
    printf("note: the draft claimed 'gtao' shares 2 of 3 bigrams with 'gato'.\n");
    printf("      bigrams(gato)={ga,at,to}, bigrams(gtao)={gt,ta,ao}: they share NONE.\n");
    printf("      n-grams tolerate substitution, not transposition. The order-free\n");
    printf("      channel is what buys transposition tolerance back.\n");
}

/* ---------------------------------------------------------------- T8 ---- */
static void t8_tier05(void)
{
    hdr("T8  Tier 0.5 graceful decode (lexicon 512, 3000 trials per point)");
    const int L = 512, TR = 3000;
    static hv_t lex[512]; static sbc_t slex[512];
    static uint32_t pos[HV_SK_BITS];
    static hv_sk_t sk_lex[512];
    for (int i = 0; i < L; i++) { hv_gen(&lex[i], DOM_SYM, (uint32_t)i); sbc_gen(&slex[i], DOM_SYM, (uint32_t)i); }
    hv_sk_positions(pos, 0xABCD);
    for (int i = 0; i < L; i++) hv_sk_make(&sk_lex[i], &lex[i], pos);

    double bers[] = { 0.0, 0.02, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30 };
    printf("raw BER   dense sketch %d b (%d B)   sbc sketch %d blk (%d B)   sbc %d blk (%d B)\n",
           HV_SK_BITS, HV_SK_BITS / 8, SBC_SKETCH, (SBC_SKETCH * 6 + 7) / 8, 32, 24);
    for (unsigned t = 0; t < sizeof(bers) / sizeof(bers[0]); t++) {
        double ber = bers[t];
        int dok = 0, s16 = 0, s32 = 0;
        for (int tr = 0; tr < TR; tr++) {
            uint32_t truth = (uint32_t)(hv_mix(&rng) % L);
            hv_sk_t q = sk_lex[truth];
            hv_sk_flip_bits(&q, ber, &rng);
            int best = 0, bd = 1 << 30;
            for (int i = 0; i < L; i++) { int d = hv_sk_dist(&q, &sk_lex[i]); if (d < bd) { bd = d; best = i; } }
            dok += (best == (int)truth);

            uint8_t s[64];
            sbc_sketch_make(s, &slex[truth], SBC_SKETCH);
            sbc_sketch_flip(s, SBC_SKETCH, ber, &rng);
            int b2 = 0, bm = -1;
            for (int i = 0; i < L; i++) { int m = sbc_sketch_matches(s, &slex[i], SBC_SKETCH); if (m > bm) { bm = m; b2 = i; } }
            s16 += (b2 == (int)truth);

            sbc_sketch_make(s, &slex[truth], 32);
            sbc_sketch_flip(s, 32, ber, &rng);
            int b3 = 0; bm = -1;
            for (int i = 0; i < L; i++) { int m = sbc_sketch_matches(s, &slex[i], 32); if (m > bm) { bm = m; b3 = i; } }
            s32 += (b3 == (int)truth);
        }
        printf("%6.0f%%        %6.2f%%                 %6.2f%%                %6.2f%%\n",
               100 * ber, 100.0 * dok / TR, 100.0 * s16 / TR, 100.0 * s32 / TR);
    }
}

/* ---------------------------------------------------------------- T9 ---- */
static void t9_bench(void)
{
    hdr("T9  exhaustive search cost, L=512");
    const int L = 512, REP = 400;
    static hv_t lex[512]; static sbc_t slex[512];
    for (int i = 0; i < L; i++) { hv_gen(&lex[i], DOM_SYM, (uint32_t)i); sbc_gen(&slex[i], DOM_SYM, (uint32_t)i); }
    hv_t q = lex[7]; sbc_t sq = slex[7];
    volatile int sink = 0;
    clock_t t0 = clock();
    for (int r = 0; r < REP; r++) for (int i = 0; i < L; i++) sink += hv_dist(&q, &lex[i]);
    double dns = 1e9 * (double)(clock() - t0) / CLOCKS_PER_SEC / REP;
    t0 = clock();
    for (int r = 0; r < REP; r++) for (int i = 0; i < L; i++) sink += sbc_matches(&sq, &slex[i]);
    double sns = 1e9 * (double)(clock() - t0) / CLOCKS_PER_SEC / REP;
    printf("dense  %d B/vector   %8.0f ns/query   %d bytes touched\n", HV_BYTES, dns, L * HV_BYTES);
    printf("sbc    %d B/vector    %8.0f ns/query   %d bytes touched   (%.1fx less traffic)\n",
           SBC_S, sns, L * SBC_S, (double)HV_BYTES / SBC_S);
    printf("speedup measured: %.1fx  (popcount path: %s)\n", dns / sns,
#ifdef HV_LUT_POPCOUNT
           "byte LUT, as on Xtensa LX7 / Cortex-M33");
#else
           "native hardware popcount — NOT representative of the MCU");
#endif
    (void)sink;
}

/* --------------------------------------------------------------- T10 ---- */
static void t10_two_stage(void)
{
    hdr("T10 two-stage search == exhaustive search");
    const int L = 512, TR = 4000, SHORT = 8;
    static sbc_t slex[512];
    for (int i = 0; i < L; i++) sbc_gen(&slex[i], DOM_SYM, (uint32_t)i);
    int agree = 0;
    for (int tr = 0; tr < TR; tr++) {
        /* query = a lexicon entry perturbed by re-randomising some blocks */
        uint32_t truth = (uint32_t)(hv_mix(&rng) % L);
        sbc_t q = slex[truth];
        for (int i = 0; i < SBC_S; i++)
            if (hv_mix(&rng) % 100 < 35) q.b[i] = (uint8_t)(hv_mix(&rng) % SBC_B);
        int ex = 0, bm = -1;
        for (int i = 0; i < L; i++) { int m = sbc_matches(&q, &slex[i]); if (m > bm) { bm = m; ex = i; } }
        /* coarse pass on SBC_SKETCH blocks, then full compare on the shortlist */
        int cand[8], cs[8];
        for (int k = 0; k < SHORT; k++) { cand[k] = -1; cs[k] = -1; }
        for (int i = 0; i < L; i++) {
            int m = sbc_sketch_matches(q.b, &slex[i], SBC_SKETCH);
            for (int k = 0; k < SHORT; k++)
                if (m > cs[k]) {
                    for (int j = SHORT - 1; j > k; j--) { cs[j] = cs[j - 1]; cand[j] = cand[j - 1]; }
                    cs[k] = m; cand[k] = i; break;
                }
        }
        int two = cand[0]; bm = -1;
        for (int k = 0; k < SHORT; k++) {
            if (cand[k] < 0) continue;
            int m = sbc_matches(&q, &slex[cand[k]]);
            if (m > bm) { bm = m; two = cand[k]; }
        }
        agree += (two == ex);
    }
    printf("agreement with exhaustive: %.2f%%  (%d blocks coarse, top-%d shortlist)\n",
           100.0 * agree / TR, SBC_SKETCH, SHORT);
    printf("cost ratio vs exhaustive: %.2f  (%d/%d + %d/%d)\n",
           (double)SBC_SKETCH / SBC_S + (double)SHORT / 512.0, SBC_SKETCH, SBC_S, SHORT, 512);
}

int main(void)
{
    printf("Herus algebra fire test — D=%d dense, SBC S=%d B=%d\n", HV_BITS, SBC_S, SBC_B);
    t1_orthogonality();
    t2_invertible();
    t3_bundle_law();
    t4_retrieval_capacity();
    t5_role_symmetry();
    t6_threshold();
    t7_typos();
    t8_tier05();
    t9_bench();
    t10_two_stage();
    printf("\ndone.\n");
    return 0;
}
