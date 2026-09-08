/* meaning_pipeline.h — host/target chain: Babel -> HIR -> Aether -> HIR -> Babel.
 *
 * This module proves transport and semantic preservation only. It does not grant
 * authority, confirm a human action, or call a critical sink. Any execution path
 * must still pass HSCA and the sink-specific assurance contract.
 */
#ifndef HERUS_MEANING_PIPELINE_H
#define HERUS_MEANING_PIPELINE_H

#include <stddef.h>
#include <stdint.h>
#include "babel.h"
#include "aether.h"

#define MEANING_PIPELINE_TEXT_MAX BABEL_RENDER_MAX

typedef enum {
    MEANING_OK       = 0,
    MEANING_E_ARG    = 1,
    MEANING_E_BABEL  = 2,
    MEANING_E_HIR    = 3,
    MEANING_E_AETHER = 4,
    MEANING_E_LOSS   = 5
} meaning_status_t;

typedef struct {
    babel_unit_t input;
    hir_t received;
    babel_unit_t output;
    ae_report_t transport;
    uint8_t frame[AE_FRAME];
    char rendered[MEANING_PIPELINE_TEXT_MAX];
    size_t rendered_len;
    uint8_t authority_granted;
} meaning_result_t;

/*
 * Round-trip a bounded meaning through the open Aether profile.
 *
 * The function guarantees only that a supported meaning can be encoded,
 * transported through an integrity-checked frame and reconstructed in another
 * language. `authority_granted` is always zero. This is not an execution API.
 */
meaning_status_t meaning_roundtrip(const char *text, size_t len,
                                   uint8_t input_lang, uint8_t output_lang,
                                   uint8_t seq, meaning_result_t *out);

const char *meaning_status_name(meaning_status_t status);

#endif
