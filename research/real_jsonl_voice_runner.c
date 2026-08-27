/*
 * Host-only runner for already extracted JSONL sentences.
 * The JSON decoding is intentionally outside this C parser; this binary only
 * receives one UTF-8 sentence per line and calls the production voice parser.
 * It never maps external SLURP intents to HERUS commands.
 */
#include "voice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_BYTES 4096u

typedef struct {
    unsigned rows;
    unsigned draft;
    unsigned cancel;
    unsigned unknown;
    unsigned rejected;
    unsigned overlong;
    unsigned non_ascii;
    unsigned empty;
} stats_t;

static int has_non_ascii(const char *text)
{
    for (const unsigned char *p = (const unsigned char *)text; *p; p++)
        if (*p >= 128u) return 1;
    return 0;
}

static int read_bounded_line(FILE *file, char line[LINE_MAX_BYTES], int *overlong)
{
    int c;
    *overlong = 0;
    if (!fgets(line, LINE_MAX_BYTES, file)) return 0;
    if (strlen(line) == LINE_MAX_BYTES - 1u && !strchr(line, '\n')) {
        c = fgetc(file);
        if (c != EOF) {
            *overlong = 1;
            while (c != '\n' && c != EOF) c = fgetc(file);
        }
    }
    return 1;
}

int main(void)
{
    char line[LINE_MAX_BYTES];
    voice_lexicon_t lexicon;
    voice_result_t result;
    stats_t stats = {0};
    int overlong;

    voice_lexicon_default(&lexicon);
    while (read_bounded_line(stdin, line, &overlong)) {
        size_t length = strlen(line);
        voice_status_t status;
        stats.rows++;
        if (overlong) {
            stats.overlong++;
            stats.rejected++;
            continue;
        }
        while (length > 0u && (line[length - 1u] == '\n' || line[length - 1u] == '\r'))
            line[--length] = '\0';
        if (length == 0u) stats.empty++;
        if (has_non_ascii(line)) stats.non_ascii++;
        status = voice_parse_pt(line, &lexicon, &result);
        switch (status) {
        case VOICE_DRAFT: stats.draft++; break;
        case VOICE_CANCEL_LOCAL: stats.cancel++; break;
        case VOICE_UNKNOWN: stats.unknown++; break;
        case VOICE_REJECTED: stats.rejected++; break;
        default: stats.rejected++; break;
        }
    }
    printf("parser_input_rows=%u\n", stats.rows);
    printf("parser_draft=%u\n", stats.draft);
    printf("parser_cancel=%u\n", stats.cancel);
    printf("parser_unknown=%u\n", stats.unknown);
    printf("parser_rejected=%u\n", stats.rejected);
    printf("parser_overlong=%u\n", stats.overlong);
    printf("parser_non_ascii=%u\n", stats.non_ascii);
    printf("parser_empty=%u\n", stats.empty);
    printf("automatic_label_mapping=0\n");
    printf("herus_command_authority=0\n");
    return EXIT_SUCCESS;
}
