# The Herus Algebra — Draft Dissected, Corrected, Extended

Companion to [00-HERUS-MASTER.md](00-HERUS-MASTER.md). Every claim here is
produced by `firmware/core/test_herus.c` or `test_algebra.c`.

The original draft is a strong piece of thinking. Its instincts are right in
every case. Four of its specific formulas are wrong, one of its encodings has a
defect that makes the reasoning engine answer backwards, and one proposed
improvement rests on a premise that does not hold. All are fixable, and fixing
them makes the system meaningfully better than the draft's own target.

---

## 1. The foundation, sharpened

The draft's core observation is correct: in high dimension, almost all random
vectors are almost orthogonal. Hamming distance between random binary vectors
concentrates at D/2 with σ = √D/2.

Measured at D = 10240 over 499 500 pairs:

```
mean distance   5120.0   (theory 5120.0)
sigma             50.55  (theory  50.60)
closest pair    4867 bits = 5.00 sigma from the mean
```

**Correction 1.** The draft states that P(two random vectors exceed 55%
similarity) is "less than 10⁻⁶". The true value is **2.3 × 10⁻²⁴** — seventeen
orders of magnitude smaller. Union-bounded over all pairs among 10 000 symbols:
**1.1 × 10⁻¹⁶**.

This is worth correcting precisely because it strengthens the argument. The
draft undersells its own foundation by a factor of 10¹⁷. The separation is not
"good enough"; it is so far past sufficient that the failure mode simply does
not exist.

---

## 2. The three operations, with their exact laws

### Bundling capacity has a closed form

The draft says `sim(C, A) > 50%`. True but uninformative — the useful question
is *how many* symbols fit in one hypervector. For odd K, the probability a
majority bit equals a given member's bit is

```
p(K) = 1/2 + C(K−1, (K−1)/2) / 2^K        →  ≈ 1/2 + 0.399/√K
```

Measured against that closed form:

| K | measured | closed form | margin (σ) | recoverable? |
|---|---|---|---|---|
| 3 | 0.7504 | 0.7500 | 50.4 | yes |
| 9 | 0.6365 | 0.6367 | 27.6 | yes |
| 51 | 0.5550 | 0.5561 | 11.1 | yes |
| 201 | 0.5280 | 0.5282 | 5.7 | yes |
| 401 | 0.5200 | 0.5199 | 4.0 | **no** |

Against a 512-entry lexicon at ε = 10⁻³ the union bound gives z = 4.6 and

```
K_max < 0.637 · D / z²  ≈  308 symbols per hypervector
```

**This number is what the draft is missing.** Every superposition in the system
— context buffers, episodic memory, composed messages — has a budget, and now
it is written down.

### Distributivity is exact, not approximate

The draft writes `A ⊕ (B + C) ≈ (A ⊕ B) + (A ⊕ C)`.

**Correction 2.** For odd K this is an **identity**, verified bit-exact in T4.
Majority is self-complementary, and XOR applies the same bijection to every
operand, so the two commute exactly. You may push binds through bundles freely
with zero accumulated error. Same for permutation over both.

The "≈" is only honest for *even* K, and there for a reason worth knowing:

### Ties are real and must be broken randomly

Majority over an even count is undefined. Measured: two different tie-break
seeds over the same four vectors produce results **1953 bits apart**.

Breaking ties toward zero — the obvious implementation — sparsifies the bundle
and silently destroys similarity structure. Herus breaks ties with a
deterministic PRNG bit, and `hcp_to_hv()` pads to odd arity so the case rarely
arises at all.

---

## 3. The encoding rule the draft blurs

> **Bind for structure. Bundle for similarity. Never both for the same job.**

The draft first presents a word encoding as a chain of binds:

```
Φ("gato") = ρ⁰(φ(g)) ⊕ ρ¹(φ(a)) ⊕ ρ²(φ(t)) ⊕ ρ³(φ(o))
```

This produces a vector orthogonal to everything, with no graceful degradation:
flip one input symbol and the output is uncorrelated noise. It is a perfect
*key* and a useless *similarity token*. The draft then gives the correct
n-gram-bundle form immediately afterwards, so the instinct is right — but the
two are presented as variants of one idea when they are opposites.

### Correction 3 — the transposition example is wrong

The draft claims `"gtao"` shares 2 of 3 bigrams with `"gato"`.

```
bigrams("gato") = {ga, at, to}
bigrams("gtao") = {gt, ta, ao}
intersection    = { }        ← zero
```

