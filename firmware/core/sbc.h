/* sbc.h — Sparse Block Codes: the second algebra.
 *
 * WHY THIS EXISTS
 * ---------------
 * The draft proposed "sparse vectors instead of dense" as an improvement, with
 * similarity = popcount(A AND B). That improvement is real but, as stated, it
 * is incompatible with XOR binding: if A and B each carry s ones out of D,
 * then A XOR B carries ~2s(1 - s/D) ones. Sparsity is not preserved, so bind
 * twice and the representation has drifted to dense. The algebra collapses.
 *
 * Sparse Block Codes fix this properly (Laiho/Frady/Rachkovskij lineage).
 * Partition D into S blocks of B slots. Exactly one slot is active per block.
 * Then:
 *      bind(a,b)[i]   = (a[i] + b[i]) mod B      <- sparsity EXACTLY preserved
 *      unbind(a,b)[i] = (a[i] - b[i]) mod B      <- exact inverse
 *      permute(a,k)[i]= a[(i - k) mod S]         <- block rotation
 *      sim(a,b)       = #{i : a[i] == b[i]} / S
 *
 * All three of Kanerva's operations survive, binding is still associative and
 * commutative, and it is still exactly invertible. What changes is the
 * geometry, and the change is large:
 *
 *      random similarity      dense 1/2        SBC 1/B  (= 1.6% at B=64)
 *      bytes per vector       D/8 = 1280       S = 128        (10x smaller)
 *      compare cost           XOR + popcount   byte compare   (no popcount!)
 *
 * That last line matters more than it looks. Neither Xtensa LX7 (ESP32-S3) nor
 * Cortex-M33 has a population-count instruction, so dense similarity costs a
 * byte-table lookup per byte. SBC similarity is a byte equality test, which
 * every core does natively and which vectorises trivially.
 *
 * THE TRADE-OFF, STATED HONESTLY
 * ------------------------------
 * SBC loses bundle capacity. Bundling K vectors, a member's index wins its
 * block roughly 1/K of the time, so the retrievable K is on the order of ten,
 * not a few hundred. Dense bundling degrades far more gracefully.
 *
 * Hence Herus runs BOTH, each where its geometry pays:
 *      SBC   -> lexicon, prototypes, relations, on-air payloads   (few operands)
 *      dense -> context and episodic accumulators                 (many operands)
 * Measured capacity for both is in test_algebra.c (T4).
 */
#ifndef HERUS_SBC_H
#define HERUS_SBC_H

#include <stdint.h>
#include "hv.h"

#ifndef SBC_S
#define SBC_S 128            /* blocks */
#endif
#ifndef SBC_B
#define SBC_B 64             /* slots per block; index fits in 6 bits */
#endif
/* Spanned space = SBC_S * SBC_B = 8192 sites, stored in SBC_S bytes. */

#ifndef SBC_SKETCH
#define SBC_SKETCH 16        /* blocks sent on air for Tier 0.5 */
#endif

typedef struct { uint8_t b[SBC_S]; } sbc_t;
typedef struct { int16_t c[SBC_S][SBC_B]; } sbc_acc_t;   /* 16 KB */

void sbc_gen(sbc_t *out, uint64_t domain, uint32_t symbol_id);

void sbc_bind  (sbc_t *out, const sbc_t *a, const sbc_t *b);
void sbc_unbind(sbc_t *out, const sbc_t *a, const sbc_t *b);
void sbc_rot   (sbc_t *out, const sbc_t *a, int k);

void sbc_acc_zero(sbc_acc_t *acc);
void sbc_acc_add (sbc_acc_t *acc, const sbc_t *v, int weight);
/* Exponential forgetting: decay all counters by alpha/256 before the next add.
 * alpha=256 is plain bundling; alpha=230 leaks ~10% per frame. Integer only —
 * the draft's per-bit Bernoulli thinning needed D random draws per frame and
 * was not reproducible; a leaky integrator is exact, cheaper, and orderable. */
void sbc_acc_decay(sbc_acc_t *acc, int alpha);
void sbc_acc_read (sbc_t *out, const sbc_acc_t *acc, uint64_t tiebreak);

int sbc_matches(const sbc_t *a, const sbc_t *b);          /* 0..SBC_S */

/* Tier 0.5 on-air sketch: the first n blocks, 6 bits each. Blocks are
 * independent, so a bit error corrupts one block and leaves the rest intact —
 * there is no avalanche, so nearest-neighbour decode still resolves. */
void sbc_sketch_make(uint8_t *out, const sbc_t *v, int n);
int  sbc_sketch_matches(const uint8_t *sk, const sbc_t *v, int n);
/* Channel model: each of the n*6 payload bits flips with probability ber. */
void sbc_sketch_flip(uint8_t *sk, int n, double ber, uint64_t *rng);

#endif /* HERUS_SBC_H */
