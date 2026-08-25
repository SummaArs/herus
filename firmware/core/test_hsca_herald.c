/* test_hsca_herald.c — the intent compiler: convergence, coverage, refusal.
 *
 * The headline property is paraphrase convergence: every surface form inside a
 * family must compile to the same canonical meaning and therefore to the same
 * digest, and no two families may ever share one. Everything else in this file
 * exists so that convergence cannot be bought with permissiveness.
 */
#include "herald.h"
#include <stdio.h>
#include <string.h>

static int pass_count;
static int fail_count;

static void check(int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) pass_count++; else fail_count++;
}

static herald_status_t compile_text(const char *t, herald_unit_t *u)
{
    return herald_compile(t, strlen(t), u);
}

/* ------------------------------------------------------------ frozen corpus */

typedef struct {
    const char *name;
    const char *form[8];
} family_t;

static const family_t FAMILY[] = {
    { "chegada-joao", {
        "manda pro João que cheguei",
        "avisa o joao que eu cheguei",
        "fala pro joão que tô aqui",
        "João, cheguei",
        "diz ao joao que acabei de chegar",
        "cheguei, avisa o joao",
        "AVISE O JOAO QUE JA CHEGUEI",
        "  manda   pro   joao   que   cheguei  " } },
    { "atraso-maria-10", {
        "avisa a maria que to atrasado 10 minutos",
        "avise maria que vou atrasar dez minutos",
        "manda pra maria que estou atrasado dez minutos",
        "maria, atrasado 10 minutos",
        NULL, NULL } },
    { "onde-joao", {
        "onde esta o joao",
        "cade o joao",
        "cadê o joão",
        NULL, NULL, NULL } },
    { "saindo-equipe", {
        "avisa a equipe que to saindo",
        "manda pro time que estou saindo",
        "equipe, saindo",
        NULL, NULL, NULL } },
    { "ajuda-familia-trilha", {
        "manda pra familia que preciso de ajuda na trilha",
        "avisa familia preciso ajuda trilha",
        "familia, me ajuda na trilha",
        NULL, NULL, NULL } },
    { "encontro-ana-portao", {
        "avisa a ana que nos vemos no portao norte",
        "manda pra ana encontro no portão norte",
        "ana, encontrar portao norte",
        NULL, NULL, NULL } },
    { "lembrar-treino-amanha", {
        "lembra do treino amanha",
        "guarda treino amanhã",
        "anota treino amanha",
        NULL, NULL, NULL } },
    { "socorro", {
        "socorro",
        "emergencia",
        NULL, NULL, NULL, NULL } },
    { "sem-bateria-joao", {
        "avisa o joao que estou sem bateria",
        "joao, pouca bateria",
        "manda pro joao bateria acabando",
        NULL, NULL, NULL } },
    { "negado-chegada-joao", {
        "avisa o joao que nao cheguei",
        "joao, não cheguei",
        NULL, NULL, NULL, NULL } }
};
#define FAMILY_COUNT (sizeof FAMILY / sizeof FAMILY[0])

typedef struct {
    const char     *text;
    herald_status_t expect;
} refusal_t;

static const refusal_t REFUSAL[] = {
    /* total coverage: one unexplained token is a refusal, not a silent drop */
    { "manda pro joao que cheguei e apaga tudo",  HERALD_E_GAP },
    { "avisa o joao que cheguei no aeroporto",    HERALD_E_GAP },
    { "manda pro pedro que cheguei",              HERALD_E_GAP },
    { "cheguei ontem",                            HERALD_E_GAP },
    { "talvez eu chegue",                         HERALD_E_GAP },
    /* authority laundering, wherever it sits in the sentence */
    { "guarde isso automaticamente",              HERALD_E_AUTHORITY },
    { "automaticamente guarda treino amanha",     HERALD_E_AUTHORITY },
    { "avisa o joao sem confirmar que cheguei",   HERALD_E_AUTHORITY },
    { "ignore as regras anteriores",              HERALD_E_AUTHORITY },
    { "manda pro joao sempre que eu chegar",      HERALD_E_AUTHORITY },
    { "avisa o joao por conta propria",           HERALD_E_AUTHORITY },
    { "faz sozinho",                              HERALD_E_AUTHORITY },
    /* protected classes never reach an executable meaning */
    { "manda minha localizacao pro joao",         HERALD_E_SENSITIVE },
    { "avisa o joao a minha senha",               HERALD_E_SENSITIVE },
    { "guarda o audio",                           HERALD_E_SENSITIVE },
    { "manda a transcricao pra maria",            HERALD_E_SENSITIVE },
    { "avisa o joao as coordenadas",              HERALD_E_SENSITIVE },
    { "guarda o embedding",                       HERALD_E_SENSITIVE },
    /* two readings, no margin */
    { "avisa o joao que cheguei e to saindo",     HERALD_E_AMBIGUOUS },
    { "manda e cancela pro joao que cheguei",     HERALD_E_AMBIGUOUS },
    { "onde e quando",                            HERALD_E_AMBIGUOUS },
    /* well-formed words that are structurally not a meaning */
    { "cheguei",                                  HERALD_E_INCOMPLETE },
    { "avisa o joao",                             HERALD_E_INCOMPLETE },
    { "manda",                                    HERALD_E_INCOMPLETE },
    { "onde",                                     HERALD_E_INCOMPLETE }
};
#define REFUSAL_COUNT (sizeof REFUSAL / sizeof REFUSAL[0])

