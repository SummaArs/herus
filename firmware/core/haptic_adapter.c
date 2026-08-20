#include "haptic_adapter.h"

#include <string.h>

static int bus_write(ha_device_t *device, uint8_t reg,
                     const uint8_t *data, size_t length)
{
    if (!device || !device->bus.write || !data || length == 0u)
        return HA_E_ARG;
    if (device->bus.write(device->bus.context, device->config.address7,
                          reg, data, length) != 0) {
        device->errors++;
        device->state = HA_STATE_FAULT;
        return HA_E_BUS;
    }
    device->transactions++;
    return HA_OK;
}

static int bus_read(ha_device_t *device, uint8_t reg,
                    uint8_t *data, size_t length)
{
    if (!device || !device->bus.read || !data || length == 0u)
        return HA_E_ARG;
    if (device->bus.read(device->bus.context, device->config.address7,
                         reg, data, length) != 0) {
        device->errors++;
        device->state = HA_STATE_FAULT;
        return HA_E_BUS;
    }
    device->transactions++;
    return HA_OK;
}

int ha_init(ha_device_t *device, const ha_bus_t *bus,
            const ha_config_t *config)
{
    if (!device || !bus || !bus->write || !bus->read || !config ||
        config->address7 > 0x7fu || config->active_mode == config->standby_mode)
        return HA_E_ARG;
    memset(device, 0, sizeof(*device));
    device->bus = *bus;
    device->config = *config;
    device->state = HA_STATE_IDLE;
    return HA_OK;
}

int ha_validate_frame(const hl_encoded_t *encoded,
                      const hl_profile_t *profile)
{
    hl_event_t decoded;
    if (!encoded || !profile || encoded->slot_count > HL_MAX_SLOTS)
        return HA_E_ARG;
    if (hl_decode_with_profile(encoded, profile, &decoded) != HL_OK)
        return HA_E_FRAME;
    if (encoded->slot_count != 6u || encoded->kind[0] != HL_SYM_SYNC ||
        encoded->kind[encoded->slot_count - 1u] != HL_SYM_END)
        return HA_E_FRAME;
    return HA_OK;
}

int ha_play(ha_device_t *device, const hl_encoded_t *encoded,
            const hl_profile_t *profile)
{
    uint8_t mode;
    uint8_t waveform[HA_WAVEFORM_SLOTS];
    uint8_t go = HA_GO_BIT;
    int result;
    if (!device || !encoded || !profile) return HA_E_ARG;
    if (device->state == HA_STATE_PLAYING) return HA_E_BUSY;
    if (device->state == HA_STATE_FAULT) return HA_E_STATE;
    result = ha_validate_frame(encoded, profile);
    if (result != HA_OK) return result;
    device->state = HA_STATE_VALIDATED;
    mode = device->config.active_mode;
    result = bus_write(device, HA_REG_MODE, &mode, 1u);
    if (result != HA_OK) return result;
    memset(waveform, 0, sizeof(waveform));
    for (uint8_t i = 0u; i < encoded->slot_count; i++)
        waveform[i] = encoded->effect_id[i];
    result = bus_write(device, HA_REG_WAVEFORM_BASE, waveform,
                       sizeof(waveform));
    if (result != HA_OK) return result;
    result = bus_write(device, HA_REG_GO, &go, 1u);
    if (result != HA_OK) return result;
    device->state = HA_STATE_PLAYING;
    return HA_OK;
}

int ha_poll(ha_device_t *device)
{
    uint8_t go;
    uint8_t status;
    int result;
    if (!device) return HA_E_ARG;
    if (device->state != HA_STATE_PLAYING) return HA_E_STATE;
    result = bus_read(device, HA_REG_GO, &go, 1u);
    if (result != HA_OK) return result;
    if ((go & HA_GO_BIT) != 0u) return HA_E_BUSY;
    result = bus_read(device, HA_REG_STATUS, &status, 1u);
    if (result != HA_OK) return result;
    if ((status & (HA_STATUS_DIAG_BIT | HA_STATUS_OVERTEMP_BIT |
                   HA_STATUS_OVERCURRENT_BIT)) != 0u) {
        device->state = HA_STATE_FAULT;
        return HA_E_FAULT;
    }
    device->state = HA_STATE_DONE;
    return HA_OK;
}

int ha_abort(ha_device_t *device)
{
    uint8_t mode;
    int result;
    if (!device) return HA_E_ARG;
    if (device->state != HA_STATE_PLAYING &&
        device->state != HA_STATE_VALIDATED)
        return HA_E_STATE;
    mode = device->config.standby_mode;
    result = bus_write(device, HA_REG_MODE, &mode, 1u);
    if (result != HA_OK) return result;
    device->state = HA_STATE_ABORTED;
    return HA_OK;
}

ha_state_t ha_state(const ha_device_t *device)
{
    return device ? device->state : HA_STATE_FAULT;
}