Measured similarity, bigram encoding:

| variant | corruption | similarity |
|---|---|---|
| `gato` | identical | +1.000 |
| `gaato` | insertion | +0.628 |
| `gate` | substitution (last) | +0.516 |
| `gata` | substitution (last) | +0.498 |
| `gxto` | substitution (middle) | +0.258 |
| **`gtao`** | **transposition** | **+0.241** |
| `cachorro` | unrelated | −0.018 |

Character n-grams are robust to substitution and insertion and are
**worst-case for transposition** — a transposition destroys every n-gram that
spans it. This is a real property of the encoding, not a detail: transposition
is one of the most common typing errors and one of the most common phoneme
errors in fast speech.

**Fix:** add an order-free channel — bundle the *unordered* character set
alongside the positional n-grams. Measured, this lifts transposition from
0.724 to 0.757 while leaving discrimination intact. Cheap, and it closes the
one hole the encoding has.

---

## 4. The defect that matters most: relations answer backwards

The draft's reasoning engine encodes a fact as

```
R = φ(capital) ⊕ φ(Brasil) ⊕ φ(Brasília)
```

and recovers the value by `R ⊕ φ(capital) ⊕ φ(Brasil) = φ(Brasília)`.

**This works. So does the reverse.** XOR is commutative and associative, so

```
R ⊕ φ(capital) ⊕ φ(Brasília) = φ(Brasil)
```

resolves *equally perfectly*. The relation has no direction. `capital(Brasil)`
and `capital(Brasília)` are the same object. Measured (T5):

```
symmetric  forwards → value 1.0000    backwards → arg 1.0000
           both resolve perfectly: the roles are indistinguishable   BUG
```

Every asymmetric relation in the knowledge base is silently bidirectional.
`parent_of`, `arrived_before`, `owes` — all of them reversible. For a device
whose whole selling point is symbolic *correctness*, this is the defect that
would have been most expensive to find late.

**Fix — break the symmetry with permutation:**

```
R = φ(capital) ⊕ ρ¹(φ(Brasil)) ⊕ ρ²(φ(Brasília))
```

Measured after the fix:

```
ordered    forwards → value 1.0000    backwards → arg 0.4910   ← chance
           forwards exact, backwards at chance: roles carry direction
```

Cost: one rotation per argument, which is a `memmove`. This is the single most
important correction in this document.

---

## 5. The rejection threshold should be derived, not chosen

The draft uses a flat θ = 60% similarity for "unknown".

At D = 10240, σ_normalised = 0.00494, so **60% is 20.2 σ from the noise floor**.
That is not conservative, it is deaf: it discards genuine matches that sit
between 52% and 60% for no gain, because nothing random ever gets close to
either bar.

Derived from a union bound over an L = 512 lexicon at ε = 10⁻³ (z = 4.62):

```
θ = 0.5 + z · σ_norm = 0.5228     (52.28%)
```

Measured: genuine K=9 members accepted 100% at both thresholds; **0 false
accepts in 2000 trials at the derived bar**. The derived threshold is strictly
better — same false-accept rate, more headroom for noisy queries.

The general form, which is what belongs in the firmware:

```
θ(D, L, ε) = 1/2 + z(L, ε) · 1/(2√D),    z = Φ⁻¹(1 − ε/L)
```

Rejection remains the property that distinguishes this from a neural
classifier. Measured out-of-domain false-alarm rate: **0.00%** over 800
queries. A softmax has no equivalent column — it always emits a distribution.

---

## 6. The seven proposed improvements, audited

### M1 — Sparse vectors · **KEEP, reimplement**

Right instinct, two errors.

*Math.* The draft writes `sim = |A∩B|/(|A|·|B|) = popcount(A AND B)/s²` and then
equates that to `popcount(A AND B)/s`. These differ. For equal-weight-s vectors
the normalised overlap is `popcount(A∧B)/s`. And `s²/d = 25` is the expected
*overlap count* (25 of 500 bits = 5%), not a similarity.

*Architecture, and this is the real problem.* **XOR does not preserve
sparsity.** If A and B each carry s ones out of D, `A ⊕ B` carries ≈ 2s(1−s/D)
ones. Bind twice and you are dense again; the algebra collapses. Sparse VSA is
not a drop-in substitution for the binding operator.

*Also:* sparsity does not make bit-packed compares faster. A sparse 10 000-bit
vector costs the same AND+popcount as a dense one. The speed claim only holds
under an index-list representation.

