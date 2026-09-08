/* loom_core.h — GERADO por tools/loom.py. NAO EDITE.
 *
 * Fonte: research/loom/core/  (todos os .hlx.json)
 * Digest da fonte: 1971b64c99487a133a5f8e79883cbe70
 *
 * Editar este arquivo a mao desliga o portao do Loom: as garantias L1-L10
 * passam a valer para um pacote que nao existe mais. Mexa na especificacao e
 * rode `python3 tools/loom.py --emit`.
 */
#ifndef HERUS_LOOM_CORE_H
#define HERUS_LOOM_CORE_H

#include <stdint.h>

#define LOOM_PACK_NAME      "core"
#define LOOM_PACK_VERSION   1
#define LOOM_PACK_DIGEST    "1971b64c99487a133a5f8e79883cbe70"
#define LOOM_LANG_COUNT     8u
#define LOOM_ENTRY_COUNT    2328u
#define LOOM_SYM_COUNT      143u
#define LOOM_OP_COUNT       8u
#define LOOM_ROLE_COUNT     7u
#define LOOM_SEG_MAX        12u
#define LOOM_MAX_FORM_BYTES 27u

/* Classes lexicais. Mesma numeracao de tools/babel_ref.py; a suite diferencial
 * falha se uma das duas mudar sozinha. */
enum {
    LOOM_STOP = 0, LOOM_OP = 1, LOOM_FILL = 2, LOOM_NEG = 3,
    LOOM_URG = 4, LOOM_QVAR = 5, LOOM_SENS = 6, LOOM_AUTH = 7,
    /* Negacao LEXICAL: a forma carrega o preenchimento E a polaridade. Em
     * japones e chines a negacao e morfologica; um marcador solto produz frase
     * que ninguem escreveria. O dado fornece a forma, o codigo segue fechado. */
    LOOM_FILLNEG = 8
};

typedef struct {
    const char *form;   /* forma dobrada, UTF-8, sem NUL interno */
    uint8_t     len;    /* bytes, nao pontos de codigo */
    uint8_t     cls;
    uint8_t     role;
    uint16_t    val;    /* simbolo, codigo de operacao ou nivel de urgencia */
} loom_entry_t;

typedef struct {
    const char *tag;
    const char *endonym;
    uint8_t     spaced;      /* 1 = fronteira por espaco, 0 = por escrita */
    uint8_t     rtl;
    uint16_t    first;       /* indice em LOOM_ENTRY */
    uint16_t    count;
    uint16_t    max_form;    /* maior forma deste idioma, em bytes */
    uint16_t    speakers_m;
} loom_lang_t;

typedef struct {
    uint16_t    sym;
    uint8_t     role;
    const char *render[LOOM_LANG_COUNT];
    const char *neg[LOOM_LANG_COUNT];   /* NULL onde o marcador basta */
} loom_sym_t;

typedef struct {
    uint8_t     n;
    const char *seg[LOOM_SEG_MAX];
} loom_tmpl_t;

extern const loom_entry_t LOOM_ENTRY[LOOM_ENTRY_COUNT];
extern const loom_lang_t  LOOM_LANG[LOOM_LANG_COUNT];
extern const loom_sym_t   LOOM_SYM[LOOM_SYM_COUNT];
extern const loom_tmpl_t  LOOM_TMPL[LOOM_LANG_COUNT][LOOM_OP_COUNT];
extern const char *const  LOOM_OP_RENDER[LOOM_LANG_COUNT][LOOM_OP_COUNT];
extern const char *const  LOOM_NEG_RENDER[LOOM_LANG_COUNT];
extern const char *const  LOOM_URG_RENDER[LOOM_LANG_COUNT][4];
extern const char *const  LOOM_QW_RENDER[LOOM_LANG_COUNT][LOOM_ROLE_COUNT];

/* Balde por primeiro byte: LOOM_BUCKET[idioma][byte] = {indice, quantas}, e as
 * entradas do balde ja vem da mais longa para a mais curta. */
typedef struct { uint16_t first; uint16_t count; } loom_bucket_t;
extern const loom_bucket_t LOOM_BUCKET[LOOM_LANG_COUNT][256];

/* Indice do idioma pela etiqueta BCP-47 curta, ou -1. */
int loom_lang_index(const char *tag);
/* Forma de renderizacao de um simbolo naquele idioma, ou NULL. */
const char *loom_sym_render(uint16_t sym, uint8_t lang);
/* Forma NEGATIVA lexical, ou NULL quando o idioma nega por marcador. */
const char *loom_sym_neg(uint16_t sym, uint8_t lang);

#endif /* HERUS_LOOM_CORE_H */
