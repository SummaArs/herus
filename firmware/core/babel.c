#include "babel.h"
#include <string.h>

/* ------------------------------------------------------------------ dobrar */

static int accent_pair(uint8_t b, char *out)
{
    if ((b >= 0xa0u && b <= 0xa5u) || (b >= 0x80u && b <= 0x85u)) { *out = 'a'; return 1; }
    if (b == 0xa7u || b == 0x87u)                                  { *out = 'c'; return 1; }
    if ((b >= 0xa8u && b <= 0xabu) || (b >= 0x88u && b <= 0x8bu)) { *out = 'e'; return 1; }
    if ((b >= 0xacu && b <= 0xafu) || (b >= 0x8cu && b <= 0x8fu)) { *out = 'i'; return 1; }
    if (b == 0xb1u || b == 0x91u)                                  { *out = 'n'; return 1; }
    if ((b >= 0xb2u && b <= 0xb6u) || (b >= 0x92u && b <= 0x96u)) { *out = 'o'; return 1; }
    if ((b >= 0xb9u && b <= 0xbcu) || (b >= 0x99u && b <= 0x9cu)) { *out = 'u'; return 1; }
    return 0;
}

/* Pontuacao de escrita CJK e de largura plena que conta como separador.
 * Sem isto, um ponto japones viraria lacuna e a frase inteira seria recusada. */
static int wide_space(const uint8_t *p, size_t width)
{
    static const char *const SEP[] = {
        "\343\200\200", "\343\200\201", "\343\200\202", "\343\203\273",
        "\357\274\214", "\357\274\201", "\357\274\237", "\357\274\232",
        "\357\274\233", "\342\200\246"
    };
    size_t i;
    if (width != 3) return 0;
    for (i = 0; i < sizeof SEP / sizeof SEP[0]; i++) {
        if (memcmp(p, SEP[i], 3) == 0) return 1;
    }
    return 0;
}

static int fold_ex(const char *text, size_t len, char *out, size_t out_cap,
                   size_t *n_out)
{
    const uint8_t *in = (const uint8_t *)text;
    size_t i = 0, n = 0;
    int last_space = 1;

    if (!text || !out || out_cap == 0) return 1;
    while (i < len) {
        uint8_t c = in[i];
        char folded[4];
        size_t flen = 1;

        if (c == 0u) return 1;
        if (c < 0x80u) {
            if (c >= 'A' && c <= 'Z')                      { folded[0] = (char)(c - 'A' + 'a'); i++; }
            else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) { folded[0] = (char)c; i++; }
            else if (c == ' ' || c == '\t' || c == ',' || c == '.' || c == '!' ||
                     c == '?' || c == ';' || c == ':' || c == '\n' || c == '\r' ||
                     c == '-' || c == '\'')                { folded[0] = ' '; i++; }
            else return 1;
        } else if (c == 0xc3u) {
            if (i + 1 >= len) return 1;
            if (!accent_pair(in[i + 1], &folded[0])) return 1;
            i += 2;
        } else {
            size_t width, k;
            if (c >= 0xc2u && c <= 0xdfu)      width = 2;
            else if (c >= 0xe0u && c <= 0xefu) width = 3;
            else if (c >= 0xf0u && c <= 0xf4u) width = 4;
            else return 1;
            if (i + width > len) return 1;
            for (k = 1; k < width; k++) {
                if (in[i + k] < 0x80u || in[i + k] > 0xbfu) return 1;
            }
            if (wide_space(in + i, width)) {
                folded[0] = ' ';
                flen = 1;
            } else {
                memcpy(folded, in + i, width);
                flen = width;
            }
            i += width;
        }

        if (flen == 1 && folded[0] == ' ') {
            if (last_space) continue;
            last_space = 1;
        } else {
            last_space = 0;
        }
        if (n + flen + 1 > out_cap) return 1;
        memcpy(out + n, folded, flen);
        n += flen;
    }
    while (n > 0 && out[n - 1] == ' ') n--;
    out[n] = '\0';
    *n_out = n;
    return 0;
}

