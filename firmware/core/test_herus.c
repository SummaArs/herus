/* test_herus.c — the proof suite.
 *
 * Every quantitative claim in docs/ is produced by this file. If a claim is
 * not printed here, it is not a claim, it is a hope. Run: make test
 */
/* clock_gettime/CLOCK_MONOTONIC are POSIX. Ask for that API explicitly so the
 * proof builds in strict C11 mode instead of depending on libc defaults. */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "lexicon.h"
#include "hcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

static uint64_t RNG = 0xC0FFEE123456789ull;
static double   now_s(void)
{
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
static void hdr(const char *s) { printf("\n== %s ==\n", s); }

#define HDOM 0x48455255530001ull   /* "HERUS" + version */

/* -------- T1: quasi-orthogonality -------- */
static void t1(void)
{
    hdr("T1  quasi-orthogonality of random codes");
    const int N = 1000;
    hv_t *v = malloc(sizeof(hv_t) * N);
    for (int i = 0; i < N; i++) hv_gen(&v[i], HDOM, i);

    double s = 0, s2 = 0; int mn = HV_BITS, mx = 0, np = 0;
    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++) {
            int d = hv_dist(&v[i], &v[j]);
            s += d; s2 += (double)d * d; np++;
            if (d < mn) mn = d;
            if (d > mx) mx = d;
        }
    double mu = s / np, sd = sqrt(s2 / np - mu * mu);
    double th_mu = HV_BITS / 2.0, th_sd = sqrt((double)HV_BITS) / 2.0;

    printf("  D = %d, pairs = %d\n", HV_BITS, np);
    printf("  mean dist   measured %.1f   theory %.1f\n", mu, th_mu);
    printf("  sigma       measured %.2f   theory %.2f\n", sd, th_sd);
    printf("  closest pair %d bits = %.2f sigma from the mean\n",
           mn, (th_mu - mn) / th_sd);
    printf("  furthest     %d bits\n", mx);
    /* Collision probability for the draft's 55%%-similarity criterion. */
    double z = (th_mu - 0.45 * HV_BITS) / th_sd;
    double p1 = 0.5 * erfc(z / sqrt(2.0));
    printf("  P(pair exceeds 55%% similarity)     = %.3e   [draft said <1e-6]\n", p1);
    printf("  P(any collision among 10k symbols) = %.3e\n", p1 * 5e7);
    free(v);
}

/* -------- T2: bundle capacity -------- */
static double p_disagree_odd(int K)   /* exact: 1 + Binom(K-1,.5) < K/2 */
{
    double p = 0.0, lg = 0.0;
    int n = K - 1, kmax = (K - 3) / 2;
    for (int b = 0; b <= kmax; b++) {
        lg = lgamma(n + 1.0) - lgamma(b + 1.0) - lgamma(n - b + 1.0);
        p += exp(lg - n * log(2.0));
    }
    return p;
}
static void t2(void)
{
    hdr("T2  bundling capacity (how many symbols fit in one hypervector)");
    printf("  %5s %10s %10s %10s %9s %s\n",
           "K", "d/D meas", "d/D exact", "0.5-.399/vK", "margin_s", "recoverable");
    const int Ks[] = {3, 5, 9, 21, 51, 101, 201, 401, 0};
    hv_acc_t *acc = malloc(sizeof(hv_acc_t));
    const double sigma = sqrt((double)HV_BITS) / 2.0;
    for (int t = 0; Ks[t]; t++) {
        int K = Ks[t];
        hv_t *v = malloc(sizeof(hv_t) * K), b;
        for (int i = 0; i < K; i++) hv_gen(&v[i], HDOM + 77, i);
        hv_acc_zero(acc);
        for (int i = 0; i < K; i++) hv_acc_add(acc, &v[i], 1);
        hv_acc_majority(&b, acc, 1);
        double m = 0;
        for (int i = 0; i < K; i++) m += hv_dist(&b, &v[i]);
        m /= K * (double)HV_BITS;
        double margin = (HV_BITS / 2.0 - m * HV_BITS) / sigma;
        printf("  %5d %10.4f %10.4f %10.4f %9.1f %s\n",
               K, m, p_disagree_odd(K), 0.5 - 0.3989 / sqrt((double)K), margin,
               margin > 4.5 ? "yes" : "NO");
        free(v);
    }
    printf("  rule: a member stays recoverable against a 512-entry lexicon\n"
           "        while its margin exceeds ~4.5 sigma (union bound).\n");
    free(acc);
}

