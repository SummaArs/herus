/* board_t3s3.h — THE ONE FILE TO EDIT IF YOUR HARDWARE IS DIFFERENT.
 *
 * Everything hardware-specific about Herus is in this file. Port to a different
 * board, or move to the Phase-4 PCB, by changing numbers here and nothing else.
 *
 * ========================= READ THIS BEFORE FLASHING =========================
 *
 * LilyGO has shipped several T3-S3 pinouts under the same name. A wrong BUSY or
 * DIO1 gives a silent failure that looks EXACTLY like a dead radio, and it will
 * cost you an afternoon — docs/03-BUILD-GUIDE.md warns about this and it is worth
 * repeating here, at the point of use.
 *
 * So do not trust this table. Verify it, in ninety seconds:
 *
 *   1. Find the schematic for YOUR board revision (silkscreen, back of the PCB).
 *   2. Compare the six SX1262 pins below against it.
 *   3. Flash, and run `selftest` on the console. It reports which of the four
 *      plausible bring-up faults you have, by name, with the fix.
 *
 * The values below are LilyGO's published T3-S3 v1.x map. They are a starting
 * point, not an authority.
 * ============================================================================
 */
#ifndef HERUS_BOARD_T3S3_H
#define HERUS_BOARD_T3S3_H

/* ---------------- SX1262 ---------------- */
#define PIN_LORA_SCK      5
#define PIN_LORA_MISO     3
#define PIN_LORA_MOSI     6
#define PIN_LORA_NSS      7
#define PIN_LORA_RST      8
#define PIN_LORA_BUSY    34
#define PIN_LORA_DIO1    33      /* the interrupt line; also the deep-sleep wake */

/* SPI clock. The SX1262 accepts 16 MHz; 8 MHz costs nothing measurable at 34-byte
 * frames and is far more forgiving of the dupont wiring you will use on day one. */
#define LORA_SPI_HZ       8000000

/* ---------------- TCXO ----------------
 * If `selftest` reports XOSC_START, this is the flag to change. A module with a
 * plain crystal instead of a TCXO will fail every calibration until this is 0. */
#define BOARD_HAS_TCXO    1
#define BOARD_TCXO_VOLT   SX_TCXO_1_8V
#define BOARD_TCXO_DELAY_US 5000

/* DIO2 drives the antenna switch on every SX1262 module I know of. If your board
 * uses an external switch on a GPIO instead, set this to 0 and drive it in
 * hal_esp32s3.c — and expect the selftest's noise-floor check to catch it if you
 * get it wrong. */
#define BOARD_DIO2_RF_SWITCH 1

/* ---------------- radio defaults ----------------
 * 915 MHz sits in the upper ANATEL/FCC sub-band (915-928). The lower Brazilian
 * sub-band is 902-907.5; the 907.5-915 gap is not ours to use. See
 * firmware/net/region.h for the compiled-in profile that enforces this. */
#define BOARD_FREQ_HZ     915000000u
#define BOARD_TX_DBM      22       /* must be 14, 17, 20 or 22 — datasheet rows */

/* ---------------- other peripherals ---------------- */
#define PIN_I2C_SDA      18        /* shared bus; verify board revision first */
#define PIN_I2C_SCL      17        /* shared bus; verify board revision first */
#ifndef BOARD_HAS_HAPTIC_I2C
#define BOARD_HAS_HAPTIC_I2C 0     /* remain disabled until schematic + fixture gate */
#endif
#ifndef PIN_HAPTIC_ENABLE
#define PIN_HAPTIC_ENABLE -1       /* no verified DRV2605L ENABLE pin yet */
#endif
#ifndef PIN_HAPTIC_ENABLE_VALID
#define PIN_HAPTIC_ENABLE_VALID 0
#endif
#ifndef HAPTIC_I2C_HZ
#define HAPTIC_I2C_HZ 100000
#endif
#ifndef HA_I2C_TIMEOUT_MS
#define HA_I2C_TIMEOUT_MS 100
#endif
#define PIN_OLED_RST     21
#define PIN_LED          37
#define PIN_BUTTON        0        /* BOOT button: push-to-talk on the devkit */
#define PIN_VBAT_ADC      1

/* ---------------- what changes on the Phase-4 PCB ----------------
 * Nothing above except the pin numbers, plus these two:
 *   - BOARD_TX_DBM drops to 14 if the antenna is capsule-internal, because the
 *     -8 dBi realised gain means the PA is heating your wrist, not radiating.
 *   - PIN_LORA_BUSY must land on an RTC-capable GPIO if you ever want the radio to
 *     wake the MCU from deep sleep through BUSY rather than DIO1.
 */
#endif /* HERUS_BOARD_T3S3_H */
