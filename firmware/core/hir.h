/* hir.h — HERUS Intent Representation: the canonical unit of meaning.
 *
 * A HIR value is the *only* thing that crosses the boundary between what a
 * person said and what the device may do. It is small, typed, closed and
 * canonical: two different sentences that mean the same thing produce a
 * byte-identical canonical form and therefore an identical digest.
 *
 * The HIR carries meaning. It does not carry text, audio, transcript,
 * embedding, identity, coordinates, keys or provenance. Provenance lives in a
 * separate struct precisely so that it cannot change the meaning.
 *
 * Wire form is 24 bytes and is bit-compatible with HCP Tier 1 of the parent
 * project: 2 header bytes plus nine 16-bit role/filler slots packed 5:11,
 * plus 4 reserved bytes. On air that is 2 (ephemeral address) + 24 + 8 (AEAD
 * tag) = 34 bytes, which is the frame every carrier in ladder.h must accept.
 */
#ifndef HSCA_HIR_H
#define HSCA_HIR_H

#include <stddef.h>
#include <stdint.h>

#define HIR_VERSION        1u
#define HIR_MAX_SLOTS      9u
#define HIR_WIRE_BYTES     24u
#define HIR_ONAIR_BYTES    34u
#define HIR_CANON_MAX      (6u + HIR_MAX_SLOTS * 3u)
#define HIR_DIGEST_BYTES   8u

#define HIR_ROLE_BITS      5u
#define HIR_FILLER_BITS    11u
#define HIR_ROLE_MAX       ((1u << HIR_ROLE_BITS) - 1u)
#define HIR_FILLER_MAX     ((1u << HIR_FILLER_BITS) - 1u)

/* Operations. 4 bits on the wire. */
typedef enum {
    HIR_OP_NONE       = 0,
    HIR_OP_COMUNICAR  = 1,
    HIR_OP_PERGUNTAR  = 2,
    HIR_OP_LEMBRAR    = 3,
    HIR_OP_PLANEJAR   = 4,
    HIR_OP_SOCORRO    = 5,
    HIR_OP_CONFIRMAR  = 6,
    HIR_OP_CANCELAR   = 7,
    HIR_OP_MAX        = 8
} hir_op_t;

/* Roles. 5 bits on the wire. Role 0 is reserved for "absent". */
typedef enum {
    HIR_ROLE_NONE   = 0,
    HIR_ROLE_QUEM   = 1,
    HIR_ROLE_O_QUE  = 2,
    HIR_ROLE_QUANDO = 3,
    HIR_ROLE_ONDE   = 4,
    HIR_ROLE_QUANTO = 5,
    HIR_ROLE_ESTADO = 6,
    HIR_ROLE_COUNT  = 7
} hir_role_t;

/* Filler 0 means "variable" — the slot a question asks about. */
#define HIR_FILLER_VAR 0u

typedef enum { HIR_URG_ROTINA = 0, HIR_URG_ATENCAO = 1, HIR_URG_URGENTE = 2, HIR_URG_SOCORRO = 3 } hir_urgency_t;
typedef enum { HIR_PERSIST_EFEMERO = 0, HIR_PERSIST_LEMBRAR = 1, HIR_PERSIST_PERMANENTE = 2 } hir_persist_t;

typedef struct {
    uint8_t  role;      /* hir_role_t */
    uint16_t filler;    /* closed symbol id, <= HIR_FILLER_MAX */
} hir_slot_t;

typedef struct {
    uint8_t    version;
    uint8_t    op;          /* hir_op_t */
    uint8_t    polarity;    /* 0 affirm, 1 negate */
    uint8_t    urgency;     /* hir_urgency_t */
    uint8_t    persistence; /* hir_persist_t */
    uint8_t    slot_count;
    hir_slot_t slot[HIR_MAX_SLOTS];
} hir_t;

/* Provenance is deliberately NOT part of the meaning. Changing any field here
 * must never change the canonical form or the digest. */
typedef struct {
    uint8_t op_was_inferred;   /* operation came from a structural rule */
    uint8_t token_count;       /* transient count, never the tokens */
    uint8_t paraphrase_depth;  /* how many multi-word forms were folded */
    uint8_t requires_confirmation;
} hir_prov_t;

typedef enum {
    HIR_OK = 0,
    HIR_E_ARG = 1,
    HIR_E_RANGE = 2,
    HIR_E_FULL = 3,
    HIR_E_DUPLICATE_ROLE = 4,
    HIR_E_EMPTY = 5
} hir_status_t;

void          hir_init(hir_t *h);
hir_status_t  hir_put(hir_t *h, uint8_t role, uint16_t filler);
hir_status_t  hir_validate(const hir_t *h);

/* Canonical form: slots sorted by (role, filler), duplicates already rejected.
 * Returns the number of bytes written, or 0 on failure. */
size_t        hir_canon(const hir_t *h, uint8_t out[HIR_CANON_MAX]);
hir_status_t  hir_digest(const hir_t *h, uint8_t out[HIR_DIGEST_BYTES]);
int           hir_equal(const hir_t *a, const hir_t *b);

hir_status_t  hir_encode_wire(const hir_t *h, uint8_t out[HIR_WIRE_BYTES]);
hir_status_t  hir_decode_wire(const uint8_t in[HIR_WIRE_BYTES], hir_t *h);

/* True when this meaning, if acted upon, leaves the device or mutates memory. */
int           hir_requires_confirmation(const hir_t *h);

#endif /* HSCA_HIR_H */
