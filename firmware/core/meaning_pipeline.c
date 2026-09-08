/* meaning_pipeline.c — semantic transport proof, not an action grant. */
#include "meaning_pipeline.h"

#include <string.h>

static meaning_status_t from_babel(babel_status_t status)
{
    return status == BABEL_OK ? MEANING_OK : MEANING_E_BABEL;
}

meaning_status_t meaning_roundtrip(const char *text, size_t len,
                                   uint8_t input_lang, uint8_t output_lang,
                                   uint8_t seq, meaning_result_t *out)
{
    uint8_t payload[AE_PAYLOAD];
    uint8_t received_payload[AE_PAYLOAD];
    size_t rendered_len = 0u;
    babel_status_t bs;
    hir_status_t hs;
    ae_status_t as;

    if (!text || !out || len > BABEL_TEXT_MAX || input_lang >= BABEL_LANGS ||
        output_lang >= BABEL_LANGS) {
        return MEANING_E_ARG;
    }
    memset(out, 0, sizeof *out);
    out->authority_granted = 0u;

    bs = babel_compile(text, len, input_lang, &out->input);
    if (bs != BABEL_OK) return from_babel(bs);

    hs = hir_encode_wire(&out->input.meaning, payload);
    if (hs != HIR_OK) return MEANING_E_HIR;

    as = ae_pack(payload, seq, out->frame);
    if (as != AE_OK) return MEANING_E_AETHER;

    as = ae_unpack(out->frame, received_payload, &out->transport);
    if (as != AE_OK) return MEANING_E_AETHER;

    hs = hir_decode_wire(received_payload, &out->received);
    if (hs != HIR_OK) return MEANING_E_HIR;
    if (!hir_equal(&out->input.meaning, &out->received)) return MEANING_E_LOSS;

    bs = babel_render(&out->received, output_lang, out->rendered,
                      sizeof out->rendered, &rendered_len);
    if (bs != BABEL_OK) return from_babel(bs);
    out->rendered_len = rendered_len;

    bs = babel_compile(out->rendered, rendered_len, output_lang, &out->output);
    if (bs != BABEL_OK) return from_babel(bs);
    if (!hir_equal(&out->received, &out->output.meaning)) return MEANING_E_LOSS;

    return MEANING_OK;
}

const char *meaning_status_name(meaning_status_t status)
{
    switch (status) {
    case MEANING_OK:       return "OK";
    case MEANING_E_ARG:    return "ARG";
    case MEANING_E_BABEL:  return "BABEL";
    case MEANING_E_HIR:    return "HIR";
    case MEANING_E_AETHER: return "AETHER";
    case MEANING_E_LOSS:   return "LOSS";
    default:               return "UNKNOWN";
    }
}
