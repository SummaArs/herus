/*
 * aether.h — o significado sai do aparelho pelos portadores que o mundo ja tem.
 *
 * ladder.c responde "quais canais do mundo sao largos o bastante para 34 bytes?"
 * e a resposta e "todos". Mas a tabela do Ladder declara alcance, latencia e
 * energia a partir de folha de dados; nenhuma linha dela foi MEDIDA, e
 * ldr_unmeasured_count() existe justamente para que nenhum documento esqueca
 * disso. O Aether ataca o outro lado do problema: dois degraus da escada — som
 * e luz — nao precisam de radio, nao precisam de par, nao precisam de conta e
 * nao precisam de hardware nenhum que a pessoa ja nao tenha no bolso.
 *
 * E isso muda o que o HERUS e hoje. Antes da Fase 0 fisica, duas pessoas com
 * dois telefones quaisquer podem trocar um significado do HERUS: uma toca, a
 * outra ouve. Sem internet, sem operadora, sem cadastro, sem servidor. O que
 * viaja sao 33 bytes que caber ia num piscar de tela.
 *
 * PERFIL ABERTO. Este quadro NAO e cifrado e NAO e autenticado, e isso e uma
 * escolha declarada, nao um esquecimento. O quadro de radio do HCP Tier 1 tem
 * 2 bytes de endereco efemero + 24 de significado + 8 de tag AEAD, e exige
 * sessao, chave e vinculo. O perfil aberto do Aether troca a tag AEAD por
 * correcao de erro, porque o problema do ar de uma sala nao e sigilo, e ruido:
 *
 *     radio   (Tier 1): 2 + 24 + 8 (tag AEAD)          = 34 bytes
 *     aether  (aberto): 1 + 24 + 4 (CRC-32) + 4 (RS)   = 33 bytes
 *
 * Os dois cabem no MESMO envelope de 34 bytes. Um significado que atravessa o
 * ar de uma sala como som e o mesmo significado, byte a byte, que atravessaria
 * 4 km de LoRa — muda a roupa, nao o conteudo. Quem quiser sigilo usa o perfil
 * cifrado; quem quiser tocar o HERUS hoje usa este.
 *
 * FALHAR FECHADO. O decodificador tem tres barreiras em serie e a ordem
 * importa: Reed-Solomon corrige ate DOIS bytes errados; se o padrao de erro
 * nao for corrigivel, ele RECUSA em vez de chutar; e o CRC-32 e a ultima
 * palavra. Um quadro que nao fecha o CRC nunca chega ao Babel. Entregar um
 * significado errado seria pior que nao entregar nada — a pessoa agiria sobre
 * uma frase que ninguem disse.
 */
#ifndef HERUS_AETHER_H
#define HERUS_AETHER_H

#include <stddef.h>
#include <stdint.h>

#define AE_PAYLOAD      24u   /* significado: hir_encode_wire()               */
#define AE_HDR           1u   /* versao(4) | perfil(2) | sequencia(2)         */
#define AE_CRC           4u   /* CRC-32, a ultima palavra                     */
#define AE_DATA         29u   /* AE_HDR + AE_PAYLOAD + AE_CRC                 */
#define AE_PARITY        4u   /* RS(33,29) sobre GF(256): corrige 2 bytes     */
#define AE_FRAME        33u   /* cabe no envelope de 34 bytes do HCP Tier 1   */
#define AE_MAX_FIX       2u   /* bytes corrigiveis por quadro                 */

#define AE_VERSION       1u
#define AE_PROFILE_OPEN  0u   /* sem cifra, sem autenticacao: declarado       */

/* --- som: 16-FSK alinhado a raia da DFT ---------------------------------
 * A raia e a chave do desenho: com 48 kHz e janela de 1024 amostras, o
 * espacamento entre tons e exatamente 48000/1024 = 46,875 Hz, que e uma raia
 * inteira da DFT. Tons alinhados a raia sao ORTOGONAIS na janela, entao o
 * detector nao precisa de FFT nem de filtro: dezesseis Goertzel de 1024
 * amostras bastam, e cada um cabe em memoria constante. E por isso que este
 * modem roda num relogio. */
