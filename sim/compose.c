/* compose.c — the question the radio work never touched.
 *
 * Herus has one button and a vibration motor. No screen, no keyboard, no
 * microphone. The lexicon is 2048 fillers across 32 roles. So the product does
 * not actually hinge on how far a frame carries — it hinges on this:
 *
 *     How many button presses does it take to say
 *     "pump trouble, at field 3, now"?
 *
 * If the answer is thirty, the range is irrelevant because nobody will use it.
 * If the answer is four, the device works. Nothing about the radio changes that
 * number, and four sessions of RF work did not move it by one press.
 *
 * THE INTERACTION, STATED SO THE METRIC MEANS SOMETHING
 * ----------------------------------------------------
 * The device proposes one symbol at a time as a haptic pattern. A short press
 * accepts it; a long press rejects it and the device proposes the next
 * candidate. So a symbol offered at rank k costs k+1 presses, and rank 0 — the
 * device guessing right first — costs exactly one.
 *
 * That makes "presses per message" a real, falsifiable number, and it makes the
 * intelligence measurable rather than adjectival: the whole job of the cognition
 * layer is to put the right symbol at rank 0.
 *
 * WHAT IS BEING COMPARED
 * ----------------------
 *   flat      the lexicon in id order. What you get with no model at all.
 *   unigram   candidates ranked by how often each is used. A frequency table.
 *   context   ranked by P(symbol | what I last said, what was last said to me,
 *             time of day, who I am talking to), backing off to unigram and
 *             then to flat when the context has not been seen.
 *
 * The model is trained on the first 70% of the traffic and MEASURED on the last
 * 30%, because a predictor scored on its own training data is a lookup table
 * wearing a hat.
 */
#include "sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ---------------------------------------------------------------- lexicon
 * A field team on a farm with no cellular coverage — the use case the range
 * numbers were finally computed for. Twelve intents, five roles, twenty
 * fillers: small enough to read, large enough that flat ranking is painful. */
#define NINT   12
#define NROLE   5
#define NFILL  20

static const char *INTENT[NINT] = {
    "estou aqui", "venha", "pronto", "aguarde", "acabou", "quanto falta",
    "problema", "resolvido", "socorro", "saindo", "chegando", "confirmado",
};
static const char *ROLE[NROLE] = { "onde", "quando", "o que", "quanto", "quem" };
static const char *FILL[NFILL] = {
    "o galpao", "a bomba", "o talhao 3", "o trator", "a estrada",
    "agora", "em 10 min", "ao meio-dia", "no fim do dia", "amanha",
    "agua", "diesel", "semente", "adubo", "a colheitadeira",
    "dois", "tres", "cinco", "eu", "voce",
};

/* A message as the composer sees it: an intent and up to two role/filler pairs,
 * which is what the Reach profile's four slots afford with room to spare. */
typedef struct { int intent, nslot, role[2], fill[2]; int hour, peer; } msg_t;

/* ------------------------------------------------------------- the corpus
 * Generated from a small grammar with realistic skew and real context
 * dependence — "problema" is followed by "venha" far more often than by
 * "confirmado", and nobody says "saindo" at 06:00. A uniform corpus would make
 * every ranker look identical, which is the easiest way to accidentally prove
 * that a model works. */
static int pick(sim_world *w, const double *p, int n)
{
    double r = sim_rand01(w), acc = 0;
    for (int i = 0; i < n; i++) { acc += p[i]; if (r <= acc) return i; }
    return n - 1;
}