/* -------- T3/T4: algebra -------- */
static void t34(void)
{
    hdr("T3/T4  algebraic invariants");
    hv_t a, b, c, x, y;
    hv_gen(&a, HDOM, 1); hv_gen(&b, HDOM, 2); hv_gen(&c, HDOM, 3);

    hv_bind(&x, &a, &b); hv_bind(&x, &x, &b);
    printf("  T3 (A^B)^B == A                        : %s\n",
           hv_dist(&x, &a) == 0 ? "EXACT" : "FAIL");
    hv_bind(&x, &a, &b);
    printf("     sim(A^B, A) = %+.4f  (must be ~0)   : %s\n", hv_sim(&x, &a),
           fabs(hv_sim(&x, &a)) < 0.05 ? "ok" : "FAIL");

    hv_rot(&x, &a, 1);
    printf("     sim(rot(A), A) = %+.4f              : %s\n", hv_sim(&x, &a),
           fabs(hv_sim(&x, &a)) < 0.05 ? "ok" : "FAIL");
    hv_rot(&x, &a, 137); hv_rot(&x, &x, -137);
    printf("     rot^-1(rot(A)) == A                 : %s\n",
           hv_dist(&x, &a) == 0 ? "EXACT" : "FAIL");

    /* Distributivity of XOR over majority, odd arity. The draft wrote "~=".
     * For odd K it is an identity: majority is self-complementary. */
    hv_acc_t *acc = malloc(sizeof(hv_acc_t));
    hv_t v[5], bv[5], m1, m2;
    for (int i = 0; i < 5; i++) hv_gen(&v[i], HDOM + 9, i);
    hv_acc_zero(acc);
    for (int i = 0; i < 5; i++) hv_acc_add(acc, &v[i], 1);
    hv_acc_majority(&m1, acc, 0);
    hv_bind(&m1, &m1, &a);
    for (int i = 0; i < 5; i++) hv_bind(&bv[i], &v[i], &a);
    hv_acc_zero(acc);
    for (int i = 0; i < 5; i++) hv_acc_add(acc, &bv[i], 1);
    hv_acc_majority(&m2, acc, 0);
    printf("  T4 A^maj(V) == maj(A^V), K=5 odd       : %s  (draft: \"approx\")\n",
           hv_dist(&m1, &m2) == 0 ? "EXACT" : "FAIL");

    /* Even arity without a tie-break rule is where implementations rot. */
    hv_acc_zero(acc);
    for (int i = 0; i < 4; i++) hv_acc_add(acc, &v[i], 1);
    hv_acc_majority(&x, acc, 42);
    hv_acc_zero(acc);
    for (int i = 0; i < 4; i++) hv_acc_add(acc, &v[i], 1);
    hv_acc_majority(&y, acc, 43);
    printf("     K=4: two tie-break seeds differ by %d bits (ties are real)\n",
           hv_dist(&x, &y));
    free(acc);
}

/* -------- T5: n-gram text encoding, and the transposition trap -------- */
static void encode_word(hv_t *out, const char *s, int n)
{
    static hv_acc_t acc;
    int len = (int)strlen(s), ng = len - n + 1;
    hv_acc_zero(&acc);
    for (int i = 0; i < ng; i++) {
        hv_t g, ch, r;
        hv_gen(&g, HDOM + 0xC4, (uint32_t)(unsigned char)s[i]);
        for (int k = 1; k < n; k++) {
            hv_gen(&ch, HDOM + 0xC4, (uint32_t)(unsigned char)s[i + k]);
            hv_rot(&r, &ch, k);
            hv_bind(&g, &g, &r);
        }
        hv_acc_add(&acc, &g, 1);
    }
    hv_acc_majority(out, &acc, (uint64_t)len);
}
static void t5(void)
{
    hdr("T5  n-gram encoding: which corruptions survive");
    const char *base = "gato";
    const char *cases[] = {"gato", "gata", "gxto", "gaato", "gtao", "gate", "cachorro", NULL};
    const char *what[]  = {"identical", "substitution (last)", "substitution (mid)",
                           "insertion", "TRANSPOSITION", "substitution (last)",
                           "unrelated", NULL};
    hv_t b, q;
    encode_word(&b, base, 2);
    printf("  bigram encoding, reference \"gato\" -> bigrams {ga,at,to}\n");
    for (int i = 0; cases[i]; i++) {
        encode_word(&q, cases[i], 2);
        printf("    %-9s %-22s sim = %+.3f\n", cases[i], what[i], hv_sim(&b, &q));
    }
    printf("  NOTE: the draft claimed \"gtao\" shares 2 of 3 bigrams with \"gato\".\n"
           "        It shares zero: {gt,ta,ao} n {ga,at,to} = {}. Character\n"
           "        n-grams are robust to substitution and insertion and are\n"
           "        WORST-CASE for transposition. Use trigrams + bigrams together\n"
           "        if transposition matters.\n");
}

