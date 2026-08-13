# The bench

A world for the Herus firmware to live in — one that supplies the two things
`firmware/core` and `firmware/net` do not have: **a radio and a clock**.

```bash
make                       # build and run every scenario
./build/herus-sim relay --verbose 1
./build/herus-sim range --band 1 --clutter 12
./build/herus-sim crowd --nodes 24 --per-hour 60
./build/herus-sim -h
```

The exit code is the number of failed invariants, so it works from a script.
`../prove.sh` runs it as its last suite.

---

### What is real and what is modelled

This distinction is the whole value of the thing, so it is stated before any
number is.

| | |
|---|---|
| **Real** | Every byte on air. `hcp` framing, the symmetric ratchet, ChaCha20-Poly1305, ephemeral addresses, Weave dedup and ttl, Beat slot maths, the Semtech airtime formula, the regional legality check. The bench compiles `../firmware/core` and `../firmware/net` **unmodified** — there is no second copy and no simulator-only variant. If a message is delivered here, the shipping firmware sealed it and opened it. |
| **Modelled** | Two-ray propagation with urban clutter, SX1262 sensitivity, LoRa co-channel capture, ±20 ppm crystal drift, and current draw. These are where a simulator lies, and they are listed so you know where to look. |
| **Bookkeeping** | The scoreboard tracks who sent what to whom so it can score delivery and latency. None of that is on air. A relay sees 34 opaque bytes, and scenario 2 asserts it. |

### The self-test comes first

Scenario 0 re-derives, from the constants in `sim.h`, the figures `docs/`
publishes from `tools/budget.py`. If they have drifted apart, every number below
is fiction and the run fails before printing one:

```
urban range, Tier 0 glyph, capsule   365 m   (docs: 365 m)
urban range, Tier 0 glyph, Band      650 m   (docs: 650 m)
meaning outranges speech by         33.4 %   (docs: +33%)
leaf        20 ms / 2.0 s   ->   1.78 mAh/day (weave.h: 1.78)
responsive  20 ms / 0.5 s   ->   5.60 mAh/day (weave.h: 5.60)
relay       continuous      -> 127.71 mAh/day (weave.h: 127.68)
```

The three energy figures are not fitted. They fall out of `BEAT_RX_MS`, the
period, the SX1262 receive current and the sleep baseline — and they land on the
published values, which is the strongest evidence available that the bench and
the budget describe the same device.

### The scenarios

| | asks |
|---|---|
| `selftest`  | does the world agree with `tools/budget.py` |
| `range`     | how far a meaning actually carries, walked in 50 m steps |
| `relay`     | can a message cross a gap no single hop covers — and does the relay stay blind |
| `crowd`     | what happens when a whole group is in one room |
| `attack`    | replay, forgery against a live address, and jamming |
| `day`       | 24 h of battery and latency, per energy role |
| `babel`     | one frame, three languages, no translation step |
| `drift`     | what the crystal costs after a long silence |
| `cognition` | recovering a role from a bundled hypervector |

### The link budget, once no part is treated as given

`tools/frontier.py` enumerates every (spreading factor, bandwidth) pair the
SX1262 supports, under both FCC 15.247 regimes, with the receiver's frequency
tolerance as a first-class axis — because at narrow bandwidths the crystal, not
the radio, decides what exists. It reproduces every published SX1262 sensitivity
figure to 0.01 dB before it is allowed to extrapolate.

Two results came out of it, and the second is the one that mattered.

**Modulation is at the ceiling.** Under a 400 ms dwell, with a ±2 ppm TCXO and
half the frequency-error budget held in reserve, the best point carrying ≥24 B is
SF10 / BW 125 kHz — exactly Reach. The whole remaining headroom is **1.02 dB**
(6.1% of range) and claiming it costs 3 of 4 message slots.

**The transmitter was not.** `budget.py` used `TX_DBM = 14.0` with the comment
*"ANATEL 14448 / FCC 15.247 conducted limit we target"*. The comment names the
Brazilian and American rules; **14 dBm is the European figure**. FCC 15.247(b)(1)
allows 1 W in 902–928 MHz for a hopping system, ANATEL Ato 14448 mirrors it, and
`region.c` has said `tx_dbm_max = 30` since the day it was written — the profile
knew and the budget did not. The SX1262 reaches +22 dBm on its own PA. **Eight
decibels, more range than every modulation change in this project put together,
sat in a mislabelled constant.**

The ladder, each step priced in dB and in milliamps (`herus-sim hardware`):

| step | budget | capsule | Band | gain |
|---|---|---|---|---|
| as specified: 14 dBm, bare chip, SF9 | 134.5 dB | 365 m | 650 m | — |
| + Reach (SF10, 24 B) — modulation, free | 137.0 | 422 m | 750 m | +2.5 dB |
| + the SX1262's own PA at 22 dBm — **no new part** | 145.0 | 669 m | 1189 m | **+8.0 dB** |
| + receive LNA, NF 6.03 → 1.30 dB — ~$1, 5 mA | 149.7 | 878 m | **1561 m** | +4.7 dB |
| + engineered strap antenna — *projected, bench* | 153.7 | 1240 m | 1965 m | +4.0 dB |