static void corpus(sim_world *w, msg_t *m, int n)
{
    /* Base rates: a field team says "estou aqui" and "pronto" all day and
     * "socorro" almost never. */
    double base[NINT] = { .17,.13,.15,.06,.07,.05,.09,.08,.01,.07,.09,.03 };
    int last = 0;
    for (int i = 0; i < n; i++) {
        double p[NINT];
        memcpy(p, base, sizeof p);
        /* Context: what was just said moves the odds hard. */
        if (last == 6)      { p[1] += 0.35; p[8] += 0.05; }   /* problema -> venha */
        else if (last == 1) { p[10] += 0.30; }                /* venha -> chegando */
        else if (last == 5) { p[4] += 0.25; p[2] += 0.15; }   /* quanto falta -> acabou/pronto */
        else if (last == 9) { p[10] += 0.30; }                /* saindo -> chegando */
        double s = 0; for (int k = 0; k < NINT; k++) s += p[k];
        for (int k = 0; k < NINT; k++) p[k] /= s;

        m[i].hour = (int)(sim_rand(w) % 3);            /* morning / midday / dusk */
        m[i].peer = (int)(sim_rand(w) % 3);
        /* A worker has a patch and a job. The first version of this corpus drew
         * fillers uniformly, which made every model look useless — and it was
         * the corpus that was wrong, not the models. Nobody works everywhere. */
        int home = m[i].peer;                          /* this person's usual place */
        m[i].intent = pick(w, p, NINT);

        /* Slots follow the intent. "problema" is nearly always about a thing in
         * a place; "estou aqui" is a place and nothing else. */
        int in = m[i].intent;
        m[i].nslot = 0;
        if (in == 0 || in == 9 || in == 10) {
            m[i].role[0] = 0;                          /* onde */
            m[i].fill[0] = (sim_rand01(w) < 0.65) ? home : (int)(sim_rand(w) % 5);
            m[i].nslot = 1;
        } else if (in == 6 || in == 7) {
            m[i].role[0] = 2;                          /* o que */
            m[i].fill[0] = 10 + ((sim_rand01(w) < 0.6) ? (home % 3) : (int)(sim_rand(w) % 5));
            m[i].role[1] = 0;
            m[i].fill[1] = (sim_rand01(w) < 0.65) ? home : (int)(sim_rand(w) % 5);
            m[i].nslot = 2;
        } else if (in == 1 || in == 3) {
            m[i].role[0] = 1;                          /* quando */
            m[i].fill[0] = 5 + ((sim_rand01(w) < 0.55) ? 0 : (int)(sim_rand(w) % 5));
            m[i].nslot = 1;
        } else if (in == 5 || in == 4) {
            m[i].role[0] = 3;                          /* quanto */
            m[i].fill[0] = 15 + (int)(sim_rand(w) % 3);
            m[i].nslot = 1;
        }
        last = in;
    }
}

/* ------------------------------------------------- the structure that was there
 * The first two versions of this file ranked all twenty fillers for every slot
 * and got nowhere. The lexicon had already answered the question: a slot carries
 * a ROLE, and a role admits one kind of filler. "onde" never takes "diesel".
 *
 * So the candidate set for a slot is not the lexicon, it is the role's group —
 * five, not twenty — and the role itself is usually implied by the intent, which
 * the device has just been told. "problema" asks what and where; it does not ask
 * how many. Both facts are learned from the same traffic as everything else.
 *
 * This is not a cleverer model. It is the same counting, applied to a structure
 * that hcp.h has been describing since the day it was written. Missing it cost
 * two rounds of measurement, which is the argument for measuring. */
static const int GROUP_LO[NROLE] = {  0,  5, 10, 15, 18 };
static const int GROUP_HI[NROLE] = {  5, 10, 15, 18, 20 };

/* ------------------------------------------------------------- the models
 * Counts, not weights. A back-off n-gram is the right tool here and pretending
 * otherwise would be decoration: the context space is tiny, the data is sparse,
 * and anything with parameters to fit would be fitting noise. It also runs in
 * a few hundred bytes on an MCU, which a fitted model would not. */
#define NCTX (NINT * 3 * 3)          /* last intent x hour x peer */
typedef struct {
    uint16_t ctx_i[NCTX][NINT];
    uint16_t uni_i[NINT];
    uint16_t ctx_f[NINT][NFILL];     /* filler given the intent */
    /* Filler given the intent AND the peer. The place a message is about is
     * decided by WHO is talking far more than by what they are saying — each
     * person works their own patch. Conditioning only on the intent threw that
     * away, and it was the largest single source of wasted presses. */
    uint16_t ctx_fp[NINT][3][NFILL];
    uint16_t uni_f[NFILL];
    uint16_t uni_r[NROLE];
    /* The role structure each intent implies, learned rather than declared. */
    uint16_t shape[NINT][NROLE];
    uint16_t nslot_of[NINT][3];
} model_t;

static int ctx_of(const msg_t *m, int last) { return (last * 3 + m->hour) * 3 + m->peer; }

