#include "aether.h"
#include <math.h>
#include <string.h>

/* ============================================================== GF(256) ===
 * Polinomio primitivo 0x11d (x^8 + x^4 + x^3 + x^2 + 1), o mesmo do QR e do
 * CCSDS. As tabelas sao construidas na primeira chamada e cabem em 512 bytes.
 */
static uint8_t GF_EXP[512];
static uint8_t GF_LOG[256];
static int gf_ready;

static void gf_init(void)
{
    unsigned x = 1, i;
    if (gf_ready) return;
    for (i = 0; i < 255u; i++) {
        GF_EXP[i] = (uint8_t)x;
        GF_LOG[x] = (uint8_t)i;
        x <<= 1;
        if (x & 0x100u) x ^= 0x11du;
    }
    for (i = 255u; i < 512u; i++) GF_EXP[i] = GF_EXP[i - 255u];
    GF_LOG[0] = 0;                     /* nunca consultado: guardado por mul */
    gf_ready = 1;
}

static uint8_t gf_mul(uint8_t a, uint8_t b)
{
    if (a == 0u || b == 0u) return 0u;
    return GF_EXP[(unsigned)GF_LOG[a] + (unsigned)GF_LOG[b]];
}

static uint8_t gf_inv(uint8_t a)
{
    if (a == 0u) return 0u;            /* chamador nunca inverte zero        */
    return GF_EXP[255u - GF_LOG[a]];
}

static uint8_t gf_div(uint8_t a, uint8_t b) { return gf_mul(a, gf_inv(b)); }

/* ======================================================= Reed-Solomon =====
 * RS(33,29) sistematico: 29 bytes de dados seguidos de 4 de paridade.
 *
 * Nao ha entrelacador, de proposito. Entrelacar serve para espalhar uma rajada
 * por varias palavras-codigo; aqui ha UMA palavra-codigo e o RS corrige dois
 * bytes errados em QUALQUER posicao. Entrelacar nao acrescentaria nada e
 * acrescentaria codigo — e codigo que nao paga por si e divida.
 *
 * A decodificacao nao usa Berlekamp-Massey. Com t = 2 o sistema e pequeno o
 * bastante para ser resolvido direto: uma equacao para um erro, um sistema
 * 2x2 para dois. Sao vinte linhas em vez de duzentas, e cada passo e
 * verificavel a olho — o que importa num modulo cuja falha silenciosa
 * entregaria um significado que ninguem disse.
 */

/* g(x) = (x - a^0)(x - a^1)(x - a^2)(x - a^3), coeficientes do maior ao menor */
static void rs_generator(uint8_t g[AE_PARITY + 1u])
{
    uint8_t i, j;
    memset(g, 0, AE_PARITY + 1u);
    g[0] = 1u;
    for (i = 0; i < AE_PARITY; i++) {
        uint8_t root = GF_EXP[i];
        for (j = (uint8_t)(i + 1u); j > 0u; j--)
            g[j] = (uint8_t)(g[j - 1u] ^ gf_mul(g[j], root));
        g[0] = gf_mul(g[0], root);
    }
}

static void rs_encode(const uint8_t data[AE_DATA], uint8_t parity[AE_PARITY])
{
    uint8_t g[AE_PARITY + 1u];
    uint8_t i, j;
    gf_init();
    rs_generator(g);
    memset(parity, 0, AE_PARITY);
    for (i = 0; i < AE_DATA; i++) {
        uint8_t feedback = (uint8_t)(data[i] ^ parity[0]);
        for (j = 0; j < AE_PARITY - 1u; j++)
            parity[j] = (uint8_t)(parity[j + 1u] ^ gf_mul(feedback, g[AE_PARITY - 1u - j]));
        parity[AE_PARITY - 1u] = gf_mul(feedback, g[0]);
    }
}

