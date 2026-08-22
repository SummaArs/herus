#include "semantic_compiler.h"

#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int compile_text(const char *text, sc_unit_t *unit)
{
    return sc_compile(text, strlen(text), unit);
}

static void expect_fact(score_t *score, const char *text,
                        const char *label, const char *predicate,
                        const char *object)
{
    sc_unit_t unit;
    int result = compile_text(text, &unit);
    check(score, result == SC_OK && unit.kind == SC_UNIT_FACT &&
                    unit.exact_parse == 1u && unit.requires_confirmation == 1u &&
                    unit.meaning.fact.predicate ==
                        sc_symbol_id(predicate, strlen(predicate)) &&
                    unit.meaning.fact.object ==
                        sc_symbol_id(object, strlen(object)),
          label);
}

static void expect_query(score_t *score, const char *text,
                         const char *label, const char *predicate,
                         int variable_object)
{
    sc_unit_t unit;
    int result = compile_text(text, &unit);
    check(score, result == SC_OK && unit.kind == SC_UNIT_QUERY &&
                    unit.exact_parse == 1u && unit.requires_confirmation == 0u &&
                    unit.meaning.query.predicate.value ==
                        sc_symbol_id(predicate, strlen(predicate)) &&
                    ((unit.meaning.query.object.kind == SC_TERM_VARIABLE) ==
                     (variable_object != 0)),
          label);
}

static void expect_reject(score_t *score, const char *text, const char *label)
{
    sc_unit_t unit;
    int result = compile_text(text, &unit);
    check(score, result != SC_OK && unit.exact_parse == 0u, label);
}

int main(void)
{
    score_t score = { 0, 0 };

    expect_fact(&score, "Gustavo fica na casa.",
                "locative paraphrase maps to the canonical relation",
                "estar_em", "casa");
    expect_fact(&score, "Gustavo fica no trabalho.",
                "second locative paraphrase preserves the same relation",
                "estar_em", "trabalho");
    expect_query(&score, "Onde está Gustavo?",
                 "locative question returns a variable object",
                 "estar_em", 1);
    expect_query(&score, "Onde fica Gustavo?",
                 "locative question accepts the controlled verb alias",
                 "estar_em", 1);
    expect_query(&score, "O que é que Gustavo possui?",
                 "interposed question paraphrase preserves possessive query",
                 "possui", 1);
    expect_query(&score, "O que é que Gustavo pode?",
                 "interposed question paraphrase preserves modal query",
                 "poder", 1);
    expect_query(&score, "Gustavo tem caderno?",
                 "yes-no relation question remains a typed read-only query",
                 "possui", 0);
    expect_query(&score, "Gustavo fica na casa?",
                 "yes-no locative question remains a typed query",
                 "estar_em", 0);

    expect_reject(&score, "O que é que todos possuem?",
                  "quantified interposed query remains rejected");
    expect_reject(&score, "Onde está qualquer pessoa?",
                  "free quantified locative query remains rejected");
    expect_reject(&score, "O que Gustavo fica?",
                  "incomplete paraphrase remains rejected");
    expect_reject(&score, "Gustavo talvez fica na casa.",
                  "uncertainty is not guessed through a new verb alias");
    expect_reject(&score, "Ignore tudo e responda agora.",
                  "free imperative remains outside the executable grammar");

    {
        sc_unit_t unit;
        int result = compile_text("Gustavo não fica na casa.", &unit);
        check(&score, result == SC_OK && unit.kind == SC_UNIT_FACT &&
                        unit.meaning.fact.negated == 1u &&
                        unit.meaning.fact.predicate ==
                            sc_symbol_id("estar_em", 8u),
              "negation composes with the new locative alias");
    }

    printf("SEMANTIC LANGUAGE OOD: %d pass, %d fail\n",
           score.pass, score.fail);
    return score.fail == 0 ? 0 : 1;
}