What it costs a leaf: transmit 0.23 → 0.61 mAh/day, receive 1.27 → 2.47 mAh/day,
**3.59 mAh/day total against 19.46 harvested** — 111 days on a 400 mAh cell.

And one consequence that decides a product SKU rather than a line of firmware: a
relay listens continuously, so the LNA costs it 247 mAh/day instead of 127. **The
part that helps sensitivity most is the one a relay can least afford.**

### Reach: SF10 at 24 bytes, and the proof that it is the ceiling

Range here was never bounded by the radio. It is bounded by the 400 ms dwell
rule, and that budget is a currency: it buys **bytes or spreading factor, never
both**. Revision 1 spent all of it on bytes — 34 B at SF9 — and left 153 ms of
dwell unspent.

```
profile   SF   frame   airtime    sensitivity   slots   urban range (capsule / Band)
Rich       9    34 B   246.8 ms   -129.5 dBm      9      365 m / 650 m
Reach     10    24 B   370.7 ms   -132.0 dBm      4      422 m / 750 m
```

Measured end to end with the real crypto running: **+14.3%** on the capsule
antenna, **+20%** on the Band, against +15.5% predicted from the link budget.

Three `_Static_assert`s in `region.h` carry the whole argument, and they are
asserts rather than prose because prose cannot fail a build:

1. 24 B at SF10 is legal.
2. **25 B at SF10 is not** — so 24 is maximal, not chosen.
3. **SF11 cannot carry one single byte** inside 400 ms — so SF10 is the last rung.

Together they say something stronger than "the range improved": there is nothing
left to take without changing the band or the law. Anyone proposing SF11 has to
delete assert 3 first, and will then see why.

The trade is stated where the gain is: +50% airtime per frame and 4 slots instead
of 9. **Reach is further. It is not cheaper and it is not richer.** The choice is
per group, never per message — a group that mixed them would be publishing which
of its messages mattered, which is what P1 exists to prevent.

The wire format did not change. A Reach frame is a Rich frame truncated: 6 fixed
bytes plus 2 per slot, one encoder, one decoder, one set of footguns.

### Two things the range work cost, and one it exposed

`max_sf` in the region profile was documented as "for a 34 B frame" — a guard
encoding an assumption rather than a rule. It silently refused every legal SF10
frame, which is the worst failure mode a guard has: it fails exactly once its
assumption stops being true, and it fails quietly. The dwell check beside it was
always the real regulation. The field is now a genuine hard ceiling and says so.

The **skipped-key cache** was 32 entries, 1664 of the 1896 bytes a session costs
— 88% of the dominant per-peer allocation, sized by round number. The bench
measured the depth actually reached across 396 sessions of a crowded, colliding
twelve-unit run: **one**. One measurement is not a bound, so the new value is not
taken from it: the only thing in the system that can reorder frames is a relay's
store-and-forward queue, which is `WEAVE_QUEUE_N` deep. Sizing the cache to
exactly that is a derivation. **1896 → 648 bytes per peer, a 66% cut**, or 39 KB
back at 32 peers.

### Listen before talk

Beat makes every node open its receiver on the same boundary — and want to
transmit on it too. The 12 ms random offset is 20× too small to avoid a 247 ms
collision, so twelve units in one room lost 9426 receptions and delivered 91%
only because flooding retried it for free.

`SetCad` costs a couple of symbol times. With it, and with the retry from S5:

| | delivered | collisions | relay TX | band | latency |
|---|---|---|---|---|---|
| neither | 91% | 9426 | 888 | 15.5% | 533 ms |
| CAD only | 94% | 422 | 713 | 13.1% | 955 ms |
| **CAD + retry** | **98%** | 1990 | 672 | **12.5%** | 1302 ms |

Delivery +7 points, collisions −79%, relay traffic −24%, band occupancy −19%.
Latency is the bill, and it is charged exactly when the band is busy — which is
when the frame would have been lost anyway.

### The stress pass: five defects, four fixes, one bug in the bench itself

`stress.c` holds scenarios written to **fail**. Each targets one place the design
was suspected of yielding and states the number that settles it. They are run by
`./build/herus-sim` along with everything else.

