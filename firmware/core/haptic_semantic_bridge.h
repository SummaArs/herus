#ifndef HERUS_HAPTIC_SEMANTIC_BRIDGE_H
#define HERUS_HAPTIC_SEMANTIC_BRIDGE_H

#include "haptic_language.h"
#include "semantic_compiler.h"

#include <stdint.h>

typedef struct {
    hl_event_t event;
    uint8_t confirmation_required;
    uint8_t abstained;
    uint8_t actionable;
} hs_signal_t;

/* Numeric presentation only: this bridge never authorizes execution or send. */
int hs_from_compiler(const sc_unit_t *unit, const sc_bridge_result_t *bridge,
                     hs_signal_t *out);

#endif /* HERUS_HAPTIC_SEMANTIC_BRIDGE_H */
