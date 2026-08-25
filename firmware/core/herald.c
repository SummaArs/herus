#include "herald.h"
#include <string.h>

/* ------------------------------------------------------------------ folding */

static int fold_byte_pair(uint8_t a, uint8_t b, char *out)
{
    if (a != 0xc3u) return 0;
    if ((b >= 0xa0u && b <= 0xa5u) || (b >= 0x80u && b <= 0x85u)) { *out = 'a'; return 1; }
    if (b == 0xa7u || b == 0x87u)                                  { *out = 'c'; return 1; }
    if ((b >= 0xa8u && b <= 0xabu) || (b >= 0x88u && b <= 0x8bu)) { *out = 'e'; return 1; }
    if ((b >= 0xacu && b <= 0xafu) || (b >= 0x8cu && b <= 0x8fu)) { *out = 'i'; return 1; }
    if (b == 0xb1u || b == 0x91u)                                  { *out = 'n'; return 1; }
    if ((b >= 0xb2u && b <= 0xb6u) || (b >= 0x92u && b <= 0x96u)) { *out = 'o'; return 1; }
    if ((b >= 0xb9u && b <= 0xbcu) || (b >= 0x99u && b <= 0x9cu)) { *out = 'u'; return 1; }
    return 0;
}

/* Returns 0 on success, 1 when a byte is outside the accepted encoding. */
static int fold_ex(const char *text, size_t len, char *out, size_t out_cap, size_t *n_out)
{
    size_t i = 0, n = 0;
    int last_space = 1;

    if (!text || !out || out_cap == 0) return 1;
    while (i < len) {
        uint8_t c = (uint8_t)text[i];
        char folded;

        if (c == 0u) return 1;                       /* embedded NUL: refuse */
        if (c >= 0x80u) {
            if (i + 1 >= len) return 1;
            if (!fold_byte_pair(c, (uint8_t)text[i+1], &folded)) return 1;
            i += 2;
        } else if (c >= 'A' && c <= 'Z') {
            folded = (char)(c - 'A' + 'a');
            i++;
        } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            folded = (char)c;
            i++;
        } else if (c == ' ' || c == '\t' || c == ',' || c == '.' || c == '!' ||
                   c == '?' || c == ';' || c == ':' || c == '\n' || c == '\r' ||
                   c == '-' || c == '\'') {
            folded = ' ';
            i++;
        } else {
            return 1;                                /* unknown ASCII byte */
        }

        if (folded == ' ') {
            if (last_space) continue;
            last_space = 1;
        } else {
            last_space = 0;
        }
        if (n + 1 >= out_cap) return 1;
        out[n++] = folded;
    }
    while (n > 0 && out[n-1] == ' ') n--;
    out[n] = '\0';
    *n_out = n;
    return 0;
}

size_t herald_fold(const char *text, size_t len, char *out, size_t out_cap)
{
    size_t n = 0;
    if (fold_ex(text, len, out, out_cap, &n) != 0) return 0;
    return n;
}

static uint32_t fnv1a(const char *s, size_t n)
{
    uint32_t h = 2166136261u;
    size_t i;
    for (i = 0; i < n; i++) {
        h ^= (uint8_t)s[i];
        h *= 16777619u;
    }
    return h;
}

/* ------------------------------------------------------------------ lexicon */

enum {
    LX_STOP = 0,      /* allowed noise, carries no meaning                     */
    LX_OP,            /* selects the operation                                 */
    LX_FILL,          /* binds a filler to its role                            */
    LX_NEG,           /* polarity                                              */
    LX_URG,           /* urgency                                               */
    LX_QVAR,          /* question word: opens a variable slot on `role`        */
    LX_SENS,          /* protected class: refuse before building any meaning   */
    LX_AUTH           /* authority laundering: refuse                          */
};

typedef struct {
    const char *form;   /* folded surface form; may contain spaces (a phrase) */
    uint8_t     cls;
    uint8_t     role;   /* for LX_FILL / LX_QVAR                              */
    uint16_t    val;    /* filler id, op id or urgency level                   */
} lex_t;