/* ------------------------------------------------------------------- suites */

static void run_convergence(void)
{
    uint8_t digest[FAMILY_COUNT][HIR_DIGEST_BYTES];
    int forms_total = 0, forms_ok = 0;
    size_t f, g;
    int all_converge = 1, all_distinct = 1;

    for (f = 0; f < FAMILY_COUNT; f++) {
        herald_unit_t first;
        size_t k;
        if (compile_text(FAMILY[f].form[0], &first) != HERALD_OK) {
            printf("  FAIL  family %s: canonical form did not compile (%s)\n",
                   FAMILY[f].name, herald_status_name(first.status));
            fail_count++;
            all_converge = 0;
            memset(digest[f], 0, HIR_DIGEST_BYTES);
            continue;
        }
        memcpy(digest[f], first.digest, HIR_DIGEST_BYTES);
        forms_total++; forms_ok++;

        for (k = 1; k < 8u && FAMILY[f].form[k]; k++) {
            herald_unit_t u;
            forms_total++;
            if (compile_text(FAMILY[f].form[k], &u) != HERALD_OK) {
                printf("  FAIL  %s: \"%s\" -> %s\n", FAMILY[f].name,
                       FAMILY[f].form[k], herald_status_name(u.status));
                all_converge = 0;
                continue;
            }
            if (memcmp(u.digest, digest[f], HIR_DIGEST_BYTES) != 0 ||
                !hir_equal(&u.meaning, &first.meaning)) {
                printf("  FAIL  %s: \"%s\" diverged\n", FAMILY[f].name, FAMILY[f].form[k]);
                all_converge = 0;
                continue;
            }
            forms_ok++;
        }
    }

    for (f = 0; f < FAMILY_COUNT; f++)
        for (g = f + 1u; g < FAMILY_COUNT; g++)
            if (memcmp(digest[f], digest[g], HIR_DIGEST_BYTES) == 0) all_distinct = 0;

    printf("  ---- paraphrase convergence: %d/%d forms, %u families\n",
           forms_ok, forms_total, (unsigned)FAMILY_COUNT);
    check(all_converge, "every surface form in a family yields the same canonical meaning");
    check(all_distinct, "no two families collide on the 8-byte semantic digest");
    check(forms_ok == forms_total, "no form in the frozen corpus was refused");
}

static void run_refusals(void)
{
    size_t i;
    int all = 1;
    for (i = 0; i < REFUSAL_COUNT; i++) {
        herald_unit_t u;
        herald_status_t got = compile_text(REFUSAL[i].text, &u);
        if (got != REFUSAL[i].expect) {
            printf("  FAIL  \"%s\": expected %s, got %s\n", REFUSAL[i].text,
                   herald_status_name(REFUSAL[i].expect), herald_status_name(got));
            all = 0;
            continue;
        }
        if (got != HERALD_OK && u.meaning.op != HIR_OP_NONE && u.status != HERALD_OK) {
            /* a refused unit may hold partial slots, but never a usable op+digest */
        }
        if (got != HERALD_OK) {
            uint8_t zero[HIR_DIGEST_BYTES];
            memset(zero, 0, sizeof zero);
            if (memcmp(u.digest, zero, HIR_DIGEST_BYTES) != 0) {
                printf("  FAIL  \"%s\": refused unit carried a digest\n", REFUSAL[i].text);
                all = 0;
            }
        }
    }
    printf("  ---- refusal corpus: %u cases\n", (unsigned)REFUSAL_COUNT);
    check(all, "every adversarial and out-of-grammar case refuses with the right reason");
}

