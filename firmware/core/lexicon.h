/* lexicon.h — Herus symbol space, associative memory, and reasoning.
 *
 * Three distinct things live here, deliberately kept apart:
 *
 *   lex_t     The immutable codebook. Codes are generated on demand from
 *             (domain, id) and cost 0 bytes of RAM. Only the sketch table is
 *             materialised (HV_SK_BITS/8 bytes per symbol) for the coarse
 *             search pass. A 512-symbol lexicon costs 16 KB, not 640 KB.
 *
 *   proto_t   The mutable, learned prototype memory. Counter-based bundling,
 *             never in-place mutation of atom codes. Carries a runtime drift
 *             guard: an update that would pull two prototypes closer than
 *             min_sep bits is rejected, so quasi-orthogonality — the property
 *             the whole architecture rests on — is an enforced invariant and
 *             not an assumption.
 *
 *   resonator The factoriser. Given S = a^b^c with unknown a,b,c drawn from
 *             known codebooks, recover all three. This is what lets a
 *             receiver decode a composed message whose role structure it was
 *             never told, which is where forward compatibility comes from.
 */
#ifndef HERUS_LEXICON_H
#define HERUS_LEXICON_H

#include "hv.h"

/* ---------------- codebook ---------------- */
typedef struct {
    uint64_t  domain;                     /* group secret; defines the space */
    uint16_t  n;                          /* symbol count                    */
    hv_sk_t  *sk;                         /* n sketches                      */
    uint32_t  pos[HV_SK_BITS];            /* sketch bit positions            */
} lex_t;

int  lex_init(lex_t *L, uint64_t domain, uint16_t n);   /* 0 ok, -1 alloc    */
void lex_free(lex_t *L);
void lex_code(const lex_t *L, hv_t *out, uint32_t id);  /* derive on demand  */

/* Exhaustive nearest neighbour. O(n * HV_BYTES). Reference implementation. */
int  lex_search_full(const lex_t *L, const hv_t *q, int *out_dist);

/* Two-stage: sketch prefilter -> shortlist -> full compare.
 * ~20x fewer bytes touched at n=512 with no measurable recall loss (see
 * test_herus T6). shortlist=8 is the tested default. */
int  lex_search(const lex_t *L, const hv_t *q, int shortlist, int *out_dist);

/* Decode a received sketch directly, with no full code and no CRC. This is
 * the Tier-0.5 receive path: bit errors degrade the match instead of
 * discarding the frame. */
int  lex_search_sketch(const lex_t *L, const hv_sk_t *q, int *out_dist);

/* ---------------- learned prototypes ---------------- */
typedef struct {
    uint16_t  n, cap;
    int16_t  *acc;                        /* cap * HV_BITS signed counters   */
    hv_t     *proto;                      /* materialised majority           */
    uint16_t *votes;
    int       min_sep;                    /* drift guard, in bits            */
} proto_t;

int  proto_init(proto_t *P, uint16_t cap, int min_sep);
void proto_free(proto_t *P);

/* Fold one observation into class c. Returns 0 on accept, -1 if the update
 * was rolled back because it violated the separation invariant. */
int  proto_learn(proto_t *P, uint16_t c, const hv_t *sample);

/* Classify with explicit rejection. Returns -1 ("unknown") when the best
 * match is closer than `thresh_sigma` standard deviations from the random
 * baseline, i.e. when the evidence is indistinguishable from noise. */
int  proto_classify(const proto_t *P, const hv_t *q, double thresh_sigma,
                    int *out_dist);

/* Smallest pairwise distance across all prototypes. The invariant monitor. */
int  proto_min_separation(const proto_t *P);

/* ---------------- REFUSAL WITH A REASON ----------------
 *
 * proto_classify answers "this one" or "-1". For a device meant to be trusted
 * with something a person will be examined on, "-1" is not enough: the four ways
 * of not knowing call for four different things from the user, and collapsing
 * them into one silence throws away the only information the device has.
 *
 *   V1 UNKNOWN     Nothing is close enough to be evidence. Say so, log a
 *                  residue, and let the offline compiler fill the gap.
 *   V2 AMBIGUOUS   Two candidates are too close to separate. The device knows
 *                  the ANSWER SET but not the answer — so it should ask, not
 *                  guess. This is the case a language model handles worst: it
 *                  picks one and sounds certain.
 *   V3 COLLISION   Three or more are equally close. The query underdetermines
 *                  the fact; more of the question is needed, not a different
 *                  answer.
 *   V4 LOCKED      Recognised, and behind a tier the user has not unlocked.
 *
 * The thresholds are in SIGMA below the random-pair baseline, not in similarity,
 * because sigma is the only unit in which "how surprising is this match" means
 * the same thing at every codex size. sigma = sqrt(D)/2 bits.
 *
 * This is the specification the earlier RSD draft got right and never
 * implemented; the numbering is kept so the two can be compared. */
typedef enum {
    LEX_OK = 0,
    LEX_V1_UNKNOWN,
    LEX_V2_AMBIGUOUS,
    LEX_V3_COLLISION,
    LEX_V4_LOCKED,
    /* V0 was added after measuring, and the measurement is the argument for it.
     *
     * Tightening the answer threshold until a never-studied query could never be
     * answered also silenced every query that was merely IMPERFECT — a student
     * who reordered their slots or misremembered one filler got the same silence
     * as one asking about a statute they had never opened. The device knew the
     * difference and was throwing it away.
     *
     * Between the strongest score any never-studied query achieves and the
     * weakest a true match earns, there is a band. A score in it carries more
     * evidence than ANY unstudied query ever produced, and less than an answer
     * requires. The honest thing to do with it is neither to assert nor to go
     * quiet, but to ask: "voce quis dizer X?"
     *
     * Both edges of the band are measured from the codex, not chosen. */
    LEX_V0_SUGGEST,
    LEX_VERDICT_COUNT
} lex_verdict_t;

typedef struct {
    lex_verdict_t verdict;
    int    id;          /* best candidate, valid for OK and for V4 */
    int    runner_up;   /* the one that made it ambiguous, or -1 */
    double sigma;       /* how far below the random baseline the best sits */
    double margin;      /* sigma between best and runner-up */
} lex_resolution;

/* `tier_of` may be NULL (everything public). `unlocked` is the highest tier the
 * user has opened. Returns the verdict and fills *out. */
/* `answer_sigma` is the bar for asserting; `suggest_sigma` the bar for offering
 * a correction. Pass suggest_sigma <= 0 to disable V0 and get the older,
 * all-or-nothing behaviour. */
lex_verdict_t proto_resolve(const proto_t *P, const hv_t *q,
                            double answer_sigma, double suggest_sigma,
                            double margin_sigma,
                            const uint8_t *tier_of, int unlocked,
                            lex_resolution *out);

const char *lex_verdict_name(lex_verdict_t v);

/* ---------------- resonator network ---------------- */
/* Factorise S = code(f[0]) ^ code(f[1]) ^ ... ^ code(f[nf-1]), where factor i
 * is drawn from ids [base[i], base[i] + size[i]). Writes the recovered ids to
 * out[]. Returns the iteration count on success, -1 if it did not converge.
 * Soft cleanup (similarity-weighted bundling) rather than hard argmax: hard
 * cleanup gets trapped in limit cycles. */
int resonator(const lex_t *L, const hv_t *S, int nf,
              const uint32_t *base, const uint32_t *size,
              uint32_t *out, int max_iter);

#endif /* HERUS_LEXICON_H */