**Fix — Sparse Block Codes** (`firmware/core/sbc.h`). Partition D into S blocks
of B slots, exactly one active slot per block:

```
bind(a,b)[i]   = (a[i] + b[i]) mod B      ← sparsity preserved EXACTLY
unbind(a,b)[i] = (a[i] − b[i]) mod B      ← exact inverse
sim(a,b)       = #{i : a[i] == b[i]} / S  ← byte equality, no popcount
```

Measured gains over dense: **10× smaller** (128 B vs 1280 B), random similarity
**1.6% instead of 50%**, and **25× faster search** — because byte equality is
native on every core while popcount is not present on either Xtensa LX7 or
Cortex-M33.

Measured cost: bundle capacity collapses from ~308 operands to ~9–15.

**So run both.** SBC for the lexicon, prototypes and relations (few operands,
needs separation). Dense for context and episodic accumulators (many operands,
needs graceful degradation) **and for the Tier 0.5 on-air sketch.**

That last assignment is a correction, and it went against expectation. SBC looks
like the natural on-air format — it is already an index list, so a 16-block
sketch is only 12 bytes. Measured against a 512-symbol lexicon at 25% raw BER
(`test_algebra.c` T8): a 256-bit dense sketch decodes at **100.00%**, a 24-byte
SBC sketch at **81.2%**, a 12-byte one at **47.5%**. The reason is error
amplification: a 6-bit block index is destroyed by *any one* of its six bits
flipping, while dense bits fail independently. SBC's win is search speed, not
channel robustness — do not let one advantage be assumed into the other domain.

### M2 — LSH for sub-linear search · **REJECT as specified**

The formula does not survive contact. Bands of D/b = 500 bits, matched exactly:

```
P(band matches) = p^500.   At p = 0.75:  0.75^500 ≈ 10^−62.5
```

The claim of ">99% collision above 60% similarity" is off by roughly sixty
orders of magnitude. Minhash banding needs bands of *4–8 hash values*, not 500
bits.

The correct construction is **bit-sampling LSH**: r sampled bit positions per
band, b bands, `P(one band collides) = p^r`. At r = 8, b = 32: a 75%-similar
vector collides with probability 96.6%; a random one, 11.8%. That is an 8×
speedup at 96.6% recall.

**But do not build it.** At the realistic lexicon size (L = 512) exhaustive
search is 10.9 ms and the **two-stage sketch search is 0.44 ms** — measured
**100% agreement with exhaustive** even at 20% query noise, 30.4× faster,
24.6× fewer bytes touched. It is simpler than LSH, has no recall parameter to
tune, and the *same* sketch doubles as the Tier 0.5 over-the-air payload. One
mechanism, two uses. Revisit LSH above L ≈ 10⁴.

*Note:* the draft's own performance target — "K=100, D=10 000, under 0.5 ms on
an ESP32-S3" — is **not** achievable by the exhaustive scan it describes. At
4 cycles/byte for table-driven popcount that scan is ~2.1 ms, 4× over. It is
achievable with the two-stage search. The improvement the draft asked for is
the one that makes the draft's own number true.

### M3 — Temporal decay in bundling · **KEEP, simplify**

Correct and valuable. The implementation should change: per-bit Bernoulli
acceptance needs D random draws per frame and is not reproducible.

**Fix — integer leaky integrator.** Decay all counters by α/256 before each
add (`sbc_acc_decay`). α = 256 is plain bundling; α = 230 leaks ~10% per frame.
Exact, deterministic, cheaper, and orderable.

Useful tie-back: exponential decay with factor λ gives an effective window of
`N_eff = 1/(1−λ)`, so λ = 0.8 means K = 5 and §2's capacity table applies
directly.

### M4 — Resonant unbinding · **KEEP, upgrade substantially**

The pseudocode has a bug: step 3 is `q₁ = majority(q₀, v*)` — majority of
**two** vectors, which is undefined at every position where they disagree
(§2).

More importantly, the draft is describing simple cleanup, which for a single
unknown converges in one step. The interesting problem — and the one Herus
actually has — is factorising a product where *all* factors are unknown.

**Upgrade — a resonator network** (`resonator()` in `lexicon.c`). Given
`S = a ⊗ b ⊗ c`, hold all candidates superposed and alternate soft cleanup on
each factor. Measured: **91.5% solved in 2.7 mean iterations** over a 32 768-
hypothesis space.

