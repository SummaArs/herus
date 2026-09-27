/* study.c — the concurseiro, and the one thing a language model cannot promise.
 *
 * A student preparing for a Brazilian public exam does not need a device that
 * talks. They need one that (a) does not distract them, (b) recalls a fact
 * exactly, and (c) SAYS SO when it is not sure. The third is the whole product.
 * An article of law invented with confidence is worse than silence, because the
 * student will carry it into the exam.
 */
#include "sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define NFACT   150
#define NSLOT     3
#define DOM     0x434F4E43555253ull
#define MARGIN_SIGMA  4.0

typedef struct { uint16_t intent; uint8_t role[NSLOT]; uint16_t fill[NSLOT]; } fact_t;

static void build_fact(sim_world *w, fact_t *f)
{
    f->intent = (uint16_t)(1 + sim_rand(w) % 40);
    for (int k = 0; k < NSLOT; k++) {
        f->role[k] = (uint8_t)(1 + k);
        f->fill[k] = (uint16_t)(1 + sim_rand(w) % 120);
    }
}

static void to_msg(const fact_t *f, int nslot, const int *order, hcp_msg_t *m)
{
    memset(m, 0, sizeof *m);
    m->tier = HCP_TIER_COMPOSED;
    m->intent = f->intent;
    m->nslot = (uint8_t)nslot;
    for (int k = 0; k < nslot; k++) {
        int src = order ? order[k] : k;
        m->slot[k].role = f->role[src];
        m->slot[k].filler = f->fill[src];
    }
}

static void encode_hv(const lex_t *L, hv_acc_t *acc, hcp_msg_t *m, hv_t *out)
{
    uint8_t pt[HCP_PLAINTEXT_LEN];
    hcp_encode(pt, m);
    hcp_to_hv(out, acc, L, m);
}

