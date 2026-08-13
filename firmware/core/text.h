/* text.h — natural language into one hypervector. See text.c for the argument.
 *
 * Word ids live in their own range so a word and a protocol symbol can never
 * collide: the codebook derives codes from ids, and these ids are above
 * everything hcp.h allocates. */
#ifndef HERUS_TEXT_H
#define HERUS_TEXT_H

#include "lexicon.h"

#define HV_TEXT_BASE       65536u     /* above HCP_ID_MAX by a wide margin */
#define HV_TEXT_SPAN     1000000u
#define HV_TEXT_WORD_MAX      32
#define HV_TEXT_MAX_WORDS     64

/* Folded, lowercased, accent-stripped word ids. Returns the count that WOULD
 * have been written had `max` been large enough, so a caller can detect
 * truncation instead of silently encoding half a sentence. */
int  hv_text_words(const char *text, uint32_t *ids, int max);

/* ---------------- WEIGHTING BY RARITY (finding T1) ----------------
 *
 * Unweighted, this encoder cannot tell two facts apart when they differ by one
 * token and share the rest — and in a legal syllabus they almost always do:
 *
 *     "o que diz o artigo  37 sobre licitacao"
 *     "o que diz o artigo 137 sobre licitacao"
 *
 * Seven tokens identical, one different. The bench measured the consequence and
 * it was worse than useless: questions about facts that were NOT in the codex
 * scored HIGHER (54 sigma) than unseen phrasings of facts that were (24 sigma),
 * because a query full of common words matches every prototype full of common
 * words. The distributions were inverted, not merely overlapping.
 *
 * The fix is the oldest one in retrieval and the accumulator already supported
 * it: weight each token by how rare it is. A word in every fact carries no
 * information and gets weight 1; a word in one fact carries all of it and gets
 * HV_IDF_MAX. The table is computed by the offline compiler — which has all the
 * text, and which is the same pass that writes the phrasings — and shipped as a
 * few kilobytes beside the codex.
 *
 * This is a second thing "neural understands, offline" delivers: not just what
 * to say, but which words in it actually mean anything. */
#define HV_IDF_MAX 16

typedef struct { uint32_t id; uint16_t w; } hv_idf_e;
typedef struct { const hv_idf_e *e; int n; uint16_t dflt; } hv_idf_t;

/* Build the table from the corpus the compiler is about to bundle. `docs` is
 * ndoc strings; a token's weight falls with the number of documents holding it.
 * Writes at most `cap` entries and returns how many it wrote. */
int  hv_idf_build(hv_idf_e *out, int cap, const char *const *docs, int ndoc);

/* Weight for one token id; `idf` may be NULL, in which case everything is 1. */
uint16_t hv_idf_of(const hv_idf_t *idf, uint32_t id);

/* One phrasing -> one vector. `acc` is caller-owned (20 KB at D=10240) so the
 * RAM shows up in the caller's budget, as everywhere else in this tree. */
void hv_text(hv_t *out, hv_acc_t *acc, const lex_t *L, const char *text);
void hv_text_idf(hv_t *out, hv_acc_t *acc, const lex_t *L, const char *text,
                 const hv_idf_t *idf);

/* MANY phrasings -> one prototype. This is where a language model's grasp of
 * paraphrase is spent, once, at compile time, and stored as HV_BYTES. */
void hv_text_bundle(hv_t *out, hv_acc_t *acc, const lex_t *L,
                    const char *const *texts, int ntext);
void hv_text_bundle_idf(hv_t *out, hv_acc_t *acc, const lex_t *L,
                        const char *const *texts, int ntext, const hv_idf_t *idf);

#endif /* HERUS_TEXT_H */
