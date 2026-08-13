/* hv.h — Herus hyperdimensional core.
 *
 * Dense binary VSA (MAP-B): bind = XOR, bundle = majority, permute = rotate.
 * Every symbol code is *derived* from (domain_seed, symbol_id), never stored,
 * never transmitted. Two Herus units sharing a 64-bit domain seed generate
 * bit-identical codebooks independently.
 *
 * Design rules enforced here:
 *   R1  Bind for structure, bundle for similarity. Never XOR a chain you
 *       later want to match approximately — XOR output is orthogonal to
 *       every input and degrades to noise under a single bit of drift.
 *   R2  Majority over an even count is undefined. Ties are broken by a
 *       deterministic PRNG bit, not toward zero (biasing toward zero
 *       sparsifies the bundle and silently destroys similarity).
 *   R3  Atom codes are immutable. Learning happens in a separate prototype
 *       memory, never by mutating the codebook (see lexicon.h).
 */
#ifndef HERUS_HV_H
#define HERUS_HV_H

#include <stdint.h>
#include <stddef.h>

/* 10240 = 160 x 64. Closest multiple of 64 above Kanerva's canonical 10000.
 * sigma of the random-pair Hamming distance = sqrt(D)/2 = 50.6 bits. */
#ifndef HV_BITS
#define HV_BITS 10240
#endif
#define HV_WORDS (HV_BITS / 64)
#define HV_BYTES (HV_BITS / 8)

/* Subsampled sketch: HV_SK_BITS pseudorandom bit positions of the full code.
 * Subsampling (not XOR-folding) is the only fold that preserves the Hamming
 * geometry: E[d_sketch] = HV_SK_BITS * (d_full / HV_BITS). XOR-folding
 * destroys it. Used for (a) the coarse pass of the two-stage search and
 * (b) the experimental Tier-0.5 over-the-air payload. One mechanism, two uses. */
#ifndef HV_SK_BITS
#define HV_SK_BITS 256
#endif
#define HV_SK_WORDS (HV_SK_BITS / 64)

typedef struct { uint64_t w[HV_WORDS]; }    hv_t;
typedef struct { uint64_t w[HV_SK_WORDS]; } hv_sk_t;

/* Integer accumulator for weighted majority. int16 holds +-32767, so up to
 * ~32k unit votes or ~2k votes at weight 16. On the MCU an int8 variant
 * (HV_BITS bytes = 10 KB) is enough for the <=127 votes we ever use. */
typedef struct { int16_t c[HV_BITS]; } hv_acc_t;

/* ---- deterministic PRNG (splitmix64) -------------------------------- */
static inline uint64_t hv_mix(uint64_t *s)
{
    uint64_t z = (*s += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

/* ---- code generation ------------------------------------------------- */
/* Deterministic atom code. Costs HV_WORDS PRNG steps (~160), which is
 * cheaper than a flash read on most MCUs — hence codes are never stored. */
void hv_gen(hv_t *out, uint64_t domain, uint32_t symbol_id);

/* ---- the three operations -------------------------------------------- */
void hv_bind(hv_t *out, const hv_t *a, const hv_t *b);   /* XOR, self-inverse */
void hv_rot (hv_t *out, const hv_t *a, int k);           /* rotate left by k  */

void hv_acc_zero(hv_acc_t *acc);
void hv_acc_add (hv_acc_t *acc, const hv_t *v, int weight);
/* n_votes = total weight added. Ties (sum == 0) are broken by tiebreak_seed. */
void hv_acc_majority(hv_t *out, const hv_acc_t *acc, uint64_t tiebreak_seed);

/* ---- measurement ------------------------------------------------------ */
int    hv_dist(const hv_t *a, const hv_t *b);            /* Hamming, 0..HV_BITS */
double hv_sim (const hv_t *a, const hv_t *b);            /* 1 - 2d/D, in [-1,1] */

/* ---- sketch ----------------------------------------------------------- */
void hv_sk_positions(uint32_t *pos, uint64_t seed);      /* HV_SK_BITS entries */
void hv_sk_make(hv_sk_t *out, const hv_t *v, const uint32_t *pos);
int  hv_sk_dist(const hv_sk_t *a, const hv_sk_t *b);

/* ---- channel model (for Tier-0.5 experiments) ------------------------- */
void hv_flip_bits(hv_t *v, double ber, uint64_t *rng);
void hv_sk_flip_bits(hv_sk_t *v, double ber, uint64_t *rng);

/* ---- popcount --------------------------------------------------------- */
/* Xtensa LX7 (ESP32-S3) and Cortex-M33 both lack a population-count
 * instruction. HV_LUT_POPCOUNT builds the byte-table path so the benchmark
 * measures what the MCU will actually execute, not what x86 gets for free. */
#ifdef HV_LUT_POPCOUNT
extern const uint8_t hv_pc8[256];
static inline int hv_pc64(uint64_t x)
{
    return hv_pc8[x & 0xff] + hv_pc8[(x >> 8) & 0xff] +
           hv_pc8[(x >> 16) & 0xff] + hv_pc8[(x >> 24) & 0xff] +
           hv_pc8[(x >> 32) & 0xff] + hv_pc8[(x >> 40) & 0xff] +
           hv_pc8[(x >> 48) & 0xff] + hv_pc8[(x >> 56) & 0xff];
}
#else
static inline int hv_pc64(uint64_t x) { return __builtin_popcountll(x); }
#endif

#endif /* HERUS_HV_H */