/* -------- T6: two-stage search -------- */
static void t6(void)
{
    hdr("T6  two-stage search (sketch prefilter) vs exhaustive");
    lex_t L; lex_init(&L, HDOM, 512);
    const int TR = 2000;
    hv_t q; uint64_t rng = 7;
    for (int sl = 1; sl <= 16; sl *= 2) {
        int agree = 0;
        for (int t = 0; t < TR; t++) {
            int id = (int)(hv_mix(&rng) % L.n);
            lex_code(&L, &q, id);
            hv_flip_bits(&q, 0.20, &rng);              /* heavy query noise */
            int a = lex_search_full(&L, &q, NULL);
            int b = lex_search(&L, &q, sl, NULL);
            if (a == b) agree++;
        }
        printf("  shortlist %2d : agreement with exhaustive = %.2f%%\n",
               sl, 100.0 * agree / TR);
    }
    lex_free(&L);
}

/* -------- T7: sketch decoding under bit errors (Tier 0.5) -------- */
static void t7(void)
{
    hdr("T7  Tier-0.5: decoding a subsampled sketch through a noisy channel");
    printf("  lexicon = 512 symbols, %d-bit subsampled sketch, 4000 trials/point\n",
           HV_SK_BITS);
    lex_t L; lex_init(&L, HDOM, 512);
    const int TR = 4000;
    uint64_t rng = 99;
    printf("  %6s %10s %14s %s\n", "BER", "sketch ok", "framed-8B ok", "verdict");
    for (int i = 1; i <= 9; i++) {
        double ber = 0.05 * i;
        int ok = 0;
        for (int t = 0; t < TR; t++) {
            int id = (int)(hv_mix(&rng) % L.n);
            hv_t v; lex_code(&L, &v, id);
            hv_sk_t s; hv_sk_make(&s, &v, L.pos);
            hv_sk_flip_bits(&s, ber, &rng);
            if (lex_search_sketch(&L, &s, NULL) == id) ok++;
        }
        /* A CRC-protected 8-byte frame survives only with zero bit errors. */
        double framed = pow(1.0 - ber, 64.0 + 16.0);
        printf("  %5.0f%% %9.2f%% %13.2e   %s\n", ber * 100, 100.0 * ok / TR, framed,
               (double)ok / TR > 0.95 ? "sketch usable" : "sketch degrading");
    }
    lex_free(&L);
}

/* -------- T8: resonator factorisation -------- */
static void t8(void)
{
    hdr("T8  resonator network: factorising a composed message");
    lex_t L; lex_init(&L, HDOM, 256);
    const uint32_t base[3] = {0, 64, 128}, size[3] = {32, 32, 32};
    const int TR = 200;
    int ok = 0, iters = 0; uint64_t rng = 5;
    for (int t = 0; t < TR; t++) {
        uint32_t truth[3], got[3];
        hv_t S, v;
        for (int f = 0; f < 3; f++) truth[f] = base[f] + (uint32_t)(hv_mix(&rng) % size[f]);
        lex_code(&L, &S, truth[0]);
        for (int f = 1; f < 3; f++) { lex_code(&L, &v, truth[f]); hv_bind(&S, &S, &v); }
        int it = resonator(&L, &S, 3, base, size, got, 120);
        if (it > 0 && got[0] == truth[0] && got[1] == truth[1] && got[2] == truth[2]) {
            ok++; iters += it;
        }
    }
    printf("  3 factors x 32 candidates = %d combinations in the search space\n",
           32 * 32 * 32);
    printf("  solved %d/%d = %.1f%%, mean %.1f iterations\n",
           ok, TR, 100.0 * ok / TR, ok ? (double)iters / ok : 0.0);
    printf("  a brute-force decoder would test 32768 hypotheses; the resonator\n"
           "  holds all of them superposed and collapses in ~%.0f steps.\n",
           ok ? (double)iters / ok : 0.0);
    lex_free(&L);
}

