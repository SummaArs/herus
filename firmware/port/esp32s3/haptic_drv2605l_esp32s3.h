#ifndef HERUS_HAPTIC_DRV2605L_ESP32S3_H
#define HERUS_HAPTIC_DRV2605L_ESP32S3_H

#include "../../core/haptic_adapter.h"

#include <stdint.h>

typedef enum {
    HT_OK = 0,
    HT_E_ARG = -1,
    HT_E_UNWIRED = -2,
    HT_E_I2C = -3,
    HT_E_GPIO = -4,
    HT_E_STATE = -5
} ht_status_t;

/* Initializes only the I2C bus/device and keeps the hardware enable low. */
int herus_haptic_target_init(ha_device_t *device);

/* Probe is electrical presence only; it is not waveform or semantic evidence. */
int herus_haptic_target_probe(void);

/* Explicit physical gate. No adapter playback calls this automatically. */
int herus_haptic_target_enable(int enabled);

/* Disables the hardware and releases the target bus when supported. */
int herus_haptic_target_shutdown(void);

#endif /* HERUS_HAPTIC_DRV2605L_ESP32S3_H */
