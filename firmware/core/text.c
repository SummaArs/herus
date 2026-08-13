/* text.c — natural language into one hypervector, and why that is enough.
 *
 * The wrist cannot hold a language model (16M parameters is the ceiling, and
 * that hallucinates). But it does not have to. The division of labour is:
 *
 *     NEURAL UNDERSTANDS, OFFLINE, WHILE COMPILING.
 *     A language model on a PC reads the statute and, for each fact, writes out
 *     the many ways a person might ask for it. Every one of those phrasings is
 *     encoded here and BUNDLED into a single prototype. The model's grasp of
 *     paraphrase is spent once, at compile time, and stored as 1280 bytes.
 *
 *     SYMBOL GUARANTEES, ONLINE, WHILE RECALLING.
 *     The device encodes what the person actually said with the same function
 *     and takes a nearest neighbour. An unseen phrasing shares most of its words
 *     and bigrams with the ones the model wrote, so it lands near the same
 *     prototype — and how near is a number, in sigma, that the device can refuse
 *     on. No fluency, no invention, no way to be confidently wrong.
 *
 * WHAT THE ENCODING IS
 * --------------------
 *   unigrams   every word, unbound, so word ORDER does not have to match
 *   bigrams    rho(w_i) XOR w_{i+1}, so local order still counts for something
 *
 * Bundling both is what makes it tolerant of reordering without becoming a bag
 * of words that cannot tell "o prazo do recurso" from "o recurso do prazo".
 *
 * PORTUGUESE IS NOT AN AFTERTHOUGHT HERE
 * --------------------------------------
 * "orgao" and "órgão" must encode identically or half the paraphrases a student
 * types will miss. Accents are folded, case is folded, punctuation is dropped.
 * This is done in the encoder rather than asked of the caller, because a caller
 * who forgets is a caller whose device silently stops recognising a third of its
 * own codex.
 */
#include "text.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* FNV-1a over the folded word. The id space is the codebook's, so a word and a
 * symbol id can never collide by construction: words live above HV_TEXT_BASE. */
static uint32_t word_id(const char *w, int n)
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < n; i++) { h = (h ^ (uint8_t)w[i]) * 16777619u; }
    return HV_TEXT_BASE + (h % HV_TEXT_SPAN);
}

/* Fold one UTF-8 codepoint to a lowercase ASCII letter where Portuguese has an
 * accented form of it, and to 0 for anything that is not part of a word. Returns
 * the number of INPUT bytes consumed. */
static int fold(const unsigned char *p, char *out)
{
    unsigned char c = p[0];
    if (c < 0x80) {
        if (c >= 'A' && c <= 'Z') { *out = (char)(c + 32); return 1; }
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) { *out = (char)c; return 1; }
        *out = 0; return 1;
    }
    /* Two-byte Latin-1 supplement: the whole of Portuguese lives here. */
    if ((c & 0xE0) == 0xC0 && p[1]) {
        unsigned cp = (unsigned)((c & 0x1F) << 6) | (p[1] & 0x3F);
        static const struct { unsigned lo, hi; char to; } M[] = {
            { 0xC0, 0xC5, 'a' }, { 0xE0, 0xE5, 'a' },
            { 0xC8, 0xCB, 'e' }, { 0xE8, 0xEB, 'e' },
            { 0xCC, 0xCF, 'i' }, { 0xEC, 0xEF, 'i' },
            { 0xD2, 0xD6, 'o' }, { 0xF2, 0xF6, 'o' },
            { 0xD9, 0xDC, 'u' }, { 0xF9, 0xFC, 'u' },
            { 0xC7, 0xC7, 'c' }, { 0xE7, 0xE7, 'c' },
            { 0xD1, 0xD1, 'n' }, { 0xF1, 0xF1, 'n' },
        };
        for (unsigned i = 0; i < sizeof M / sizeof *M; i++)
            if (cp >= M[i].lo && cp <= M[i].hi) { *out = M[i].to; return 2; }
        *out = 0; return 2;
    }
    *out = 0;
    return ((c & 0xF0) == 0xE0) ? 3 : ((c & 0xF8) == 0xF0) ? 4 : 1;
}

int hv_text_words(const char *text, uint32_t *ids, int max)
{
    int n = 0;
    const unsigned char *p = (const unsigned char *)text;
    char buf[HV_TEXT_WORD_MAX];
    int len = 0;
    for (;;) {
        char c = 0;
        int adv = *p ? fold(p, &c) : 0;
        if (c && len < HV_TEXT_WORD_MAX) buf[len++] = c;
        if ((!c || !*p) && len) {
            if (n < max) ids[n] = word_id(buf, len);
            n++;
            len = 0;
        }
        if (!*p) break;
        p += adv;
    }
    return n;
}