static void train(model_t *M, const msg_t *m, int n)
{
    memset(M, 0, sizeof *M);
    int last = 0;
    for (int i = 0; i < n; i++) {
        M->uni_i[m[i].intent]++;
        M->ctx_i[ctx_of(&m[i], last)][m[i].intent]++;
        M->nslot_of[m[i].intent][m[i].nslot]++;
        for (int k = 0; k < m[i].nslot; k++) {
            M->uni_r[m[i].role[k]]++;
            M->uni_f[m[i].fill[k]]++;
            M->ctx_f[m[i].intent][m[i].fill[k]]++;
            M->ctx_fp[m[i].intent][m[i].peer][m[i].fill[k]]++;
            M->shape[m[i].intent][m[i].role[k]]++;
        }
        last = m[i].intent;
    }
}

/* Rank of `target` when candidates are ordered by (primary, then backoff, then
 * id). Returns 0 for "the device offered it first". */
static int rank_of(const uint16_t *primary, const uint16_t *backoff, int n, int target)
{
    int r = 0;
    for (int i = 0; i < n; i++) {
        if (i == target) continue;
        long a = primary ? primary[i] : 0, b = primary ? primary[target] : 0;
        if (a > b) { r++; continue; }
        if (a < b) continue;
        long ba = backoff ? backoff[i] : 0, bb = backoff ? backoff[target] : 0;
        if (ba > bb) { r++; continue; }
        if (ba < bb) continue;
        if (i < target) r++;                    /* stable: id order */
    }
    return r;
}

/* ------------------------------------------------- whole-message proposals
 * Symbol-by-symbol composition is what a keyboard forces on you, and copying it
 * onto a device whose entire thesis is that it transmits MEANING is the wrong
 * instinct. Even with a perfect symbol model, a two-slot message cannot cost
 * less than five presses, because it is five accept actions. The first version
 * of this file measured exactly that floor and called it a result.
 *
 * A device that sends meanings should propose meanings. It keeps the K most
 * likely COMPLETE messages for the current context and offers them whole; one
 * press sends one. Only when none is right does it fall back to composing
 * symbol by symbol.
 *
 * K x 12 contexts x 6 bytes is under half a kilobyte. This is not a model in
 * any interesting sense — it is the observation that people repeat themselves,
 * spent where it pays. */
#define TOPK 2
typedef struct { uint32_t key; uint16_t n; } cand_t;
typedef struct { cand_t top[NINT][TOPK]; } msgmodel_t;

static uint32_t msgkey(const msg_t *m)
{
    uint32_t k = (uint32_t)m->intent;
    for (int i = 0; i < 2; i++) {
        int r = i < m->nslot ? m->role[i] + 1 : 0;
        int f = i < m->nslot ? m->fill[i] + 1 : 0;
        k = k * 40u + (uint32_t)r;
        k = k * 40u + (uint32_t)f;
    }
    return k;
}

static void train_msg(msgmodel_t *MM, const msg_t *m, int n)
{
    memset(MM, 0, sizeof *MM);
    enum { SCRATCH = 4096 };
    static cand_t sc[NINT][SCRATCH];
    static int nsc[NINT];
    memset(sc, 0, sizeof sc); memset(nsc, 0, sizeof nsc);
    int last = 0;
    for (int i = 0; i < n; i++) {
        uint32_t k = msgkey(&m[i]);
        int c = last, found = 0;
        for (int j = 0; j < nsc[c]; j++)
            if (sc[c][j].key == k) { sc[c][j].n++; found = 1; break; }
        if (!found && nsc[c] < SCRATCH) { sc[c][nsc[c]].key = k; sc[c][nsc[c]].n = 1; nsc[c]++; }
        last = m[i].intent;
    }
    for (int c = 0; c < NINT; c++)
        for (int slot = 0; slot < TOPK; slot++) {
            int best = -1;
            for (int j = 0; j < nsc[c]; j++) {
                int taken = 0;
                for (int t = 0; t < slot; t++) if (MM->top[c][t].key == sc[c][j].key) taken = 1;
                if (taken || sc[c][j].n == 0) continue;
                if (best < 0 || sc[c][j].n > sc[c][best].n) best = j;
            }
            if (best >= 0) MM->top[c][slot] = sc[c][best];
        }
}

static int msg_rank(const msgmodel_t *MM, int last, const msg_t *m)
{
    uint32_t k = msgkey(m);
    for (int i = 0; i < TOPK; i++)
        if (MM->top[last][i].n && MM->top[last][i].key == k) return i;
    return -1;
}