void scenario_study(sim_score *s, int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("\nE1. study — a device a concurseiro can trust\n");
    printf("----------------------------------------------\n");
    printf("  %d facts compiled offline into hypervector prototypes. Queries are\n", NFACT);
    printf("  bundles of role/filler bindings, degraded the way a tired student\n");
    printf("  degrades them. The answer threshold is derived below, not chosen;\n"
           "  %.0f sigma of margin separates a decision from an ambiguity.\n\n",
           MARGIN_SIGMA);

    sim_world w; sim_world_init(&w, 1, 0xE57D0);
    lex_t L; lex_init(&L, DOM, 4096);
    proto_t P; proto_init(&P, NFACT, 1000);
    hv_acc_t *acc = malloc(sizeof *acc);
    hv_t v;

    static fact_t F[NFACT];
    for (int i = 0; i < NFACT; i++) {
        build_fact(&w, &F[i]);
        hcp_msg_t m; to_msg(&F[i], NSLOT, NULL, &m);
        encode_hv(&L, acc, &m, &v);
        proto_learn(&P, (uint16_t)i, &v);
    }
    printf("  codex built: %d facts, minimum pairwise separation %d bits (D/2 = %d)\n\n",
           NFACT, proto_min_separation(&P), HV_BITS / 2);

    struct { const char *name; int nslot; int reorder; int corrupt; int novel; } T[] = {
        { "exact          — the student remembers it all", NSLOT, 0, 0, 0 },
        { "one slot short — forgot the orgao", 2, 0, 0, 0 },
        { "two slots short — only the artigo", 1, 0, 0, 0 },
        { "reordered      — same slots, different order", NSLOT, 1, 0, 0 },
        { "one wrong      — misremembered a filler (correction)", NSLOT, 0, 1, 0 },
        { "not in codex   — never studied it", NSLOT, 0, 0, 1 },
    };
    int nt = (int)(sizeof T / sizeof *T);

    double thresh[NSLOT + 1] = {0}, sugg[NSLOT + 1] = {0};
    printf("  calibration, per query size, over %d facts:\n", NFACT);
    printf("    slots   weakest TRUE   strongest FALSE   gap    threshold\n");
    for (int ns = 1; ns <= NSLOT; ns++) {
        double wt = 1e9, bf = -1e9;
        for (int i = 0; i < NFACT; i++) {
            hcp_msg_t m; lex_resolution r;
            to_msg(&F[i], ns, NULL, &m);
            encode_hv(&L, acc, &m, &v);
            proto_resolve(&P, &v, -1e9, 0.0, 0.0, NULL, 0, &r);
            if (r.id == i && r.sigma < wt) wt = r.sigma;

            fact_t q = F[i];
            for (int k = 0; k < NSLOT; k++) q.fill[k] = (uint16_t)(500 + k + i);
            to_msg(&q, ns, NULL, &m);
            encode_hv(&L, acc, &m, &v);
            proto_resolve(&P, &v, -1e9, 0.0, 0.0, NULL, 0, &r);
            if (r.sigma > bf) bf = r.sigma;
        }
        thresh[ns] = (wt > bf) ? (wt + bf) / 2.0 : wt;
        sugg[ns] = bf + 0.5;
        printf("      %d     %9.1f      %11.1f   %+6.1f   %8.1f%s\n",
               ns, wt, bf, wt - bf, thresh[ns], (wt > bf) ? "" : "   OVERLAP");
    }

    printf("\n   query                                      certo  sugestao  recusa  ERRADO\n");
    int total_wrong = 0, exact_ok = 0, partial_ok = 0, novel_refused = 0, reorder_ok = 0;

    for (int t = 0; t < nt; t++) {
        int ok = 0, refused = 0, wrong = 0, sug = 0, sug_right = 0;
        int by[LEX_VERDICT_COUNT] = {0};
        for (int i = 0; i < NFACT; i++) {
            fact_t q = F[i];
            if (T[t].novel) for (int k = 0; k < NSLOT; k++) q.fill[k] = (uint16_t)(500 + k + i);
            if (T[t].corrupt) q.fill[1] = (uint16_t)(1 + (q.fill[1] + 37) % 120);

            int order[NSLOT] = { 0, 1, 2 };
            if (T[t].reorder) { order[0] = 2; order[2] = 0; }

            hcp_msg_t m; to_msg(&q, T[t].nslot, order, &m);
            encode_hv(&L, acc, &m, &v);

            lex_resolution r;
            proto_resolve(&P, &v, thresh[T[t].nslot], sugg[T[t].nslot], MARGIN_SIGMA, NULL, 0, &r);
            if (r.verdict < 0 || r.verdict >= LEX_VERDICT_COUNT) {
                fprintf(stderr, "invalid lexicon verdict %d\n", (int)r.verdict);
                total_wrong++;
                continue;
            }
            by[r.verdict]++;
            if (r.verdict == LEX_OK) {
                if (r.id == i && !T[t].novel) ok++;
                else wrong++;
            } else if (r.verdict == LEX_V0_SUGGEST) {
                sug++;
                if (r.id == i && !T[t].novel) sug_right++;
            } else refused++;
        }
        printf("   %-42s %4d   %5d%s %5d    %4d\n", T[t].name, ok, sug,
               sug ? (sug_right == sug ? "*" : " ") : " ", refused, wrong);
        if (by[LEX_V1_UNKNOWN] || by[LEX_V2_AMBIGUOUS] || by[LEX_V3_COLLISION])
            printf("        desconhecido=%d  ambiguo=%d  colisao=%d\n",
                   by[LEX_V1_UNKNOWN], by[LEX_V2_AMBIGUOUS], by[LEX_V3_COLLISION]);
        total_wrong += wrong;
        if (t == 0) exact_ok = ok;
        if (t == 1) partial_ok = ok;
        if (t == 3) reorder_ok = ok + sug_right;
        if (t == 5) novel_refused = refused;
    }

    printf("\n  exact recall               %3d / %d\n", exact_ok, NFACT);
    printf("  recall from 2 of 3 slots   %3d / %d\n", partial_ok, NFACT);
    printf("  reordered: answered or correctly suggested   %3d / %d\n", reorder_ok, NFACT);
    printf("  never-studied refused      %3d / %d\n", novel_refused, NFACT);
    printf("  CONFIDENTLY WRONG          %3d\n", total_wrong);

    printf("\n  (* = every suggestion pointed at the right fact)\n");
    printf("\n  A language model asked these same six questions answers all six\n"
           "  fluently, and has no mechanism to produce the fourth column. That\n"
           "  column is the product.\n");

    sim_ok(s, exact_ok == NFACT, "every fact is recalled exactly from a complete query");
    sim_ok(s, total_wrong == 0, "the device is never confidently wrong");
    sim_ok(s, novel_refused == NFACT, "a fact never studied is always refused, never invented");
    sim_ok(s, partial_ok > NFACT / 2, "most facts survive losing a third of the query");
    sim_ok(s, thresh[1] > 0 && thresh[NSLOT] > thresh[1],
           "a shorter query needs a lower bar, and the bars are measured not chosen");

    free(acc);
    proto_free(&P);
    lex_free(&L);
    sim_world_free(&w);
}