uint16_t hv_idf_of(const hv_idf_t *idf, uint32_t id)
{
    if (!idf || !idf->e) return 1;
    int lo = 0, hi = idf->n - 1;
    while (lo <= hi) {                                  /* the table is sorted */
        int m = (lo + hi) / 2;
        if (idf->e[m].id == id) return idf->e[m].w;
        if (idf->e[m].id <  id) lo = m + 1; else hi = m - 1;
    }
    return idf->dflt ? idf->dflt : 1;
}

static int idf_cmp(const void *a, const void *b)
{
    uint32_t x = ((const hv_idf_e *)a)->id, y = ((const hv_idf_e *)b)->id;
    return x < y ? -1 : x > y ? 1 : 0;
}

int hv_idf_build(hv_idf_e *out, int cap, const char *const *docs, int ndoc)
{
    int n = 0;
    for (int d = 0; d < ndoc; d++) {
        uint32_t ids[HV_TEXT_MAX_WORDS];
        int m = hv_text_words(docs[d], ids, HV_TEXT_MAX_WORDS);
        if (m > HV_TEXT_MAX_WORDS) m = HV_TEXT_MAX_WORDS;
        for (int i = 0; i < m; i++) {
            /* Document frequency, not term frequency: a word repeated inside one
             * fact is not thereby more common across the codex. */
            int seen_here = 0;
            for (int j = 0; j < i; j++) if (ids[j] == ids[i]) { seen_here = 1; break; }
            if (seen_here) continue;
            int k = -1;
            for (int j = 0; j < n; j++) if (out[j].id == ids[i]) { k = j; break; }
            if (k < 0) { if (n >= cap) continue; k = n++; out[k].id = ids[i]; out[k].w = 0; }
            out[k].w++;                                  /* df, converted below */
        }
    }
    for (int j = 0; j < n; j++) {
        double df = out[j].w < 1 ? 1 : out[j].w;
        double idf = log((double)ndoc / df) / log((double)(ndoc > 1 ? ndoc : 2));
        int wv = (int)(HV_IDF_MAX * idf + 0.5);
        out[j].w = (uint16_t)(wv < 1 ? 1 : wv > HV_IDF_MAX ? HV_IDF_MAX : wv);
    }
    qsort(out, (size_t)n, sizeof *out, idf_cmp);
    return n;
}

/* ---------------- IDENTIFIERS ARE STRUCTURE, NOT VOCABULARY (finding T2) ----
 *
 * "o que diz o artigo 37 sobre licitacao" and the same sentence with 137 differ
 * in one token out of eight. Inverse document frequency raises that token's
 * weight but cannot change the fact that it is being bundled with seven others
 * in the same undifferentiated heap — and the bench measured the result: an
 * unseen phrasing of a KNOWN fact scored 40.9 sigma while a well-formed question
 * about an article that does not exist scored 38.4. A margin of 2.5 sigma is not
 * a margin, and the device correctly refused three quarters of what it knew.
 *
 * The fix is not a bigger weight, it is a different KIND of term. A number in
 * this domain is not a word, it is an identifier — the same thing hcp.h calls a
 * filler and binds under a role. So it is bound to a role marker before being
 * accumulated, which makes it orthogonal to every word in the sentence instead
 * of merely louder than them, and it is added with the full accumulator weight
 * because an identifier that matches is close to conclusive.
 *
 * This is the same lesson the composer taught in compose.c, arriving from the
 * other direction: the structure was in the data, and treating it as text threw
 * it away. */
#define HV_TEXT_NUM_ROLE  (HV_TEXT_BASE - 1u)
#define HV_TEXT_NUM_W     (HV_IDF_MAX * 6)

static int all_digits(const char *w, int n)
{
    if (n == 0) return 0;
    for (int i = 0; i < n; i++) if (w[i] < '0' || w[i] > '9') return 0;
    return 1;
}

/* hv_text_words hands back ids, not spellings, so the digit test has to happen
 * where the spelling still exists. A second, tiny tokeniser pass is cheaper than
 * threading a parallel array through every caller. */
static void mark_numbers(const char *text, uint8_t *isnum, int max)
{
    const unsigned char *p = (const unsigned char *)text;
    char buf[HV_TEXT_WORD_MAX]; int len = 0, n = 0;
    for (;;) {
        char c = 0;
        int adv = *p ? fold(p, &c) : 0;
        if (c && len < HV_TEXT_WORD_MAX) buf[len++] = c;
        if ((!c || !*p) && len) {
            if (n < max) isnum[n] = (uint8_t)all_digits(buf, len);
            n++; len = 0;
        }
        if (!*p) break;
        p += adv;
    }
}