/* Sindromes S_j = R(a^j). A posicao i do vetor tem peso x^(N-1-i). */
static void rs_syndromes(const uint8_t r[AE_FRAME], uint8_t s[AE_PARITY])
{
    uint8_t j;
    for (j = 0; j < AE_PARITY; j++) {
        uint8_t acc = 0u, i;
        for (i = 0; i < AE_FRAME; i++)
            acc = (uint8_t)(gf_mul(acc, GF_EXP[j]) ^ r[i]);
        s[j] = acc;
    }
}

/* Devolve quantos bytes foram consertados, ou -1 quando o padrao de erro esta
 * fora do alcance do codigo. Fora do alcance e RECUSA, nunca chute. */
static int rs_correct(uint8_t r[AE_FRAME])
{
    uint8_t s[AE_PARITY];
    uint8_t check[AE_PARITY];
    gf_init();
    rs_syndromes(r, s);
    if (!(s[0] | s[1] | s[2] | s[3])) return 0;

    /* --- um erro: X = S1/S0, magnitude = S0 -------------------------------- */
    if (s[0] != 0u) {
        uint8_t X = gf_div(s[1], s[0]);
        if (X != 0u && s[1] == gf_mul(s[0], X) &&
            s[2] == gf_mul(s[0], gf_mul(X, X)) &&
            s[3] == gf_mul(s[0], gf_mul(X, gf_mul(X, X)))) {
            unsigned loc = GF_LOG[X];              /* X = a^loc              */
            if (loc < AE_FRAME) {
                /* peso x^(N-1-i) => i = N-1-loc */
                r[AE_FRAME - 1u - loc] ^= s[0];
                rs_syndromes(r, check);
                if (!(check[0] | check[1] | check[2] | check[3])) return 1;
                r[AE_FRAME - 1u - loc] ^= s[0];    /* desfaz                 */
            }
        }
    }

    /* --- dois erros: L(x) = 1 + l1 x + l2 x^2 ------------------------------
     *   l1*S1 + l2*S0 = S2
     *   l1*S2 + l2*S1 = S3                                                  */
    {
        uint8_t det = (uint8_t)(gf_mul(s[1], s[1]) ^ gf_mul(s[0], s[2]));
        uint8_t l1, l2, X1 = 0u, X2 = 0u;
        unsigned found = 0, i;
        if (det == 0u) return -1;
        l1 = gf_div((uint8_t)(gf_mul(s[2], s[1]) ^ gf_mul(s[3], s[0])), det);
        l2 = gf_div((uint8_t)(gf_mul(s[1], s[3]) ^ gf_mul(s[2], s[2])), det);
        if (l2 == 0u) return -1;

        /* Chien: as raizes de L sao X^-1. Varre so as posicoes que existem. */
        for (i = 0; i < 255u && found < 2u; i++) {
            uint8_t x = GF_EXP[i];                 /* candidato a raiz       */
            uint8_t v = (uint8_t)(1u ^ gf_mul(l1, x) ^ gf_mul(l2, gf_mul(x, x)));
            if (v != 0u) continue;
            {
                uint8_t X = gf_inv(x);
                if (found == 0u) X1 = X; else X2 = X;
                found++;
            }
        }
        if (found != 2u || X1 == X2) return -1;
        {
            unsigned p1 = GF_LOG[X1], p2 = GF_LOG[X2];
            uint8_t d, e1, e2;
            if (p1 >= AE_FRAME || p2 >= AE_FRAME) return -1;
            /* e1 + e2 = S0 ; e1*X1 + e2*X2 = S1 */
            d = (uint8_t)(X1 ^ X2);
            if (d == 0u) return -1;
            e1 = gf_div((uint8_t)(gf_mul(s[0], X2) ^ s[1]), d);
            e2 = (uint8_t)(s[0] ^ e1);
            /* Havia aqui um `if (e1 == 0 || e2 == 0) return -1`. A campanha de
             * mutantes mostrou que ele NAO era carregador, e o argumento e
             * simples: magnitude zero significa que aquela posicao nao foi
             * corrigida, entao as sindromes nao zeram e a reverificacao abaixo
             * recusa de qualquer jeito. E o caso em que ela zeraria e um erro
             * de um byte so — que o caminho de UM erro, que roda antes, ja
             * teria resolvido. Guard removido: codigo defensivo que duplica
             * uma barreira mais forte nao e profundidade, e ruido. */
            r[AE_FRAME - 1u - p1] ^= e1;
            r[AE_FRAME - 1u - p2] ^= e2;
            rs_syndromes(r, check);
            if (!(check[0] | check[1] | check[2] | check[3])) return 2;
            r[AE_FRAME - 1u - p1] ^= e1;
            r[AE_FRAME - 1u - p2] ^= e2;
        }
    }
    return -1;
}

