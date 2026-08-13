#include "../stub.h"
#include "FreeRTOS.h"
static inline QueueHandle_t xQueueCreate(int n, size_t sz) { (void)n;(void)sz; return (QueueHandle_t)1; }
static inline BaseType_t xQueueSend(QueueHandle_t q, const void *i, int t) { (void)q;(void)i;(void)t; return pdTRUE; }
static inline BaseType_t xQueueReceive(QueueHandle_t q, void *i, int t) { (void)q;(void)i;(void)t; return pdFALSE; }
