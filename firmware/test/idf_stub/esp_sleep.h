#include "stub.h"
static inline esp_err_t esp_sleep_enable_timer_wakeup(uint64_t us) { (void)us; return ESP_OK; }
static inline esp_err_t esp_sleep_enable_gpio_wakeup(void) { return ESP_OK; }
static inline esp_err_t esp_light_sleep_start(void) { return ESP_OK; }
