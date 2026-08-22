#ifndef HERUS_HAPTIC_ADAPTER_H
#define HERUS_HAPTIC_ADAPTER_H

#include "haptic_language.h"

#include <stddef.h>
#include <stdint.h>

#define HA_DRV2605L_ADDR7       0x5au
#define HA_REG_STATUS           0x00u
#define HA_REG_MODE             0x01u
#define HA_REG_WAVEFORM_BASE    0x04u
#define HA_REG_GO               0x0cu
#define HA_WAVEFORM_SLOTS       8u
#define HA_MODE_ACTIVE          0x00u
#define HA_MODE_STANDBY         0x40u
#define HA_GO_BIT               0x01u
#define HA_STATUS_DIAG_BIT      0x08u
#define HA_STATUS_OVERTEMP_BIT  0x02u
#define HA_STATUS_OVERCURRENT_BIT 0x01u

typedef enum {
    HA_STATE_IDLE = 0,
    HA_STATE_VALIDATED,
    HA_STATE_PLAYING,
    HA_STATE_DONE,
    HA_STATE_FAULT,
    HA_STATE_ABORTED
} ha_state_t;

typedef enum {
    HA_OK = 0,
    HA_E_ARG = -1,
    HA_E_STATE = -2,
    HA_E_BUSY = -3,
    HA_E_FRAME = -4,
    HA_E_BUS = -5,
    HA_E_FAULT = -6
} ha_status_t;

typedef struct {
    int (*write)(void *context, uint8_t address7, uint8_t reg,
                 const uint8_t *data, size_t length);
    int (*read)(void *context, uint8_t address7, uint8_t reg,
                uint8_t *data, size_t length);
    void *context;
} ha_bus_t;

typedef struct {
    uint8_t address7;
    uint8_t active_mode;
    uint8_t standby_mode;
} ha_config_t;

typedef struct {
    ha_bus_t bus;
    ha_config_t config;
    ha_state_t state;
    uint32_t transactions;
    uint32_t errors;
} ha_device_t;

int ha_init(ha_device_t *device, const ha_bus_t *bus,
            const ha_config_t *config);
int ha_validate_frame(const hl_encoded_t *encoded,
                      const hl_profile_t *profile);
int ha_play(ha_device_t *device, const hl_encoded_t *encoded,
            const hl_profile_t *profile);
int ha_poll(ha_device_t *device);
int ha_abort(ha_device_t *device);
ha_state_t ha_state(const ha_device_t *device);

#endif /* HERUS_HAPTIC_ADAPTER_H */