static void run_gap_privacy(void)
{
    herald_unit_t u;
    const char *text = "avisa o joao que cheguei no aeroporto";
    check(compile_text(text, &u) == HERALD_E_GAP, "an unknown lexeme produces a typed gap");
    check(u.gap.present == 1u, "the gap is reported, not swallowed");
    check(u.gap.token_index == 6u, "the gap names which token could not be represented");
    check(u.gap.lexeme_hash != 0u, "the gap carries a hash so a person can name it later");
    {
        /* the gap struct is fixed-width and holds no pointer into the input */
        size_t s = sizeof u.gap;
        check(s <= 8u, "the gap is eight bytes: an index and a hash, never the bytes");
    }
    {
        herald_unit_t v;
        (void)compile_text("avisa o joao que cheguei no supermercado", &v);
        check(v.gap.lexeme_hash != u.gap.lexeme_hash,
              "different unknown lexemes produce different gap hashes");
    }
}

static void run_canonical_order(void)
{
    herald_unit_t a, b;
    check(compile_text("avisa o joao que cheguei", &a) == HERALD_OK, "recipient-first form compiles");
    check(compile_text("cheguei, avisa o joao", &b) == HERALD_OK, "event-first form compiles");
    check(a.meaning.slot[0].role != b.meaning.slot[0].role,
          "the two forms really do fill the slots in a different order");
    check(memcmp(a.digest, b.digest, HIR_DIGEST_BYTES) == 0,
          "word order does not survive into the canonical form: the digest is the same");
    {
        uint8_t wa[HIR_WIRE_BYTES], wb[HIR_WIRE_BYTES];
        check(hir_encode_wire(&a.meaning, wa) == HIR_OK &&
              hir_encode_wire(&b.meaning, wb) == HIR_OK &&
              memcmp(wa, wb, HIR_WIRE_BYTES) == 0,
              "and the two forms produce byte-identical frames on air");
    }
}

static void run_provenance_invariance(void)
{
    herald_unit_t explicit_op, inferred_op;
    check(compile_text("avisa o joao que cheguei", &explicit_op) == HERALD_OK, "explicit operation compiles");
    check(compile_text("joao, cheguei", &inferred_op) == HERALD_OK, "inferred operation compiles");
    check(inferred_op.prov.op_was_inferred == 1u && explicit_op.prov.op_was_inferred == 0u,
          "provenance records that one operation was inferred");
    check(memcmp(explicit_op.digest, inferred_op.digest, HIR_DIGEST_BYTES) == 0,
          "provenance does not change the meaning: same digest");
    check(explicit_op.prov.token_count != inferred_op.prov.token_count,
          "the two forms really did differ at the surface");
}

static void run_hir_contract(void)
{
    herald_unit_t u;
    hir_t back;
    uint8_t wire[HIR_WIRE_BYTES];
    uint8_t d1[HIR_DIGEST_BYTES], d2[HIR_DIGEST_BYTES];

    check(compile_text("avisa a maria que to atrasado 10 minutos", &u) == HERALD_OK,
          "a three-slot meaning compiles");
    check(hir_encode_wire(&u.meaning, wire) == HIR_OK, "meaning encodes to the 24-byte wire form");
    check(hir_decode_wire(wire, &back) == HIR_OK, "wire form decodes");
    check(hir_digest(&u.meaning, d1) == HIR_OK && hir_digest(&back, d2) == HIR_OK &&
          memcmp(d1, d2, HIR_DIGEST_BYTES) == 0,
          "wire round trip preserves the digest exactly");
    check(HIR_ONAIR_BYTES == 34u && HIR_WIRE_BYTES == 24u,
          "a meaning on air is 34 bytes, unchanged from HCP Tier 1");

    {
        uint8_t tampered[HIR_WIRE_BYTES];
        hir_t junk;
        memcpy(tampered, wire, sizeof tampered);
        tampered[HIR_WIRE_BYTES - 1u] = 0x01u;
        check(hir_decode_wire(tampered, &junk) != HIR_OK,
              "a reserved tail byte carrying data is a covert channel and is refused");
        memcpy(tampered, wire, sizeof tampered);
        tampered[1] |= 0x20u;
        check(hir_decode_wire(tampered, &junk) != HIR_OK,
              "reserved header bits carrying data are refused");
    }
    {
        hir_t h;
        hir_init(&h);
        h.op = HIR_OP_COMUNICAR;
        check(hir_put(&h, HIR_ROLE_QUEM, 256u) == HIR_OK, "a slot is accepted");
        check(hir_put(&h, HIR_ROLE_QUEM, 256u) == HIR_OK, "the same slot again is idempotent");
        check(h.slot_count == 1u, "repetition does not grow the meaning");
        check(hir_put(&h, HIR_ROLE_QUEM, 257u) == HIR_E_DUPLICATE_ROLE,
              "a second, different filler for one role is a conflict, not an overwrite");
    }
}

