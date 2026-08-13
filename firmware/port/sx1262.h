/* sx1262.h — Semtech SX1262 driver, written against the command set rather than
 * against a library.
 *
 * WHY OUR OWN DRIVER
 * ------------------
 * Herus needs three things no general-purpose LoRa library gives cleanly at once:
 * implicit-header CRC-off receive (Tier 0.5 exists only for that), autonomous
 * SetRxDutyCycle so the MCU sleeps through empty Beat windows, and a private sync
 * word. RadioLib does two of the three and hides the third. Six hundred lines of
 * command sequencing is cheaper than fighting an abstraction, and it is auditable.
 *
 * WHY IT IS TESTABLE WITHOUT HARDWARE
 * -----------------------------------
 * Every byte reaches the chip through `sx_bus_t`. The proof suite plugs in a
 * recording mock and asserts the things that are ordering-sensitive in the
 * datasheet and therefore easy to get wrong and hard to debug on a bench:
 *   - SetPacketType(LORA) must precede SetModulationParams, or the modulation
 *     bytes are interpreted as GFSK and the radio transmits garbage;
 *   - the frequency word is freq * 2^25 / 32 MHz, and an off-by-a-factor here
 *     looks exactly like a dead antenna;
 *   - PA config and TX power must come from the same row of the datasheet table,
 *     or the PA runs outside its safe operating area;
 *   - the private sync word must be written, or every LoRaWAN gateway in range
 *     wakes up for our traffic.
 * A bench session should be spent on antennas, not on a mistyped opcode.
 */
#ifndef HERUS_SX1262_H
#define HERUS_SX1262_H

#include <stdint.h>
#include <stddef.h>

/* ---------------- the four things a platform must provide ---------------- */
typedef struct {
    /* Full-duplex SPI transfer of `len` bytes. rx may be NULL. Returns 0 on ok. */
    int  (*xfer)(void *ctx, const uint8_t *tx, uint8_t *rx, size_t len);
    /* Block until the chip's BUSY line is low. Every command needs this first;
     * ignoring BUSY is the single most common cause of a driver that works at
     * room temperature and fails at -10 C. */
    void (*wait_busy)(void *ctx);
    /* Pulse NRESET low for at least 100 us, then release. */
    void (*reset)(void *ctx);
    void (*delay_ms)(void *ctx, uint32_t ms);
    void *ctx;
} sx_bus_t;

/* ---------------- configuration ---------------- */
typedef struct {
    uint32_t freq_hz;
    uint8_t  sf;              /* 5..12 */
    uint8_t  bw;              /* SX_BW_* */
    uint8_t  cr;              /* SX_CR_* */
    uint8_t  ldro;            /* 0/1 — off below SF11 */
    int8_t   tx_dbm;          /* 14, 17, 20 or 22 — the datasheet's four rows */
    uint16_t preamble_sym;
    uint8_t  implicit_header; /* 1 for Tier 0.5 */
    uint8_t  crc_on;          /* 0 for Tier 0.5 */
    uint8_t  payload_len;     /* required when implicit_header */
    uint8_t  use_tcxo;        /* 1 if a TCXO hangs off DIO3 */
    uint8_t  tcxo_voltage;    /* SX_TCXO_* */
    uint32_t tcxo_delay_us;
    uint8_t  dio2_as_rf_switch;
    uint8_t  boosted_rx;      /* +2 dB sensitivity for ~2 mA more */
} sx_cfg_t;

typedef struct {
    sx_bus_t bus;
    sx_cfg_t cfg;
    uint8_t  packet_type_set;
} sx1262_t;

/* Bandwidth codes (datasheet table 13-47). 125 kHz is the only one Herus uses;
 * the rest are here so the LAB profile can experiment. */
enum { SX_BW_7  = 0x00, SX_BW_10 = 0x08, SX_BW_15 = 0x01, SX_BW_20 = 0x09,
       SX_BW_31 = 0x02, SX_BW_41 = 0x0A, SX_BW_62 = 0x03, SX_BW_125 = 0x04,
       SX_BW_250 = 0x05, SX_BW_500 = 0x06 };
