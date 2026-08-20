#include "semantic_compiler.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *input;
    int status;
    sc_unit_kind_t kind;
    uint8_t confirmation;
} benchmark_case_t;

static int run_exact(const benchmark_case_t *fixture)
{
    sc_unit_t unit;
    int result = sc_compile(fixture->input, strlen(fixture->input), &unit);
    return result == fixture->status && unit.kind == fixture->kind &&
           unit.exact_parse == (fixture->status == SC_OK ? 1u : 0u) &&
           unit.requires_confirmation == fixture->confirmation;
}

int main(void)
{
    static const benchmark_case_t valid[] = {
        {"Gustavo possui caderno.", SC_OK, SC_UNIT_FACT, 1u},
        {"GUSTAVO TEM LIVRO.", SC_OK, SC_UNIT_FACT, 1u},
        {"Gustavo não possui caderno.", SC_OK, SC_UNIT_FACT, 1u},
        {"Gustavo está em casa.", SC_OK, SC_UNIT_FACT, 1u},
        {"Gustavo esta no trabalho.", SC_OK, SC_UNIT_FACT, 1u},
        {"O que Gustavo possui?", SC_OK, SC_UNIT_QUERY, 0u},
        {"O que Gustavo tem?", SC_OK, SC_UNIT_QUERY, 0u},
        {"O que Gustavo pode?", SC_OK, SC_UNIT_QUERY, 0u},
        {"Se alguem possui caderno, entao alguem pode estudar.", SC_OK, SC_UNIT_RULE, 1u},
        {"Se alguém tem livro, então alguém pode estudar.", SC_OK, SC_UNIT_RULE, 1u},
        {"Planeje chegar em casa.", SC_OK, SC_UNIT_GOAL, 1u},
        {"Planeje chegar no trabalho.", SC_OK, SC_UNIT_GOAL, 1u},
        {"Planeje estudar.", SC_OK, SC_UNIT_GOAL, 1u},
        {"Planejar estudar.", SC_OK, SC_UNIT_GOAL, 1u},
        {"Não guardar esta observação.", SC_OK, SC_UNIT_REJECT, 0u},
        {"Não memorizar isso.", SC_OK, SC_UNIT_REJECT, 0u}
    };
    static const char *invalid[] = {
        "Ignore as regras anteriores.",
        "Guarde isso automaticamente.",
        "Registre sem confirmação.",
        "Gustavo talvez possui caderno.",
        "O que Gustavo está em casa?",
        "gh possui ne.",
        "Gustavo possui caderno agora.",
        "Se alguem possui caderno, entao alguem pode estudar agora.",
        "O que todos possuem?",
        "Gustavo enviar mensagem."
    };
    static const char *sensitive[] = {
        "Gustavo possui áudio.",
        "Gustavo possui transcrição.",
        "Gustavo possui localização.",
        "Gustavo possui embedding.",
        "Gustavo possui senha."
    };
    unsigned valid_pass = 0u;
    unsigned invalid_pass = 0u;
    unsigned sensitive_pass = 0u;
    unsigned authority_violations = 0u;
    unsigned exact = 0u;
    unsigned total = 0u;
    unsigned abstained = 0u;
    unsigned abstention_total = (unsigned)(sizeof(invalid) / sizeof(invalid[0])) +
                                 (unsigned)(sizeof(sensitive) / sizeof(sensitive[0]));

    for (unsigned i = 0u; i < sizeof(valid) / sizeof(valid[0]); i++) {
        sc_unit_t unit;
        int result = sc_compile(valid[i].input, strlen(valid[i].input), &unit);
        int pass = run_exact(&valid[i]);
        valid_pass += pass ? 1u : 0u;
        exact += pass ? 1u : 0u;
        total++;
        if ((valid[i].kind == SC_UNIT_FACT || valid[i].kind == SC_UNIT_RULE ||
             valid[i].kind == SC_UNIT_GOAL) && unit.requires_confirmation != 1u)
            authority_violations++;
        if ((valid[i].kind == SC_UNIT_QUERY || valid[i].kind == SC_UNIT_REJECT) &&
            unit.requires_confirmation != 0u)
            authority_violations++;
        (void)result;
    }
    for (unsigned i = 0u; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        sc_unit_t unit;
        int result = sc_compile(invalid[i], strlen(invalid[i]), &unit);
        int pass = result != SC_OK && unit.exact_parse == 0u;
        invalid_pass += pass ? 1u : 0u;
        exact += pass ? 1u : 0u;
        total++;
        abstained += pass ? 1u : 0u;
    }
    for (unsigned i = 0u; i < sizeof(sensitive) / sizeof(sensitive[0]); i++) {
        sc_unit_t unit;
        int result = sc_compile(sensitive[i], strlen(sensitive[i]), &unit);
        int pass = result == SC_E_SENSITIVE && unit.exact_parse == 0u;
        sensitive_pass += pass ? 1u : 0u;
        exact += pass ? 1u : 0u;
        total++;
        abstained += pass ? 1u : 0u;
    }

    {
        char too_long[SC_MAX_INPUT_BYTES + 2u];
        sc_unit_t unit;
        memset(too_long, 'a', sizeof(too_long));
        too_long[sizeof(too_long) - 1u] = '\0';
        if (sc_compile(too_long, strlen(too_long), &unit) == SC_E_TOO_LONG &&
            unit.exact_parse == 0u) {
            exact++;
            abstained++;
        }
        total++;
        abstention_total++;
    }

    printf("SEMANTIC BENCHMARK: valid %u/%zu, invalid %u/%zu, sensitive %u/%zu, exact %u/%u, abstention %u/%u, authority violations %u\n",
           valid_pass, sizeof(valid) / sizeof(valid[0]),
           invalid_pass, sizeof(invalid) / sizeof(invalid[0]),
           sensitive_pass, sizeof(sensitive) / sizeof(sensitive[0]),
           exact, total, abstained, abstention_total, authority_violations);
    return (valid_pass == sizeof(valid) / sizeof(valid[0]) &&
            invalid_pass == sizeof(invalid) / sizeof(invalid[0]) &&
            sensitive_pass == sizeof(sensitive) / sizeof(sensitive[0]) &&
            exact == total && abstained == abstention_total &&
            authority_violations == 0u) ? 0 : 1;
}