#define AE_SR         48000u
#define AE_SYM_LEN     1024u  /* amostras por simbolo: 21,33 ms              */
#define AE_TONES         16u  /* 4 bits por simbolo                          */
#define AE_BIN_BASE      40u  /* 1875,0 Hz                                   */
#define AE_SYNC_LEN       4u  /* simbolos de preambulo                       */
#define AE_DATA_SYMS    (AE_FRAME * 2u)            /* 66 nibbles             */
#define AE_TOTAL_SYMS   (AE_SYNC_LEN + AE_DATA_SYMS)
#define AE_SAMPLES      (AE_TOTAL_SYMS * AE_SYM_LEN)  /* 71.680 = 1,493 s    */

/* --- glifo: selo optico 18x18 -------------------------------------------- */
#define AE_GLYPH_SIDE    18u
#define AE_GLYPH_CELLS   (AE_GLYPH_SIDE * AE_GLYPH_SIDE)   /* 324            */
#define AE_GLYPH_BITS    (AE_FRAME * 8u)                    /* 264           */

typedef enum {
    AE_OK            = 0,
    AE_E_ARG         = 1,
    AE_E_CRC         = 2,  /* o quadro chegou corrompido alem do corrigivel  */
    AE_E_UNCORRECTED = 3,  /* padrao de erro fora do alcance do RS           */
    AE_E_VERSION     = 4,
    AE_E_PROFILE     = 5,
    AE_E_NO_SYNC     = 6,  /* nenhum preambulo encontrado no audio           */
    AE_E_SHORT       = 7,  /* amostras ou celulas insuficientes              */
    AE_E_GLYPH       = 8   /* marcadores de orientacao invalidos             */
} ae_status_t;

typedef struct {
    uint8_t  corrected;    /* bytes que o RS consertou (0..AE_MAX_FIX)       */
    uint8_t  seq;
    uint8_t  profile;
    int32_t  sync_offset;  /* amostra onde o preambulo foi encontrado        */
    uint16_t symbol_errors;/* simbolos cuja segunda melhor raia ficou perto  */
} ae_report_t;

/* --------------------------------------------------------------- quadro --- */
ae_status_t ae_pack(const uint8_t payload[AE_PAYLOAD], uint8_t seq,
                    uint8_t out[AE_FRAME]);
ae_status_t ae_unpack(const uint8_t frame[AE_FRAME], uint8_t out[AE_PAYLOAD],
                      ae_report_t *rep);
/* Anexa a paridade RS a 29 bytes de dados quaisquer, sem montar cabecalho nem
 * CRC. Serve para construir quadros com versao ou perfil que ae_pack() nunca
 * produz — e assim provar que o decodificador REALMENTE olha esses campos. */
ae_status_t ae_pack_raw(const uint8_t data[AE_DATA], uint8_t out[AE_FRAME]);

/* ------------------------------------------------------------------ som --- */
/* Escreve AE_SAMPLES amostras em [-1,1]. `cap` em amostras. */
ae_status_t ae_sound_encode(const uint8_t frame[AE_FRAME], float *samples,
                            size_t cap, size_t *written);
/* Procura o preambulo, demodula e devolve o quadro cru (ainda sem RS/CRC). */
ae_status_t ae_sound_decode(const float *samples, size_t n,
                            uint8_t frame[AE_FRAME], ae_report_t *rep);

/* ---------------------------------------------------------------- glifo --- */
/* Uma celula por byte: 0 = claro, 1 = escuro. Ordem linha-a-linha. */
ae_status_t ae_glyph_encode(const uint8_t frame[AE_FRAME],
                            uint8_t cells[AE_GLYPH_CELLS]);
/* Aceita o selo em qualquer uma das quatro rotacoes. */
ae_status_t ae_glyph_decode(const uint8_t cells[AE_GLYPH_CELLS],
                            uint8_t frame[AE_FRAME], ae_report_t *rep);

const char *ae_status_name(ae_status_t s);
uint32_t    ae_crc32(const uint8_t *p, size_t n);
uint16_t    ae_tone_hz(uint8_t tone);   /* frequencia do tom, Hz arredondado */
/* Exposto para que a suite MEDIA a ortogonalidade das raias em vez de afirma-la. */
double      ae_goertzel_probe(const float *window, unsigned bin);

#endif /* HERUS_AETHER_H */
