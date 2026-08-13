#include "stub.h"
typedef int nvs_handle_t;
#define NVS_READONLY 0
#define NVS_READWRITE 1
static inline esp_err_t nvs_open(const char *n, int m, nvs_handle_t *h) { (void)n;(void)m;*h=0; return ESP_OK; }
static inline esp_err_t nvs_get_blob(nvs_handle_t h, const char *k, void *o, size_t *l) { (void)h;(void)k;(void)o;(void)l; return ESP_OK; }
static inline esp_err_t nvs_set_blob(nvs_handle_t h, const char *k, const void *d, size_t l) { (void)h;(void)k;(void)d;(void)l; return ESP_OK; }
static inline esp_err_t nvs_commit(nvs_handle_t h) { (void)h; return ESP_OK; }
static inline void nvs_close(nvs_handle_t h) { (void)h; }