/* ================================================================ CRC-32 == */
uint32_t ae_crc32(const uint8_t *p, size_t n)
{
    uint32_t crc = 0xffffffffu;
    size_t i;
    uint8_t k;
    if (!p) return 0u;
    for (i = 0; i < n; i++) {
        crc ^= p[i];
        for (k = 0; k < 8u; k++)
            crc = (crc >> 1) ^ (0xedb88320u & (uint32_t)(-(int32_t)(crc & 1u)));
    }
    return ~crc;
}

/* ================================================================ quadro == */
ae_status_t ae_pack(const uint8_t payload[AE_PAYLOAD], uint8_t seq,
                    uint8_t out[AE_FRAME])
{
    uint8_t data[AE_DATA];
    uint32_t crc;
    if (!payload || !out) return AE_E_ARG;
    data[0] = (uint8_t)((AE_VERSION << 4) | ((AE_PROFILE_OPEN & 3u) << 2) | (seq & 3u));
    memcpy(data + AE_HDR, payload, AE_PAYLOAD);
    crc = ae_crc32(data, AE_HDR + AE_PAYLOAD);
    data[AE_HDR + AE_PAYLOAD + 0u] = (uint8_t)(crc >> 24);
    data[AE_HDR + AE_PAYLOAD + 1u] = (uint8_t)(crc >> 16);
    data[AE_HDR + AE_PAYLOAD + 2u] = (uint8_t)(crc >> 8);
    data[AE_HDR + AE_PAYLOAD + 3u] = (uint8_t)crc;
    memcpy(out, data, AE_DATA);
    rs_encode(data, out + AE_DATA);
    return AE_OK;
}

ae_status_t ae_pack_raw(const uint8_t data[AE_DATA], uint8_t out[AE_FRAME])
{
    if (!data || !out) return AE_E_ARG;
    memcpy(out, data, AE_DATA);
    rs_encode(data, out + AE_DATA);
    return AE_OK;
}

ae_status_t ae_unpack(const uint8_t frame[AE_FRAME], uint8_t out[AE_PAYLOAD],
                      ae_report_t *rep)
{
    uint8_t work[AE_FRAME];
    uint32_t crc, want;
    int fixed;
    if (!frame || !out) return AE_E_ARG;
    memcpy(work, frame, AE_FRAME);

    fixed = rs_correct(work);
    if (fixed < 0) return AE_E_UNCORRECTED;

    want = ((uint32_t)work[AE_HDR + AE_PAYLOAD + 0u] << 24) |
           ((uint32_t)work[AE_HDR + AE_PAYLOAD + 1u] << 16) |
           ((uint32_t)work[AE_HDR + AE_PAYLOAD + 2u] << 8) |
            (uint32_t)work[AE_HDR + AE_PAYLOAD + 3u];
    crc = ae_crc32(work, AE_HDR + AE_PAYLOAD);
    if (crc != want) return AE_E_CRC;        /* a ultima palavra             */

    if ((work[0] >> 4) != AE_VERSION) return AE_E_VERSION;
    if (((work[0] >> 2) & 3u) != AE_PROFILE_OPEN) return AE_E_PROFILE;

    memcpy(out, work + AE_HDR, AE_PAYLOAD);
    if (rep) {
        rep->corrected = (uint8_t)fixed;
        rep->seq = (uint8_t)(work[0] & 3u);
        rep->profile = (uint8_t)((work[0] >> 2) & 3u);
    }
    return AE_OK;
}

