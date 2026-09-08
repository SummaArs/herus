/*
 * babel.h — o significado atravessa a fronteira do idioma.
 *
 * Herald compila portugues controlado. Babel compila QUALQUER idioma do pacote
 * e, o que e novo, faz o caminho de volta: um HIR de 24 bytes volta a ser frase
 * natural em qualquer idioma do pacote. As duas direcoes juntas produzem a
 * propriedade que o HERUS sempre carregou sem cobrar:
 *
 *     eu falo portugues, o outro le japones, e o que viajou foram 24 bytes.
 *
 * Sem tradutor, sem nuvem, sem conta, sem modelo. Nao ha traducao nenhuma
 * acontecendo: o significado nunca foi texto. O texto e so a roupa que ele
 * veste na entrada e na saida, e cada lado escolhe a sua.
 *
 * Isto e uma INTERLINGUA FECHADA, e a palavra que importa e "fechada". Trinta
 * anos de interlinguas (UNL, AMR e companhia) tentaram representar todo
 * significado possivel e nunca fecharam. Babel representa 143 conceitos
 * essenciais, prova a canonicidade deles, e o que nao cabe vira lacuna tipada
 * em vez de aproximacao. O que faz funcionar e exatamente aquilo que os
 * antecessores se recusaram a fazer: recusar.
 *
 * Tres garantias, todas verificadas em host (tools/loom.py, test_babel.c):
 *
 *   1. Convergencia de parafrase E de idioma. "cheguei", "to aqui",
 *      "i arrived" e "着いた" produzem o MESMO HIR, byte a byte.
 *   2. Ida-e-volta total. Para todo significado alcancavel h e todo idioma L,
 *      babel_compile(babel_render(h, L), L) == h.
 *   3. Cobertura total da entrada. Um lexema nao explicado e recusa, nunca
 *      descarte silencioso; e duas leituras validas diferentes sao AMBIGUOUS,
 *      nunca "escolhe a primeira".
 *
 * Diferenca de metodo em relacao ao herald.c: o Herald casa da esquerda para a
 * direita, o mais longo primeiro, e aceita a primeira leitura que fecha. Babel
 * enumera TODAS as leituras (com orcamento) e so aceita se exatamente uma
 * delas produzir significado. Onde o Herald escolheria em silencio, Babel
 * recusa. Herald continua no repositorio, congelado, e o corpus dele passa
 * identico: Babel e generalizacao, nao substituicao.
 *
 * Babel nao aloca, nao guarda estado entre chamadas, nao registra nada e nao
 * tem autoridade. A saida ainda passa por confirmacao fisica.
 */
#ifndef HERUS_BABEL_H
#define HERUS_BABEL_H

#include <stddef.h>
#include <stdint.h>
#include "hir.h"
#include "loom_core.h"

#define BABEL_TEXT_MAX     160u   /* bytes de entrada                        */
#define BABEL_RENDER_MAX   320u   /* bytes de saida, com folga para CJK      */
#define BABEL_TOKEN_MAX     24u   /* tokens num idioma com espaco            */
#define BABEL_PATH_MAX      64u   /* leituras exploradas antes de falhar fechado */
#define BABEL_DEPTH_MAX     48u   /* lexemas numa leitura                    */
#define BABEL_MEANING_MAX    4u   /* significados distintos guardados        */
#define BABEL_LANGS        LOOM_LANG_COUNT

typedef enum {
    BABEL_OK          = 0,
    BABEL_E_ARG       = 1,  /* ponteiro nulo ou comprimento impossivel       */
    BABEL_E_EMPTY     = 2,  /* nada para compilar                            */
    BABEL_E_OVERFLOW  = 3,  /* texto, leitura ou orcamento de arestas estourou */
    BABEL_E_GAP       = 4,  /* lexema fora do vocabulario fechado            */
    BABEL_E_AMBIGUOUS = 5,  /* duas leituras com significado: recusa as duas */
    BABEL_E_SENSITIVE = 6,  /* classe protegida nomeada na entrada           */
    BABEL_E_AUTHORITY = 7,  /* a frase tentou se conceder autoridade         */
    BABEL_E_INCOMPLETE= 8,  /* palavras validas que nao formam significado   */
    BABEL_E_BYTE      = 9,  /* byte fora da codificacao aceita               */
    BABEL_E_LANG      = 10  /* idioma nao existe neste pacote                */
} babel_status_t;

typedef struct {
    babel_status_t status;
    hir_t          meaning;
    hir_prov_t     prov;
    uint8_t        digest[HIR_DIGEST_BYTES];
    uint8_t        lang;
    uint8_t        gap_at;       /* byte onde a lacuna comeca                */
    uint8_t        gap_len;
    uint32_t       gap_hash;     /* FNV-1a do lexema; nunca os bytes         */
    uint8_t        missing_role; /* quando status == INCOMPLETE              */
    uint8_t        readings;      /* leituras COM significado encontradas    */
    uint8_t        paths;         /* leituras completas exploradas           */
    uint8_t        truncated;     /* o orcamento de leituras foi atingido    */
} babel_unit_t;

/* Forma canonica de superficie.
 * ASCII vira minusculo, acento latino cai para a letra base, pontuacao vira
 * espaco, e qualquer outro ponto de codigo UTF-8 valido passa INTACTO — e por
 * isso que japones e chines entram sem quebrar o contrato de bytes do Herald.
 *
 * O status e devolvido, e o comprimento sai por `out_len`, DE PROPOSITO: uma
 * entrada legitima pode dobrar para o vazio ("?" e so pontuacao) e isso nao e
 * o mesmo que byte recusado. herald_fold() devolve 0 nos dois casos e por isso
 * nao distingue os dois — aqui distingue. */
babel_status_t babel_fold(const char *text, size_t len, char *out,
                          size_t out_cap, size_t *out_len);

/* Texto -> significado tipado, ou recusa tipada. Nao ha terceiro resultado. */
babel_status_t babel_compile(const char *text, size_t len, uint8_t lang,
                             babel_unit_t *out);

/* Significado -> frase telegrafica. Telegrafico e escolha: e o registro em que
 * a frase e natural para uma pessoa E reconhecivel de volta pelo compilador. */
babel_status_t babel_render(const hir_t *h, uint8_t lang,
                            char *out, size_t out_cap, size_t *out_len);

int         babel_lang(const char *tag);     /* indice, ou -1               */
uint8_t     babel_lang_count(void);
const char *babel_lang_tag(uint8_t lang);
const char *babel_lang_endonym(uint8_t lang);
const char *babel_status_name(babel_status_t s);

#endif /* HERUS_BABEL_H */
