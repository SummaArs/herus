/* Minimal ESP-IDF surface, ONLY to syntax-check firmware/port/esp32s3 on a host.
 * This is not an emulator and it never runs: it exists so a typo in app_main.c is
 * found on a Mac in one second instead of on a bench in twenty minutes. If the
 * real IDF disagrees with a signature here, the real IDF is right — fix the stub. */
#ifndef HERUS_IDF_STUB_H
#define HERUS_IDF_STUB_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_NVS_NO_FREE_PAGES 0x1100
#define ESP_ERROR_CHECK(x) do { (void)(x); } while (0)
#define ESP_LOGI(t, ...) do { (void)(t); printf(__VA_ARGS__); } while (0)
#define ESP_LOGW(t, ...) do { (void)(t); printf(__VA_ARGS__); } while (0)
#define ESP_LOGE(t, ...) do { (void)(t); printf(__VA_ARGS__); } while (0)
#define CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG 1
#endif
