/*
 * Host-only audit runner for real MIntRec metadata.
 * It deliberately measures the Portuguese finite parser against real English
 * utterances as an out-of-domain rejection/coverage experiment. It does not
 * map MIntRec labels to HERUS commands and it does not load media.
 */
#include "voice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_BYTES 1024u

typedef struct {
    unsigned total;
    unsigned draft;
    unsigned cancel;
    unsigned unknown;
    unsigned rejected;
    unsigned too_long;
    unsigned overlong_corpus_lines;
    unsigned non_ascii;
    unsigned empty;
} stats_t;

static int has_non_ascii(const char *text)
{
    for (const unsigned char *p = (const unsigned char *)text; *p; p++)
        if (*p >= 128u) return 1;
    return 0;
}

static int parse_row(char *line, char **text)
{
    char *field;
    unsigned column = 0;
    *text = NULL;
    for (field = strtok(line, "\t\n");
         field != NULL;
         field = strtok(NULL, "\t\n")) {
        if (column == 3u) *text = field;
        column++;
    }
    return column == 5u && *text != NULL;
}

static int read_bounded_line(FILE *file, char line[LINE_MAX_BYTES], int *overlong)
{
    int c;
    *overlong = 0;
    if (!fgets(line, LINE_MAX_BYTES, file)) return 0;
    if (strlen(line) == LINE_MAX_BYTES - 1u && !strchr(line, '\n')) {
        *overlong = 1;
        while ((c = fgetc(file)) != '\n' && c != EOF) { }
    }
    return 1;
}

static void run_file(const char *path, stats_t *stats)
{
    FILE *file = fopen(path, "rb");
    char line[LINE_MAX_BYTES];
    voice_lexicon_t lexicon;
    voice_result_t result;
    int overlong;

    if (!file) {
        perror(path);
        exit(EXIT_FAILURE);
    }
    voice_lexicon_default(&lexicon);
    if (!read_bounded_line(file, line, &overlong)) {
        fclose(file);
        fprintf(stderr, "empty corpus: %s\n", path);
        exit(EXIT_FAILURE);
    }
    while (read_bounded_line(file, line, &overlong)) {
        char *text = NULL;
        size_t length = strlen(line);
        voice_status_t status;
        if (!length || line[0] == '\n' || line[0] == '\r') continue;
        stats->total++;
        if (overlong) {
            stats->overlong_corpus_lines++;
            stats->rejected++;
            continue;
        }
        if (!parse_row(line, &text)) {
            stats->rejected++;
            continue;
        }
        if (!*text) stats->empty++;
        if (strlen(text) >= VOICE_TRANSCRIPT_MAX) stats->too_long++;
        if (has_non_ascii(text)) stats->non_ascii++;
        status = voice_parse_pt(text, &lexicon, &result);
        switch (status) {
        case VOICE_DRAFT: stats->draft++; break;
        case VOICE_CANCEL_LOCAL: stats->cancel++; break;
        case VOICE_UNKNOWN: stats->unknown++; break;
        case VOICE_REJECTED: stats->rejected++; break;
        default: stats->rejected++; break;
        }
    }
    fclose(file);
}

int main(int argc, char **argv)
{
    stats_t aggregate = {0};
    if (argc < 2) {
        fprintf(stderr, "usage: %s SPLIT.tsv [SPLIT.tsv ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    for (int i = 1; i < argc; i++) run_file(argv[i], &aggregate);
    printf("real_corpus_files=%d\n", argc - 1);
    printf("real_corpus_rows=%u\n", aggregate.total);
    printf("parser_draft=%u\n", aggregate.draft);
    printf("parser_cancel=%u\n", aggregate.cancel);
    printf("parser_unknown=%u\n", aggregate.unknown);
    printf("parser_rejected=%u\n", aggregate.rejected);
    printf("rows_too_long_for_voice_api=%u\n", aggregate.too_long);
    printf("overlong_corpus_lines=%u\n", aggregate.overlong_corpus_lines);
    printf("rows_non_ascii=%u\n", aggregate.non_ascii);
    printf("rows_empty_text=%u\n", aggregate.empty);
    printf("automatic_label_mapping=0\n");
    printf("multimodal_convergence_proven=0\n");
    return EXIT_SUCCESS;
}