Two implementation notes that cost time to find: soft cleanup with **signed**
weights is required — dropping the negative half degenerates to argmax and
stops converging; and a stable-but-wrong state is a limit cycle, escaped by
perturbation rather than more iterations.

### M5 — Spectral clustering · **CUT from v1**

The premise does not hold. If concept vectors are i.i.d. random, the similarity
matrix S is ≈ 0.5·J + noise and its principal eigenvector is the all-ones
vector. **There are no clusters, because random codes have no structure to
find.** Cluster structure only exists among vectors that were *learned* (bundled
from data) or *composed* — never among atoms.

Secondary problems: a K×K float matrix at K=512 is 1 MB, and the whole thing
contradicts the draft's own "zero float" claim.

Keep the idea for the *learned prototype* memory, at K ≤ 64, in fixed point,
after there is a user story for it. It has none yet.

### M6 — Online reinforcement · **KEEP, reimplement**

Right goal — a device that adapts to its owner without retraining — wrong
mechanism.

Flipping bits of an atom code toward a query **destroys the quasi-orthogonality
the whole architecture rests on**, and the draft's version has no guard. Two
concepts that co-occur with similar queries will drift together and eventually
collide. The bug is silent and unrecoverable: once codes have drifted there is
no way back.

Minor, also: `mask = diff & random_sparse_byte(rate)` selects `rate` of *all*
bits, but disagreeing bits are only 25–50% of them, so the effective rate is
2–4× off.

**Fix — never mutate atoms.** Keep the codebook immutable and maintain a
separate learned prototype per class via signed integer accumulation, then
threshold. This is stable, supports *unlearning* by decrementing, and is
standard practice.

**Then add the guard.** `proto_learn()` rejects and rolls back any update that
would pull two prototypes closer than a floor. Measured: fed 40 deliberately
mislabelled samples, it refused 33 and held separation at 3013 bits against a
3000-bit floor.

That guard is the difference between an assumption and an invariant. It is
~15 lines and it is the most important thing in the learning path.

### M7 — Hierarchical memory · **KEEP as designed**

Sound. Context (30 s ring) → episode (session, decayed) → semantic (flash).
Cascade the query; if context resolves, the deeper layers never run.

One thing to add: **an eviction policy for L3.** Unbounded semantic memory on
flash needs one, and the reinforcement counter is already a frequency estimate —
use LFU with age decay.

---

## 7. What the draft does not have, and should

| Addition | Why | Measured |
|---|---|---|
| **Derived codebook** — codes generated from `(domain, id)` on demand | Codebook costs **0 bytes of RAM**, and two units sharing a 64-bit seed generate bit-identical spaces with zero provisioning bandwidth | 40 KB total, not 500 KB |
| **Two-stage sketch search** | Makes the draft's own latency target achievable; the same sketch is the Tier 0.5 payload | 30.4×, 100% agreement |
| **Resonator network** | Decodes composed messages whose structure was never negotiated → forward compatibility | 91.5% / 2.7 iters |
| **Drift guard** | Turns quasi-orthogonality from assumption into enforced invariant | 33/40 refused |
| **Derived reject threshold** | Replaces a chosen constant with `θ(D, L, ε)` | 0 false accepts |
| **Permutation-ordered relations** | Fixes §4 | backwards → chance |
| **Odd-arity padding + PRNG tie-break** | Removes the even-majority failure mode | 1953-bit divergence avoided |

---

## 8. Corrected performance envelope

```
D = 10240 bits (1280 B dense) · SBC 128 B · sketch 256 bits

search, L=512    exhaustive 10.92 ms   two-stage 0.44 ms   (ESP32-S3 @240 MHz)
                 exhaustive 20.48 ms   two-stage 0.83 ms   (nRF54L15 @128 MHz)
energy/query     0.006 µAh (S3)        0.001 µAh (nRF54L15)
RAM              codebook 0 B · sketch table 16 KB · working set 5 KB
bundle capacity  ~308 operands (dense) · ~9–15 (SBC)
reject threshold 52.28% derived · 0.00% false alarms measured
float operations zero
```

The draft's summary claimed "< 1 ms pipeline, < 500 KB RAM, zero float." The
corrected system is **0.44 ms, 40 KB, and genuinely zero float** — better than
the target on every axis, once the improvements are implemented in forms that
actually work.

---

## 9. Reproduce

```bash
make -C firmware algebra          # T1..T11, dense core + SBC
cc -O2 -DHV_LUT_POPCOUNT firmware/core/{hv,sbc,test_algebra}.c -o ta -lm && ./ta
```
