#ifndef HERUS_IDF_I2C_MASTER_STUB_H
#define HERUS_IDF_I2C_MASTER_STUB_H

#include "../stub.h"

typedef void *i2c_master_bus_handle_t;
typedef void *i2c_master_dev_handle_t;

typedef struct {
    int i2c_port;
    int sda_io_num;
    int scl_io_num;
    int clk_source;
    int glitch_ignore_cnt;
    int intr_priority;
    struct {
        unsigned enable_internal_pullup : 1;
        unsigned allow_pd : 1;
    } flags;
} i2c_master_bus_config_t;

typedef struct {
    int dev_addr_length;
    uint16_t device_address;
    uint32_t scl_speed_hz;
    uint32_t scl_wait_us;
} i2c_device_config_t;

#define I2C_NUM_0 0
#define I2C_NUM_1 1
#define I2C_CLK_SRC_DEFAULT 0
#define I2C_ADDR_BIT_LEN_7 7
#define I2C_ADDR_BIT_LEN_10 10

static inline esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *config,
                                           i2c_master_bus_handle_t *bus)
{
    (void)config;
    if (bus) *bus = (void *)1;
    return ESP_OK;
}

static inline esp_err_t i2c_master_bus_add_device(
    i2c_master_bus_handle_t bus, const i2c_device_config_t *config,
    i2c_master_dev_handle_t *device)
{
    (void)bus;
    (void)config;
    if (device) *device = (void *)2;
    return ESP_OK;
}

static inline esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t device)
{
    (void)device;
    return ESP_OK;
}

static inline esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus)
{
    (void)bus;
    return ESP_OK;
}

static inline esp_err_t i2c_master_transmit(i2c_master_dev_handle_t device,
                                            const uint8_t *data, size_t length,
                                            int timeout_ms)
{
    (void)device;
    (void)data;
    (void)length;
    (void)timeout_ms;
    return ESP_OK;
}

static inline esp_err_t i2c_master_transmit_receive(
    i2c_master_dev_handle_t device, const uint8_t *write_data,
    size_t write_length, uint8_t *read_data, size_t read_length,
    int timeout_ms)
{
    (void)device;
    (void)write_data;
    (void)write_length;
    (void)read_data;
    (void)read_length;
    (void)timeout_ms;
    return ESP_OK;
}

static inline esp_err_t i2c_master_probe(i2c_master_bus_handle_t bus,
                                         uint16_t address7, int timeout_ms)
{
    (void)bus;
    (void)address7;
    (void)timeout_ms;
    return ESP_OK;
}

#endif /* HERUS_IDF_I2C_MASTER_STUB_H */