/* =================================================================== som == */
uint16_t ae_tone_hz(uint8_t tone)
{
    /* raia inteira: f = (AE_BIN_BASE + tone) * AE_SR / AE_SYM_LEN */
    return (uint16_t)(((uint32_t)(AE_BIN_BASE + (tone & 15u)) * AE_SR) / AE_SYM_LEN);
}

static const uint8_t SYNC[AE_SYNC_LEN] = { 15u, 0u, 15u, 0u };

/* Goertzel de uma raia inteira sobre uma janela de AE_SYM_LEN amostras.
 * Devolve a magnitude ao quadrado. Alinhado a raia, entao nao ha vazamento. */
static double goertzel(const float *x, unsigned bin)
{
    double w = 2.0 * 3.14159265358979323846 * (double)bin / (double)AE_SYM_LEN;
    double coeff = 2.0 * cos(w);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    unsigned i;
    for (i = 0; i < AE_SYM_LEN; i++) {
        s0 = (double)x[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}

double ae_goertzel_probe(const float *window, unsigned bin)
{
    return window ? goertzel(window, bin) : 0.0;
}

static uint8_t demod_symbol(const float *x, double *best, double *second)
{
    uint8_t t, arg = 0;
    double b = -1.0, s = -1.0;
    for (t = 0; t < AE_TONES; t++) {
        double m = goertzel(x, AE_BIN_BASE + t);
        if (m > b) { s = b; b = m; arg = t; }
        else if (m > s) { s = m; }
    }
    if (best) *best = b;
    if (second) *second = s;
    return arg;
}

ae_status_t ae_sound_encode(const uint8_t frame[AE_FRAME], float *samples,
                            size_t cap, size_t *written)
{
    unsigned sym, i;
    uint8_t stream[AE_TOTAL_SYMS];
    if (!frame || !samples) return AE_E_ARG;
    if (cap < AE_SAMPLES) return AE_E_SHORT;

    memcpy(stream, SYNC, AE_SYNC_LEN);
    for (i = 0; i < AE_FRAME; i++) {
        stream[AE_SYNC_LEN + i * 2u]      = (uint8_t)(frame[i] >> 4);
        stream[AE_SYNC_LEN + i * 2u + 1u] = (uint8_t)(frame[i] & 15u);
    }

    for (sym = 0; sym < AE_TOTAL_SYMS; sym++) {
        unsigned bin = AE_BIN_BASE + stream[sym];
        double w = 2.0 * 3.14159265358979323846 * (double)bin / (double)AE_SYM_LEN;
        for (i = 0; i < AE_SYM_LEN; i++)
            samples[sym * AE_SYM_LEN + i] = (float)(0.5 * sin(w * (double)i));
    }
    if (written) *written = AE_SAMPLES;
    return AE_OK;
}

ae_status_t ae_sound_decode(const float *samples, size_t n,
                            uint8_t frame[AE_FRAME], ae_report_t *rep)
{
    size_t limit, off, best_off = 0;
    double best_score = -1.0;
    unsigned step, i;

    if (!samples || !frame) return AE_E_ARG;
    if (n < AE_SAMPLES) return AE_E_SHORT;
    limit = n - AE_SAMPLES;

    /* Busca do preambulo em duas passadas: varredura grossa e refino local.
     * O escore e a margem media entre a raia esperada e a melhor rival, e nao
     * a energia: energia premia ruido alto, margem premia tom limpo. */
    for (step = 0; step < 2u; step++) {
        size_t from = 0, to = limit, grain = 64u;
        if (step == 1u) {
            from = (best_off > 64u) ? best_off - 64u : 0u;
            to = (best_off + 64u < limit) ? best_off + 64u : limit;
            grain = 4u;
            best_score = -1.0;
        }
        for (off = from; off <= to; off += grain) {
            double score = 0.0;
            int ok = 1;
            /* Os QUATRO simbolos de preambulo tem de casar. Tolerar um errado
             * foi tentado e MEDIDO: os quadros sem sincronia cairam de 127 para
             * 0 em 2000 ensaios, e as entregas certas subiram de 1840 para
             * 1841. Ou seja, a tolerancia nao entrega mais quadro nenhum — ela
             * so move a recusa do detector de sincronia para o CRC, e no
             * caminho destroi um diagnostico que o produto usa: "nao ouvi
             * nada" e uma mensagem diferente de "ouvi e chegou quebrado,
             * repita". Um ganho de um quadro em dois mil nao paga por isso.
             * Resultado negativo registrado de proposito: a proxima pessoa nao
             * precisa gastar a tarde de novo. */
            for (i = 0; i < AE_SYNC_LEN; i++) {
                double b, s;
                uint8_t got = demod_symbol(samples + off + (size_t)i * AE_SYM_LEN, &b, &s);
                if (got != SYNC[i]) { ok = 0; break; }
                score += (b - s) / (b + s + 1e-12);
            }
            if (ok && score > best_score) { best_score = score; best_off = off; }
        }
        if (best_score < 0.0) return AE_E_NO_SYNC;
    }

    {
        const float *data = samples + best_off + (size_t)AE_SYNC_LEN * AE_SYM_LEN;
        uint16_t weak = 0;
        for (i = 0; i < AE_FRAME; i++) {
            double b1, s1, b2, s2;
            uint8_t hi = demod_symbol(data + (size_t)(i * 2u) * AE_SYM_LEN, &b1, &s1);
            uint8_t lo = demod_symbol(data + (size_t)(i * 2u + 1u) * AE_SYM_LEN, &b2, &s2);
            frame[i] = (uint8_t)((hi << 4) | lo);
            if (b1 < 2.0 * s1) weak++;
            if (b2 < 2.0 * s2) weak++;
        }
        if (rep) {
            rep->sync_offset = (int32_t)best_off;
            rep->symbol_errors = weak;
            rep->corrected = 0;
            rep->seq = 0;
            rep->profile = AE_PROFILE_OPEN;
        }
    }
    return AE_OK;
}

/* ================================================================= glifo == */
/*
 * Selo optico 18x18. Os quatro cantos levam um bloco 2x2; TRES sao cheios e UM
 * e vazio, e essa assimetria e a orientacao: girar o selo move o canto vazio,
 * entao o decodificador descobre a rotacao sem precisar de marca de tempo nem
 * de mascara. Sobram 308 celulas e 264 bits de quadro; as 44 restantes ficam
 * claras, disponiveis para uma versao futura sem mexer no que ja existe.
 *
 * O que ESTE modulo prova e o codec: bits -> celulas -> bits, nas quatro
 * rotacoes. Achar o selo numa foto — perspectiva, iluminacao, foco — e outro
 * problema, continua fora do provado, e esta declarado como tal.
 */
static const uint8_t CORNER_R[4] = { 0u, 0u, AE_GLYPH_SIDE - 2u, AE_GLYPH_SIDE - 2u };
static const uint8_t CORNER_C[4] = { 0u, AE_GLYPH_SIDE - 2u, 0u, AE_GLYPH_SIDE - 2u };

static int is_corner(unsigned r, unsigned c)
{
    unsigned k;
    for (k = 0; k < 4u; k++) {
        if (r >= CORNER_R[k] && r < CORNER_R[k] + 2u &&
            c >= CORNER_C[k] && c < CORNER_C[k] + 2u) return 1;
    }
    return 0;
}

ae_status_t ae_glyph_encode(const uint8_t frame[AE_FRAME],
                            uint8_t cells[AE_GLYPH_CELLS])
{
    unsigned r, c, bit = 0, k;
    if (!frame || !cells) return AE_E_ARG;
    memset(cells, 0, AE_GLYPH_CELLS);
    for (k = 0; k < 4u; k++) {
        unsigned dr, dc;
        for (dr = 0; dr < 2u; dr++)
            for (dc = 0; dc < 2u; dc++)
                cells[(CORNER_R[k] + dr) * AE_GLYPH_SIDE + CORNER_C[k] + dc] =
                    (k == 3u) ? 0u : 1u;      /* canto 3 vazio = orientacao */
    }
    for (r = 0; r < AE_GLYPH_SIDE; r++) {
        for (c = 0; c < AE_GLYPH_SIDE; c++) {
            if (is_corner(r, c)) continue;
            if (bit >= AE_GLYPH_BITS) continue;
            cells[r * AE_GLYPH_SIDE + c] =
                (uint8_t)((frame[bit >> 3] >> (7u - (bit & 7u))) & 1u);
            bit++;
        }
    }
    return AE_OK;
}

static void rot90(const uint8_t in[AE_GLYPH_CELLS], uint8_t out[AE_GLYPH_CELLS])
{
    unsigned r, c;
    for (r = 0; r < AE_GLYPH_SIDE; r++)
        for (c = 0; c < AE_GLYPH_SIDE; c++)
            out[c * AE_GLYPH_SIDE + (AE_GLYPH_SIDE - 1u - r)] = in[r * AE_GLYPH_SIDE + c];
}

static int corners_ok(const uint8_t cells[AE_GLYPH_CELLS])
{
    unsigned k, dr, dc;
    for (k = 0; k < 4u; k++) {
        uint8_t want = (k == 3u) ? 0u : 1u;
        for (dr = 0; dr < 2u; dr++)
            for (dc = 0; dc < 2u; dc++)
                if (cells[(CORNER_R[k] + dr) * AE_GLYPH_SIDE + CORNER_C[k] + dc] != want)
                    return 0;
    }
    return 1;
}

ae_status_t ae_glyph_decode(const uint8_t cells[AE_GLYPH_CELLS],
                            uint8_t frame[AE_FRAME], ae_report_t *rep)
{
    uint8_t a[AE_GLYPH_CELLS], b[AE_GLYPH_CELLS];
    unsigned turn, r, c, bit;
    if (!cells || !frame) return AE_E_ARG;
    memcpy(a, cells, AE_GLYPH_CELLS);
    for (turn = 0; turn < 4u; turn++) {
        if (corners_ok(a)) break;
        rot90(a, b);
        memcpy(a, b, AE_GLYPH_CELLS);
    }
    if (turn == 4u) return AE_E_GLYPH;

    memset(frame, 0, AE_FRAME);
    bit = 0;
    for (r = 0; r < AE_GLYPH_SIDE; r++) {
        for (c = 0; c < AE_GLYPH_SIDE; c++) {
            if (is_corner(r, c)) continue;
            if (bit >= AE_GLYPH_BITS) continue;
            if (a[r * AE_GLYPH_SIDE + c])
                frame[bit >> 3] |= (uint8_t)(1u << (7u - (bit & 7u)));
            bit++;
        }
    }
    if (rep) {
        rep->sync_offset = (int32_t)turn;    /* quantos quartos de volta      */
        rep->symbol_errors = 0;
        rep->corrected = 0;
    }
    return AE_OK;
}

const char *ae_status_name(ae_status_t s)
{
    switch (s) {
    case AE_OK:            return "OK";
    case AE_E_ARG:         return "ARG";
    case AE_E_CRC:         return "CRC";
    case AE_E_UNCORRECTED: return "UNCORRECTED";
    case AE_E_VERSION:     return "VERSION";
    case AE_E_PROFILE:     return "PROFILE";
    case AE_E_NO_SYNC:     return "NO_SYNC";
    case AE_E_SHORT:       return "SHORT";
    case AE_E_GLYPH:       return "GLYPH";
    default:               return "?";
    }
}
