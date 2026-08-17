/* voice.c — a deliberately small Portuguese grammar for safe local interaction. */
#include "voice.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int lexicon_valid(const voice_lexicon_t *l)
{
    return l && l->intent_arrive < HCP_INTENT_N && l->intent_help < HCP_INTENT_N &&
           l->role_time < HCP_ROLE_N && l->minute_filler_base <= HCP_FILLER_N - 60u;
}

/* Preserve only lowercase ASCII letters, digits and spaces. UTF-8 punctuation
 * and accents are treated as separators; the starter grammar intentionally uses
 * accent-free command stems so ASR normalisation differences fail closed. */
static int normalise(const char *in, char out[VOICE_TRANSCRIPT_MAX])
{
    unsigned n = 0;
    int previous_space = 1;
    if (!in || !*in) return 0;
    for (; *in; in++) {
        unsigned char c = (unsigned char)*in;
        if (n + 1 >= VOICE_TRANSCRIPT_MAX) return 0;
        if (isalnum(c) && c < 128) {
            out[n++] = (char)tolower(c);
            previous_space = 0;
        } else if (!previous_space) {
            out[n++] = ' ';
            previous_space = 1;
        }
    }
    while (n && out[n - 1] == ' ') n--;
    out[n] = '\0';
    return n != 0;
}

static int word_present(const char *s, const char *word)
{
    size_t n = strlen(word);
    const char *p = s;
    while ((p = strstr(p, word)) != NULL) {
        int left = (p == s || p[-1] == ' ');
        int right = (p[n] == '\0' || p[n] == ' ');
        if (left && right) return 1;
        p++;
    }
    return 0;
}

static int any_word(const char *s, const char *const *words, unsigned n)
{
    for (unsigned i = 0; i < n; i++) if (word_present(s, words[i])) return 1;
    return 0;
}

