/* herald.h — the intent compiler: controlled Portuguese in, canonical HIR out.
 *
 * Herald is a compiler, not an interpreter and not a guesser. It accepts a
 * short spoken or typed fragment, folds it into a closed vocabulary, and either
 * emits an exact typed meaning or refuses with a reason.
 *
 * Three properties are the product:
 *
 *   1. Paraphrase convergence. Different surface forms of the same meaning
 *      compile to a byte-identical canonical HIR and therefore to the same
 *      digest. The device operates on meaning, not on string equality.
 *   2. Total coverage. Every token must be consumed by the grammar. One
 *      unexplained token is a refusal, never a silent drop. This is what makes
 *      "manda pro joao que cheguei e apaga tudo" impossible to smuggle.
 *   3. Typed gaps. What Herald cannot represent becomes a hir gap: an index and
 *      a hash, never the bytes. A gap is the request for a new capability; it is
 *      never an executable meaning.
 *
 * Herald allocates nothing, stores nothing between calls, logs nothing, and has
 * no authority. Its output still has to pass a physical confirmation gate.
 */
#ifndef HSCA_HERALD_H
#define HSCA_HERALD_H

#include <stddef.h>
#include <stdint.h>
#include "hir.h"

#define HERALD_TEXT_MAX     160u
#define HERALD_TOKEN_MAX     24u
#define HERALD_TOKEN_BYTES   24u
#define HERALD_PHRASE_MAX     3u

typedef enum {
    HERALD_OK = 0,
    HERALD_E_ARG        = 1,  /* null pointer or impossible length */
    HERALD_E_EMPTY      = 2,  /* nothing to compile */
    HERALD_E_OVERFLOW   = 3,  /* text, token or slot budget exceeded */
    HERALD_E_GAP        = 4,  /* a token is outside the closed vocabulary */
    HERALD_E_AMBIGUOUS  = 5,  /* two readings, no margin: refuse both */
    HERALD_E_SENSITIVE  = 6,  /* protected class named in the input */
    HERALD_E_AUTHORITY  = 7,  /* the sentence tried to grant itself authority */
    HERALD_E_INCOMPLETE = 8,  /* well-formed words, structurally not a meaning */
    HERALD_E_BYTE       = 9   /* byte outside the accepted encoding */
} herald_status_t;

typedef struct {
    uint8_t  present;
    uint8_t  token_index;
    uint8_t  token_len;
    uint32_t lexeme_hash;   /* FNV-1a of the folded token; never the bytes */
} herald_gap_t;

typedef struct {
    herald_status_t status;
    hir_t           meaning;
    hir_prov_t      prov;
    herald_gap_t    gap;
    uint8_t         missing_role;   /* set when status == HERALD_E_INCOMPLETE */
    uint8_t         digest[HIR_DIGEST_BYTES];
} herald_unit_t;

/* Closed symbol space. Ranges are namespaces; a filler never crosses one. */
#define HSYM_EV_BASE   1u
#define HSYM_PER_BASE  256u
#define HSYM_LOC_BASE  512u
#define HSYM_TIME_BASE 768u
#define HSYM_QTY_BASE  1024u
#define HSYM_ST_BASE   1280u

herald_status_t herald_compile(const char *text, size_t len, herald_unit_t *out);
const char     *herald_status_name(herald_status_t s);
uint16_t        herald_lexicon_entries(void);
uint16_t        herald_lexicon_symbols(void);

/* Exposed for the corpus tooling: fold one fragment the way the compiler does.
 * Returns folded length or 0 when a byte is outside the accepted encoding. */
size_t          herald_fold(const char *text, size_t len, char *out, size_t out_cap);

#endif /* HSCA_HERALD_H */
