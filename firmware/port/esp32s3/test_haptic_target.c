#include "haptic_drv2605l_esp32s3.h"
#include "board_t3s3.h"

#include <stdio.h>

static int failures;

static void check(int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (!condition) failures++;
}

int main(void)
{
    ha_device_t device;
    int result = herus_haptic_target_init(&device);
#if BOARD_HAS_HAPTIC_I2C && PIN_HAPTIC_ENABLE_VALID
    check(result == HT_OK, "verified test override initializes the target bus");
    check(herus_haptic_target_probe() == HT_OK,
          "stub probe reports only the electrical API path");
    check(herus_haptic_target_enable(0) == HT_OK,
          "explicit disable is available through the target port");
    check(herus_haptic_target_shutdown() == HT_OK,
          "shutdown disables and releases the target bus");
#else
    check(result == HT_E_UNWIRED,
          "default board map refuses unverified haptic hardware");
    check(herus_haptic_target_probe() == HT_E_STATE,
          "uninitialized target cannot claim electrical presence");
    check(herus_haptic_target_enable(0) == HT_E_STATE,
          "uninitialized target cannot toggle an unknown enable pin");
    check(herus_haptic_target_shutdown() == HT_OK,
          "shutdown remains harmless when target is disabled");
#endif
    printf("HAPTIC TARGET: %d pass, %d fail\n", 4 - failures, failures);
    return failures ? 1 : 0;
}