static int word_number(const char *w)
{
    static const struct { const char *word; int value; } map[] = {
        { "um", 1 }, { "uma", 1 }, { "dois", 2 }, { "duas", 2 },
        { "tres", 3 }, { "quatro", 4 }, { "cinco", 5 }, { "seis", 6 },
        { "sete", 7 }, { "oito", 8 }, { "nove", 9 }, { "dez", 10 },
        { "onze", 11 }, { "doze", 12 }, { "treze", 13 }, { "quatorze", 14 },
        { "quinze", 15 }, { "dezesseis", 16 }, { "dezessete", 17 },
        { "dezoito", 18 }, { "dezenove", 19 }, { "vinte", 20 },
        { "trinta", 30 }, { "quarenta", 40 }, { "cinquenta", 50 },
        { "sessenta", 60 }
    };
    for (unsigned i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (!strcmp(w, map[i].word)) return map[i].value;
    return -1;
}

/* Return 0 when no duration is mentioned, 1..60 when valid, -1 when a duration
 * marker is present but its value is outside the supported semantic vocabulary. */
static int parse_minutes(const char *s)
{
    char copy[VOICE_TRANSCRIPT_MAX];
    char *tok[VOICE_TRANSCRIPT_MAX / 2];
    unsigned ntok = 0;
    int has_marker = word_present(s, "minuto") || word_present(s, "minutos") ||
                     word_present(s, "min") || word_present(s, "mins");

    if (!has_marker) return 0;
    strncpy(copy, s, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';
    for (char *p = strtok(copy, " "); p && ntok < sizeof(tok) / sizeof(tok[0]);
         p = strtok(NULL, " ")) tok[ntok++] = p;

    for (unsigned i = 0; i < ntok; i++) {
        char *end = NULL;
        long numeric = strtol(tok[i], &end, 10);
        if (*tok[i] && end && !*end) return (numeric >= 1 && numeric <= 60) ? (int)numeric : -1;

        int first = word_number(tok[i]);
        if (first < 0) continue;
        if (first >= 20 && first < 60 && i + 1 < ntok) {
            unsigned j = i + 1;
            if (!strcmp(tok[j], "e") && j + 1 < ntok) j++;
            int second = word_number(tok[j]);
            if (second > 0 && second < 10) first += second;
        }
        return (first >= 1 && first <= 60) ? first : -1;
    }
    return -1;
}

static void result_reset(voice_result_t *out, voice_status_t status, voice_event_t event)
{
    memset(out, 0, sizeof(*out));
    out->status = status;
    out->event = event;
}

void voice_lexicon_default(voice_lexicon_t *out)
{
    if (!out) return;
    out->intent_arrive = 1;
    out->intent_help = 2;
    out->role_time = 1;
    out->minute_filler_base = 1;
}

voice_status_t voice_from_command(voice_command_t command, uint8_t minutes,
                                  const voice_lexicon_t *lexicon,
                                  voice_result_t *out)
{
    if (!out) return VOICE_REJECTED;
    result_reset(out, VOICE_REJECTED, VOICE_EVENT_REJECTED);
    if (!lexicon_valid(lexicon)) return out->status;

    if (command == VOICE_COMMAND_CANCEL) {
        if (minutes) return out->status;
        result_reset(out, VOICE_CANCEL_LOCAL, VOICE_EVENT_CANCEL);
        return out->status;
    }
    if (command == VOICE_COMMAND_HELP) {
        if (minutes) return out->status;
        result_reset(out, VOICE_DRAFT, VOICE_EVENT_CRITICAL_DRAFT);
        out->draft.ver = HCP_VERSION;
        out->draft.tier = HCP_TIER_GLYPH;
        out->draft.intent = lexicon->intent_help;
        out->requires_confirmation = 1;
        return out->status;
    }
    if (command != VOICE_COMMAND_ARRIVE || minutes > 60u) return out->status;

    result_reset(out, VOICE_DRAFT, VOICE_EVENT_DRAFT);
    out->draft.ver = HCP_VERSION;
    out->draft.tier = minutes ? HCP_TIER_COMPOSED : HCP_TIER_GLYPH;
    out->draft.intent = lexicon->intent_arrive;
    if (minutes) {
        out->draft.nslot = 1;
        out->draft.slot[0].role = lexicon->role_time;
        out->draft.slot[0].filler = (uint16_t)(lexicon->minute_filler_base + minutes);
        out->minutes = minutes;
    }
    out->requires_confirmation = 1;
    return out->status;
}

voice_status_t voice_parse_pt(const char *transcript, const voice_lexicon_t *lexicon,
                              voice_result_t *out)
{
    static const char *const cancel_words[] = { "cancelar", "cancela", "pare", "parar" };
    static const char *const help_words[] = { "socorro", "ajuda", "ajude" };
    static const char *const arrive_words[] = { "chego", "chegando", "chegar", "chegarei" };
    char text[VOICE_TRANSCRIPT_MAX];
    int minutes;

    if (!out) return VOICE_REJECTED;
    result_reset(out, VOICE_REJECTED, VOICE_EVENT_REJECTED);
    if (!lexicon_valid(lexicon) || !normalise(transcript, text)) return out->status;

    if (any_word(text, cancel_words, sizeof(cancel_words) / sizeof(cancel_words[0]))) {
        result_reset(out, VOICE_CANCEL_LOCAL, VOICE_EVENT_CANCEL);
        return out->status;
    }
    if (any_word(text, help_words, sizeof(help_words) / sizeof(help_words[0]))) {
        result_reset(out, VOICE_DRAFT, VOICE_EVENT_CRITICAL_DRAFT);
        out->draft.ver = HCP_VERSION;
        out->draft.tier = HCP_TIER_GLYPH;
        out->draft.intent = lexicon->intent_help;
        out->requires_confirmation = 1;
        return out->status;
    }
    if (!any_word(text, arrive_words, sizeof(arrive_words) / sizeof(arrive_words[0]))) {
        result_reset(out, VOICE_UNKNOWN, VOICE_EVENT_UNKNOWN);
        return out->status;
    }

    minutes = parse_minutes(text);
    if (minutes < 0) return out->status;

    result_reset(out, VOICE_DRAFT, VOICE_EVENT_DRAFT);
    out->draft.ver = HCP_VERSION;
    out->draft.tier = minutes ? HCP_TIER_COMPOSED : HCP_TIER_GLYPH;
    out->draft.intent = lexicon->intent_arrive;
    if (minutes) {
        out->draft.nslot = 1;
        out->draft.slot[0].role = lexicon->role_time;
        out->draft.slot[0].filler = (uint16_t)(lexicon->minute_filler_base + minutes);
        out->minutes = (uint8_t)minutes;
    }
    out->requires_confirmation = 1;
    return out->status;
}

static void add_pulse(haptic_plan_t *p, uint16_t on, uint16_t off)
{
    if (p->n >= HAPTIC_MAX_PULSE) return;
    p->pulse[p->n].on_ms = on;
    p->pulse[p->n].off_ms = off;
    p->n++;
}

void voice_haptic_plan(voice_event_t event, haptic_plan_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    switch (event) {
    case VOICE_EVENT_DRAFT:
        add_pulse(out, 70, 90); add_pulse(out, 70, 0); break;
    case VOICE_EVENT_CONFIRMED:
        add_pulse(out, 160, 60); add_pulse(out, 80, 0); break;
    case VOICE_EVENT_CRITICAL_DRAFT:
        add_pulse(out, 140, 80); add_pulse(out, 70, 80); add_pulse(out, 140, 0); break;
    case VOICE_EVENT_CANCEL:
        add_pulse(out, 180, 0); break;
    case VOICE_EVENT_UNKNOWN:
        add_pulse(out, 45, 55); add_pulse(out, 45, 55); add_pulse(out, 45, 0); break;
    default:
        add_pulse(out, 110, 0); break;
    }
}

int haptic_plan_safe(const haptic_plan_t *plan)
{
    uint32_t total = 0;
    if (!plan || !plan->n || plan->n > HAPTIC_MAX_PULSE) return 0;
    for (unsigned i = 0; i < plan->n; i++) {
        if (!plan->pulse[i].on_ms || plan->pulse[i].on_ms > HAPTIC_MAX_ON_MS) return 0;
        total += (uint32_t)plan->pulse[i].on_ms + plan->pulse[i].off_ms;
        if (total > HAPTIC_MAX_TOTAL_MS) return 0;
    }
    return 1;
}
