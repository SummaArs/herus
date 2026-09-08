/*
 * test_babel.c — invariantes do Babel em C, no aparelho que embarca.
 *
 * As suites em Python provam mais casos (340 mil idas-e-voltas no portao do
 * Loom, 681 mil comparacoes diferenciais, 350 mil tentativas adversariais).
 * Esta suite existe por outra razao: ela roda no MESMO build do firmware, sem
 * Python, e e ela que a campanha de mutantes usa para decidir se um controle e
 * carregador ou decoracao. Um invariante que so vive num script nao protege o
 * artefato.
 */
#include "babel.h"
#include <stdio.h>
#include <string.h>

static int pass_count, fail_count;

static void check(int cond, const char *what)
{
    if (cond) { pass_count++; printf("  PASS  %s\n", what); }
    else      { fail_count++; printf("  FAIL  %s\n", what); }
}

static babel_status_t comp(const char *tag, const char *text, babel_unit_t *u)
{
    int l = babel_lang(tag);
    if (l < 0) return BABEL_E_LANG;
    return babel_compile(text, strlen(text), (uint8_t)l, u);
}

static int digest_of(const char *tag, const char *text, uint8_t out[8])
{
    babel_unit_t u;
    if (comp(tag, text, &u) != BABEL_OK) return 0;
    memcpy(out, u.digest, 8);
    return 1;
}

/* --------------------------------------------------------------------------
 * 1. Convergencia de IDIOMA. Oito frases, oito linguas, um significado.
 *    Se este teste passa, "24 bytes atravessam a fronteira do idioma" deixa de
 *    ser slogan e passa a ser um fato verificado no build.
 * -------------------------------------------------------------------------- */
static const struct { const char *tag; const char *text; } SAME_MEANING[] = {
    { "pt", "avisa maria urgente nao cheguei em casa agora" },
    { "en", "tell maria urgent not arrived at home now" },
    { "es", "avisa maria urgente no llegue en casa ahora" },
    { "fr", "dis a maria urgent pas arrive a maison maintenant" },
    { "it", "avvisa maria urgente non arrivato a casa adesso" },
    { "de", "sag maria dringend nicht angekommen in zuhause jetzt" },
    { "ja", "\343\203\236\343\203\252\343\202\242\343\201\253\344\273\212\345\256\266"
            "\343\201\247\347\267\212\346\200\245\343\201\253\347\235\200\343\201\204"
            "\343\201\246\343\201\204\343\201\252\343\201\204\343\201\250\344\274\235"
            "\343\201\210\343\201\246" },
    { "zh", "\345\221\212\350\257\211\347\216\233\344\270\275\344\272\232\347\264\247"
            "\346\200\245\347\232\204\347\216\260\345\234\250\345\256\266\351\207\214"
            "\346\262\241\345\210\260" },
};

/* Parafrases dentro do mesmo idioma. */
static const char *const PARAPHRASE_PT[] = {
    "avisa joao cheguei", "avise o joao que cheguei", "manda pro joao que to aqui",
    "diz pro joao que estou aqui", "fala pro joao: acabei de chegar",
};

