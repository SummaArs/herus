#include "stub.h"
#define MALLOC_CAP_DEFAULT 1
#define MALLOC_CAP_INTERNAL 2
static inline size_t heap_caps_get_free_size(int c) { (void)c; return 200000; }
