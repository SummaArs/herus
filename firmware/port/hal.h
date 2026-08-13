/* hal.h — everything the portable half of Herus needs from a platform.
 *
 * Eight functions. That is the whole contract, and keeping it this small is why
 * firmware/core and firmware/net run unmodified on a Mac, on an ESP32-S3, and on
 * whatever the v2 silicon turns out to be.
 *
 * If you find yourself wanting to add a ninth, check first whether the thing you
 * want belongs above the line instead. Every function added here has to be
 * written again for every port, forever.
 */
#ifndef HERUS_HAL_H
#define HERUS_HAL_H

#include <stdint.h>
#include <stddef.h>

/* Monotonic milliseconds since boot. Must not jump backwards; the session rate
 * limiter and Beat both assume monotonicity (and both defend against a reset
 * anyway, because "must not" is not "cannot"). */
uint64_t hal_millis(void);

/* Microsecond-resolution counter for the field log and latency measurement. */
uint64_t hal_micros(void);

void hal_delay_ms(uint32_t ms);

/* Cryptographically strong random bytes. On the ESP32-S3 this is the hardware
 * RNG with the radio active (which is what makes it a true RNG rather than a
 * PRNG — see the note in hal_esp32s3.c: reading it with radios off is a
 * documented footgun in the ESP-IDF reference manual).
 * Returns 0 on success. A failure here must abort key generation, never fall
 * back to something weaker. */
int  hal_random(void *out, size_t len);

/* Persistent storage for the domain seed, ratchet state and provisioning. Keys
 * are short strings. Returns bytes read, or -1. */
int  hal_nvs_get(const char *key, void *out, size_t max);
int  hal_nvs_set(const char *key, const void *data, size_t len);

/* Log line. Deliberately not printf-shaped: a format string in a firmware log is
 * a stack-overflow waiting for a %s that points at nothing. */
void hal_log(const char *s);

/* Deep sleep for `ms`, waking early if the radio asserts DIO1. Returns the
 * actual elapsed milliseconds. On a host port this is just a delay. */
uint32_t hal_sleep_until_radio(uint32_t ms);

#endif /* HERUS_HAL_H */
