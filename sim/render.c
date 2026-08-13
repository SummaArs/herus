/* render.c — where the thesis becomes visible.
 *
 * Nothing linguistic is ever transmitted. The wire carries symbol ids; the words
 * below exist only at the receiver, and each unit holds the table for the
 * language its owner reads. Change a node's language and the SAME thirty-four
 * bytes come out in a different tongue, with no translation step anywhere,
 * because there was never any text to translate.
 *
 * This table is deliberately tiny. A shipping lexicon is generated from the
 * domain seed and holds 2048 fillers across 32 roles; twenty words are enough to
 * demonstrate the property, and a demo that needs a big dictionary to be
 * convincing was not demonstrating the property.
 */
#include "sim.h"
#include <stdio.h>
#include <string.h>

typedef struct { uint16_t id; const char *w[LANG_N]; } sym_t;

/* intents — the verb of the message */
static const sym_t INTENT[] = {
    {  1, { "estou aqui",   "i am here",   "estoy aqui"   } },
    {  2, { "venha",        "come",        "ven"          } },
    {  3, { "socorro",      "help",        "socorro"      } },
    {  4, { "estou bem",    "i am safe",   "estoy bien"   } },
    {  5, { "aguarde",      "wait",        "espera"       } },
    {  6, { "encontrei",    "found",       "encontre"     } },
    {  7, { "ferido",       "injured",     "herido"       } },
    {  8, { "em movimento", "moving",      "en movimiento"} },
    {  9, { "pare",         "stop",        "alto"         } },
    { 10, { "sem agua",     "out of water","sin agua"     } },
};

/* roles — the grammar. 5 bits, so 32 exist; these are the four every unit ships
 * knowing. A slot whose role is not here is dropped and the message still
 * arrives: that is P4, and scenario_babel demonstrates it. */
static const sym_t ROLE[] = {
    { 1, { "quem",   "who",   "quien" } },
    { 2, { "onde",   "where", "donde" } },
    { 3, { "quando", "when",  "cuando"} },
    { 4, { "quanto", "how many", "cuanto" } },
};

/* fillers — the nouns */
static const sym_t FILLER[] = {
    {  1, { "eu",         "me",         "yo"          } },
    {  2, { "voce",       "you",        "tu"          } },
    {  3, { "norte",      "north",      "norte"       } },
    {  4, { "sul",        "south",      "sur"         } },
    {  5, { "o rio",      "the river",  "el rio"      } },
    {  6, { "o acampamento", "the camp","el campamento"} },
    {  7, { "a trilha",   "the trail",  "el sendero"  } },
    {  8, { "agora",      "now",        "ahora"       } },
    {  9, { "em breve",   "soon",       "pronto"      } },
    { 10, { "ao anoitecer","at nightfall","al anochecer"} },
    { 11, { "dois",       "two",        "dos"         } },
    { 12, { "tres",       "three",      "tres"        } },
};

static const char *look(const sym_t *t, int n, uint16_t id, int lang)
{
    for (int i = 0; i < n; i++) if (t[i].id == id) return t[i].w[lang];
    return NULL;
}

const char *lang_name(int lang)
{
    static const char *N[LANG_N] = { "pt-BR", "en", "es" };
    return (lang >= 0 && lang < LANG_N) ? N[lang] : "?";
}

void render_msg(char *out, size_t n, const hcp_msg_t *m, int lang)
{
    const char *iv = look(INTENT, (int)(sizeof INTENT / sizeof *INTENT), m->intent, lang);
    int k = snprintf(out, n, "%s", iv ? iv : "<simbolo desconhecido>");

    for (int i = 0; i < m->nslot && k > 0 && (size_t)k < n; i++) {
        const char *r = look(ROLE,   (int)(sizeof ROLE   / sizeof *ROLE),   m->slot[i].role,   lang);
        const char *f = look(FILLER, (int)(sizeof FILLER / sizeof *FILLER), m->slot[i].filler, lang);
        /* An unknown filler is rendered as its id rather than dropped: the
         * receiver genuinely received it and hiding that would be a lie about
         * what arrived. An unknown ROLE never reaches here — link_recv already
         * filtered it, which is the point of P4. */
        if (!r) continue;
        if (f) k += snprintf(out + k, n - (size_t)k, " | %s: %s", r, f);
        else   k += snprintf(out + k, n - (size_t)k, " | %s: #%u", r, m->slot[i].filler);
    }
}