babel_status_t babel_fold(const char *text, size_t len, char *out,
                          size_t out_cap, size_t *out_len)
{
    size_t n = 0;
    if (!text || !out || out_cap == 0) return BABEL_E_ARG;
    if (len + 1u > out_cap) return BABEL_E_OVERFLOW;
    if (fold_ex(text, len, out, out_cap, &n) != 0) return BABEL_E_BYTE;
    if (out_len) *out_len = n;
    return BABEL_OK;
}

static uint32_t fnv1a(const char *s, size_t n)
{
    uint32_t h = 2166136261u;
    size_t i;
    for (i = 0; i < n; i++) { h ^= (uint8_t)s[i]; h *= 16777619u; }
    return h;
}

/* -------------------------------------------------------------- casamento */

static int is_alnum(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

/* Fronteira, uma por escrita.
 *   idioma com espaco: comeco e fim de palavra. 'casa' nunca casa em 'casaco'.
 *   idioma sem espaco: transicao de escrita. Um numeral latino embutido e
 *   ATOMICO, entao '10' nunca se le como '1' seguido de '0'. */
static int boundary_ok(const char *s, size_t n, size_t i, size_t j, uint8_t spaced)
{
    if (spaced) {
        if (i > 0 && s[i - 1] != ' ') return 0;
        if (j < n && s[j] != ' ') return 0;
        return 1;
    }
    if (i > 0 && is_alnum(s[i - 1]) && is_alnum(s[i])) return 0;
    if (j < n && is_alnum(s[j - 1]) && is_alnum(s[j])) return 0;
    return 1;
}

static size_t skip_spaces(const char *s, size_t n, size_t i)
{
    while (i < n && s[i] == ' ') i++;
    return i;
}

/* Proximo candidato em `pos` a partir do offset `from` dentro do balde.
 * Devolve o offset encontrado, ou o tamanho do balde quando acabou. */
static uint16_t next_match(const char *s, size_t n, size_t pos, uint8_t lang,
                           uint16_t from, uint16_t *entry_out)
{
    const loom_bucket_t *b = &LOOM_BUCKET[lang][(uint8_t)s[pos]];
    uint8_t spaced = LOOM_LANG[lang].spaced;
    uint16_t k;
    for (k = from; k < b->count; k++) {
        uint16_t idx = (uint16_t)(b->first + k);
        const loom_entry_t *e = &LOOM_ENTRY[idx];
        size_t j = pos + e->len;
        if (j > n) continue;
        if (memcmp(s + pos, e->form, e->len) != 0) continue;
        if (!boundary_ok(s, n, pos, j, spaced)) continue;
        *entry_out = idx;
        return k;
    }
    return b->count;
}

/* Posicao mais distante alcancavel do inicio: onde a lacuna comeca. */
static size_t reachable_end(const char *s, size_t n, uint8_t lang)
{
    uint8_t seen[BABEL_TEXT_MAX + 2];
    size_t i, best = 0;
    memset(seen, 0, sizeof seen);
    seen[skip_spaces(s, n, 0)] = 1;
    for (i = 0; i <= n; i++) {
        uint16_t off = 0, idx = 0;
        if (!seen[i]) continue;
        if (i > best) best = i;
        if (i == n) continue;
        for (;;) {
            const loom_entry_t *e;
            off = next_match(s, n, i, lang, off, &idx);
            if (off >= LOOM_BUCKET[lang][(uint8_t)s[i]].count) break;
            e = &LOOM_ENTRY[idx];
            seen[skip_spaces(s, n, i + e->len)] = 1;
            off++;
        }
    }
    return best;
}

/* ---------------------------------------------------------------- construir */

static babel_status_t build(const uint16_t *path, uint8_t depth, hir_t *h,
                            uint8_t *missing_role, uint8_t *paraphrase)
{
    uint8_t op = HIR_OP_NONE, qcount = 0, i;
    uint8_t has_quem = 0, has_oque = 0, has_onde = 0, has_quando = 0, has_var = 0;

    hir_init(h);
    *missing_role = 0;
    *paraphrase = 0;

    for (i = 0; i < depth; i++) {
        const loom_entry_t *e = &LOOM_ENTRY[path[i]];
        hir_status_t st;
        switch (e->cls) {
        case LOOM_SENS: return BABEL_E_SENSITIVE;
        case LOOM_AUTH: return BABEL_E_AUTHORITY;
        case LOOM_STOP: break;
        case LOOM_NEG:  h->polarity = 1u; break;
        case LOOM_URG:
            if (h->urgency != 0u && h->urgency != (uint8_t)e->val) return BABEL_E_AMBIGUOUS;
            h->urgency = (uint8_t)e->val;
            break;
        case LOOM_OP:
            if (op != HIR_OP_NONE && op != (uint8_t)e->val) return BABEL_E_AMBIGUOUS;
            op = (uint8_t)e->val;
            break;
        case LOOM_QVAR:
            st = hir_put(h, e->role, HIR_FILLER_VAR);
            if (st == HIR_E_DUPLICATE_ROLE) return BABEL_E_AMBIGUOUS;
            if (st == HIR_E_FULL) return BABEL_E_OVERFLOW;
            if (st != HIR_OK) return BABEL_E_ARG;
            qcount++;
            break;
        case LOOM_FILL:
        case LOOM_FILLNEG:
            st = hir_put(h, e->role, e->val);
            if (st == HIR_E_DUPLICATE_ROLE) return BABEL_E_AMBIGUOUS;
            if (st == HIR_E_FULL) return BABEL_E_OVERFLOW;
            if (st != HIR_OK) return BABEL_E_ARG;
            /* a forma negativa lexical carrega tambem a polaridade */
            if (e->cls == LOOM_FILLNEG) h->polarity = 1u;
            break;
        default: return BABEL_E_ARG;
        }
        if (e->cls != LOOM_STOP) {
            const char *f = e->form;
            while (*f) { if (*f == ' ') { (*paraphrase)++; break; } f++; }
        }
    }

    /* inferencia estrutural: nunca inventa conteudo, so a operacao */
    if (op == HIR_OP_NONE) {
        if (qcount > 0) op = HIR_OP_PERGUNTAR;
        else if (h->slot_count > 0) op = HIR_OP_COMUNICAR;
        else return BABEL_E_INCOMPLETE;
    }
    if (qcount > 0 && op != HIR_OP_PERGUNTAR) return BABEL_E_AMBIGUOUS;
    h->op = op;

    if (op == HIR_OP_SOCORRO) {
        h->urgency = (uint8_t)HIR_URG_SOCORRO;
        /* um destinatario explicito vence a difusao padrao */
        (void)hir_put(h, HIR_ROLE_QUEM, 261u /* PER.TODOS */);
    }
    if (op == HIR_OP_LEMBRAR) h->persistence = (uint8_t)HIR_PERSIST_LEMBRAR;

    for (i = 0; i < h->slot_count; i++) {
        if (h->slot[i].filler == HIR_FILLER_VAR) has_var = 1;
        if (h->slot[i].role == HIR_ROLE_QUEM)   has_quem = 1;
        if (h->slot[i].role == HIR_ROLE_O_QUE)  has_oque = 1;
        if (h->slot[i].role == HIR_ROLE_ONDE)   has_onde = 1;
        if (h->slot[i].role == HIR_ROLE_QUANDO) has_quando = 1;
    }
    switch (op) {
    case HIR_OP_COMUNICAR:
        if (!has_quem) { *missing_role = HIR_ROLE_QUEM;  return BABEL_E_INCOMPLETE; }
        if (!has_oque) { *missing_role = HIR_ROLE_O_QUE; return BABEL_E_INCOMPLETE; }
        break;
    case HIR_OP_PERGUNTAR:
        if (qcount > 1u) return BABEL_E_AMBIGUOUS;
        if (!has_var || qcount != 1u || h->slot_count < 2u) return BABEL_E_INCOMPLETE;
        break;
    case HIR_OP_LEMBRAR:
        if (!has_oque && !has_onde && !has_quando) {
            *missing_role = HIR_ROLE_O_QUE; return BABEL_E_INCOMPLETE;
        }
        break;
    case HIR_OP_CONFIRMAR:
    case HIR_OP_CANCELAR:
        if (!has_oque) { *missing_role = HIR_ROLE_O_QUE; return BABEL_E_INCOMPLETE; }
        break;
    case HIR_OP_PLANEJAR:
        if (!has_oque && !has_onde) { *missing_role = HIR_ROLE_O_QUE; return BABEL_E_INCOMPLETE; }
        break;
    case HIR_OP_SOCORRO: break;
    default: return BABEL_E_INCOMPLETE;
    }
    if (has_var && op != HIR_OP_PERGUNTAR) return BABEL_E_AMBIGUOUS;
    if (hir_validate(h) != HIR_OK) return BABEL_E_INCOMPLETE;
    return BABEL_OK;
}

/* Severidade: quando nenhuma leitura produz significado, a recusa mais
 * restritiva ganha. Falhar fechado nao pode depender da ordem de exploracao. */
static uint8_t severity(babel_status_t s)
{
    switch (s) {
    case BABEL_E_SENSITIVE:  return 90;
    case BABEL_E_AUTHORITY:  return 80;
    case BABEL_E_AMBIGUOUS:  return 70;
    case BABEL_E_OVERFLOW:   return 60;
    case BABEL_E_INCOMPLETE: return 50;
    case BABEL_E_GAP:        return 40;
    case BABEL_E_EMPTY:      return 30;
    case BABEL_E_BYTE:       return 20;
    default:                 return 10;
    }
}

/* a ≼ b: mesma moldura, e todo slot de `a` esta em `b` com o mesmo
 * preenchimento. Ou seja, `b` diz tudo o que `a` diz, e mais. */
static int dominates(const hir_t *a, const hir_t *b)
{
    uint8_t i, j;
    if (a->op != b->op || a->polarity != b->polarity ||
        a->urgency != b->urgency || a->persistence != b->persistence) return 0;
    for (i = 0; i < a->slot_count; i++) {
        for (j = 0; j < b->slot_count; j++) {
            if (b->slot[j].role != a->slot[i].role) continue;
            if (b->slot[j].filler != a->slot[i].filler) return 0;
            break;
        }
        if (j == b->slot_count) return 0;
    }
    return 1;
}

/* O maximo unico da ordem ≼, ou NULL.
 *
 * A unica regra de PREFERENCIA de todo o compilador, e ela nao e heuristica —
 * e um teorema pequeno: RUIDO NUNCA ACRESCENTA SIGNIFICADO. Uma palavra
 * classificada como ruido nao contribui papel nem preenchimento. Entao, se
 * duas leituras diferem apenas porque uma explicou como ruido o que a outra
 * explicou como significado, a segunda contem a primeira e a primeira nao
 * contem nada que a segunda perca. Nao ha o que escolher: uma domina.
 *
 * "emergencia estou aqui" tem duas leituras: SOCORRO sozinho, lendo "estou
 * aqui" como duas palavras de ruido, e SOCORRO + CHEGUEI, lendo a frase como o
 * evento. A segunda domina, e e obviamente o que a pessoa disse.
 *
 * Quando as leituras se CONTRADIZEM — mesmo papel com preenchimento diferente,
 * operacao diferente, polaridade diferente — nao ha dominancia e as duas sao
 * recusadas. "avisa joao maria cheguei" continua AMBIGUOUS. */
static const hir_t *dominant(const hir_t *cands, uint8_t n)
{
    const hir_t *best = NULL;
    uint8_t i, k;
    for (i = 0; i < n; i++) {
        int top = 1;
        for (k = 0; k < n; k++) {
            if (!dominates(&cands[k], &cands[i])) { top = 0; break; }
        }
        if (!top) continue;
        /* Dois maximos distintos sao IMPOSSIVEIS, e vale registrar o
         * argumento: os candidatos passam por hir_equal() antes de entrar em
         * `cands`, entao sao dois a dois diferentes; se h1 ≼ h2 e h2 ≼ h1
         * entao h1 == h2 (a ordem e antissimetrica), o que contradiz a
         * deduplicacao. Este ramo e guarda defensiva de um caso argumentado
         * como inalcancavel — nao um controle carregador, e a campanha de
         * mutantes o declara excluido por essa razao em vez de fingir que a
         * suite o mata. */
        if (best != NULL) return NULL;
        best = &cands[i];
    }
    return best;
}

/* ---------------------------------------------------------------- compilar */

babel_status_t babel_compile(const char *text, size_t len, uint8_t lang,
                             babel_unit_t *out)
{
    char folded[BABEL_TEXT_MAX + 1];
    uint16_t path[BABEL_DEPTH_MAX];
    struct { uint16_t pos; uint16_t off; } stack[BABEL_DEPTH_MAX];
    size_t flen, n, i;
    uint8_t spaced, depth = 0, paths = 0, truncated = 0;
    hir_t cands[BABEL_MEANING_MAX];
    uint8_t ncand = 0, too_many = 0;
    babel_status_t worst = BABEL_E_INCOMPLETE;
    uint8_t worst_seen = 0, worst_missing = 0;
    uint8_t paraphrase = 0;

    if (!out) return BABEL_E_ARG;
    memset(out, 0, sizeof *out);
    hir_init(&out->meaning);
    out->lang = lang;
    if (lang >= LOOM_LANG_COUNT) { out->status = BABEL_E_LANG; return out->status; }
    if (!text) { out->status = BABEL_E_ARG; return out->status; }
    if (len == 0) { out->status = BABEL_E_EMPTY; return out->status; }
    if (len > BABEL_TEXT_MAX) { out->status = BABEL_E_OVERFLOW; return out->status; }
    if (fold_ex(text, len, folded, sizeof folded, &flen) != 0) {
        out->status = BABEL_E_BYTE; return out->status;
    }
    if (flen == 0) { out->status = BABEL_E_EMPTY; return out->status; }

    n = flen;
    spaced = LOOM_LANG[lang].spaced;
    if (spaced) {
        uint8_t ntok = 0;
        int in_tok = 0;
        for (i = 0; i < n; i++) {
            if (folded[i] == ' ') { in_tok = 0; continue; }
            if (!in_tok) { in_tok = 1; ntok++; }
        }
        if (ntok > BABEL_TOKEN_MAX) { out->status = BABEL_E_OVERFLOW; return out->status; }
        out->prov.token_count = ntok;
    }

    /* Passo um: classe protegida e lavagem de autoridade sao recusadas ONDE
     * QUER que aparecam, antes de qualquer significado. A recusa nao pode
     * depender da sorte da ordem da esquerda para a direita. */
    for (i = 0; i < n; i++) {
        uint16_t off = 0, idx = 0;
        if (folded[i] == ' ') continue;
        if (spaced && i > 0 && folded[i - 1] != ' ') continue;
        for (;;) {
            const loom_entry_t *e;
            off = next_match(folded, n, i, lang, off, &idx);
            if (off >= LOOM_BUCKET[lang][(uint8_t)folded[i]].count) break;
            e = &LOOM_ENTRY[idx];
            if (e->cls == LOOM_SENS) { out->status = BABEL_E_SENSITIVE; return out->status; }
            if (e->cls == LOOM_AUTH) { out->status = BABEL_E_AUTHORITY; return out->status; }
            off++;
        }
    }

    /* Busca em profundidade sobre TODAS as leituras, com orcamento. Onde o
     * Herald aceitaria a primeira que fecha, aqui a unicidade e obrigacao. */
    stack[0].pos = (uint16_t)skip_spaces(folded, n, 0);
    stack[0].off = 0;
    for (;;) {
        uint16_t idx = 0, found;
        size_t pos = stack[depth].pos;
        const loom_entry_t *e;
        size_t npos;

        if (pos >= n) {                       /* nao deveria acontecer aqui */
            if (depth == 0) break;
            depth--; continue;
        }
        found = next_match(folded, n, pos, lang, stack[depth].off, &idx);
        if (found >= LOOM_BUCKET[lang][(uint8_t)folded[pos]].count) {
            if (depth == 0) break;
            depth--; continue;                /* retrocede */
        }
        stack[depth].off = (uint16_t)(found + 1u);
        path[depth] = idx;
        e = &LOOM_ENTRY[idx];
        npos = skip_spaces(folded, n, pos + e->len);

        if (npos >= n) {                      /* leitura completa */
            hir_t cand;
            uint8_t missing = 0, para = 0;
            babel_status_t st = build(path, (uint8_t)(depth + 1u), &cand,
                                      &missing, &para);
            paths++;
            if (st == BABEL_OK) {
                uint8_t k, dup = 0;
                for (k = 0; k < ncand; k++) {
                    if (hir_equal(&cands[k], &cand)) { dup = 1; break; }
                }
                if (!dup) {
                    if (ncand >= BABEL_MEANING_MAX) { too_many = 1; break; }
                    cands[ncand++] = cand;
                    if (ncand == 1u) paraphrase = para;
                }
            } else if (severity(st) > worst_seen) {
                worst_seen = severity(st); worst = st; worst_missing = missing;
            }
            if (paths >= BABEL_PATH_MAX) { truncated = 1; break; }
            continue;                          /* tenta outro candidato aqui */
        }
        if ((uint8_t)(depth + 1u) >= BABEL_DEPTH_MAX) {
            out->status = BABEL_E_OVERFLOW; return out->status;
        }
        depth++;
        stack[depth].pos = (uint16_t)npos;
        stack[depth].off = 0;
    }

    out->paths = paths;
    out->truncated = truncated;
    out->readings = ncand;

    if (too_many) { out->status = BABEL_E_AMBIGUOUS; return out->status; }
    if (paths == 0) {
        size_t at = reachable_end(folded, n, lang), end;
        at = skip_spaces(folded, n, at);
        end = at;
        if (spaced) { while (end < n && folded[end] != ' ') end++; }
        else        { end = (at < n) ? at + 1u : at; }
        if (end == at && at < n) end = at + 1u;
        out->gap_at = (uint8_t)at;
        out->gap_len = (uint8_t)(end - at);
        out->gap_hash = fnv1a(folded + at, end - at);
        out->status = BABEL_E_GAP;
        return out->status;
    }
    if (ncand > 0) {
        /* Orcamento estourado: nao da para provar unicidade, entao recusa.
         * Falhar fechado vale mais que estar quase certo. */
        const hir_t *pick;
        if (truncated) { out->status = BABEL_E_AMBIGUOUS; return out->status; }
        pick = (ncand == 1u) ? &cands[0] : dominant(cands, ncand);
        if (pick == NULL) { out->status = BABEL_E_AMBIGUOUS; return out->status; }
        out->meaning = *pick;
        out->prov.paraphrase_depth = paraphrase;
        out->prov.op_was_inferred = 0;
        out->prov.requires_confirmation = (uint8_t)hir_requires_confirmation(pick);
        if (hir_digest(pick, out->digest) != HIR_OK) {
            out->status = BABEL_E_ARG; return out->status;
        }
        out->status = BABEL_OK;
        return out->status;
    }
    out->missing_role = worst_missing;
    out->status = worst;
    return out->status;
}

/* -------------------------------------------------------------- renderizar */

static const char *marker_value(const hir_t *h, uint8_t lang, const char *name,
                                size_t name_len, uint8_t var_role,
                                uint8_t lexical_neg)
{
    static const struct { const char *name; uint8_t role; } ROLES[] = {
        { "QUEM", HIR_ROLE_QUEM },   { "O_QUE", HIR_ROLE_O_QUE },
        { "QUANDO", HIR_ROLE_QUANDO }, { "ONDE", HIR_ROLE_ONDE },
        { "QUANTO", HIR_ROLE_QUANTO }, { "ESTADO", HIR_ROLE_ESTADO }
    };
    size_t k;

    if (name_len == 2 && memcmp(name, "OP", 2) == 0)
        return LOOM_OP_RENDER[lang][h->op & 7u];
    if (name_len == 3 && memcmp(name, "NEG", 3) == 0) {
        /* O marcador so aparece quando a polaridade NAO foi absorvida pela
         * forma lexical. Os dois juntos seriam negacao dupla. */
        if (!h->polarity || lexical_neg) return "";
        return LOOM_NEG_RENDER[lang];
    }
    if (name_len == 3 && memcmp(name, "URG", 3) == 0) {
        if (h->urgency == HIR_URG_ATENCAO || h->urgency == HIR_URG_URGENTE)
            return LOOM_URG_RENDER[lang][h->urgency];
        if (h->urgency == HIR_URG_SOCORRO && h->op != HIR_OP_SOCORRO)
            return LOOM_URG_RENDER[lang][HIR_URG_URGENTE];
        return "";
    }
    if (name_len == 4 && memcmp(name, "QVAR", 4) == 0)
        return var_role ? LOOM_QW_RENDER[lang][var_role] : "";
    /* Um marcador que nao existe no vocabulario de marcadores e defeito de
     * template, nao lacuna de idioma. Ele cai fora do laco e devolve NULL
     * tambem, mas o portao do Loom prova que nao ha template com marcador
     * desconhecido, entao o unico NULL alcancavel em producao e o do simbolo. */
    for (k = 0; k < sizeof ROLES / sizeof ROLES[0]; k++) {
        uint8_t i;
        if (strlen(ROLES[k].name) != name_len) continue;
        if (memcmp(name, ROLES[k].name, name_len) != 0) continue;
        if (ROLES[k].role == var_role) return "";
        for (i = 0; i < h->slot_count; i++) {
            if (h->slot[i].role != ROLES[k].role) continue;
            if (h->slot[i].filler == HIR_FILLER_VAR) return "";
            if (h->polarity && ROLES[k].role == HIR_ROLE_O_QUE) {
                const char *neg = loom_sym_neg(h->slot[i].filler, lang);
                if (neg) return neg;
            }
            /* NULL aqui e LACUNA, nao erro: ou o simbolo vem de um pacote de
             * dominio que este aparelho nao instalou, ou o pacote declara que
             * nao ha forma nesta lingua. Os dois casos tem a mesma resposta
             * certa — dizer que nao da para vestir este significado nesta
             * lingua — e a resposta errada seria a mesma nos dois: aproximar.
             * Devolver NULL faz babel_render() responder BABEL_E_GAP. */
            return loom_sym_render(h->slot[i].filler, lang);
        }
        return "";
    }
    return NULL;
}

babel_status_t babel_render(const hir_t *h, uint8_t lang,
                            char *out, size_t out_cap, size_t *out_len)
{
    char buf[BABEL_RENDER_MAX];
    size_t n = 0;
    uint8_t var_role = 0, i, s, lexical_neg = 0;
    const loom_tmpl_t *t;
    char sep;

    if (!h || !out || out_cap == 0) return BABEL_E_ARG;
    if (lang >= LOOM_LANG_COUNT) return BABEL_E_LANG;
    if (hir_validate(h) != HIR_OK) return BABEL_E_ARG;
    if ((h->op & 7u) != h->op) return BABEL_E_ARG;
    t = &LOOM_TMPL[lang][h->op];
    if (t->n == 0) return BABEL_E_LANG;
    sep = LOOM_LANG[lang].spaced ? ' ' : '\0';

    for (i = 0; i < h->slot_count; i++) {
        if (h->slot[i].filler == HIR_FILLER_VAR) var_role = h->slot[i].role;
        if (h->polarity && h->slot[i].role == HIR_ROLE_O_QUE &&
            loom_sym_neg(h->slot[i].filler, lang) != NULL) {
            lexical_neg = 1u;
        }
    }

    for (s = 0; s < t->n; s++) {
        const char *seg = t->seg[s];
        char piece[BABEL_RENDER_MAX];
        size_t pn = 0;
        int drop = 0;
        const char *p = seg;

        while (*p) {
            if (*p == '%') {
                const char *close = strchr(p + 1, '%');
                const char *val;
                size_t vlen;
                if (!close) return BABEL_E_ARG;
                val = marker_value(h, lang, p + 1, (size_t)(close - p - 1),
                                   var_role, lexical_neg);
                if (!val) return BABEL_E_GAP;   /* simbolo sem forma aqui */
                if (*val == '\0') { drop = 1; break; }
                vlen = strlen(val);
                if (pn + vlen >= sizeof piece) return BABEL_E_OVERFLOW;
                memcpy(piece + pn, val, vlen);
                pn += vlen;
                p = close + 1;
            } else {
                if (pn + 1 >= sizeof piece) return BABEL_E_OVERFLOW;
                piece[pn++] = *p++;
            }
        }
        if (drop || pn == 0) continue;
        piece[pn] = '\0';
        /* Cola. Num idioma sem espaco ela e vazia — MENOS quando o fim do que
         * ja saiu e o comeco do proximo pedaco sao os dois alfanumericos
         * latinos. Sem isso, "grupo 1" seguido de "1 hora" vira "小组11小时" e
         * o "11" deixa de ser dois numeros: passa a ser um numeral que o
         * compilador nao sabe cortar, porque a regra de fronteira trata
         * numeral latino como atomico de proposito, para que "10" nunca se
         * leia como "1" e "0". */
        if (n > 0 && (sep || (is_alnum(buf[n - 1]) && is_alnum(piece[0])))) {
            if (n + 1 >= sizeof buf) return BABEL_E_OVERFLOW;
            buf[n++] = ' ';
        }
        if (n + pn >= sizeof buf) return BABEL_E_OVERFLOW;
        memcpy(buf + n, piece, pn);
        n += pn;
    }

    /* normaliza: espacos colapsados e sem borda, para que a saida do render
     * seja exatamente aquilo que o dobramento produziria na entrada */
    {
        size_t r = 0, w = 0;
        int last_space = 1;
        while (r < n) {
            char c = buf[r++];
            if (c == ' ') {
                if (last_space) continue;
                last_space = 1;
            } else last_space = 0;
            buf[w++] = c;
        }
        while (w > 0 && buf[w - 1] == ' ') w--;
        n = w;
    }
    if (n + 1 > out_cap) return BABEL_E_OVERFLOW;
    memcpy(out, buf, n);
    out[n] = '\0';
    if (out_len) *out_len = n;
    return n ? BABEL_OK : BABEL_E_INCOMPLETE;
}

/* ------------------------------------------------------------------ idiomas */

int         babel_lang(const char *tag)        { return loom_lang_index(tag); }
uint8_t     babel_lang_count(void)             { return (uint8_t)LOOM_LANG_COUNT; }
const char *babel_lang_tag(uint8_t lang)
{ return lang < LOOM_LANG_COUNT ? LOOM_LANG[lang].tag : NULL; }
const char *babel_lang_endonym(uint8_t lang)
{ return lang < LOOM_LANG_COUNT ? LOOM_LANG[lang].endonym : NULL; }

const char *babel_status_name(babel_status_t s)
{
    switch (s) {
    case BABEL_OK:           return "OK";
    case BABEL_E_ARG:        return "ARG";
    case BABEL_E_EMPTY:      return "EMPTY";
    case BABEL_E_OVERFLOW:   return "OVERFLOW";
    case BABEL_E_GAP:        return "GAP";
    case BABEL_E_AMBIGUOUS:  return "AMBIGUOUS";
    case BABEL_E_SENSITIVE:  return "SENSITIVE";
    case BABEL_E_AUTHORITY:  return "AUTHORITY";
    case BABEL_E_INCOMPLETE: return "INCOMPLETE";
    case BABEL_E_BYTE:       return "BYTE";
    case BABEL_E_LANG:       return "LANG";
    default:                 return "?";
    }
}
