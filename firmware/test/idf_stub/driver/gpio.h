#include "../stub.h"
#define GPIO_MODE_OUTPUT 2
#define GPIO_MODE_INPUT 1
#define GPIO_PULLUP_ENABLE 1
#define GPIO_INTR_HIGH_LEVEL 5
typedef struct { uint64_t pin_bit_mask; int mode, pull_up_en, pull_down_en, intr_type; } gpio_config_t;
static inline esp_err_t gpio_config(const gpio_config_t *c) { (void)c; return ESP_OK; }
static inline void gpio_set_level(int p, int v) { (void)p;(void)v; }
static inline int  gpio_get_level(int p) { (void)p; return 0; }
static inline esp_err_t gpio_wakeup_enable(int p, int t) { (void)p;(void)t; return ESP_OK; }
static inline void esp_rom_delay_us(uint32_t us) { (void)us; }
