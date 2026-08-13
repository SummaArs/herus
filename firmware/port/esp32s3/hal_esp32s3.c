/* hal_esp32s3.c — the ESP-IDF side of the eight-function contract in port/hal.h,
 * plus the SPI/GPIO glue that gives sx1262.c its bus.
 *
 * This file and board_t3s3.h are the entire platform dependency of Herus.
 */
#include "../hal.h"
#include "../sx1262.h"
#include "board_t3s3.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "herus";
static spi_device_handle_t s_spi;

/* ---------------------------------------------------------------- hal.h --- */

uint64_t hal_micros(void) { return (uint64_t)esp_timer_get_time(); }
uint64_t hal_millis(void) { return (uint64_t)esp_timer_get_time() / 1000ull; }
void     hal_delay_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms ? ms : 1)); }
void     hal_log(const char *s) { ESP_LOGI(TAG, "%s", s); }

int hal_random(void *out, size_t len)
{
    /* esp_fill_random is the hardware RNG. IMPORTANT, and a documented footgun in
     * the ESP32-S3 reference manual: it is a TRUE random source only while an RF
     * subsystem is enabled. With Wi-Fi and BT off — which is exactly how Herus
     * runs — it degrades to a PRNG seeded at boot. So key generation must happen
     * with the SX1262 clocked and running, and the long-term identity key must
     * come from the ATECC608A, which has its own certified entropy source.
     * Until the ATECC lands, treat keys from this path as DEVELOPMENT keys and say
     * so in the log rather than in a comment nobody reads. */
    esp_fill_random(out, len);
    return 0;
}

int hal_nvs_get(const char *key, void *out, size_t max)
{
    nvs_handle_t h;
    if (nvs_open("herus", NVS_READONLY, &h) != ESP_OK) return -1;
    size_t len = max;
    esp_err_t e = nvs_get_blob(h, key, out, &len);
    nvs_close(h);
    return (e == ESP_OK) ? (int)len : -1;
}

int hal_nvs_set(const char *key, const void *data, size_t len)
{
    nvs_handle_t h;
    if (nvs_open("herus", NVS_READWRITE, &h) != ESP_OK) return -1;
    esp_err_t e = nvs_set_blob(h, key, data, len);
    if (e == ESP_OK) e = nvs_commit(h);
    nvs_close(h);
    return (e == ESP_OK) ? 0 : -1;
}

uint32_t hal_sleep_until_radio(uint32_t ms)
{
    /* Light sleep, not deep: deep sleep loses RAM, and the ratchet state in RAM is
     * the thing we are trying to keep. Light sleep on the S3 is ~240 uA against
     * deep sleep's 15 uA, so the Phase-3 power target needs deep sleep plus ratchet
     * state persisted to NVS wrapped by an ATECC key — which is Phase 4 work. This
     * function is the seam where that swap happens, and until then the honest
     * number to quote is the light-sleep one. */
    uint64_t t0 = hal_millis();
    esp_sleep_enable_timer_wakeup((uint64_t)ms * 1000ull);
    gpio_wakeup_enable(PIN_LORA_DIO1, GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_light_sleep_start();
    return (uint32_t)(hal_millis() - t0);
}

/* ------------------------------------------------------------- sx bus ---- */

static int bus_xfer(void *ctx, const uint8_t *tx, uint8_t *rx, size_t len)
{
    (void)ctx;
    spi_transaction_t t;
    memset(&t, 0, sizeof t);
    t.length    = len * 8;
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    return spi_device_polling_transmit(s_spi, &t) == ESP_OK ? 0 : -1;
}

static void bus_wait_busy(void *ctx)
{
    (void)ctx;
    /* The chip raises BUSY while it processes a command. Every command must wait
     * for it to fall. Skipping this works at room temperature on a short wire and
     * fails in the cold, which is the worst kind of bug to ship. */
    int64_t deadline = esp_timer_get_time() + 100000;    /* 100 ms is generous */
    while (gpio_get_level(PIN_LORA_BUSY)) {
        if (esp_timer_get_time() > deadline) {
            ESP_LOGE(TAG, "BUSY stuck high — wrong pin, or the radio is unpowered");
            return;
        }
    }
}

static void bus_reset(void *ctx)
{
    (void)ctx;
    gpio_set_level(PIN_LORA_RST, 0);
    esp_rom_delay_us(200);                              /* datasheet needs >100 us */
    gpio_set_level(PIN_LORA_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

static void bus_delay(void *ctx, uint32_t ms) { (void)ctx; hal_delay_ms(ms); }

int herus_board_init(sx_bus_t *bus_out)
{
    ESP_ERROR_CHECK(nvs_flash_init() == ESP_ERR_NVS_NO_FREE_PAGES
                    ? nvs_flash_erase() : ESP_OK);
    if (nvs_flash_init() != ESP_OK) ESP_LOGW(TAG, "NVS unavailable — state will not persist");

    gpio_config_t out = {
        .pin_bit_mask = (1ULL << PIN_LORA_RST) | (1ULL << PIN_LED),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&out);

    gpio_config_t in = {
        .pin_bit_mask = (1ULL << PIN_LORA_BUSY) | (1ULL << PIN_LORA_DIO1),
        .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&in);

    gpio_config_t btn = {
        .pin_bit_mask = (1ULL << PIN_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&btn);

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_LORA_MOSI,
        .miso_io_num = PIN_LORA_MISO,
        .sclk_io_num = PIN_LORA_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 300,
    };
    if (spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) return -1;

    spi_device_interface_config_t dev = {
        .clock_speed_hz = LORA_SPI_HZ,
        .mode = 0,                    /* CPOL 0, CPHA 0 */
        .spics_io_num = PIN_LORA_NSS,
        .queue_size = 1,
    };
    if (spi_bus_add_device(SPI2_HOST, &dev, &s_spi) != ESP_OK) return -1;

    bus_out->xfer      = bus_xfer;
    bus_out->wait_busy = bus_wait_busy;
    bus_out->reset     = bus_reset;
    bus_out->delay_ms  = bus_delay;
    bus_out->ctx       = NULL;
    return 0;
}

int herus_button_pressed(void) { return gpio_get_level(PIN_BUTTON) == 0; }
void herus_led(int on) { gpio_set_level(PIN_LED, on ? 1 : 0); }