/* Symbols. Each namespace is disjoint by construction. */
#define EV_CHEGUEI      (HSYM_EV_BASE + 0u)
#define EV_SAINDO       (HSYM_EV_BASE + 1u)
#define EV_ATRASADO     (HSYM_EV_BASE + 2u)
#define EV_TUDO_BEM     (HSYM_EV_BASE + 3u)
#define EV_AJUDA        (HSYM_EV_BASE + 4u)
#define EV_ESPERANDO    (HSYM_EV_BASE + 5u)
#define EV_ENCONTRAR    (HSYM_EV_BASE + 6u)
#define EV_A_CAMINHO    (HSYM_EV_BASE + 7u)
#define EV_TERMINEI     (HSYM_EV_BASE + 8u)
#define EV_SEM_BATERIA  (HSYM_EV_BASE + 9u)
#define EV_PERDIDO      (HSYM_EV_BASE + 10u)
#define EV_ESTUDAR      (HSYM_EV_BASE + 11u)

#define PER_JOAO        (HSYM_PER_BASE + 0u)
#define PER_MARIA       (HSYM_PER_BASE + 1u)
#define PER_ANA         (HSYM_PER_BASE + 2u)
#define PER_EQUIPE      (HSYM_PER_BASE + 3u)
#define PER_FAMILIA     (HSYM_PER_BASE + 4u)
#define PER_TODOS       (HSYM_PER_BASE + 5u)

#define LOC_CASA        (HSYM_LOC_BASE + 0u)
#define LOC_TRABALHO    (HSYM_LOC_BASE + 1u)
#define LOC_PORTAO_N    (HSYM_LOC_BASE + 2u)
#define LOC_TREINO      (HSYM_LOC_BASE + 3u)
#define LOC_ESTRADA     (HSYM_LOC_BASE + 4u)
#define LOC_TRILHA      (HSYM_LOC_BASE + 5u)

#define T_AGORA         (HSYM_TIME_BASE + 0u)
#define T_MIN_5         (HSYM_TIME_BASE + 1u)
#define T_MIN_10        (HSYM_TIME_BASE + 2u)
#define T_MIN_30        (HSYM_TIME_BASE + 3u)
#define T_HORA_1        (HSYM_TIME_BASE + 4u)
#define T_HOJE          (HSYM_TIME_BASE + 5u)
#define T_AMANHA        (HSYM_TIME_BASE + 6u)
#define T_NOITE         (HSYM_TIME_BASE + 7u)

#define ST_BEM          (HSYM_ST_BASE + 0u)
#define ST_MAL          (HSYM_ST_BASE + 1u)
#define ST_SEGURO       (HSYM_ST_BASE + 2u)
#define ST_EM_PERIGO    (HSYM_ST_BASE + 3u)

/* Longest forms must be reachable: the matcher tries 3, then 2, then 1 token,
 * so ordering inside the table is irrelevant and duplicates are a test failure
 * rather than a silent precedence rule. */
