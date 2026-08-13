#include "../stub.h"
#include "FreeRTOS.h"
static inline void vTaskDelay(int t) { (void)t; }
static inline BaseType_t xTaskCreate(void (*f)(void*), const char*n, int s, void*p, int pr, TaskHandle_t*h)
{ (void)f;(void)n;(void)s;(void)p;(void)pr;(void)h; return pdTRUE; }
