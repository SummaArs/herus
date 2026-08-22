/* generative_haptic_bridge.h — bounded presentation seam for local generation. */
#ifndef HERUS_GENERATIVE_HAPTIC_BRIDGE_H
#define HERUS_GENERATIVE_HAPTIC_BRIDGE_H

#include "generative_core.h"
#include "haptic_language.h"

#include <stdint.h>

typedef struct {
    hl_event_t event;
    uint8_t abstained;
    uint8_t confirmation_required;
    uint8_t actionable;
} gh_signal_t;

/* Maps a generated candidate to a numeric haptic event only.
 * It never executes, persists, transmits or grants authority. */
int gh_from_result(const gc_result_t *result, gh_signal_t *out);

#endif /* HERUS_GENERATIVE_HAPTIC_BRIDGE_H */