static void encode_one(hv_acc_t *acc, const lex_t *L, const char *text,
                       const hv_idf_t *idf)
{
    uint32_t ids[HV_TEXT_MAX_WORDS];
    uint8_t  isnum[HV_TEXT_MAX_WORDS];
    int n = hv_text_words(text, ids, HV_TEXT_MAX_WORDS);
    if (n > HV_TEXT_MAX_WORDS) n = HV_TEXT_MAX_WORDS;
    memset(isnum, 0, sizeof isnum);
    mark_numbers(text, isnum, n);

    hv_t w, prev, b, role;
    lex_code(L, &role, HV_TEXT_NUM_ROLE);

    for (int i = 0; i < n; i++) {
        if (isnum[i]) {
            /* An identifier: bound to its role, so it is orthogonal to the
             * sentence around it rather than competing inside it. */
            lex_code(L, &w, ids[i]);
            hv_bind(&b, &role, &w);
            hv_acc_add(acc, &b, HV_TEXT_NUM_W);
            prev = w;
            continue;
        }
        int wu = (int)hv_idf_of(idf, ids[i]);
        lex_code(L, &w, ids[i]);
        hv_acc_add(acc, &w, wu);
        if (i > 0) {
            /* A bigram is as informative as its rarer half, and no more. */
            int wp = (int)hv_idf_of(idf, ids[i - 1]);
            hv_rot(&b, &prev, 1);
            hv_bind(&b, &b, &w);
            hv_acc_add(acc, &b, wu < wp ? wp : wu);
        }
        prev = w;
    }
}

void hv_text_idf(hv_t *out, hv_acc_t *acc, const lex_t *L, const char *text,
                 const hv_idf_t *idf)
{
    uint32_t probe[1];
    if (hv_text_words(text, probe, 1) == 0) { memset(out, 0, sizeof *out); return; }
    hv_acc_zero(acc);
    encode_one(acc, L, text, idf);
    hv_acc_majority(out, acc, 0x5EED5EEDull);
}

void hv_text_bundle_idf(hv_t *out, hv_acc_t *acc, const lex_t *L,
                        const char *const *texts, int ntext, const hv_idf_t *idf)
{
    hv_acc_zero(acc);
    for (int t = 0; t < ntext; t++) encode_one(acc, L, texts[t], idf);
    hv_acc_majority(out, acc, 0x5EED5EEDull);
}

void hv_text(hv_t *out, hv_acc_t *acc, const lex_t *L, const char *text)
{
    uint32_t ids[HV_TEXT_MAX_WORDS];
    int n = hv_text_words(text, ids, HV_TEXT_MAX_WORDS);
    if (n > HV_TEXT_MAX_WORDS) n = HV_TEXT_MAX_WORDS;

    hv_acc_zero(acc);
    hv_t w, prev, b;
    for (int i = 0; i < n; i++) {
        lex_code(L, &w, ids[i]);
        hv_acc_add(acc, &w, 1);                     /* unigram: order-free */
        if (i > 0) {
            hv_rot(&b, &prev, 1);                  /* rho(w_{i-1}) */
            hv_bind(&b, &b, &w);                     /* ... XOR w_i  */
            hv_acc_add(acc, &b, 1);                 /* bigram: order counts */
        }
        prev = w;
    }
    /* An empty string must not read out as a valid vector: a device that
     * confidently matches silence is worse than one that refuses. */
    if (n == 0) { memset(out, 0, sizeof *out); return; }
    hv_acc_majority(out, acc, 0x5EED5EEDull);
}

void hv_text_bundle(hv_t *out, hv_acc_t *acc, const lex_t *L,
                    const char *const *texts, int ntext)
{
    /* One accumulator across every phrasing: the prototype IS the paraphrase
     * set, not an average of separately-read vectors. Reading each one first and
     * then averaging would quantise twice and throw away the evidence that makes
     * a rare wording recoverable. */
    hv_acc_zero(acc);
    hv_t w, prev, b;
    for (int t = 0; t < ntext; t++) {
        uint32_t ids[HV_TEXT_MAX_WORDS];
        int n = hv_text_words(texts[t], ids, HV_TEXT_MAX_WORDS);
        if (n > HV_TEXT_MAX_WORDS) n = HV_TEXT_MAX_WORDS;
        for (int i = 0; i < n; i++) {
            lex_code(L, &w, ids[i]);
            hv_acc_add(acc, &w, 1);
            if (i > 0) {
                hv_rot(&b, &prev, 1);
                hv_bind(&b, &b, &w);
                hv_acc_add(acc, &b, 1);
            }
            prev = w;
        }
    }
    hv_acc_majority(out, acc, 0x5EED5EEDull);
}