/* -------- T9: learning, drift guard, rejection -------- */
static void t9(void)
{
    hdr("T9  online learning with an enforced separation invariant");
    lex_t L; lex_init(&L, HDOM, 64);
    proto_t P; proto_init(&P, 8, 3000);
    uint64_t rng = 11;

    /* Eight intent classes, each observed as a noisy version of its atom. */
    for (int rep = 0; rep < 12; rep++)
        for (int c = 0; c < 8; c++) {
            hv_t s; lex_code(&L, &s, (uint32_t)c);
            hv_flip_bits(&s, 0.25, &rng);
            proto_learn(&P, (uint16_t)c, &s);
        }
    printf("  8 classes x 12 noisy samples (25%% bit noise)\n");
    printf("  min pairwise prototype separation = %d bits (floor %d)\n",
           proto_min_separation(&P), P.min_sep);

    int hit = 0, rej = 0;
    for (int t = 0; t < 800; t++) {
        int c = (int)(hv_mix(&rng) % 8);
        hv_t s; lex_code(&L, &s, (uint32_t)c);
        hv_flip_bits(&s, 0.25, &rng);
        int r = proto_classify(&P, &s, 4.5, NULL);
        if (r == c) hit++; else if (r < 0) rej++;
    }
    printf("  in-domain : %.1f%% correct, %.1f%% rejected\n",
           hit / 8.0, rej / 8.0);

    int fa = 0;
    for (int t = 0; t < 800; t++) {
        hv_t s; lex_code(&L, &s, (uint32_t)(32 + (hv_mix(&rng) % 32)));
        if (proto_classify(&P, &s, 4.5, NULL) >= 0) fa++;
    }
    printf("  out-of-domain queries accepted (false alarms) : %.2f%%\n", fa / 8.0);
    printf("  a softmax classifier has no equivalent of this column: it always\n"
           "  emits a distribution. Rejection is a native property here.\n");

    /* Now try to break it: feed class 1 samples labelled class 0. */
    int rollback = 0;
    for (int t = 0; t < 40; t++) {
        hv_t s; lex_code(&L, &s, 1);
        hv_flip_bits(&s, 0.10, &rng);
        if (proto_learn(&P, 0, &s) < 0) rollback++;
    }
    printf("  adversarial mislabelling: %d/40 updates refused by the drift guard,\n"
           "  separation held at %d bits\n", rollback, proto_min_separation(&P));
    proto_free(&P); lex_free(&L);
}

/* -------- T10: throughput -------- */
static void t10(void)
{
    hdr("T10  throughput (host measurement -> MCU projection)");
    lex_t L; lex_init(&L, HDOM, 512);
    hv_t q; lex_code(&L, &q, 123);
    uint64_t rng = 3; hv_flip_bits(&q, 0.2, &rng);

    const int N = 2000;
    double t0 = now_s(); volatile int sink = 0;
    for (int i = 0; i < N; i++) sink += lex_search_full(&L, &q, NULL);
    double tf = (now_s() - t0) / N;

    t0 = now_s();
    for (int i = 0; i < N * 20; i++) sink += lex_search(&L, &q, 8, NULL);
    double ts = (now_s() - t0) / (N * 20);
    (void)sink;

    printf("  exhaustive  512 x %d B : %8.1f us/query\n", HV_BYTES, tf * 1e6);
    printf("  two-stage   sketch+8   : %8.1f us/query   (%.1fx faster)\n",
           ts * 1e6, tf / ts);
    printf("  bytes touched: exhaustive %d, two-stage %d (%.1fx less)\n",
           512 * HV_BYTES, 512 * (HV_SK_BITS / 8) + 8 * HV_BYTES,
           512.0 * HV_BYTES / (512.0 * (HV_SK_BITS / 8) + 8.0 * HV_BYTES));
    printf("  MCU projection at 4 cycles/byte (LUT popcount, no HW popcount on\n");
    printf("  Xtensa LX7 or Cortex-M33):\n");
    double be = 512.0 * HV_BYTES, bt = 512.0 * (HV_SK_BITS / 8) + 8.0 * HV_BYTES;
    printf("    ESP32-S3  @240 MHz : exhaustive %.2f ms, two-stage %.2f ms\n",
           be * 4 / 240e3, bt * 4 / 240e3);
    printf("    nRF54L15  @128 MHz : exhaustive %.2f ms, two-stage %.2f ms\n",
           be * 4 / 128e3, bt * 4 / 128e3);
    printf("  energy/query (active current x time):\n");
    printf("    ESP32-S3  @45 mA   : %.3f uAh   two-stage\n", bt * 4 / 240e6 * 45.0 / 3600.0 * 1000.0);
    printf("    nRF54L15  @3.2 mA  : %.3f uAh   two-stage\n", bt * 4 / 128e6 * 3.2 / 3600.0 * 1000.0);
    printf("  RAM: codebook 0 B (derived), sketch table %d B, working set ~%d B\n",
           512 * (HV_SK_BITS / 8), 4 * HV_BYTES);
    lex_free(&L);
}

