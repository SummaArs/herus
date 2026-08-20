#include "haptic_drv2605l_esp32s3.h"

#include "board_t3s3.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include <string.h>

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_device;
static int s_bus_ready;
static int s_device_ready;
static int s_enable_ready;

static int target_write(void *context, uint8_t address7, uint8_t reg,
                        const uint8_t *data, size_t length)
{
    i2c_master_dev_handle_t device = context;
    uint8_t buffer[1u + HA_WAVEFORM_SLOTS];
    if (!device || address7 != HA_DRV2605L_ADDR7 || !data || length == 0u ||
        length > HA_WAVEFORM_SLOTS)
        return -1;
    buffer[0] = reg;
    memcpy(&buffer[1], data, length);
    return i2c_master_transmit(device, buffer, length + 1u, HA_I2C_TIMEOUT_MS) ==
                   ESP_OK
               ? 0
               : -1;
}

static int target_read(void *context, uint8_t address7, uint8_t reg,
                       uint8_t *data, size_t length)
{
    i2c_master_dev_handle_t device = context;
    if (!device || address7 != HA_DRV2605L_ADDR7 || !data || length == 0u)
        return -1;
    return i2c_master_transmit_receive(device, &reg, 1u, data, length,
                                       HA_I2C_TIMEOUT_MS) == ESP_OK
               ? 0
               : -1;
}

int herus_haptic_target_init(ha_device_t *device)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = 0,
    };
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = HA_DRV2605L_ADDR7,
        .scl_speed_hz = HAPTIC_I2C_HZ,
        .scl_wait_us = 0,
    };
    ha_bus_t bus;
    ha_config_t config;
    if (!device || !BOARD_HAS_HAPTIC_I2C || !PIN_HAPTIC_ENABLE_VALID)
        return HT_E_UNWIRED;
    s_bus = NULL;
    s_device = NULL;
    s_bus_ready = 0;
    s_device_ready = 0;
    s_enable_ready = 0;
    gpio_set_level(PIN_HAPTIC_ENABLE, 0);
    if (i2c_new_master_bus(&bus_config, &s_bus) != ESP_OK) return HT_E_I2C;
    s_bus_ready = 1;
    if (i2c_master_bus_add_device(s_bus, &device_config, &s_device) != ESP_OK) {
        i2c_del_master_bus(s_bus);
        s_bus_ready = 0;
        return HT_E_I2C;
    }
    s_device_ready = 1;
    s_enable_ready = 1;
    bus.write = target_write;
    bus.read = target_read;
    bus.context = s_device;
    config.address7 = HA_DRV2605L_ADDR7;
    config.active_mode = HA_MODE_ACTIVE;
    config.standby_mode = HA_MODE_STANDBY;
    if (ha_init(device, &bus, &config) != HA_OK) {
        (void)herus_haptic_target_shutdown();
        return HT_E_STATE;
    }
    return HT_OK;
}

int herus_haptic_target_probe(void)
{
    if (!s_bus_ready || !s_device_ready || !s_enable_ready)
        return HT_E_STATE;
    gpio_set_level(PIN_HAPTIC_ENABLE, 1);
    if (i2c_master_probe(s_bus, HA_DRV2605L_ADDR7, HA_I2C_TIMEOUT_MS) != ESP_OK) {
        gpio_set_level(PIN_HAPTIC_ENABLE, 0);
        return HT_E_I2C;
    }
    gpio_set_level(PIN_HAPTIC_ENABLE, 0);
    return HT_OK;
}

int herus_haptic_target_enable(int enabled)
{
    if (!s_enable_ready) return HT_E_STATE;
    gpio_set_level(PIN_HAPTIC_ENABLE, enabled ? 1 : 0);
    return HT_OK;
}

int herus_haptic_target_shutdown(void)
{
    if (s_enable_ready) gpio_set_level(PIN_HAPTIC_ENABLE, 0);
    if (s_device_ready && i2c_master_bus_rm_device(s_device) != ESP_OK)
        return HT_E_I2C;
    if (s_bus_ready && i2c_del_master_bus(s_bus) != ESP_OK)
        return HT_E_I2C;
    s_device_ready = 0;
    s_bus_ready = 0;
    s_enable_ready = 0;
    return HT_OK;
}