/* Rank within a role's own group, which is the only place the answer can be. */
static int rank_in_group(const uint16_t *primary, const uint16_t *backoff,
                         int role, int target)
{
    int r = 0;
    for (int i = GROUP_LO[role]; i < GROUP_HI[role]; i++) {
        if (i == target) continue;
        long a = primary ? primary[i] : 0, b = primary ? primary[target] : 0;
        if (a > b) { r++; continue; }
        if (a < b) continue;
        long ba = backoff ? backoff[i] : 0, bb = backoff ? backoff[target] : 0;
        if (ba > bb) { r++; continue; }
        if (ba < bb) continue;
        if (i < target) r++;
    }
    return r;
}

/* Is this role the one the intent usually asks for at this position? If so the
 * device does not ask at all — it just prompts for the filler. */
static int role_implied(const model_t *M, int intent, int role, int pos)
{
    int better = 0;
    for (int r = 0; r < NROLE; r++)
        if (r != role && M->shape[intent][r] > M->shape[intent][role]) better++;
    return better <= pos;
}

enum { M_FLAT = 0, M_UNI, M_CTX, M_MSG, M_STRUCT };

static const msgmodel_t *MMG;
static double g_hit;

static double presses(const model_t *M, const msg_t *m, int n, int mode0,
                      double *out_worst)
{
    long total = 0; int worst = 0, last = 0; long hits = 0;
    for (int i = 0; i < n; i++) {
        int mode = mode0, pre = 0, r;
        if (mode == M_MSG) {
            int mr = msg_rank(MMG, last, &m[i]);
            if (mr >= 0) {
                total += mr + 1;
                if (mr + 1 > worst) worst = mr + 1;
                hits++; last = m[i].intent;
                continue;
            }
            pre = TOPK;                        /* reject all K, then compose */
            mode = M_STRUCT;
        }
        if (mode == M_FLAT)      r = rank_of(NULL, NULL, NINT, m[i].intent);
        else if (mode == M_UNI)  r = rank_of(M->uni_i, NULL, NINT, m[i].intent);
        else                     r = rank_of(M->ctx_i[ctx_of(&m[i], last)],
                                             M->uni_i, NINT, m[i].intent);
        int cost = r + 1;
        for (int k = 0; k < m[i].nslot; k++) {
            int rr, rf;
            if (mode == M_FLAT) { rr = rank_of(NULL, NULL, NROLE, m[i].role[k]);
                                  rf = rank_of(NULL, NULL, NFILL, m[i].fill[k]); }
            else if (mode == M_UNI) { rr = rank_of(M->uni_r, NULL, NROLE, m[i].role[k]);
                                      rf = rank_of(M->uni_f, NULL, NFILL, m[i].fill[k]); }
            else if (mode == M_CTX) { rr = rank_of(M->uni_r, NULL, NROLE, m[i].role[k]);
                   rf = rank_of(M->ctx_f[m[i].intent], M->uni_f, NFILL, m[i].fill[k]); }
            else {
                /* Structured: the intent implies the role, and the role scopes
                 * the fillers to its own group. */
                rr = role_implied(M, m[i].intent, m[i].role[k], k)
                     ? -1 : rank_of(M->shape[m[i].intent], NULL, NROLE, m[i].role[k]);
                rf = rank_in_group(M->ctx_fp[m[i].intent][m[i].peer],
                                   M->ctx_f[m[i].intent],
                                   m[i].role[k], m[i].fill[k]);
            }
            cost += (rr < 0 ? 0 : rr + 1) + rf + 1;
        }
        cost += pre;
        total += cost;
        if (cost > worst) worst = cost;
        last = m[i].intent;
    }
    g_hit = (double)hits / n;
    if (out_worst) *out_worst = worst;
    return (double)total / n;
}

