# 04 — Product: solving cold start, and the four inventions

Revision 0.1 · strategy and mechanism · numbers from `tools/budget.py`

---

## 1. The only problem that matters

Herus is a communications device, so its value follows Metcalfe: worth zero until
someone near you owns one. Every previous document optimised range, power and
algebra. None of them addressed the fact that **unit number one is useless.**

Quantified, this is worse than it sounds:

| Configuration | Urban reach | Area covered |
|---|---|---|
| wrist ↔ wrist | 365 m | **0.42 km²** |
| wrist ↔ anchor at 8 m | **1838 m** | **10.6 km²** |

Two people with wrist units share a 0.42 km² bubble *only while both are inside
it*. That is not a product, it is a demo.

So the first invention is not a feature. It is a second device.

---

## 2. Invention I — the Anchor

**A mains-powered LoRa anchor, mounted in a window.** ESP32 + SX1262 + a real
antenna at building height. No battery, no seal, no O-ring, no injection mould.

**+28 dB across the link. 5× the range. 25× the area.**

| Anchor height | Budget | Open field | Suburban | Dense urban |
|---|---|---|---|---|
| 3 m | 132 dB | 3558 m | 1783 m | 1003 m |
| **8 m** | **134 dB** | **6520 m** | **3268 m** | **1838 m** |
| 12 m | 138 dB | 9490 m | 4756 m | 2675 m |
| 20 m | 138 dB | 12252 m | 6141 m | 3453 m |

Why this changes the business and not just the link budget:

1. **It creates value at N=1.** One Core plus one Anchor is a *private cellular
   network over your neighbourhood*. No SIM, no subscription, no account, no
   cloud. That is a complete product you can sell to someone whose friends own
   nothing.
2. **It de-risks manufacturing.** The sealed 30×30×10 wearable is where hardware
   projects die: IPX7, mic membranes, O-rings, pogo pins, tooling. The Anchor is
   a box on a shelf. **The easy product can ship and earn while the hard product
   slips.**
3. **Its power budget is trivial.** Continuous RX is 5.3 mA on mains. Every
   constraint that shaped the wearable — duty cycling, Beat, solar harvesting,
   role switching — simply does not apply. An Anchor is always a relay.
4. **It makes the network effect deliberate rather than hoped for.** Every Anchor
   sold expands coverage for every Core in range, including strangers' Cores.
   That is a positive externality you can *sell*: "extend the bubble".

**Design note.** The Anchor is also the internet bridge. That is where the
privacy story needs discipline: an Anchor must forward **ciphertext only** and
must never hold a group seed. It is a dumb repeater with a WAN port, and it must
be architecturally incapable of reading what passes through it. If the Anchor can
read traffic, the entire product promise is a lie.

---

## 3. Invention II — Cunhagem (minting): the radio whose capacity grows with use

This is the technical invention, and it is also the emotional hook. It comes from
noticing something the architecture already made possible and nobody had used.

### The mechanism

Symbol codes are *derived* from a seed, so **creating a new symbol costs nothing**
— no storage, no transmission, no coordination. And the device already learns
prototypes on-device from a handful of examples with a separation guard
(01-ALGEBRA M6).

So: when two units repeatedly exchange the same composed record, they **mint a new
single glyph for it.**

```
Day 1   "arriving / in 10 min / at home"     = intent + 2 slots
Day 9   the fifth time it is sent, the sender proposes:
            MINT(local_id 1043, [ARRIVE, T_10MIN, HOME])
        the receiver stores and confirms
Day 10  the same meaning is now ONE glyph, id 1043
```

- Minted ids live in a reserved range of the 11-bit filler space, **per
  relationship** — 1024 minted glyphs per pair.
- Both ends keep the expansion, so a minted glyph is always renderable and always
  reversible. Desync degrades to the long form. **Idempotent, and it fails soft.**
- The proposal is an ordinary Tier 1 frame. No new tier, no new crypto, no
  protocol version bump.

### Why it is a real result, not a gimmick

Frames stay **constant at 34 bytes** — invariant P1 holds, so the privacy claim
survives. The bitrate never changes. What changes is meaning per byte:

| | atomic concepts per 34-byte frame |
|---|---|
| Day 1 (9 slots × 1 atom) | 9 |
| After minting (9 slots × up to 9 atoms) | up to **81** |
| Depth-2 minting | into the hundreds |

> **The claim: Herus is the only radio whose capacity increases the more you use
> it.** Physically the channel is fixed. Semantically it compounds, at zero
> bytes and zero airtime.

**Falsification criterion (Phase 2).** Define *semantic density* = recoverable
atomic concepts per frame. Simulate realistic message traffic and measure the
growth curve. If density does not at least triple within 500 simulated messages,
minting is a curiosity and should be cut. Measure it before believing it.