enum { SX_CR_4_5 = 0x01, SX_CR_4_6 = 0x02, SX_CR_4_7 = 0x03, SX_CR_4_8 = 0x04 };
enum { SX_TCXO_1_6V = 0x00, SX_TCXO_1_7V = 0x01, SX_TCXO_1_8V = 0x02,
       SX_TCXO_2_2V = 0x03, SX_TCXO_2_4V = 0x04, SX_TCXO_2_7V = 0x05,
       SX_TCXO_3_0V = 0x06, SX_TCXO_3_3V = 0x07 };

/* IRQ bits (datasheet table 13-29). */
#define SX_IRQ_TX_DONE      0x0001u
#define SX_IRQ_RX_DONE      0x0002u
#define SX_IRQ_PREAMBLE     0x0004u
#define SX_IRQ_SYNC_VALID   0x0008u
#define SX_IRQ_HEADER_VALID 0x0010u
#define SX_IRQ_HEADER_ERR   0x0020u
#define SX_IRQ_CRC_ERR      0x0040u
#define SX_IRQ_CAD_DONE     0x0080u
#define SX_IRQ_CAD_DETECTED 0x0100u
#define SX_IRQ_TIMEOUT      0x0200u

/* Device error bits from GetDeviceErrors — the field that tells you WHICH of the
 * plausible bring-up mistakes you made. */
#define SX_ERR_RC64K_CALIB  0x0001u
#define SX_ERR_RC13M_CALIB  0x0002u
#define SX_ERR_PLL_CALIB    0x0004u
#define SX_ERR_ADC_CALIB    0x0008u
#define SX_ERR_IMG_CALIB    0x0010u
#define SX_ERR_XOSC_START   0x0020u   /* TCXO misconfigured: the classic */
#define SX_ERR_PLL_LOCK     0x0040u
#define SX_ERR_PA_RAMP      0x0100u

enum { SX_OK = 0, SX_E_BUS = -1, SX_E_ARG = -2, SX_E_TIMEOUT = -3, SX_E_CRC = -4,
       SX_E_SILENT = -5 };

/* Full bring-up: reset, TCXO, calibrate, packet type, modulation, packet params,
 * PA, sync word. Returns SX_OK or a negative code. */
int sx1262_init(sx1262_t *r, const sx_bus_t *bus, const sx_cfg_t *cfg);

/* Reconfigure modulation/packet parameters without a full reset — used to switch
 * between the meaning tiers (SF9, explicit, CRC) and Tier 0.5 (SF9, implicit, no
 * CRC), and between SF9 and SF7 for voice. */
int sx1262_set_mode(sx1262_t *r, uint8_t sf, uint8_t implicit_header,
                    uint8_t crc_on, uint8_t payload_len);

int sx1262_tx(sx1262_t *r, const uint8_t *data, uint8_t len, uint32_t timeout_ms);
int sx1262_rx_continuous(sx1262_t *r);

/* Autonomous duty-cycled receive: the radio alternates RX and sleep by itself and
 * only interrupts on preamble detection, so the MCU stays asleep through every
 * empty Beat window. Waking the MCU 43 200 times a day to run CAD instead would
 * cost ~1.8 mAh/day — more than the radio it was meant to help. */
int sx1262_rx_duty_cycle(sx1262_t *r, uint32_t rx_ms, uint32_t sleep_ms);

int sx1262_sleep(sx1262_t *r, int warm_start);
int sx1262_standby(sx1262_t *r);

uint16_t sx1262_irq(sx1262_t *r);
int sx1262_irq_clear(sx1262_t *r, uint16_t mask);

/* Read a received payload. Returns the length, or a negative code. `rssi` and
 * `snr` are in dBm and dB. A CRC error returns SX_E_CRC — except in
 * implicit/CRC-off mode where there is nothing to check, which is the point. */
int sx1262_read(sx1262_t *r, uint8_t *out, size_t max, int16_t *rssi, int8_t *snr);

uint16_t sx1262_device_errors(sx1262_t *r);
uint8_t  sx1262_status(sx1262_t *r);
int16_t  sx1262_rssi_inst(sx1262_t *r);

/* Bring-up self test. Proves, in order: BUSY responds, SPI reads and writes
 * survive a round trip through a register, the oscillator started, the PLL locks
 * at the configured frequency, and the receiver produces a plausible noise floor.
 * Writes a human-readable verdict into `report`. This is what to run FIRST on a
 * new board — before suspecting the antenna. */
int sx1262_selftest(sx1262_t *r, char *report, size_t report_len);

#endif /* HERUS_SX1262_H */