/* ------------------------------------------------------------------ report */
void scenario_compose(sim_score *s, int argc, char **argv)
{
    int n = opt_int(argc, argv, "--messages", 4000);

    printf("\nC1. compose — how many presses is a sentence\n");
    printf("---------------------------------------------\n");
    printf("  One button, one vibration motor, no screen. The device proposes a\n");
    printf("  symbol; a short press accepts, a long press asks for the next. A\n");
    printf("  symbol offered at rank k therefore costs k+1 presses, and the whole\n");
    printf("  job of the cognition layer is to put the right one at rank 0.\n\n");

    sim_world w; sim_world_init(&w, 1, 0xC0FFEE11);
    msg_t *m = malloc((size_t)n * sizeof *m);
    corpus(&w, m, n);
    int ntrain = n * 7 / 10, ntest = n - ntrain;

    model_t M;
    train(&M, m, ntrain);
    static msgmodel_t MM;
    train_msg(&MM, m, ntrain);
    MMG = &MM;

    printf("  %d messages, trained on the first %d, measured on the last %d.\n\n",
           n, ntrain, ntest);
    printf("   model     presses/message   worst message   vs no model\n");
    double flat = 0, res[5], worst[5], hit[5];
    for (int mode = 0; mode < 5; mode++) {
        res[mode] = presses(&M, m + ntrain, ntest, mode, &worst[mode]);
        hit[mode] = g_hit;
        if (mode == 0) flat = res[0];
        static const char *NAME[5] = { "flat         ", "unigram      ",
                                       "context      ", "top-2 whole  ",
                                       "structured   " };
        printf("   %s %11.2f   %13.0f   %8.1fx\n",
               NAME[mode], res[mode], worst[mode], flat / res[mode]);
    }
    printf("\n   the whole-message model offered the right sentence outright\n"
           "   %.0f%% of the time, from %d proposals per context.\n",
           100.0 * hit[M_MSG], TOPK);

    /* One message, spelled out, so the number stops being abstract. */
    msg_t ex = { 6, 2, {2, 0}, {1, 2}, 1, 0 };     /* problema | o que: a bomba | onde: o talhao 3 */
    printf("\n  the example, end to end:\n");
    printf("    \"%s | %s: %s | %s: %s\"\n",
           INTENT[ex.intent], ROLE[ex.role[0]], FILL[ex.fill[0]],
           ROLE[ex.role[1]], FILL[ex.fill[1]]);
    for (int mode = 0; mode < 5; mode++) {
        double wq; double p = presses(&M, &ex, 1, mode, &wq);
        static const char *NAME[5] = { "flat         ", "unigram      ",
                                       "context      ", "top-2 whole  ",
                                       "structured   " };
        printf("      %s  %2.0f presses\n", NAME[mode], p);
    }

    printf("\n  What this costs on the device: %zu bytes of counters, no floats,\n"
           "  no training on the wrist — the table is built by the same offline\n"
           "  compiler that builds the codex, from the group's own traffic.\n",
           sizeof(model_t) + sizeof(msgmodel_t));

    /* The floor: what a PERFECT model would still cost, because accepting is
     * itself a press. One for the intent, one per filler, roles implied. */
    double floor_p = 0;
    for (int i = ntrain; i < n; i++) floor_p += 1.0 + m[i].nslot;
    floor_p /= ntest;

    printf("\n   a perfect model would still cost %.2f presses — accepting IS a press.\n",
           floor_p);
    printf("   so the model is worth %.1f of the %.1f presses that were on the table,\n",
           flat - res[M_STRUCT], flat - floor_p);
    printf("   and %.2f presses of slack remain between it and the floor.\n",
           res[M_STRUCT] - floor_p);

    printf("\n   OPEN TARGET, not asserted: an average message under 5 presses.\n"
           "   Measured %.2f. Four sessions of radio work moved the range from\n"
           "   650 m to 1561 m and did not move this number by one press, and it\n"
           "   is this number a person will judge the device by. The binding\n"
           "   constraint on the product is the input model, not the link.\n",
           res[M_STRUCT]);

    sim_ok(s, res[M_CTX] < res[M_UNI], "context ranking beats a frequency table");
    sim_ok(s, res[M_STRUCT] < res[M_CTX],
           "scoping fillers by their role beats ranking the whole lexicon");
    sim_ok(s, res[M_STRUCT] < flat / 2.0, "the structured composer halves the cost of speaking");
    sim_ok(s, res[M_STRUCT] > floor_p,
           "the composer is measured against a floor it cannot beat, not against zero");
    sim_ok(s, worst[M_STRUCT] <= 20, "the worst message in 1200 stays inside twenty presses");

    free(m);
    sim_world_free(&w);
}
