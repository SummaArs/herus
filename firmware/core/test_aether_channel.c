/*
 * test_aether_channel.c — campanha adversarial do canal acustico.
 *
 * O invariante e um so, e e o unico que importa num portador:
 *
 *     o Aether NUNCA entrega um significado diferente do que foi enviado.
 *
 * Nao entregar e aceitavel — a pessoa repete o gesto. Entregar outra coisa
 * seria a pior falha possivel do produto inteiro, porque quem recebe agiria
 * sobre uma frase que ninguem disse. A campanha empilha degradacoes ate o
 * quadro morrer e cobra que ele morra CALADO.
 *
 * Sete degradacoes, sorteadas e combinadas, com escala em rampa:
 *   ruido branco, ganho, componente continua, recorte de saturacao,
 *   deriva de relogio (reamostragem), eco de sala, e um tom interferente
 *   dentro da propria banda.
 *
 * O QUE ISTO NAO E. Canal simulado nao e ar. Reverberacao real de sala,
 * resposta de alto-falante de telefone, controle automatico de ganho do
 * microfone, ruido de multidao e a distancia em que isso ainda funciona
 * continuam PENDENTES da Fase 0 fisica. O que esta provado aqui e o codec e a
 * barreira: dado um sinal com estas degradacoes, o quadro chega certo ou nao
 * chega. A curva de alcance no ar de uma sala e uma medicao que ninguem fez
 * ainda, e este arquivo nao a substitui.
 */
#include "aether.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define TRIALS 2000u
#define PREFIX_MAX 2048u
#define WORK (AE_SAMPLES + PREFIX_MAX + 8192u)

static uint32_t st_rng = 1u;
static uint32_t rnd(void)
{
    st_rng ^= st_rng << 13; st_rng ^= st_rng >> 17; st_rng ^= st_rng << 5;
    return st_rng;
}
static double uni(void) { return (double)(rnd() % 1000001u) / 1000000.0; }

/* Box-Muller. O intervalo ABERTO (0,1) nao e capricho: uni() pode devolver
 * exatamente 1,0, e log(1) = 0 com o epsilon somado do lado errado da a
 * sqrt(-2*log(u1)) um argumento NEGATIVO. O resultado e NaN, o NaN contamina
 * o sinal inteiro, o quadro morre, e a tabela de SNR ganha uma faixa
 * ">20 dB" que na verdade era "a conta quebrou". Foi exatamente o que
 * aconteceu na primeira campanha: 28% de falha atribuida ao modem que era
 * defeito do modelo de canal. */
static double open_uni(void) { return (double)(1u + rnd() % 999999u) / 1000000.0; }
static double gauss(void)
{
    double u1 = open_uni(), u2 = open_uni();
    return sqrt(-2.0 * log(u1)) * cos(6.283185307179586 * u2);
}

static float clean[AE_SAMPLES];
static float chan[WORK];
static float tmp[WORK];

