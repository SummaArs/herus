/*
 * test_aether.c — invariantes do portador. O que nunca pode acontecer:
 * o Aether entregar um significado DIFERENTE do que foi enviado.
 *
 * Nao entregar nada e um resultado aceitavel: a pessoa repete. Entregar outra
 * coisa e o pior resultado possivel, porque ela age sobre uma frase que
 * ninguem disse. Todo o desenho — RS, CRC-32, ordem das barreiras — existe
 * para tornar isso improvavel e, quando improvavel nao basta, detectavel.
 */
#include "aether.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int pass_count, fail_count;

static void check(int cond, const char *what)
{
    if (cond) { pass_count++; printf("  PASS  %s\n", what); }
    else      { fail_count++; printf("  FAIL  %s\n", what); }
}

/* PRNG determinista: toda falha e reproduzivel pela semente. */
static uint32_t rng_state = 0x5eed1234u;
static uint32_t rnd(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static void fill_payload(uint8_t p[AE_PAYLOAD], uint32_t seed)
{
    unsigned i;
    rng_state = seed ? seed : 1u;
    for (i = 0; i < AE_PAYLOAD; i++) p[i] = (uint8_t)(rnd() >> 13);
}

int main(void)
{
    uint8_t payload[AE_PAYLOAD], back[AE_PAYLOAD], frame[AE_FRAME], work[AE_FRAME];
    ae_report_t rep;
    unsigned i, j;

    printf("--- aether: perfil aberto, %u bytes de quadro ---\n", (unsigned)AE_FRAME);

    /* ---- envelope --------------------------------------------------- */
    check(AE_FRAME <= 34u,
          "o quadro aberto cabe no mesmo envelope de 34 bytes do HCP Tier 1");
    check(AE_DATA + AE_PARITY == AE_FRAME, "29 de dados + 4 de paridade = 33");
    check(ae_tone_hz(0) == 1875u && ae_tone_hz(15) == 2578u,
          "os dezesseis tons ficam entre 1875 e 2578 Hz");
    {
        /* Ortogonalidade MEDIDA, nao afirmada. A propriedade nao e "a
         * frequencia e um numero inteiro de Hz" — nao e, 2578,125 Hz nao e — e
         * sim "um numero inteiro de ciclos cabe na janela", que e o que anula
         * a resposta das outras raias. A forma de saber e medir: sintetiza um
         * tom, roda os dezesseis Goertzel e olha o pior vazamento. */
        static float one[AE_SYM_LEN];
        double worst_ratio = 0.0;
        uint8_t t, k;
        for (t = 0; t < AE_TONES; t++) {
            double on = 0.0, off = 0.0;
            unsigned bin = AE_BIN_BASE + t;
            double w = 2.0 * 3.14159265358979323846 * (double)bin / (double)AE_SYM_LEN;
            for (i = 0; i < AE_SYM_LEN; i++) one[i] = (float)(0.5 * sin(w * (double)i));
            for (k = 0; k < AE_TONES; k++) {
                double m = ae_goertzel_probe(one, AE_BIN_BASE + k);
                if (k == t) on = m; else if (m > off) off = m;
            }
            if (on > 0.0 && off / on > worst_ratio) worst_ratio = off / on;
        }
        printf("        pior vazamento entre raias: %.3g\n", worst_ratio);
        check(worst_ratio < 1e-9,
              "os dezesseis tons sao ortogonais na janela: vazamento medido < 1e-9");
    }

    /* ---- ida e volta limpa ----------------------------------------- */
    fill_payload(payload, 1u);
    check(ae_pack(payload, 2u, frame) == AE_OK, "o significado empacota");
    check(ae_unpack(frame, back, &rep) == AE_OK &&
          memcmp(payload, back, AE_PAYLOAD) == 0 && rep.corrected == 0u,
          "e volta identico, sem correcao nenhuma");
    check(rep.seq == 2u && rep.profile == AE_PROFILE_OPEN,
          "a sequencia e o perfil chegam declarados");

    /* ---- UM byte errado, EXAUSTIVO: 33 posicoes x 255 valores ------ */
    {
        unsigned tried = 0, bad = 0;
        for (i = 0; i < AE_FRAME; i++) {
            for (j = 1; j < 256u; j++) {
                memcpy(work, frame, AE_FRAME);
                work[i] ^= (uint8_t)j;
                tried++;
                if (ae_unpack(work, back, &rep) != AE_OK ||
                    memcmp(payload, back, AE_PAYLOAD) != 0 || rep.corrected != 1u)
                    bad++;
            }
        }
        printf("        %u padroes de um byte errado\n", tried);
        check(bad == 0, "todo erro de UM byte, em qualquer posicao, e corrigido");
    }

    /* ---- DOIS bytes errados: todos os 528 pares, oito valores ------ */
    {
        static const uint8_t V[8][2] = {
            {0x01u,0x01u},{0xffu,0xffu},{0x80u,0x01u},{0x0fu,0xf0u},
            {0x5au,0xa5u},{0x7fu,0x80u},{0x11u,0xeeu},{0xc3u,0x3cu}
        };
        unsigned tried = 0, bad = 0, v;
        for (i = 0; i < AE_FRAME; i++) {
            for (j = i + 1u; j < AE_FRAME; j++) {
                for (v = 0; v < 8u; v++) {
                    memcpy(work, frame, AE_FRAME);
                    work[i] ^= V[v][0];
                    work[j] ^= V[v][1];
                    tried++;
                    if (ae_unpack(work, back, &rep) != AE_OK ||
                        memcmp(payload, back, AE_PAYLOAD) != 0 || rep.corrected != 2u)
                        bad++;
                }
            }
        }
        printf("        %u padroes de dois bytes errados\n", tried);
        check(bad == 0, "todo erro de DOIS bytes, em qualquer par, e corrigido");
    }

    /* ---- TRES OU MAIS bytes errados: nunca entrega errado ---------- */
    {
        unsigned tried = 0, wrong = 0, refused = 0, ok_right = 0;
        rng_state = 0xa5a5a5a5u;
        for (i = 0; i < 40000u; i++) {
            /* Posicoes DISTINTAS: se a mesma posicao fosse sorteada duas
             * vezes, "tres erros" viraria um ou dois de verdade e o numero do
             * relatorio mentiria. */
            unsigned k, nerr = 3u + (rnd() % 6u);
            uint8_t used[AE_FRAME];
            memset(used, 0, sizeof used);
            memcpy(work, frame, AE_FRAME);
            for (k = 0; k < nerr; k++) {
                unsigned pos, guard = 0;
                do { pos = rnd() % AE_FRAME; } while (used[pos] && ++guard < 64u);
                if (used[pos]) continue;
                used[pos] = 1u;
                work[pos] ^= (uint8_t)(1u + (rnd() % 255u));
            }
            tried++;
            if (ae_unpack(work, back, &rep) == AE_OK) {
                if (memcmp(payload, back, AE_PAYLOAD) == 0) ok_right++;
                else wrong++;
            } else refused++;
        }
        printf("        %u quadros com 3..8 bytes errados: %u recusados, "
               "%u aceitos e corretos, %u aceitos e ERRADOS\n",
               tried, refused, ok_right, wrong);
        check(wrong == 0,
              "com erro acima do corrigivel o Aether RECUSA, nunca entrega outro significado");
    }

    /* ---- versao e perfil sao REALMENTE olhados ---------------------- */
    {
        uint8_t data[AE_DATA], f2[AE_FRAME];
        uint32_t crc;
        /* Monta um quadro bem formado — CRC certo, paridade certa — mas com
         * versao 2. Se o decodificador nao olhasse o campo, ele entregaria um
         * significado de um protocolo que ele nao conhece. */
        data[0] = (uint8_t)((2u << 4) | (AE_PROFILE_OPEN << 2) | 1u);
        memcpy(data + AE_HDR, payload, AE_PAYLOAD);
        crc = ae_crc32(data, AE_HDR + AE_PAYLOAD);
        data[25] = (uint8_t)(crc >> 24); data[26] = (uint8_t)(crc >> 16);
        data[27] = (uint8_t)(crc >> 8);  data[28] = (uint8_t)crc;
        check(ae_pack_raw(data, f2) == AE_OK &&
              ae_unpack(f2, back, &rep) == AE_E_VERSION,
              "quadro perfeito de outra VERSAO e recusado, nao interpretado");

        data[0] = (uint8_t)((AE_VERSION << 4) | (2u << 2) | 1u);
        crc = ae_crc32(data, AE_HDR + AE_PAYLOAD);
        data[25] = (uint8_t)(crc >> 24); data[26] = (uint8_t)(crc >> 16);
        data[27] = (uint8_t)(crc >> 8);  data[28] = (uint8_t)crc;
        check(ae_pack_raw(data, f2) == AE_OK &&
              ae_unpack(f2, back, &rep) == AE_E_PROFILE,
              "quadro perfeito de outro PERFIL e recusado, nao interpretado");
    }

    /* ---- som: ida e volta em canal limpo --------------------------- */
    {
        static float buf[AE_SAMPLES + 4096u];
        size_t written = 0;
        uint8_t got[AE_FRAME];
        check(ae_sound_encode(frame, buf, sizeof buf / sizeof buf[0], &written) == AE_OK
              && written == AE_SAMPLES,
              "o quadro vira 1,493 s de som a 48 kHz");
        check(ae_sound_decode(buf, written, got, &rep) == AE_OK &&
              memcmp(got, frame, AE_FRAME) == 0,
              "e o som volta a ser exatamente o mesmo quadro");
        check(rep.sync_offset == 0, "o preambulo foi encontrado na amostra zero");
    }

    /* ---- som: preambulo deslocado e silencio antes ----------------- */
    {
        static float buf[AE_SAMPLES + 8192u];
        static float shifted[AE_SAMPLES + 8192u];
        size_t written = 0, shift = 1731u, k;
        uint8_t got[AE_FRAME];
        ae_sound_encode(frame, buf, sizeof buf / sizeof buf[0], &written);
        for (k = 0; k < shift; k++) shifted[k] = 0.0f;
        memcpy(shifted + shift, buf, written * sizeof buf[0]);
        check(ae_sound_decode(shifted, shift + written, got, &rep) == AE_OK &&
              memcmp(got, frame, AE_FRAME) == 0,
              "com 36 ms de silencio na frente, o preambulo ainda e achado");
    }

    /* ---- som: sem preambulo nenhum --------------------------------- */
    {
        static float noise[AE_SAMPLES + 512u];
        size_t k;
        uint8_t got[AE_FRAME];
        rng_state = 0x1337u;
        for (k = 0; k < sizeof noise / sizeof noise[0]; k++)
            noise[k] = ((float)(rnd() % 2000u) - 1000.0f) / 1000.0f;
        {
            ae_status_t st = ae_sound_decode(noise, sizeof noise / sizeof noise[0],
                                             got, &rep);
            /* ou nao acha sincronia, ou acha e o quadro nao fecha o CRC */
            int safe = (st != AE_OK) ||
                       (ae_unpack(got, back, &rep) != AE_OK);
            check(safe, "ruido puro nunca produz um significado");
        }
    }

    /* ---- som: tom na banda SEM preambulo -> nao ouvi nada ---------- */
    {
        /* O preambulo nao e barreira de seguranca — o CRC-32 e. O que ele
         * carrega e um DIAGNOSTICO que o produto precisa: "nao ouvi nada" e
         * uma mensagem diferente de "ouvi e chegou quebrado, repita". Sem esta
         * checagem o relogio decodificaria musica e diria "corrompido" em vez
         * de "silencio". */
        static float tones[AE_SAMPLES];
        uint8_t got[AE_FRAME];
        size_t k;
        unsigned sym;
        rng_state = 0x0f00du;
        for (sym = 0; sym < AE_TOTAL_SYMS; sym++) {
            unsigned bin = AE_BIN_BASE + (rnd() % AE_TONES);
            double w = 2.0 * 3.14159265358979323846 * (double)bin / (double)AE_SYM_LEN;
            for (k = 0; k < AE_SYM_LEN; k++)
                tones[sym * AE_SYM_LEN + k] = (float)(0.5 * sin(w * (double)k));
        }
        check(ae_sound_decode(tones, AE_SAMPLES, got, &rep) == AE_E_NO_SYNC,
              "tons na banda sem preambulo dao 'nao ouvi nada', nao lixo decodificado");
    }

    /* ---- glifo: as quatro rotacoes --------------------------------- */
    {
        uint8_t cells[AE_GLYPH_CELLS], turned[AE_GLYPH_CELLS], got[AE_FRAME];
        unsigned turn, r, c;
        int all = 1;
        check(ae_glyph_encode(frame, cells) == AE_OK, "o quadro vira um selo 18x18");
        check(AE_GLYPH_BITS <= AE_GLYPH_CELLS - 16u,
              "os 264 bits cabem nas 308 celulas que sobram dos quatro cantos");
        memcpy(turned, cells, sizeof cells);
        for (turn = 0; turn < 4u; turn++) {
            if (ae_glyph_decode(turned, got, &rep) != AE_OK ||
                memcmp(got, frame, AE_FRAME) != 0) all = 0;
            {
                uint8_t tmp[AE_GLYPH_CELLS];
                for (r = 0; r < AE_GLYPH_SIDE; r++)
                    for (c = 0; c < AE_GLYPH_SIDE; c++)
                        tmp[c * AE_GLYPH_SIDE + (AE_GLYPH_SIDE - 1u - r)] =
                            turned[r * AE_GLYPH_SIDE + c];
                memcpy(turned, tmp, sizeof tmp);
            }
        }
        check(all, "o selo le igual nas quatro rotacoes: o canto vazio da a orientacao");

        cells[0] = 0u;   /* estraga um canto */
        check(ae_glyph_decode(cells, got, &rep) == AE_E_GLYPH,
              "canto de orientacao invalido e recusa, nao leitura torta");
    }

    /* ---- glifo: uma celula trocada e consertada pelo RS ------------ */
    {
        uint8_t cells[AE_GLYPH_CELLS], got[AE_FRAME];
        ae_glyph_encode(frame, cells);
        /* inverte oito bits do mesmo byte do quadro = um byte errado */
        for (i = 0; i < 8u; i++) {
            unsigned bit = 8u * 5u + i, seen = 0, r, c;
            for (r = 0; r < AE_GLYPH_SIDE; r++) {
                for (c = 0; c < AE_GLYPH_SIDE; c++) {
                    unsigned idx = r * AE_GLYPH_SIDE + c;
                    if ((r < 2u && c < 2u) ||
                        (r < 2u && c >= AE_GLYPH_SIDE - 2u) ||
                        (r >= AE_GLYPH_SIDE - 2u && c < 2u) ||
                        (r >= AE_GLYPH_SIDE - 2u && c >= AE_GLYPH_SIDE - 2u)) continue;
                    if (seen == bit) { cells[idx] ^= 1u; }
                    seen++;
                }
            }
        }
        check(ae_glyph_decode(cells, got, &rep) == AE_OK &&
              ae_unpack(got, back, &rep) == AE_OK &&
              memcmp(payload, back, AE_PAYLOAD) == 0 && rep.corrected == 1u,
              "um byte do selo estragado por sujeira e corrigido pelo RS");
    }

    printf("AETHER: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