static const lex_t LEX[] = {
    /* --- operations ------------------------------------------------------ */
    {"avisa",           LX_OP, 0, HIR_OP_COMUNICAR},
    {"avise",           LX_OP, 0, HIR_OP_COMUNICAR},
    {"avisar",          LX_OP, 0, HIR_OP_COMUNICAR},
    {"manda",           LX_OP, 0, HIR_OP_COMUNICAR},
    {"mande",           LX_OP, 0, HIR_OP_COMUNICAR},
    {"mandar",          LX_OP, 0, HIR_OP_COMUNICAR},
    {"fala",            LX_OP, 0, HIR_OP_COMUNICAR},
    {"falar",           LX_OP, 0, HIR_OP_COMUNICAR},
    {"diz",             LX_OP, 0, HIR_OP_COMUNICAR},
    {"dizer",           LX_OP, 0, HIR_OP_COMUNICAR},
    {"comunica",        LX_OP, 0, HIR_OP_COMUNICAR},
    {"conta",           LX_OP, 0, HIR_OP_COMUNICAR},
    {"lembra",          LX_OP, 0, HIR_OP_LEMBRAR},
    {"lembrar",         LX_OP, 0, HIR_OP_LEMBRAR},
    {"guarda",          LX_OP, 0, HIR_OP_LEMBRAR},
    {"guarde",          LX_OP, 0, HIR_OP_LEMBRAR},
    {"guardar",         LX_OP, 0, HIR_OP_LEMBRAR},
    {"anota",           LX_OP, 0, HIR_OP_LEMBRAR},
    {"anotar",          LX_OP, 0, HIR_OP_LEMBRAR},
    {"planeja",         LX_OP, 0, HIR_OP_PLANEJAR},
    {"planejar",        LX_OP, 0, HIR_OP_PLANEJAR},
    {"socorro",         LX_OP, 0, HIR_OP_SOCORRO},
    {"emergencia",      LX_OP, 0, HIR_OP_SOCORRO},
    {"confirma",        LX_OP, 0, HIR_OP_CONFIRMAR},
    {"confirmo",        LX_OP, 0, HIR_OP_CONFIRMAR},
    {"confirmar",       LX_OP, 0, HIR_OP_CONFIRMAR},
    {"cancela",         LX_OP, 0, HIR_OP_CANCELAR},
    {"cancelar",        LX_OP, 0, HIR_OP_CANCELAR},
    {"pergunta",        LX_OP, 0, HIR_OP_PERGUNTAR},

    /* --- question words -------------------------------------------------- */
    {"onde",            LX_QVAR, HIR_ROLE_ONDE,   0},
    {"cade",            LX_QVAR, HIR_ROLE_ONDE,   0},
    {"quando",          LX_QVAR, HIR_ROLE_QUANDO, 0},
    {"quem",            LX_QVAR, HIR_ROLE_QUEM,   0},
    {"que horas",       LX_QVAR, HIR_ROLE_QUANDO, 0},

    /* --- events (role O_QUE): the paraphrase families --------------------- */
    {"cheguei",             LX_FILL, HIR_ROLE_O_QUE, EV_CHEGUEI},
    {"to aqui",             LX_FILL, HIR_ROLE_O_QUE, EV_CHEGUEI},
    {"tou aqui",            LX_FILL, HIR_ROLE_O_QUE, EV_CHEGUEI},
    {"estou aqui",          LX_FILL, HIR_ROLE_O_QUE, EV_CHEGUEI},
    {"acabei de chegar",    LX_FILL, HIR_ROLE_O_QUE, EV_CHEGUEI},
    {"ja cheguei",          LX_FILL, HIR_ROLE_O_QUE, EV_CHEGUEI},

    {"saindo",              LX_FILL, HIR_ROLE_O_QUE, EV_SAINDO},
    {"to saindo",           LX_FILL, HIR_ROLE_O_QUE, EV_SAINDO},
    {"estou saindo",        LX_FILL, HIR_ROLE_O_QUE, EV_SAINDO},
    {"vou sair",            LX_FILL, HIR_ROLE_O_QUE, EV_SAINDO},

    {"atrasado",            LX_FILL, HIR_ROLE_O_QUE, EV_ATRASADO},
    {"to atrasado",         LX_FILL, HIR_ROLE_O_QUE, EV_ATRASADO},
    {"estou atrasado",      LX_FILL, HIR_ROLE_O_QUE, EV_ATRASADO},
    {"vou atrasar",         LX_FILL, HIR_ROLE_O_QUE, EV_ATRASADO},

    {"tudo bem",            LX_FILL, HIR_ROLE_O_QUE, EV_TUDO_BEM},
    {"tudo certo",          LX_FILL, HIR_ROLE_O_QUE, EV_TUDO_BEM},
    {"sem problema",        LX_FILL, HIR_ROLE_O_QUE, EV_TUDO_BEM},

    {"preciso de ajuda",    LX_FILL, HIR_ROLE_O_QUE, EV_AJUDA},
    {"preciso ajuda",       LX_FILL, HIR_ROLE_O_QUE, EV_AJUDA},
    {"me ajuda",            LX_FILL, HIR_ROLE_O_QUE, EV_AJUDA},
    {"ajuda",               LX_FILL, HIR_ROLE_O_QUE, EV_AJUDA},

    {"esperando",           LX_FILL, HIR_ROLE_O_QUE, EV_ESPERANDO},
    {"to esperando",        LX_FILL, HIR_ROLE_O_QUE, EV_ESPERANDO},
    {"estou esperando",     LX_FILL, HIR_ROLE_O_QUE, EV_ESPERANDO},
    {"aguardando",          LX_FILL, HIR_ROLE_O_QUE, EV_ESPERANDO},

    {"encontrar",           LX_FILL, HIR_ROLE_O_QUE, EV_ENCONTRAR},
    {"encontro",            LX_FILL, HIR_ROLE_O_QUE, EV_ENCONTRAR},
    {"nos vemos",           LX_FILL, HIR_ROLE_O_QUE, EV_ENCONTRAR},

    {"a caminho",           LX_FILL, HIR_ROLE_O_QUE, EV_A_CAMINHO},
    {"to indo",             LX_FILL, HIR_ROLE_O_QUE, EV_A_CAMINHO},
    {"estou indo",          LX_FILL, HIR_ROLE_O_QUE, EV_A_CAMINHO},
    {"chegando",            LX_FILL, HIR_ROLE_O_QUE, EV_A_CAMINHO},

    {"terminei",            LX_FILL, HIR_ROLE_O_QUE, EV_TERMINEI},
    {"acabei",              LX_FILL, HIR_ROLE_O_QUE, EV_TERMINEI},
    {"finalizei",           LX_FILL, HIR_ROLE_O_QUE, EV_TERMINEI},

    {"sem bateria",         LX_FILL, HIR_ROLE_O_QUE, EV_SEM_BATERIA},
    {"pouca bateria",       LX_FILL, HIR_ROLE_O_QUE, EV_SEM_BATERIA},
    {"bateria acabando",    LX_FILL, HIR_ROLE_O_QUE, EV_SEM_BATERIA},

    {"perdido",             LX_FILL, HIR_ROLE_O_QUE, EV_PERDIDO},
    {"to perdido",          LX_FILL, HIR_ROLE_O_QUE, EV_PERDIDO},
    {"estou perdido",       LX_FILL, HIR_ROLE_O_QUE, EV_PERDIDO},
    {"me perdi",            LX_FILL, HIR_ROLE_O_QUE, EV_PERDIDO},

    {"estudar",             LX_FILL, HIR_ROLE_O_QUE, EV_ESTUDAR},
    {"estudo",              LX_FILL, HIR_ROLE_O_QUE, EV_ESTUDAR},

    /* --- people (role QUEM) ---------------------------------------------- */
    {"joao",            LX_FILL, HIR_ROLE_QUEM, PER_JOAO},
    {"maria",           LX_FILL, HIR_ROLE_QUEM, PER_MARIA},
    {"ana",             LX_FILL, HIR_ROLE_QUEM, PER_ANA},
    {"equipe",          LX_FILL, HIR_ROLE_QUEM, PER_EQUIPE},
    {"time",            LX_FILL, HIR_ROLE_QUEM, PER_EQUIPE},
    {"familia",         LX_FILL, HIR_ROLE_QUEM, PER_FAMILIA},
    {"todos",           LX_FILL, HIR_ROLE_QUEM, PER_TODOS},
    {"geral",           LX_FILL, HIR_ROLE_QUEM, PER_TODOS},

    /* --- places (role ONDE) ---------------------------------------------- */
    {"casa",            LX_FILL, HIR_ROLE_ONDE, LOC_CASA},
    {"trabalho",        LX_FILL, HIR_ROLE_ONDE, LOC_TRABALHO},
    {"portao norte",    LX_FILL, HIR_ROLE_ONDE, LOC_PORTAO_N},
    {"treino",          LX_FILL, HIR_ROLE_ONDE, LOC_TREINO},
    {"academia",        LX_FILL, HIR_ROLE_ONDE, LOC_TREINO},
    {"estrada",         LX_FILL, HIR_ROLE_ONDE, LOC_ESTRADA},
    {"trilha",          LX_FILL, HIR_ROLE_ONDE, LOC_TRILHA},

    /* --- times (role QUANDO) --------------------------------------------- */
    {"agora",           LX_FILL, HIR_ROLE_QUANDO, T_AGORA},
    {"5 minutos",       LX_FILL, HIR_ROLE_QUANDO, T_MIN_5},
    {"cinco minutos",   LX_FILL, HIR_ROLE_QUANDO, T_MIN_5},
    {"10 minutos",      LX_FILL, HIR_ROLE_QUANDO, T_MIN_10},
    {"dez minutos",     LX_FILL, HIR_ROLE_QUANDO, T_MIN_10},
    {"30 minutos",      LX_FILL, HIR_ROLE_QUANDO, T_MIN_30},
    {"meia hora",       LX_FILL, HIR_ROLE_QUANDO, T_MIN_30},
    {"uma hora",        LX_FILL, HIR_ROLE_QUANDO, T_HORA_1},
    {"1 hora",          LX_FILL, HIR_ROLE_QUANDO, T_HORA_1},
    {"hoje",            LX_FILL, HIR_ROLE_QUANDO, T_HOJE},
    {"amanha",          LX_FILL, HIR_ROLE_QUANDO, T_AMANHA},
    {"hoje a noite",    LX_FILL, HIR_ROLE_QUANDO, T_NOITE},
    {"a noite",         LX_FILL, HIR_ROLE_QUANDO, T_NOITE},

    /* --- states (role ESTADO) -------------------------------------------- */
    {"bem",             LX_FILL, HIR_ROLE_ESTADO, ST_BEM},
    {"mal",             LX_FILL, HIR_ROLE_ESTADO, ST_MAL},
    {"seguro",          LX_FILL, HIR_ROLE_ESTADO, ST_SEGURO},
    {"em perigo",       LX_FILL, HIR_ROLE_ESTADO, ST_EM_PERIGO},

    /* --- modifiers -------------------------------------------------------- */
    {"nao",             LX_NEG, 0, 0},
    {"urgente",         LX_URG, 0, HIR_URG_URGENTE},
    {"rapido",          LX_URG, 0, HIR_URG_ATENCAO},
    {"com pressa",      LX_URG, 0, HIR_URG_ATENCAO},

    /* --- protected classes: refused before any meaning is built ----------- */
    {"senha",           LX_SENS, 0, 0},
    {"cartao",          LX_SENS, 0, 0},
    {"cpf",             LX_SENS, 0, 0},
    {"audio",           LX_SENS, 0, 0},
    {"gravacao",        LX_SENS, 0, 0},
    {"transcricao",     LX_SENS, 0, 0},
    {"localizacao",     LX_SENS, 0, 0},
    {"coordenada",      LX_SENS, 0, 0},
    {"coordenadas",     LX_SENS, 0, 0},
    {"senhas",          LX_SENS, 0, 0},
    {"gravacoes",       LX_SENS, 0, 0},
    {"biometria",       LX_SENS, 0, 0},
    {"gps",             LX_SENS, 0, 0},
    {"embedding",       LX_SENS, 0, 0},
    {"chave",           LX_SENS, 0, 0},

    /* --- authority laundering: refused ------------------------------------ */
    {"automaticamente", LX_AUTH, 0, 0},
    {"sem confirmar",   LX_AUTH, 0, 0},
    {"sem confirmacao", LX_AUTH, 0, 0},
    {"por conta propria", LX_AUTH, 0, 0},
    {"ignore",          LX_AUTH, 0, 0},
    {"ignora",          LX_AUTH, 0, 0},
    {"desconsidere",    LX_AUTH, 0, 0},
    {"sempre que",      LX_AUTH, 0, 0},
    {"sozinho",         LX_AUTH, 0, 0},

    /* --- allowed noise ---------------------------------------------------- */
    {"pro",  LX_STOP,0,0}, {"pra",  LX_STOP,0,0}, {"para", LX_STOP,0,0},
    {"o",    LX_STOP,0,0}, {"a",    LX_STOP,0,0}, {"os",   LX_STOP,0,0},
    {"as",   LX_STOP,0,0}, {"que",  LX_STOP,0,0}, {"eu",   LX_STOP,0,0},
    {"de",   LX_STOP,0,0}, {"do",   LX_STOP,0,0}, {"da",   LX_STOP,0,0},
    {"em",   LX_STOP,0,0}, {"no",   LX_STOP,0,0}, {"na",   LX_STOP,0,0},
    {"e",    LX_STOP,0,0}, {"ao",   LX_STOP,0,0}, {"aos",  LX_STOP,0,0},
    {"um",   LX_STOP,0,0}, {"uma",  LX_STOP,0,0}, {"meu",  LX_STOP,0,0},
    {"minha",LX_STOP,0,0}, {"com",  LX_STOP,0,0}, {"ta",   LX_STOP,0,0},
    {"to",   LX_STOP,0,0}, {"tou",  LX_STOP,0,0}, {"esta", LX_STOP,0,0},
    {"estou",LX_STOP,0,0}, {"ja",   LX_STOP,0,0}, {"se",   LX_STOP,0,0},
    {"por",  LX_STOP,0,0}, {"favor",LX_STOP,0,0}, {"voce", LX_STOP,0,0},
    {"vc",   LX_STOP,0,0}, {"ate",  LX_STOP,0,0}, {"esse", LX_STOP,0,0},
    {"isso", LX_STOP,0,0}, {"aqui", LX_STOP,0,0}, {"la",   LX_STOP,0,0},
    {"tenho",LX_STOP,0,0}, {"tem",  LX_STOP,0,0}, {"vou",  LX_STOP,0,0},
    {"sobre",LX_STOP,0,0}, {"me",   LX_STOP,0,0}, {"nos",  LX_STOP,0,0}
};