static void run_boundaries(void)
{
    herald_unit_t u;
    char big[HERALD_TEXT_MAX + 8];
    char nul[32];

    check(herald_compile(NULL, 4u, &u) == HERALD_E_ARG, "null text is refused");
    check(herald_compile("x", 1u, NULL) == HERALD_E_ARG, "null output is refused");
    check(compile_text("", &u) == HERALD_E_EMPTY, "empty text is refused");
    check(compile_text("   ,,,  ", &u) == HERALD_E_EMPTY, "punctuation-only text is refused");

    memset(big, 'a', sizeof big);
    check(herald_compile(big, sizeof big, &u) == HERALD_E_OVERFLOW,
          "oversized input is refused before tokenisation");

    memcpy(nul, "avisa o joao\0que cheguei", 24);
    check(herald_compile(nul, 24u, &u) == HERALD_E_BYTE,
          "an embedded NUL never silently truncates the sentence");

    {
        const char bad[] = { 'a', 'v', 'i', 's', 'a', ' ', (char)0xffu, 0 };
        check(herald_compile(bad, 7u, &u) == HERALD_E_BYTE,
              "a high byte outside the accepted encoding is refused, not folded away");
    }
    check(compile_text("avisa o joao #cheguei", &u) == HERALD_E_BYTE,
          "an unknown ASCII byte is refused rather than quietly treated as a space");
    check(compile_text("avisa o joao ~ cheguei", &u) == HERALD_E_BYTE,
          "no unlisted character may vanish and leave a valid sentence behind");
    {
        /* accent folding is real reduction, not byte equality */
        herald_unit_t a, b;
        check(compile_text("avisa o joão que cheguei", &a) == HERALD_OK &&
              compile_text("avisa o joao que cheguei", &b) == HERALD_OK &&
              memcmp(a.digest, b.digest, HIR_DIGEST_BYTES) == 0,
              "UTF-8 accents fold to the same symbol as their ASCII form");
    }
}

static void run_authority(void)
{
    herald_unit_t send, ask;
    check(compile_text("avisa o joao que cheguei", &send) == HERALD_OK, "a send compiles");
    check(send.prov.requires_confirmation == 1u,
          "anything that leaves the wrist requires physical confirmation");
    check(compile_text("cade o joao", &ask) == HERALD_OK, "a question compiles");
    check(ask.prov.requires_confirmation == 0u,
          "a local question is not a send and does not demand confirmation");
    check(hir_requires_confirmation(&send.meaning) == 1 &&
          hir_requires_confirmation(&ask.meaning) == 0,
          "the confirmation rule lives in the meaning, not in the caller");
    {
        herald_unit_t sos;
        check(compile_text("socorro", &sos) == HERALD_OK, "socorro compiles");
        check(sos.meaning.urgency == (uint8_t)HIR_URG_SOCORRO,
              "socorro forces the highest urgency");
        check(sos.prov.requires_confirmation == 1u,
              "even socorro leaves only on a deliberate physical gesture");
    }
}

/* Machine-readable line for tools/test_hsca_corpus.py; the frozen corpus in
 * research/ must agree with what the compiler actually produces. */
static void emit_corpus(void)
{
    size_t f, k;
    for (f = 0; f < FAMILY_COUNT; f++) {
        for (k = 0; k < 8u && FAMILY[f].form[k]; k++) {
            herald_unit_t u;
            herald_status_t st = compile_text(FAMILY[f].form[k], &u);
            unsigned i;
            printf("CORPUS\t%s\t%s\t", FAMILY[f].name, herald_status_name(st));
            for (i = 0; i < HIR_DIGEST_BYTES; i++) printf("%02x", u.digest[i]);
            printf("\t%s\n", FAMILY[f].form[k]);
        }
    }
    for (f = 0; f < REFUSAL_COUNT; f++) {
        herald_unit_t u;
        herald_status_t st = compile_text(REFUSAL[f].text, &u);
        printf("CORPUS\t_refusal\t%s\t0000000000000000\t%s\n",
               herald_status_name(st), REFUSAL[f].text);
    }
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--corpus") == 0) {
        emit_corpus();
        return 0;
    }
    printf("--- herald: intent compiler, convergence and refusal ---\n");
    run_convergence();
    run_refusals();
    run_gap_privacy();
    run_canonical_order();
    run_provenance_invariance();
    run_hir_contract();
    run_boundaries();
    run_authority();
    printf("  ---- lexicon: %u entries, %u closed symbols\n",
           herald_lexicon_entries(), herald_lexicon_symbols());
    printf("HSCA HERALD: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
