/* aether_probe.c — sonda de linha do Aether, para gerar vetores de paridade.
 *
 * O pacote do navegador reimplementa RS, CRC e o modem em JavaScript. Dizer
 * "e o mesmo algoritmo" sem provar seria exatamente o tipo de alegacao que
 * este repositorio nao aceita. Esta sonda exporta o comportamento do C em
 * forma de vetores; a pagina os reproduz na carga e mostra o resultado.
 *
 *   P <payload-hex>            -> quadro de 33 bytes
 *   U <frame-hex>              -> "OK <payload-hex> <corrigidos>" ou "<STATUS>"
 *   S <frame-hex>              -> sequencia de 70 indices de tom
 *   G <frame-hex>              -> 324 celulas do selo, uma por caractere
 *   C <hex>                    -> CRC-32
 */
#include "aether.h"
#include <stdio.h>
#include <string.h>

static int get_hex(const char *s, uint8_t *out, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        unsigned v;
        if (sscanf(s + i * 2u, "%2x", &v) != 1) return 0;
        out[i] = (uint8_t)v;
    }
    return 1;
}
static void put_hex(const uint8_t *b, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) printf("%02x", b[i]);
}

int main(void)
{
    char line[512];
    while (fgets(line, sizeof line, stdin)) {
        char *nl = strchr(line, '\n');
        char op;
        const char *arg;
        if (nl) *nl = '\0';
        if (!line[0]) continue;
        op = line[0];
        arg = (line[1] == ' ') ? line + 2 : line + 1;

        if (op == 'P') {
            uint8_t pl[AE_PAYLOAD], fr[AE_FRAME];
            if (strlen(arg) < AE_PAYLOAD * 2u || !get_hex(arg, pl, AE_PAYLOAD)) { printf("ARG\n"); }
            else if (ae_pack(pl, 0u, fr) != AE_OK) printf("ARG\n");
            else { printf("OK "); put_hex(fr, AE_FRAME); printf("\n"); }
        } else if (op == 'U') {
            uint8_t fr[AE_FRAME], pl[AE_PAYLOAD];
            ae_report_t rep;
            ae_status_t st;
            if (strlen(arg) < AE_FRAME * 2u || !get_hex(arg, fr, AE_FRAME)) { printf("ARG\n"); continue; }
            st = ae_unpack(fr, pl, &rep);
            if (st != AE_OK) printf("%s\n", ae_status_name(st));
            else { printf("OK "); put_hex(pl, AE_PAYLOAD); printf(" %u\n", rep.corrected); }
        } else if (op == 'S') {
            uint8_t fr[AE_FRAME];
            unsigned i;
            if (strlen(arg) < AE_FRAME * 2u || !get_hex(arg, fr, AE_FRAME)) { printf("ARG\n"); continue; }
            printf("OK f0f0");             /* preambulo 15,0,15,0 em nibbles hex */
            for (i = 0; i < AE_FRAME; i++) printf("%x%x", fr[i] >> 4, fr[i] & 15u);
            printf("\n");
        } else if (op == 'G') {
            uint8_t fr[AE_FRAME], cells[AE_GLYPH_CELLS];
            unsigned i;
            if (strlen(arg) < AE_FRAME * 2u || !get_hex(arg, fr, AE_FRAME)) { printf("ARG\n"); continue; }
            ae_glyph_encode(fr, cells);
            printf("OK ");
            for (i = 0; i < AE_GLYPH_CELLS; i++) putchar(cells[i] ? '1' : '0');
            printf("\n");
        } else if (op == 'C') {
            uint8_t buf[256];
            size_t n = strlen(arg) / 2u;
            if (n > sizeof buf || !get_hex(arg, buf, n)) { printf("ARG\n"); continue; }
            printf("OK %08x\n", ae_crc32(buf, n));
        } else {
            printf("ARG\n");
        }
        fflush(stdout);
    }
    return 0;
}