/* -------- T11: wire protocol -------- */
static void t11(void)
{
    hdr("T11  HCP wire format rev 0.2: constant length, tier-opaque, 9 slots");
    hcp_msg_t a = {0}, b = {0}, r;
    uint8_t fa[HCP_PLAINTEXT_LEN], fb[HCP_PLAINTEXT_LEN];

    a.tier = HCP_TIER_GLYPH; a.intent = 7; a.seq = 1;
    b.tier = HCP_TIER_COMPOSED; b.intent = 12; b.seq = 2; b.nslot = HCP_MAX_SLOT;
    for (int i = 0; i < HCP_MAX_SLOT; i++) {
        b.slot[i].role   = (uint8_t)(i + 1);
        b.slot[i].filler = (uint16_t)(300 + i);
    }

    int na = hcp_encode(fa, &a), nb = hcp_encode(fb, &b);
    printf("  Tier-0 glyph          encodes to %d bytes\n", na);
    printf("  Tier-1, %d full slots encodes to %d bytes\n", HCP_MAX_SLOT, nb);
    printf("  constant length       : %s  (defeats length-based traffic analysis)\n",
           (na == nb && na == HCP_PLAINTEXT_LEN) ? "YES" : "NO — LEAK");

    hcp_decode(&r, fb);
    int ok = (r.tier == b.tier && r.intent == b.intent && r.nslot == b.nslot);
    for (int i = 0; i < HCP_MAX_SLOT && ok; i++)
        ok = (r.slot[i].role == b.slot[i].role && r.slot[i].filler == b.slot[i].filler);
    printf("  round-trip 9 slots    : %s\n", ok ? "ok" : "FAIL");

    lex_t L; lex_init(&L, HDOM, 1024);
    hv_acc_t *acc = malloc(sizeof(hv_acc_t));
    hv_t H; hcp_to_hv(&H, acc, &L, &b);

    /* The hypothesis test, at each slot position. rev 0.2 binds slot k under
     * rho^(k+2), so the probe must unrotate by the same amount — which is also
     * what makes slot ORDER part of the meaning. */
    int hits = 0;
    for (int k = 0; k < HCP_MAX_SLOT; k++) {
        uint16_t f = 0;
        if (hcp_query_role(&H, &L, (uint8_t)(k + 1), k, 0, 512, 12.0, &f)
            && f == (uint16_t)(300 + k)) hits++;
    }
    printf("  unbind all 9 slots    : %d/%d recovered exactly  : %s\n",
           hits, HCP_MAX_SLOT, hits == HCP_MAX_SLOT ? "ok" : "FAIL");

    uint16_t f = 0;
    int found = hcp_query_role(&H, &L, 20, 0, 0, 512, 12.0, &f);
    printf("  unbind an absent role : correctly reports absent  : %s\n",
           !found ? "ok" : "FAIL");
    printf("  an unknown role id is simply not queried, so old firmware ignores\n"
           "  new fields instead of failing: forward compatibility for free.\n");
    free(acc);
    lex_free(&L);
}

int main(void)
{
    printf("HERUS core proof suite   D=%d bits (%d B)  sketch=%d bits\n",
           HV_BITS, HV_BYTES, HV_SK_BITS);
    (void)RNG;
    t1(); t2(); t34(); t5(); t6(); t7(); t8(); t9(); t10(); t11();
    printf("\n");
    return 0;
}
