/* babel_probe.c — sonda de linha para provar o C contra a referencia Python.
 *
 * Existe por uma razao de metodo: a suite de invariantes do Loom roda sobre
 * tools/babel_ref.py, que e uma REFERENCIA, nao o artefato. Um espelho pode
 * concordar consigo mesmo e discordar do firmware. Esta sonda expoe o C real
 * numa interface de linha para que tools/test_babel_cross.py compare os dois
 * caso a caso — e a divergencia, quando houver, seja o achado.
 *
 * Protocolo, um pedido por linha, campos separados por TAB:
 *    C <tag> <texto>       compilar   -> "OK <digest> <wire>"  ou "<STATUS>"
 *    R <tag> <wire-hex>    renderizar -> "OK <texto>"          ou "<STATUS>"
 *    F -    <texto>        dobrar     -> "OK <dobrado>"        ou "BYTE"
 *    L                     idiomas    -> "OK tag:endonimo ..."
 */
#include "babel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void put_hex(const uint8_t *b, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) printf("%02x", b[i]);
}

static int get_hex(const char *s, uint8_t *out, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        unsigned v;
        if (sscanf(s + i * 2, "%2x", &v) != 1) return 0;
        out[i] = (uint8_t)v;
    }
    return 1;
}

int main(void)
{
    char line[1024];

    while (fgets(line, sizeof line, stdin)) {
        char *nl = strchr(line, '\n');
        char *op, *tag, *payload;
        int lang;

        if (nl) *nl = '\0';
        if (line[0] == '\0') continue;
        op = line;
        tag = strchr(line, '\t');
        if (!tag) {
            if (op[0] == 'L') {
                uint8_t i;
                printf("OK");
                for (i = 0; i < babel_lang_count(); i++)
                    printf(" %s:%s", babel_lang_tag(i), babel_lang_endonym(i));
                printf("\n");
                fflush(stdout);
                continue;
            }
            printf("ARG\n"); fflush(stdout); continue;
        }
        *tag++ = '\0';
        payload = strchr(tag, '\t');
        if (!payload) { printf("ARG\n"); fflush(stdout); continue; }
        *payload++ = '\0';
        lang = babel_lang(tag);

        if (op[0] == 'F') {
            char folded[BABEL_TEXT_MAX + 1];
            size_t n = 0;
            babel_status_t st = babel_fold(payload, strlen(payload), folded,
                                           sizeof folded, &n);
            if (st != BABEL_OK) printf("%s\n", babel_status_name(st));
            else { folded[n] = '\0'; printf("OK %s\n", folded); }
        } else if (op[0] == 'C') {
            babel_unit_t u;
            babel_status_t st = babel_compile(payload, strlen(payload),
                                              (uint8_t)(lang < 0 ? 255 : lang), &u);
            if (st != BABEL_OK) {
                printf("%s\n", babel_status_name(st));
            } else {
                uint8_t wire[HIR_WIRE_BYTES];
                if (hir_encode_wire(&u.meaning, wire) != HIR_OK) printf("ARG\n");
                else {
                    printf("OK ");
                    put_hex(u.digest, HIR_DIGEST_BYTES);
                    printf(" ");
                    put_hex(wire, HIR_WIRE_BYTES);
                    printf("\n");
                }
            }
        } else if (op[0] == 'R') {
            uint8_t wire[HIR_WIRE_BYTES];
            hir_t h;
            char text[BABEL_RENDER_MAX];
            if (strlen(payload) < HIR_WIRE_BYTES * 2 ||
                !get_hex(payload, wire, HIR_WIRE_BYTES)) { printf("ARG\n"); }
            else if (hir_decode_wire(wire, &h) != HIR_OK) { printf("ARG\n"); }
            else {
                babel_status_t st = babel_render(&h, (uint8_t)(lang < 0 ? 255 : lang),
                                                 text, sizeof text, NULL);
                if (st != BABEL_OK) printf("%s\n", babel_status_name(st));
                else printf("OK %s\n", text);
            }
        } else {
            printf("ARG\n");
        }
        fflush(stdout);
    }
    return 0;
}
