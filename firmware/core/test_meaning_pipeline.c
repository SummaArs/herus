#include "meaning_pipeline.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *name)
{
    if (condition) printf("  PASS  %s\n", name);
    else { printf("  FAIL  %s\n", name); failures++; }
}

int main(void)
{
    int pt = babel_lang("pt");
    int ja = babel_lang("ja");
    meaning_result_t result;
    uint8_t corrupted[AE_FRAME];
    uint8_t payload[AE_PAYLOAD];
    ae_report_t report;
    size_t i;
    meaning_status_t status;

    check(pt >= 0 && ja >= 0, "pacote possui portugues e japones");
    status = meaning_roundtrip("emergencia estou aqui", 21u,
                               (uint8_t)pt, (uint8_t)ja, 7u, &result);
    check(status == MEANING_OK, "intencao atravessa Babel-HIR-Aether-HIR-Babel");
    check(result.input.meaning.version == HIR_VERSION &&
          hir_equal(&result.input.meaning, &result.received) &&
          hir_equal(&result.received, &result.output.meaning),
          "o significado canonico permanece identico");
    check(result.transport.seq == (7u & 3u) && result.transport.corrected == 0u,
          "relatorio preserva sequencia limitada e quadro limpo");
    check(result.rendered_len > 0u && result.rendered[result.rendered_len] == '\0',
          "o receptor renderiza uma forma valida no idioma de destino");
    check(result.authority_granted == 0u,
          "transporte semantico nao concede autoridade");

    memcpy(corrupted, result.frame, sizeof corrupted);
    corrupted[3] ^= 0x11u;
    corrupted[14] ^= 0x22u;
    corrupted[27] ^= 0x44u;
    check(ae_unpack(corrupted, payload, &report) != AE_OK,
          "corrupcao acima da correcao nao vira significado");

    for (i = 0u; i < sizeof result.frame; i++) {
        if (result.frame[i] != 0u) break;
    }
    check(i < sizeof result.frame, "o quadro produzido possui conteudo observavel");

    status = meaning_roundtrip("texto que nao pertence ao vocabulario", 35u,
                               (uint8_t)pt, (uint8_t)ja, 8u, &result);
    check(status != MEANING_OK, "entrada fora do dominio e recusada");
    status = meaning_roundtrip("emergencia estou aqui", 21u,
                               (uint8_t)pt, 255u, 9u, &result);
    check(status == MEANING_E_ARG, "idioma de destino desconhecido e recusado");

    if (failures) {
        printf("MEANING PIPELINE: FAIL %d\n", failures);
        return 1;
    }
    printf("MEANING PIPELINE: PASS 7 invariantes, 0 fail\n");
    return 0;
}