| | what was wrong | how it showed | what fixed it |
|---|---|---|---|
| **S1 `deaf`** | Losing more than `SESS_WINDOW` frames ended the link **permanently**. A 150 s walk through a basement, nothing hostile. | 50 frames lost against a 32-frame window; counters 90 apart; nothing ever opened again, at any distance. | `session_recover()` — a bounded, twice-gated forward walk of the receive chain. Walks forward only, advances only on an AEAD success. |
| **S3 `unmask`** | Every node that *cannot* read a frame relays it; the one that *can* stays silent. Relay behaviour named the recipient. | 8 units, 40 messages, recipient identified with **100%** accuracy by an observer holding no key. | The recipient relays too (leak L1). Identification **0%**. A link that does not want the mesh sends `ttl 0` and nobody relays at all. |
| **S2 `drain`** | A relay forwards anything from anyone — it must, that is what a mesh is. Nothing bounded the bill. | One stranger with a transmitter drove a leaf to **23.55 mAh/day**, above the 19.46 mAh/day a Band harvests. Off-grid, ended by a passer-by. | A relay governor. P1 makes it exact: every meaning frame has identical airtime, so a budget in frames *is* a budget in mAh. **6.39 mAh/day** under sustained attack. |
| **B1 `day`** | `beat_resync` adopted the arrival instant whole. But senders must add a random transmit offset or a group collides deterministically — so the reference is noisy, and adopting noise in full is a unity-gain loop that does not lock. | Two leaves, clean 200 m link, nothing hostile: **93.5%** delivery, purely from phase noise the resync was manufacturing. | Step/slew filtering plus removal of the offset's known mean — which required making the transmit jitter *protocol* (`BEAT_TX_JITTER_MS`) rather than a local choice. |
| **S5 `reliable`** | There is no retry, and the obvious one is **forbidden by design**: single-use keys mean a byte-identical retransmission has no key left to open it. Anti-replay and un-retryability are the same mechanism seen from two sides. | A lost frame was simply lost, forever. | A retry is a *new* sealed frame carrying the *same* `hcp.seq`, with duplicate suppression at the receiver reusing the already-tested `herus_replay` bitmap. **93.5% → 100%** for +0.85 mAh/day. |
| **S4 `birthday`** | Nothing. E-P2's extra address collisions really are cheap: 1 wasted AEAD per 17 foreign frames at 32 peers, 0.0004% of a core. | — | The 3× discrepancy against the birthday model was **the bench's own bug**: a pairing seed of `(base + i) ^ i` collides for small `i`, so several "distinct" sessions shared a root. Fixed in `sim_pair`; measurement now matches theory to four decimals. |

Two of these are only visible in a world. S1 needs someone to walk into a
basement; B1 needs two clocks and a reason to jitter. No unit test has either.

One is worth reading twice: **S4 was the bench lying, not the firmware**. That is
why scenario 0 checks the world against closed form before any scenario reports a
number, and why the comment recording the mistake was left in `sim_pair` rather
than quietly deleted.

### Three more things the bench found that the unit tests could not

Each is a statement about the *system*, which is why no test of the code alone
could have produced it.

**1. The rate limiter is not the binding constraint.** At SF9 a 34-byte frame
occupies the band for 246.8 ms, so a single transmitter cannot exceed ~4.1
forgery attempts per second. `SESS_RATE_TOKENS` is 20/s. Airtime is 5× tighter
than the token bucket. Sustained for a year that is 2²⁷ attempts against a 2⁻⁶⁴
tag — 2⁻³⁷ odds of a single success. The rate limiter still earns its place
against a multi-radio attacker, but the honest number here is set by physics.

**2. Flooding costs about 9× the offered load.** Twelve units in one room,
60 user messages, 522 relay transmissions, 24% band occupancy. Delivery is high
*because* of that amplification, not despite it. Weave is flooding with dedup by
design, and this is the price in one number.

**3. The Beat tolerance is asymmetric, and the resync interval has ~8× margin.**
Detection is not "was the window open when the frame started" — it is "did the
20 ms window overlap the 32.8 ms preamble". So the receiver's boundary may sit
anywhere from 20 ms *before* the sender's to 32.8 ms *after* it. At 40 µs/s of
relative drift that band is worth roughly 500–800 s of silence, against a
`BEAT_RESYNC_MS` of 60 s.

That third point began as a bug in the bench, not in the design: modelling
detection as an instant test made the relay deliver nothing at all, because the
sender was 80 microseconds early. The fix was to model what the radio does, and
the comment in `world.c` says so — a simulator that quietly corrects itself is
worth less than one that records where it was wrong.

### Evolving it

Everything is deterministic: same seed, same result, byte for byte. That is what
makes a change measurable rather than merely plausible. Sweep a parameter and
compare:

```bash
for n in 4 8 12 16 24 32; do ./build/herus-sim crowd --nodes $n --per-hour 60; done
for c in 0 12 22; do ./build/herus-sim range --clutter $c --band 1; done
```

To add a scenario: write it in `scenarios.c`, register it in the table in
`main.c`, and end it in `sim_ok()` lines. A scenario that cannot fail is
decoration — the same rule `prove.sh` runs on.

### What this does not decide

Nothing here is evidence about antennas, enclosures, bodies, or streets. The
propagation model is a model. **Phase 0 is still the gate**: print the
30×30×10 shell, strap a LilyGO T3-S3 to a wrist, and walk a measured urban
route — `docs/03-BUILD-GUIDE.md § Phase 0`. The bench exists so that everything
*above* the antenna is already settled by the time you are standing outside with
a devkit on your arm.