**Known risk to test honestly:** minted glyphs are prototypes, and prototypes
collide. The separation invariant already refuses updates that push the minimum
pairwise distance below the floor (measured: 33 of 40 adversarial updates
refused). Minting must run through that same guard, and a mint that would collide
must be **refused, not forced**. Otherwise the vocabulary rots as it grows.

### Why it sells

Two devices that talk a lot **develop a private language.** Forty words that
nobody else on earth can read, that you did not choose — you *earned* by using it.

That is:

- **braggable** — the phone app renders your pair's dictionary as a growing
  artifact. Screenshot-able. It is marketing generated by use, not bought.
- **structurally impossible alone** — you cannot mint with someone who has no
  device. The single best feature of the product requires you to hand someone
  else a device. **Therefore sell in pairs by default.**
- **switching cost that the user owns, not a lock-in you imposed** — the seed and
  the dictionary must be exportable. Anything else poisons a privacy brand.

---

## 4. Invention III — Seed = Language

A 64-bit domain seed generates an entire symbol universe with zero storage and
zero transmission. Therefore **a language is 16 hexadecimal characters.** You can
write it on paper, say it aloud, engrave it on a strap.

Consequences nobody has exploited:

| Property | What it enables |
|---|---|
| **Publishable** | a climbing club publishes its seed and symbol list; anyone adopts it instantly, no server, no app store |
| **Forkable** | fork a lexicon, add 30 symbols, share the fork. Vocabularies evolve like repositories |
| **A second confidentiality layer at zero cost** | even holding the session keys, traffic is uninterpretable without the codebook mapping. Independent of the ratchet, costing no bytes |
| **Revocable wholesale** | rotate the seed and an entire vocabulary dies at once. Useful when a device is lost |
| **A moat** | see below |

### The moat, and why it is specifically yours

A bespoke lexicon extracted from a client's actual jargon is worthless to a
competitor and impossible to copy from the outside. It is not software — it is
captured tacit knowledge, in a form that runs on a chip.

**And extracting tacit process knowledge from companies is already the AIRBPO
skill.** The interviews you already run to build automations are exactly the
process that produces a lexicon. The hardware becomes the delivery vehicle for
consulting you already know how to sell.

That is the difference between "a gadget I hope goes viral" and "a product I can
sell on Monday to a client I already have."

---

## 5. Packaging

| SKU | Contents | Why it exists |
|---|---|---|
| **Herus Par** | 2 Cores + 1 Anchor | the flagship. Minting needs two; the Anchor gives the bubble. Never sell a lone Core as the entry product |
| **Herus Núcleo** | 1 Core | add a person to an existing crew |
| **Herus Âncora** | 1 Anchor | "extend the bubble". Cheap, easy to build, and every unit improves the network for everyone in range |
| **Herus Turma** (B2B) | 20 Cores + 3 Anchors + **a bespoke lexicon built from the client's own vocabulary** | this is where the money is. The lexicon is the deliverable; the hardware is the medium |

---

## 6. The growth loop, honestly

Real loops only. Manufactured scarcity and bought social proof are not just
distasteful here, they are *fatal* — a privacy brand caught faking anything loses
the only asset it has.

1. **Someone buys for a single-player reason.** Reach a kid inside the bubble
   with no SIM and no account; locate a bike, a dog, a tool within 1.8 km of your
   window. Works with zero friends. This is the AirTag mechanic and it is what
   defeats cold start.
2. **Use mints vocabulary.** A visible, growing, private dictionary appears.
3. **The dictionary is braggable and requires a second person.** Gifting is the
   natural next step, and the pull is intrinsic rather than incentivised.
4. **Anchors seed communities.** Climbing clubs, moto groups, rural properties,
   neighbourhood associations. One anchor covers 10.6 km² — a single motivated
   person can light up a whole district.
5. **Strangers create value without any relationship.** Any Herus relays any SOS;
   any Anchor can report a lost-thing beacon. The network helps you even when
   nobody you know participates.

**Anti-stalking is a design requirement, not a policy.** A tracker that can be
hidden on a person is a stalking tool, and the fact that this one has 1.8 km of
range makes it worse than the products that already caused that scandal. A Core
being located must announce it — haptic and audible, non-defeatable in firmware,
and the eFuse lockdown makes it non-defeatable in practice too. Build it in Phase
1, not after the first news story.

---

## 7. Honest odds, revised

I said 5–10% for a sellable product before the Anchor existed. With single-player
value, that moves to roughly **20–25%.** Nobody can honestly say 100%.

"A fever" is a genuinely low-probability outcome for hardware and it is not
something invention can guarantee — it is mostly timing and luck acting on a
product that is already good.

**So the strategic recommendation is to stop aiming at the fever.** The B2B
lexicon business does not need one: twenty units and a bespoke vocabulary sold to
one agricultural or construction client is real revenue, it is defensible, it
uses a skill you already have, and it funds the consumer shot without depending
on it.

Aim at the client you can name. Let the fever be an upside, not the plan.