int main(void)
{
    babel_unit_t u, v;
    uint8_t d0[8], d[8];
    char text[BABEL_RENDER_MAX];
    size_t n;
    uint8_t i;

    printf("--- babel: interlingua fechada, %u idiomas, pacote %s ---\n",
           (unsigned)babel_lang_count(), LOOM_PACK_DIGEST);

    /* ---- o pacote gerado e o codigo estao acoplados ---------------- */
    check(strlen(LOOM_PACK_DIGEST) == 32, "o pacote gerado declara o digest da fonte");
    check(babel_lang_count() == LOOM_LANG_COUNT, "a contagem de idiomas vem do pacote");
    check(babel_lang(SAME_MEANING[0].tag) == 0, "o primeiro idioma do pacote e pt");
    check(babel_lang("xx") < 0, "um idioma inexistente e recusado, nao adivinhado");

    /* ---- 1. convergencia de idioma --------------------------------- */
    check(digest_of(SAME_MEANING[0].tag, SAME_MEANING[0].text, d0),
          "a frase de referencia compila em portugues");
    {
        int all = 1;
        for (i = 1; i < (uint8_t)(sizeof SAME_MEANING / sizeof SAME_MEANING[0]); i++) {
            if (!digest_of(SAME_MEANING[i].tag, SAME_MEANING[i].text, d) ||
                memcmp(d0, d, 8) != 0) {
                printf("        divergiu em %s\n", SAME_MEANING[i].tag);
                all = 0;
            }
        }
        check(all, "oito idiomas, um significado: digest identico byte a byte");
    }

    /* ---- 2. convergencia de parafrase ------------------------------ */
    check(digest_of("pt", PARAPHRASE_PT[0], d0), "a parafrase base compila");
    {
        int all = 1;
        for (i = 1; i < (uint8_t)(sizeof PARAPHRASE_PT / sizeof PARAPHRASE_PT[0]); i++) {
            if (!digest_of("pt", PARAPHRASE_PT[i], d) || memcmp(d0, d, 8) != 0) {
                printf("        divergiu em %s\n", PARAPHRASE_PT[i]);
                all = 0;
            }
        }
        check(all, "cinco maneiras de dizer a mesma coisa, um unico digest");
    }

    /* ---- 3. ida-e-volta em todos os idiomas ------------------------ */
    /* A frase de ida carrega urgencia E negacao de proposito: se o render
     * deixar cair um marcador, a ida-e-volta acusa. */
    check(comp("pt", "avisa maria urgente nao cheguei em casa agora", &u) == BABEL_OK,
          "a frase de ida compila");
    {
        int all = 1;
        for (i = 0; i < babel_lang_count(); i++) {
            babel_unit_t back;
            if (babel_render(&u.meaning, i, text, sizeof text, &n) != BABEL_OK) {
                printf("        nao renderiza em %s\n", babel_lang_tag(i));
                all = 0; continue;
            }
            if (babel_compile(text, n, i, &back) != BABEL_OK ||
                !hir_equal(&back.meaning, &u.meaning)) {
                printf("        ida-e-volta quebrou em %s: %s\n",
                       babel_lang_tag(i), text);
                all = 0;
            }
        }
        check(all, "compile(render(h,L),L) == h em todos os idiomas do pacote");
    }

    /* ---- 4. negacao lexical onde o idioma exige -------------------- */
    check(comp("pt", "avisa maria nao cheguei", &u) == BABEL_OK,
          "uma negacao compila em portugues");
    check(u.meaning.polarity == 1u, "a polaridade entrou no significado");
    {
        int lja = babel_lang("ja");
        babel_unit_t back;
        check(babel_render(&u.meaning, (uint8_t)lja, text, sizeof text, &n) == BABEL_OK,
              "a negacao renderiza em japones");
        /* 着いていない: forma negativa LEXICAL, nao o marcador solto. Um
         * marcador produziria 着いたない, que ninguem escreveria. */
        check(strstr(text, "\347\235\200\343\201\204\343\201\246\343\201\204"
                           "\343\201\252\343\201\204") != NULL,
              "japones nega por morfologia, nao por marcador solto");
        check(babel_compile(text, n, (uint8_t)lja, &back) == BABEL_OK &&
              back.meaning.polarity == 1u &&
              hir_equal(&back.meaning, &u.meaning),
              "a forma negativa lexical volta a ser o mesmo significado");
    }

    /* ---- 5. recusas tipadas ---------------------------------------- */
    check(comp("pt", "avisa joao pizza", &u) == BABEL_E_GAP,
          "um lexema fora do vocabulario e lacuna tipada, nunca descarte");
    check(u.gap_len > 0 && u.gap_hash != 0,
          "a lacuna carrega posicao e hash, nunca os bytes");
    check(comp("pt", "avisa a equipe a senha", &u) == BABEL_E_SENSITIVE,
          "classe protegida recusa antes de qualquer significado");
    check(comp("en", "tell team the password", &u) == BABEL_E_SENSITIVE,
          "classe protegida vale em todo idioma, nao so em portugues");
    /* A pre-varredura existe para isto: a palavra protegida e recusada mesmo
     * quando NENHUMA leitura completa existe. Sem ela, "senha" numa frase que
     * nao fecha sairia como lacuna e a classe protegida passaria batida. */
    check(comp("pt", "avisa joao senha zzqq", &u) == BABEL_E_SENSITIVE,
          "classe protegida vence a lacuna: recusa antes de tentar ler");
    check(comp("pt", "avisa joao automaticamente zzqq", &u) == BABEL_E_AUTHORITY,
          "autoridade vence a lacuna: recusa antes de tentar ler");
    check(comp("pt", "guarda isso automaticamente", &u) == BABEL_E_AUTHORITY,
          "a frase nao pode se conceder autoridade");
    check(comp("pt", "automaticamente guarda isso", &u) == BABEL_E_AUTHORITY,
          "a recusa de autoridade nao depende da ordem das palavras");
    check(comp("pt", "cheguei", &u) == BABEL_E_INCOMPLETE,
          "COMUNICAR sem destinatario e recusa estrutural");
    check(u.missing_role == HIR_ROLE_QUEM, "a recusa diz QUAL papel falta");
    check(comp("pt", "avisa joao maria cheguei", &u) == BABEL_E_AMBIGUOUS,
          "dois destinatarios em conflito recusam as duas leituras");
    check(comp("pt", "", &u) == BABEL_E_EMPTY, "entrada vazia e recusa");
    check(comp("pt", "\xff\xfe cheguei", &u) == BABEL_E_BYTE,
          "byte fora da codificacao aceita e recusa");
    {
        char big[BABEL_TEXT_MAX + 40];
        memset(big, 'a', sizeof big - 1);
        big[sizeof big - 1] = '\0';
        check(comp("pt", big, &u) == BABEL_E_OVERFLOW,
              "texto acima do orcamento e recusa, nunca truncamento");
    }
    check(babel_compile("cheguei", 7, 200, &u) == BABEL_E_LANG,
          "idioma fora da tabela e recusa");

    /* ---- 5b. dominancia: ruido nunca acrescenta significado -------- */
    check(comp("pt", "emergencia estou aqui", &u) == BABEL_OK,
          "'emergencia estou aqui' tem duas leituras e uma DOMINA a outra");
    check(u.readings == 2u, "as duas leituras foram de fato exploradas");
    check(u.meaning.op == HIR_OP_SOCORRO &&
          u.meaning.slot_count == 2u,
          "vence a leitura que preserva o evento, nao a que o joga no ruido");
    check(comp("pt", "avisa amiga muito calor", &u) == BABEL_OK &&
          u.meaning.slot_count == 3u,
          "'muito calor' preserva a quantidade em vez de descarta-la");
    {
        /* Alemao: "komme spater" e ATRASADO, e tambem "komme"(a caminho) +
         * "spater"(depois). Sao dois eventos diferentes e nenhum contem o
         * outro: nao ha dominancia, e as duas sao recusadas. */
        check(comp("de", "abbrechen komme spater", &u) == BABEL_E_AMBIGUOUS,
              "duas leituras que se contradizem recusam as duas, sem escolher");
    }

    /* ---- 6. idempotencia do papel, e nao ambiguidade --------------- */
    check(comp("pt", "avisa joao cheguei cheguei", &u) == BABEL_OK,
          "repetir o MESMO preenchimento nao e ambiguidade");
    check(digest_of("pt", "avisa joao cheguei", d0) &&
          memcmp(u.digest, d0, 8) == 0,
          "e produz exatamente o mesmo significado");

    /* ---- 6b. orcamento de leituras: falha fechado ------------------ */
    /* "to aqui" tem duas leituras (a frase, e duas palavras de ruido) e as
     * duas dao o mesmo significado. Seis repeticoes dao 2^6 = 64 leituras e
     * estouram o orcamento. Nao da para PROVAR unicidade dentro do orcamento,
     * entao recusa — mesmo sabendo que todas as leituras concordavam. Falhar
     * fechado vale mais que estar quase certo. */
    check(comp("pt", "avisa joao to aqui to aqui", &u) == BABEL_OK,
          "duas leituras concordantes ainda produzem significado");
    check(comp("pt", "avisa joao to aqui to aqui to aqui to aqui to aqui to aqui",
               &u) == BABEL_E_AMBIGUOUS,
          "estourar o orcamento de leituras recusa, nao arrisca");

    /* ---- 7. fronteira de palavra e atomicidade do numeral ---------- */
    check(comp("pt", "avisa joao cheguei em casaco", &u) == BABEL_E_GAP,
          "'casa' nunca casa dentro de 'casaco'");
    {
        int lzh = babel_lang("zh");
        babel_unit_t a, b;
        /* 告诉玛丽亚到了10 x ...1 0 : o numeral latino e ATOMICO dentro de uma
         * escrita sem espaco, senao '10' se leria como '1' e '0'. */
        static const char *const ZH10  = "\345\221\212\350\257\211\347\216\233"
            "\344\270\275\344\272\232\345\210\260\344\272\206" "10";
        static const char *const ZH1_0 = "\345\221\212\350\257\211\347\216\233"
            "\344\270\275\344\272\232\345\210\260\344\272\206" "1 0";
        check(babel_compile(ZH10, strlen(ZH10), (uint8_t)lzh, &a) == BABEL_OK,
              "em escrita sem espaco, '10' e um numeral so");
        check(babel_compile(ZH1_0, strlen(ZH1_0), (uint8_t)lzh, &b) == BABEL_E_AMBIGUOUS,
              "'1 0' sao dois numeros em conflito, e isso recusa");
        /* Aqui esta o que a atomicidade do numeral compra de verdade:
         * "15分钟" (quinze minutos) contem "1" e "5分钟" (cinco minutos). Sem
         * tratar o numeral como atomico, a frase tem duas leituras validas com
         * QUANDO diferente — nenhuma domina a outra — e viraria AMBIGUOUS. */
        {
            static const char *const ZH15 = "\345\221\212\350\257\211\347\216\233"
                "\344\270\275\344\272\232\345\210\260\344\272\206" "15"
                "\345\210\206\351\222\237";
            babel_unit_t c15;
            check(babel_compile(ZH15, strlen(ZH15), (uint8_t)lzh, &c15) == BABEL_OK &&
                  c15.meaning.slot_count == 3u,
                  "'15 minutos' nao se le como '1' + '5 minutos'");
        }
    }

    /* ---- 8. separador fino: dois numerais nunca colam -------------- */
    {
        hir_t h;
        babel_unit_t back;
        int lzh = babel_lang("zh"), lja = babel_lang("ja");
        hir_init(&h);
        h.op = HIR_OP_SOCORRO;
        h.urgency = HIR_URG_SOCORRO;
        hir_put(&h, HIR_ROLE_QUEM, 274u);    /* PER.GRUPO_1 -> "grupo 1"  */
        hir_put(&h, HIR_ROLE_QUANDO, 772u);  /* TIME.HORA_1 -> "1 hora"   */
        check(babel_render(&h, (uint8_t)lzh, text, sizeof text, &n) == BABEL_OK &&
              babel_compile(text, n, (uint8_t)lzh, &back) == BABEL_OK &&
              hir_equal(&back.meaning, &h),
              "'grupo 1' seguido de '1 hora' nao produz o numeral colado '11'");
        check(babel_render(&h, (uint8_t)lja, text, sizeof text, &n) == BABEL_OK &&
              babel_compile(text, n, (uint8_t)lja, &back) == BABEL_OK &&
              hir_equal(&back.meaning, &h),
              "e o mesmo vale em japones");
    }

    /* ---- 8b. simbolo de outro pacote vira LACUNA, nao aproximacao -- */
    {
        /* A faixa 1536-2047 e a faixa de DOMINIO: um pacote de campo, de
         * marinha, de defesa civil. Um aparelho que nao instalou aquele pacote
         * recebe 24 bytes perfeitamente validos e NAO consegue vesti-los na
         * propria lingua. A resposta certa e dizer isso. A resposta errada
         * seria escolher a palavra mais parecida — que e exatamente o que este
         * projeto recusa em todo lugar. */
        hir_t h;
        hir_init(&h);
        h.op = HIR_OP_COMUNICAR;
        hir_put(&h, HIR_ROLE_QUEM, 257u);      /* PER.MARIA, do nucleo      */
        hir_put(&h, HIR_ROLE_O_QUE, 1600u);    /* faixa de dominio, ausente */
        check(hir_validate(&h) == HIR_OK,
              "um significado com simbolo de dominio e estruturalmente valido");
        {
            int all_gap = 1;
            for (i = 0; i < babel_lang_count(); i++) {
                if (babel_render(&h, i, text, sizeof text, &n) != BABEL_E_GAP)
                    all_gap = 0;
            }
            check(all_gap,
                  "simbolo de um pacote nao instalado vira LACUNA em todo idioma");
        }
        /* e o mesmo significado, sem o simbolo desconhecido, renderiza */
        hir_init(&h);
        h.op = HIR_OP_COMUNICAR;
        hir_put(&h, HIR_ROLE_QUEM, 257u);
        hir_put(&h, HIR_ROLE_O_QUE, 1u);
        check(babel_render(&h, 0, text, sizeof text, &n) == BABEL_OK,
              "e o mesmo significado com simbolo conhecido renderiza normal");
    }

    /* ---- 9. envelope: 24 bytes, sempre --------------------------- */
    {
        uint8_t wire[HIR_WIRE_BYTES];
        hir_t round;
        check(comp("pt", "avisa maria urgente nao cheguei em casa agora", &u)
              == BABEL_OK, "a frase mais carregada do teste compila");
        check(hir_encode_wire(&u.meaning, wire) == HIR_OK,
              "o significado serializa em exatamente 24 bytes");
        check(hir_decode_wire(wire, &round) == HIR_OK &&
              hir_equal(&round, &u.meaning),
              "e volta dos 24 bytes sem perder nada");
        check(HIR_ONAIR_BYTES == 34u,
              "no ar sao 34 bytes: 2 de endereco efemero, 24, 8 de tag");
    }

    /* ---- 10. determinismo e ausencia de estado -------------------- */
    check(comp("pt", "avisa ana esperando no portao norte", &u) == BABEL_OK &&
          comp("pt", "avisa ana esperando no portao norte", &v) == BABEL_OK &&
          hir_equal(&u.meaning, &v.meaning) &&
          memcmp(u.digest, v.digest, 8) == 0,
          "duas compilacoes iguais dao o mesmo significado, sem estado entre elas");

    /* ---- 11. proveniencia nao muda o significado ------------------ */
    {
        uint8_t a[8], b[8];
        babel_unit_t x, y;
        check(comp("pt", "cheguei em casa avisa joao", &x) == BABEL_OK &&
              comp("pt", "avisa joao cheguei em casa", &y) == BABEL_OK,
              "a mesma mensagem em duas ordens compila");
        memcpy(a, x.digest, 8); memcpy(b, y.digest, 8);
        check(memcmp(a, b, 8) == 0,
              "a ordem das palavras nao entra no significado canonico");
    }

    /* ---- 12. dobramento ------------------------------------------ */
    {
        char folded[BABEL_TEXT_MAX + 1];
        size_t fn = 0;
        static const char *const RAW = "CHEGUEI, T\303\224 aqui!!!";
        check(babel_fold(RAW, strlen(RAW), folded,
                         sizeof folded, &fn) == BABEL_OK &&
              strcmp(folded, "cheguei to aqui") == 0,
              "caixa, acento e pontuacao caem na forma canonica");
        check(babel_fold("?", 1, folded, sizeof folded, &fn) == BABEL_OK && fn == 0,
              "dobrar para o vazio e resultado valido, nao erro");
        check(babel_fold("\xff", 1, folded, sizeof folded, &fn) == BABEL_E_BYTE,
              "byte invalido e recusa tipada, distinta do vazio");
        check(babel_fold("cheguei", 7, folded, 3, &fn) == BABEL_E_OVERFLOW,
              "buffer pequeno e recusa, nunca escrita fora do limite");
    }

    printf("BABEL: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