#define LEX_COUNT (sizeof LEX / sizeof LEX[0])

uint16_t herald_lexicon_entries(void) { return (uint16_t)LEX_COUNT; }

uint16_t herald_lexicon_symbols(void)
{
    uint16_t seen[LEX_COUNT];
    uint16_t n = 0, i, j;
    for (i = 0; i < (uint16_t)LEX_COUNT; i++) {
        if (LEX[i].cls != LX_FILL) continue;
        for (j = 0; j < n; j++) if (seen[j] == LEX[i].val) break;
        if (j == n) seen[n++] = LEX[i].val;
    }
    return n;
}

const char *herald_status_name(herald_status_t s)
{
    switch (s) {
    case HERALD_OK:          return "OK";
    case HERALD_E_ARG:       return "ARG";
    case HERALD_E_EMPTY:     return "EMPTY";
    case HERALD_E_OVERFLOW:  return "OVERFLOW";
    case HERALD_E_GAP:       return "GAP";
    case HERALD_E_AMBIGUOUS: return "AMBIGUOUS";
    case HERALD_E_SENSITIVE: return "SENSITIVE";
    case HERALD_E_AUTHORITY: return "AUTHORITY";
    case HERALD_E_INCOMPLETE:return "INCOMPLETE";
    case HERALD_E_BYTE:      return "BYTE";
    default:                 return "?";
    }
}