int main(int argc, char **argv)
{
    unsigned trial, i;
    unsigned delivered = 0, refused = 0, wrong = 0, nosync = 0;
    /* faixas de SNR: <0, 0-6, 6-12, 12-20, >20 dB */
    unsigned snr_try[5] = {0,0,0,0,0}, snr_ok[5] = {0,0,0,0,0};
    /* faixas de recorte (ganho de pico / limite de saturacao): 1x, 2x, 4x, >4x */
    unsigned clip_try[4] = {0,0,0,0}, clip_ok[4] = {0,0,0,0}, clip_nosync[4] = {0,0,0,0};
    unsigned seed = 20260907u, diag = 0;
    uint8_t payload[AE_PAYLOAD], frame[AE_FRAME], got[AE_FRAME], back[AE_PAYLOAD];
    ae_report_t rep;

    if (argc > 1) seed = (unsigned)strtoul(argv[1], NULL, 10);
    printf("--- aether: canal adversarial (simulado, NAO e ar), semente %u ---\n", seed);

    for (trial = 0; trial < TRIALS; trial++) {
        double escala = (double)(trial % 100u + 1u) / 100.0;
        size_t prefix, n, k;
        double noise_amp, gain, dc, clip, drift, echo_g, tone_amp;
        unsigned echo_d, tone_bin, band;
        double sig_pow = 0.0, noi_pow = 0.0, snr_db;

        st_rng = (seed << 8) ^ (trial + 1u);

        for (i = 0; i < AE_PAYLOAD; i++) payload[i] = (uint8_t)(rnd() >> 13);
        if (ae_pack(payload, (uint8_t)(trial & 3u), frame) != AE_OK) { printf("  FAIL pack\n"); return 1; }
        if (ae_sound_encode(frame, clean, AE_SAMPLES, NULL) != AE_OK) { printf("  FAIL encode\n"); return 1; }

        prefix   = (size_t)(uni() * (double)PREFIX_MAX);
        noise_amp= 0.02 + 0.65 * escala * uni();
        gain     = 0.15 + 1.7 * uni();
        dc       = (uni() - 0.5) * 0.30 * escala;
        clip     = 0.25 + 0.9 * uni();
        drift    = 1.0 + (uni() - 0.5) * 0.004 * escala;   /* +-0,2% de relogio */
        echo_g   = 0.45 * escala * uni();
        echo_d   = 200u + (unsigned)(uni() * 3000.0);
        tone_amp = 0.35 * escala * uni();
        tone_bin = AE_BIN_BASE + (unsigned)(uni() * (double)AE_TONES);
        band     = (rnd() % 3u);

        /* --- deriva de relogio: reamostra por interpolacao linear ------- */
        n = (size_t)((double)AE_SAMPLES / drift);
        if (n > AE_SAMPLES + 4096u) n = AE_SAMPLES + 4096u;
        for (k = 0; k < n; k++) {
            double pos = (double)k * drift;
            size_t i0 = (size_t)pos;
            double f = pos - (double)i0;
            double a = (i0 < AE_SAMPLES) ? clean[i0] : 0.0;
            double b = (i0 + 1u < AE_SAMPLES) ? clean[i0 + 1u] : 0.0;
            tmp[k] = (float)(a + f * (b - a));
        }

        /* --- prefixo de silencio, ganho, eco, tom, continua, ruido ----- */
        for (k = 0; k < prefix; k++) chan[k] = 0.0f;
        for (k = 0; k < n; k++) {
            double v = (double)tmp[k] * gain;
            if (echo_g > 0.0 && k >= echo_d) v += echo_g * (double)tmp[k - echo_d] * gain;
            if (tone_amp > 0.0)
                v += tone_amp * sin(6.283185307179586 * (double)tone_bin
                                    * (double)k / (double)AE_SYM_LEN);
            v += dc;
            sig_pow += (double)tmp[k] * (double)tmp[k] * gain * gain;
            {
                double nz = noise_amp * gauss();
                noi_pow += nz * nz;
                v += nz;
            }
            if (v > clip) v = clip;
            if (v < -clip) v = -clip;
            chan[prefix + k] = (float)v;
        }
        for (k = prefix + n; k < prefix + n + 1024u && k < WORK; k++) chan[k] = 0.0f;

        /* --- limitacao de banda: um polo, passa-alta ou passa-baixa ---- */
        if (band) {
            double a = (band == 1u) ? 0.25 : 0.75, prev = 0.0;
            for (k = 0; k < prefix + n; k++) {
                double x = (double)chan[k];
                double y = a * x + (1.0 - a) * prev;
                prev = y;
                chan[k] = (float)((band == 1u) ? y : (x - y));
            }
        }

        snr_db = 10.0 * log10((sig_pow + 1e-18) / (noi_pow + 1e-18));
        if (!(snr_db == snr_db)) {          /* NaN: o modelo de canal quebrou */
            printf("  FAIL  o modelo de canal produziu NaN no ensaio %u\n", trial);
            return 1;
        }
        {
            unsigned b = (snr_db < 0.0) ? 0u : (snr_db < 6.0) ? 1u :
                         (snr_db < 12.0) ? 2u : (snr_db < 20.0) ? 3u : 4u;
            /* Severidade do recorte: quantas vezes o pico do sinal passa do
             * limite de saturacao. 1x = nao recorta. */
            double over = (0.5 * gain * (1.0 + echo_g) + tone_amp) / clip;
            unsigned cb = (over <= 1.0) ? 0u : (over <= 2.0) ? 1u :
                          (over <= 4.0) ? 2u : 3u;
            snr_try[b]++;
            clip_try[cb]++;

            ae_status_t sst = ae_sound_decode(chan, prefix + n + 1024u, got, &rep);
            if (sst != AE_OK || ae_unpack(got, back, &rep) != AE_OK) {
                if (b == 4u && diag < 6u) {
                    diag++;
                    printf("  [diag SNR>20] %s snr=%.0f ruido=%.3f ganho=%.2f "
                           "recorte=%.2f dc=%.3f deriva=%.4f eco=%.2f tom=%.2f "
                           "banda=%u pref=%u\n",
                           sst != AE_OK ? "sem-sync" : "crc",
                           snr_db, noise_amp, gain, clip, dc, drift, echo_g,
                           tone_amp, band, (unsigned)prefix);
                }
            }
            if (sst != AE_OK) {
                nosync++;
                clip_nosync[cb]++;
            } else if (ae_unpack(got, back, &rep) != AE_OK) {
                refused++;
            } else if (memcmp(payload, back, AE_PAYLOAD) != 0) {
                wrong++;
                printf("  FAIL  entrega ERRADA no ensaio %u (semente %u): "
                       "snr=%.1f dB ruido=%.2f ganho=%.2f recorte=%.2f "
                       "deriva=%.4f eco=%.2f tom=%.2f banda=%u\n",
                       trial, seed, snr_db, noise_amp, gain, clip, drift,
                       echo_g, tone_amp, band);
            } else {
                delivered++;
                snr_ok[b]++;
                clip_ok[cb]++;
            }
        }
    }

    printf("\n  %u ensaios: %u entregues certos, %u recusados pelo CRC/RS, "
           "%u sem sincronia, %u ERRADOS\n",
           TRIALS, delivered, refused, nosync, wrong);
    printf("  entrega por faixa de SNR (numero MEDIDO, canal simulado):\n");
    {
        static const char *const NAMES[5] = {"    < 0 dB", "  0 a 6 dB",
                                             " 6 a 12 dB", "12 a 20 dB",
                                             "   > 20 dB"};
        for (i = 0; i < 5u; i++) {
            if (!snr_try[i]) continue;
            printf("    %s : %4u/%4u = %5.1f%%\n", NAMES[i], snr_ok[i],
                   snr_try[i], 100.0 * (double)snr_ok[i] / (double)snr_try[i]);
        }
    }
    printf("  entrega por SEVERIDADE DE RECORTE (pico/saturacao):\n");
    {
        static const char *const CN[4] = {"  sem recorte", "  ate 2x",
                                          "  2x a 4x", "  acima de 4x"};
        for (i = 0; i < 4u; i++) {
            if (!clip_try[i]) continue;
            printf("    %s : %4u/%4u = %5.1f%%  (sem sincronia: %u)\n", CN[i],
                   clip_ok[i], clip_try[i],
                   100.0 * (double)clip_ok[i] / (double)clip_try[i],
                   clip_nosync[i]);
        }
    }
    printf("\n%s\n", wrong == 0
        ? "AETHER CHANNEL: PASS o quadro chega certo ou nao chega — nunca chega errado"
        : "AETHER CHANNEL: FAIL um quadro errado foi entregue");
    printf("  lembrete: canal simulado nao e ar. Alcance, reverberacao real,\n"
           "  resposta de alto-falante e AGC de microfone seguem na Fase 0.\n");
    return wrong ? 1 : 0;
}
