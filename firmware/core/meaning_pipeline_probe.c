#include "meaning_pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void put_hex(const uint8_t *bytes, size_t count)
{
    size_t i;
    for (i = 0u; i < count; i++) printf("%02x", bytes[i]);
}

int main(void)
{
    char line[1024];
    while (fgets(line, sizeof line, stdin)) {
        char *nl = strchr(line, '\n');
        char *input_tag;
        char *output_tag;
        char *seq_text;
        char *text;
        char *end;
        int input_lang;
        int output_lang;
        unsigned long seq;
        meaning_result_t result;
        meaning_status_t status;

        if (nl) *nl = '\0';
        if (line[0] == '\0') continue;
        if (line[0] != 'P' || line[1] != '\t') {
            printf("ARG\n");
            fflush(stdout);
            continue;
        }
        input_tag = line + 2;
        output_tag = strchr(input_tag, '\t');
        if (!output_tag) { printf("ARG\n"); fflush(stdout); continue; }
        *output_tag++ = '\0';
        seq_text = strchr(output_tag, '\t');
        if (!seq_text) { printf("ARG\n"); fflush(stdout); continue; }
        *seq_text++ = '\0';
        text = strchr(seq_text, '\t');
        if (!text) { printf("ARG\n"); fflush(stdout); continue; }
        *text++ = '\0';

        input_lang = babel_lang(input_tag);
        output_lang = babel_lang(output_tag);
        seq = strtoul(seq_text, &end, 10);
        if (*seq_text == '\0' || *end != '\0' || seq > 255u ||
            input_lang < 0 || output_lang < 0) {
            printf("ARG\n");
            fflush(stdout);
            continue;
        }

        status = meaning_roundtrip(text, strlen(text), (uint8_t)input_lang,
                                   (uint8_t)output_lang, (uint8_t)seq, &result);
        if (status != MEANING_OK) {
            printf("%s\n", meaning_status_name(status));
        } else {
            uint8_t wire[HIR_WIRE_BYTES];
            if (hir_encode_wire(&result.received, wire) != HIR_OK) {
                printf("HIR\n");
            } else {
                printf("OK ");
                put_hex(result.frame, AE_FRAME);
                printf(" ");
                put_hex(wire, HIR_WIRE_BYTES);
                printf(" %.*s\n", (int)result.rendered_len, result.rendered);
            }
        }
        fflush(stdout);
    }
    return 0;
}