/* ---------------------------------------------------------------- matching */

typedef struct {
    const char *p;
    size_t      n;
} tok_t;

static int span_equals(const tok_t *tok, uint8_t first, uint8_t count, const char *form)
{
    size_t fi = 0;
    uint8_t k;
    for (k = 0; k < count; k++) {
        size_t j;
        if (k > 0) {
            if (form[fi] != ' ') return 0;
            fi++;
        }
        for (j = 0; j < tok[first + k].n; j++) {
            if (form[fi + j] == '\0' || form[fi + j] != tok[first + k].p[j]) return 0;
        }
        fi += tok[first + k].n;
    }
    return form[fi] == '\0';
}

static const lex_t *lookup(const tok_t *tok, uint8_t first, uint8_t count)
{
    uint16_t i;
    for (i = 0; i < (uint16_t)LEX_COUNT; i++) {
        if (span_equals(tok, first, count, LEX[i].form)) return &LEX[i];
    }
    return NULL;
}

/* ---------------------------------------------------------------- compiler */

static herald_status_t finish(herald_unit_t *out, herald_status_t st)
{
    out->status = st;
    return st;
}

herald_status_t herald_compile(const char *text, size_t len, herald_unit_t *out)
{
    char folded[HERALD_TEXT_MAX + 1];
    tok_t tok[HERALD_TOKEN_MAX];
    uint8_t ntok = 0, i;
    size_t flen, k;
    uint8_t op = HIR_OP_NONE;
    uint8_t inferred = 0, depth = 0;
    uint8_t qcount = 0;
    hir_t *h;

    if (!out) return HERALD_E_ARG;
    memset(out, 0, sizeof *out);
    hir_init(&out->meaning);
    h = &out->meaning;

    if (!text) return finish(out, HERALD_E_ARG);
    if (len == 0) return finish(out, HERALD_E_EMPTY);
    if (len > HERALD_TEXT_MAX) return finish(out, HERALD_E_OVERFLOW);

    if (fold_ex(text, len, folded, sizeof folded, &flen) != 0)
        return finish(out, HERALD_E_BYTE);
    if (flen == 0) return finish(out, HERALD_E_EMPTY);

    /* tokenize */
    k = 0;
    while (k < flen) {
        size_t start;
        while (k < flen && folded[k] == ' ') k++;
        if (k >= flen) break;
        start = k;
        while (k < flen && folded[k] != ' ') k++;
        if (ntok >= HERALD_TOKEN_MAX) return finish(out, HERALD_E_OVERFLOW);
        if (k - start > HERALD_TOKEN_BYTES) return finish(out, HERALD_E_OVERFLOW);
        tok[ntok].p = folded + start;
        tok[ntok].n = k - start;
        ntok++;
    }
    if (ntok == 0) return finish(out, HERALD_E_EMPTY);
    out->prov.token_count = ntok;

    /* Pass one: protected classes and authority laundering are refused wherever
     * they appear, before any meaning is built. A refusal must not depend on a
     * lucky left-to-right order — "guarde isso automaticamente" and
     * "automaticamente guarde isso" have to fail the same way. */
    for (i = 0; i < ntok; i++) {
        uint8_t w;
        for (w = HERALD_PHRASE_MAX; w >= 1u; w--) {
            const lex_t *probe;
            if ((uint8_t)(i + w) > ntok) { if (w == 1u) break; continue; }
            probe = lookup(tok, i, w);
            if (probe && probe->cls == LX_SENS) return finish(out, HERALD_E_SENSITIVE);
            if (probe && probe->cls == LX_AUTH) return finish(out, HERALD_E_AUTHORITY);
            if (w == 1u) break;
        }
    }

    /* greedy longest match, total coverage */
    i = 0;
    while (i < ntok) {
        const lex_t *hit = NULL;
        uint8_t width = 0, w;

        for (w = HERALD_PHRASE_MAX; w >= 1u; w--) {
            if ((uint8_t)(i + w) > ntok) continue;
            hit = lookup(tok, i, w);
            if (hit) { width = w; break; }
            if (w == 1u) break;
        }
        if (!hit) {
            out->gap.present = 1;
            out->gap.token_index = i;
            out->gap.token_len = (uint8_t)tok[i].n;
            out->gap.lexeme_hash = fnv1a(tok[i].p, tok[i].n);
            return finish(out, HERALD_E_GAP);
        }
        if (width > 1u) depth++;

        switch (hit->cls) {
        case LX_SENS:
            return finish(out, HERALD_E_SENSITIVE);
        case LX_AUTH:
            return finish(out, HERALD_E_AUTHORITY);
        case LX_STOP:
            break;
        case LX_NEG:
            h->polarity = 1u;
            break;
        case LX_URG:
            if (h->urgency != 0u && h->urgency != (uint8_t)hit->val)
                return finish(out, HERALD_E_AMBIGUOUS);
            h->urgency = (uint8_t)hit->val;
            break;
        case LX_OP:
            if (op != HIR_OP_NONE && op != (uint8_t)hit->val)
                return finish(out, HERALD_E_AMBIGUOUS);
            op = (uint8_t)hit->val;
            break;
        case LX_QVAR: {
            hir_status_t st = hir_put(h, hit->role, HIR_FILLER_VAR);
            if (st == HIR_E_DUPLICATE_ROLE) return finish(out, HERALD_E_AMBIGUOUS);
            if (st == HIR_E_FULL) return finish(out, HERALD_E_OVERFLOW);
            if (st != HIR_OK) return finish(out, HERALD_E_ARG);
            qcount++;
            break;
        }
        case LX_FILL: {
            hir_status_t st = hir_put(h, hit->role, hit->val);
            if (st == HIR_E_DUPLICATE_ROLE) return finish(out, HERALD_E_AMBIGUOUS);
            if (st == HIR_E_FULL) return finish(out, HERALD_E_OVERFLOW);
            if (st != HIR_OK) return finish(out, HERALD_E_ARG);
            break;
        }
        default:
            return finish(out, HERALD_E_ARG);
        }
        i = (uint8_t)(i + width);
    }

    out->prov.paraphrase_depth = depth;

    /* structural inference: never invent content, only the operation */
    if (op == HIR_OP_NONE) {
        if (qcount > 0) { op = HIR_OP_PERGUNTAR; inferred = 1; }
        else if (h->slot_count > 0) { op = HIR_OP_COMUNICAR; inferred = 1; }
        else return finish(out, HERALD_E_INCOMPLETE);
    }
    if (qcount > 0 && op != HIR_OP_PERGUNTAR) return finish(out, HERALD_E_AMBIGUOUS);
    h->op = op;
    out->prov.op_was_inferred = inferred;

    if (op == HIR_OP_SOCORRO) {
        h->urgency = (uint8_t)HIR_URG_SOCORRO;
        if (hir_put(h, HIR_ROLE_QUEM, PER_TODOS) == HIR_E_DUPLICATE_ROLE) {
            /* an explicit recipient beats the broadcast default */
        }
    }
    if (op == HIR_OP_LEMBRAR) h->persistence = (uint8_t)HIR_PERSIST_LEMBRAR;

    /* structural type check — the gate that mirrors a manifest validator */
    {
        uint8_t has_quem = 0, has_oque = 0, has_onde = 0, has_quando = 0, has_var = 0;
        for (i = 0; i < h->slot_count; i++) {
            if (h->slot[i].filler == HIR_FILLER_VAR) has_var = 1;
            if (h->slot[i].role == HIR_ROLE_QUEM)  has_quem = 1;
            if (h->slot[i].role == HIR_ROLE_O_QUE) has_oque = 1;
            if (h->slot[i].role == HIR_ROLE_ONDE)  has_onde = 1;
            if (h->slot[i].role == HIR_ROLE_QUANDO) has_quando = 1;
        }
        switch (op) {
        case HIR_OP_COMUNICAR:
            if (!has_quem) { out->missing_role = HIR_ROLE_QUEM;  return finish(out, HERALD_E_INCOMPLETE); }
            if (!has_oque) { out->missing_role = HIR_ROLE_O_QUE; return finish(out, HERALD_E_INCOMPLETE); }
            break;
        case HIR_OP_PERGUNTAR:
            if (qcount > 1u) return finish(out, HERALD_E_AMBIGUOUS);
            /* a question needs both the variable and the thing it asks about */
            if (!has_var || qcount != 1u || h->slot_count < 2u)
                return finish(out, HERALD_E_INCOMPLETE);
            break;
        case HIR_OP_LEMBRAR:
            if (!has_oque && !has_onde && !has_quando) {
                out->missing_role = HIR_ROLE_O_QUE;
                return finish(out, HERALD_E_INCOMPLETE);
            }
            break;
        case HIR_OP_CONFIRMAR:
        case HIR_OP_CANCELAR:
            if (!has_oque) { out->missing_role = HIR_ROLE_O_QUE; return finish(out, HERALD_E_INCOMPLETE); }
            break;
        case HIR_OP_PLANEJAR:
            if (!has_oque && !has_onde) { out->missing_role = HIR_ROLE_O_QUE; return finish(out, HERALD_E_INCOMPLETE); }
            break;
        case HIR_OP_SOCORRO:
            break;
        default:
            return finish(out, HERALD_E_INCOMPLETE);
        }
        if (has_var && op != HIR_OP_PERGUNTAR) return finish(out, HERALD_E_AMBIGUOUS);
    }

    if (hir_validate(h) != HIR_OK) return finish(out, HERALD_E_INCOMPLETE);
    if (hir_digest(h, out->digest) != HIR_OK) return finish(out, HERALD_E_ARG);
    out->prov.requires_confirmation = (uint8_t)hir_requires_confirmation(h);
    return finish(out, HERALD_OK);
}
