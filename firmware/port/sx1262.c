/* sx1262.c — see sx1262.h. Command opcodes and register addresses follow the
 * SX1261-2 datasheet (rev 2.1) chapter 13; each one is named so a review against
 * the datasheet is a text comparison rather than an archaeology project. */
#include "sx1262.h"
#include <string.h>
#include <stdio.h>

/* ---------------- opcodes (datasheet 13.1 - 13.6) ---------------- */
#define OP_SET_SLEEP            0x84
#define OP_SET_STANDBY          0x80
#define OP_SET_TX               0x83
#define OP_SET_RX               0x82
#define OP_STOP_TIMER_ON_PREAMB 0x9F
#define OP_SET_RX_DUTY_CYCLE    0x94
#define OP_SET_REGULATOR_MODE   0x96
#define OP_CALIBRATE            0x89
#define OP_CALIBRATE_IMAGE      0x98
#define OP_SET_PA_CONFIG        0x95
#define OP_SET_DIO_IRQ_PARAMS   0x08
#define OP_GET_IRQ_STATUS       0x12
#define OP_CLR_IRQ_STATUS       0x02
#define OP_SET_DIO2_RF_SWITCH   0x9D
#define OP_SET_DIO3_TCXO        0x97
#define OP_SET_RF_FREQUENCY     0x86
#define OP_SET_PACKET_TYPE      0x8A
#define OP_SET_TX_PARAMS        0x8E
#define OP_SET_MODULATION_PARAMS 0x8B
#define OP_SET_PACKET_PARAMS    0x8C
#define OP_SET_BUFFER_BASE      0x8F
#define OP_SET_LORA_SYMB_TO     0xA0
#define OP_GET_STATUS           0xC0
#define OP_GET_RSSI_INST        0x15
#define OP_GET_RX_BUFFER_STATUS 0x13
#define OP_GET_PACKET_STATUS    0x14
#define OP_GET_DEVICE_ERRORS    0x17
#define OP_CLR_DEVICE_ERRORS    0x07
#define OP_WRITE_REGISTER       0x0D
#define OP_READ_REGISTER        0x1D
#define OP_WRITE_BUFFER         0x0E
#define OP_READ_BUFFER          0x1E

/* ---------------- registers ---------------- */
#define REG_LORA_SYNC_MSB       0x0740
#define REG_LORA_SYNC_LSB       0x0741
#define REG_RANDOM_NUMBER       0x0819
#define REG_RX_GAIN             0x08AC
#define REG_OCP                 0x08E7
#define REG_TX_CLAMP            0x08D8
#define REG_TX_MODULATION       0x0889

/* A private network. Nomenclature trap worth knowing about, because the two
 * numbers appear in different places in this repository and both are correct:
 * docs/02-PROTOCOL.md §3.1 and firmware/ranger/ranger.ino say "sync word 0x12",
 * which is the SX1276 single-byte value and also what RadioLib's
 * setSyncWord(0x12) takes — RadioLib expands it to the 16-bit register pair
 * 0x1424. Writing to the registers directly, as this driver does, means writing
 * 0x1424. Writing the literal 0x12 into 0x0740 would leave the radio near the
 * public LoRaWAN word (0x3444) and every gateway in range would wake up for our
 * traffic. Verified against the ranger's RadioLib path in test_radio R5. */
#define LORA_SYNC_PRIVATE       0x1424u

#define PKT_TYPE_LORA           0x01

static int cmd(sx1262_t *r, uint8_t op, const uint8_t *args, size_t n)
{
    uint8_t buf[16];
    if (n > sizeof buf - 1) return SX_E_ARG;
    buf[0] = op;
    if (n) memcpy(buf + 1, args, n);
    r->bus.wait_busy(r->bus.ctx);
    return r->bus.xfer(r->bus.ctx, buf, NULL, n + 1) ? SX_E_BUS : SX_OK;
}

static int cmd_read(sx1262_t *r, uint8_t op, uint8_t *out, size_t n)
{
    uint8_t tx[16] = {0}, rx[16] = {0};
    if (n + 2 > sizeof tx) return SX_E_ARG;
    tx[0] = op;                       /* op, then NOP bytes while the chip answers */
    r->bus.wait_busy(r->bus.ctx);
    if (r->bus.xfer(r->bus.ctx, tx, rx, n + 2)) return SX_E_BUS;
    memcpy(out, rx + 2, n);
    return SX_OK;
}

static int reg_write(sx1262_t *r, uint16_t addr, const uint8_t *data, size_t n)
{
    uint8_t buf[16];
    if (n + 3 > sizeof buf) return SX_E_ARG;
    buf[0] = OP_WRITE_REGISTER;
    buf[1] = (uint8_t)(addr >> 8);
    buf[2] = (uint8_t)(addr & 0xff);
    memcpy(buf + 3, data, n);
    r->bus.wait_busy(r->bus.ctx);
    return r->bus.xfer(r->bus.ctx, buf, NULL, n + 3) ? SX_E_BUS : SX_OK;
}

static int reg_read(sx1262_t *r, uint16_t addr, uint8_t *data, size_t n)
{
    uint8_t tx[20] = {0}, rx[20] = {0};
    if (n + 4 > sizeof tx) return SX_E_ARG;
    tx[0] = OP_READ_REGISTER;
    tx[1] = (uint8_t)(addr >> 8);
    tx[2] = (uint8_t)(addr & 0xff);
    r->bus.wait_busy(r->bus.ctx);
    if (r->bus.xfer(r->bus.ctx, tx, rx, n + 4)) return SX_E_BUS;
    memcpy(data, rx + 4, n);          /* op + 2 addr + 1 status byte */
    return SX_OK;
}

static int reg_write8(sx1262_t *r, uint16_t addr, uint8_t v)
{
    return reg_write(r, addr, &v, 1);
}

/* freq_hz * 2^25 / 32 MHz, computed in 64 bits. Getting this wrong by a factor
 * produces a radio that transmits perfectly into the wrong band and looks
 * exactly like a broken antenna. */
static uint32_t freq_word(uint32_t hz)
{
    return (uint32_t)(((uint64_t)hz << 25) / 32000000ull);
}

/* Timeouts are in 15.625 us steps (datasheet 13.1.4). */
static uint32_t to_steps(uint32_t ms)
{
    return (uint32_t)(((uint64_t)ms * 1000ull * 64ull) / 1000ull);   /* ms -> /15.625us */
}

static int set_standby_rc(sx1262_t *r)
{
    uint8_t a = 0x00;                            /* STDBY_RC */
    return cmd(r, OP_SET_STANDBY, &a, 1);
}

int sx1262_standby(sx1262_t *r) { return set_standby_rc(r); }

static int apply_modulation(sx1262_t *r)
{
    /* ORDERING REQUIREMENT: SetPacketType must already have been issued, or these
     * four bytes are parsed as GFSK modulation parameters. Asserted by the mock
     * bus test rather than trusted. */
    if (!r->packet_type_set) return SX_E_ARG;
    uint8_t m[4] = { r->cfg.sf, r->cfg.bw, r->cfg.cr, r->cfg.ldro };
    return cmd(r, OP_SET_MODULATION_PARAMS, m, 4);
}

static int apply_packet_params(sx1262_t *r)
{
    uint8_t p[6];
    p[0] = (uint8_t)(r->cfg.preamble_sym >> 8);
    p[1] = (uint8_t)(r->cfg.preamble_sym & 0xff);
    p[2] = r->cfg.implicit_header ? 0x01 : 0x00;
    p[3] = r->cfg.payload_len;
    p[4] = r->cfg.crc_on ? 0x01 : 0x00;
    p[5] = 0x00;                                 /* standard IQ */
    return cmd(r, OP_SET_PACKET_PARAMS, p, 6);
}

/* The four rows of datasheet table 13-21. Mixing a PA config from one row with
 * the TX power of another runs the PA outside its safe operating area — it works
 * on the bench and fails in the field, which is the worst failure mode there is. */
static int apply_pa(sx1262_t *r)
{
    uint8_t duty, hpmax;
    int8_t  power = r->cfg.tx_dbm;
    switch (power) {
        case 22: duty = 0x04; hpmax = 0x07; break;
        case 20: duty = 0x03; hpmax = 0x05; break;
        case 17: duty = 0x02; hpmax = 0x03; break;
        case 14: duty = 0x02; hpmax = 0x02; break;
        default: return SX_E_ARG;                /* refuse to invent a row */
    }
    uint8_t pa[4] = { duty, hpmax, 0x00, 0x01 }; /* deviceSel 0 = SX1262 */
    int rc = cmd(r, OP_SET_PA_CONFIG, pa, 4);
    if (rc) return rc;

    /* Over-current protection: 140 mA for the SX1262's high-power PA. The reset
     * default is 60 mA and would clip a +22 dBm transmission. */
    rc = reg_write8(r, REG_OCP, 0x38);
    if (rc) return rc;

    /* Datasheet 15.2 workaround: better PA clamping, all four bits set. */
    uint8_t clamp;
    if (reg_read(r, REG_TX_CLAMP, &clamp, 1) == SX_OK)
        reg_write8(r, REG_TX_CLAMP, (uint8_t)(clamp | 0x1E));

    uint8_t tp[2] = { (uint8_t)power, 0x02 };    /* ramp 40 us */
    return cmd(r, OP_SET_TX_PARAMS, tp, 2);
}

int sx1262_init(sx1262_t *r, const sx_bus_t *bus, const sx_cfg_t *cfg)
{
    if (!r || !bus || !cfg) return SX_E_ARG;
    memset(r, 0, sizeof *r);
    r->bus = *bus;
    r->cfg = *cfg;

    r->bus.reset(r->bus.ctx);
    r->bus.wait_busy(r->bus.ctx);

    int rc = set_standby_rc(r);
    if (rc) return rc;

    /* DC-DC regulator: roughly half the current of the LDO in RX. On a device
     * whose entire story is 1.78 mAh/day this is not an optimisation. */
    uint8_t reg = 0x01;
    if ((rc = cmd(r, OP_SET_REGULATOR_MODE, &reg, 1))) return rc;

    if (r->cfg.dio2_as_rf_switch) {
        uint8_t on = 0x01;
        if ((rc = cmd(r, OP_SET_DIO2_RF_SWITCH, &on, 1))) return rc;
    }

    if (r->cfg.use_tcxo) {
        /* DIO3 powers the TCXO. The delay must cover the crystal's start-up or
         * every subsequent calibration fails with XOSC_START_ERR — which is what
         * sx1262_selftest reports, in words, instead of leaving you guessing. */
        uint32_t d = to_steps(r->cfg.tcxo_delay_us / 1000u ? r->cfg.tcxo_delay_us / 1000u : 5u);
        uint8_t t[4] = { r->cfg.tcxo_voltage,
                         (uint8_t)(d >> 16), (uint8_t)(d >> 8), (uint8_t)d };
        if ((rc = cmd(r, OP_SET_DIO3_TCXO, t, 4))) return rc;
        /* Everything calibrated against the old clock is now invalid. */
        uint8_t all = 0x7F;
        if ((rc = cmd(r, OP_CALIBRATE, &all, 1))) return rc;
        r->bus.delay_ms(r->bus.ctx, 5);
        r->bus.wait_busy(r->bus.ctx);
    }

    /* Packet type FIRST. Everything below is interpreted relative to it. */
    uint8_t pt = PKT_TYPE_LORA;
    if ((rc = cmd(r, OP_SET_PACKET_TYPE, &pt, 1))) return rc;
    r->packet_type_set = 1;

    uint32_t fw = freq_word(r->cfg.freq_hz);
    uint8_t f[4] = { (uint8_t)(fw >> 24), (uint8_t)(fw >> 16), (uint8_t)(fw >> 8), (uint8_t)fw };
    if ((rc = cmd(r, OP_SET_RF_FREQUENCY, f, 4))) return rc;

    /* Image calibration for the 902-928 MHz band (datasheet table 13-20). */
    uint8_t img[2] = { 0xE1, 0xE9 };
    if ((rc = cmd(r, OP_CALIBRATE_IMAGE, img, 2))) return rc;

    if ((rc = apply_modulation(r))) return rc;
    if ((rc = apply_packet_params(r))) return rc;
    if ((rc = apply_pa(r))) return rc;

    uint8_t base[2] = { 0x00, 0x00 };
    if ((rc = cmd(r, OP_SET_BUFFER_BASE, base, 2))) return rc;

    uint8_t sync[2] = { (uint8_t)(LORA_SYNC_PRIVATE >> 8), (uint8_t)(LORA_SYNC_PRIVATE & 0xff) };
    if ((rc = reg_write(r, REG_LORA_SYNC_MSB, sync, 2))) return rc;

    if (r->cfg.boosted_rx) reg_write8(r, REG_RX_GAIN, 0x96);   /* boosted gain */

    /* Route everything we care about to DIO1: one interrupt line, and the MCU can
     * distinguish causes by reading the IRQ status once awake. */
    uint16_t mask = SX_IRQ_TX_DONE | SX_IRQ_RX_DONE | SX_IRQ_PREAMBLE |
                    SX_IRQ_HEADER_ERR | SX_IRQ_CRC_ERR | SX_IRQ_TIMEOUT;
    uint8_t irq[8] = { (uint8_t)(mask >> 8), (uint8_t)mask,
                       (uint8_t)(mask >> 8), (uint8_t)mask,
                       0, 0, 0, 0 };
    if ((rc = cmd(r, OP_SET_DIO_IRQ_PARAMS, irq, 8))) return rc;

    return SX_OK;
}

int sx1262_set_mode(sx1262_t *r, uint8_t sf, uint8_t implicit_header,
                    uint8_t crc_on, uint8_t payload_len)
{
    r->cfg.sf              = sf;
    r->cfg.ldro            = (uint8_t)(sf >= 11);
    r->cfg.implicit_header = implicit_header;
    r->cfg.crc_on          = crc_on;
    r->cfg.payload_len     = payload_len;
    int rc = set_standby_rc(r);
    if (rc) return rc;
    if ((rc = apply_modulation(r))) return rc;
    return apply_packet_params(r);
}

int sx1262_tx(sx1262_t *r, const uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
    int rc;
    if ((rc = set_standby_rc(r))) return rc;

    /* In implicit-header mode the length is not on air, so the packet params must
     * carry it and both ends must agree a priori. */
    if (r->cfg.payload_len != len) {
        r->cfg.payload_len = len;
        if ((rc = apply_packet_params(r))) return rc;
    }

    uint8_t buf[1 + 1 + 255];
    if (len > 255) return SX_E_ARG;
    buf[0] = OP_WRITE_BUFFER;
    buf[1] = 0x00;                               /* offset */
    memcpy(buf + 2, data, len);
    r->bus.wait_busy(r->bus.ctx);
    if (r->bus.xfer(r->bus.ctx, buf, NULL, (size_t)len + 2)) return SX_E_BUS;

    if ((rc = sx1262_irq_clear(r, 0xFFFF))) return rc;

    uint32_t t = to_steps(timeout_ms);
    uint8_t a[3] = { (uint8_t)(t >> 16), (uint8_t)(t >> 8), (uint8_t)t };
    return cmd(r, OP_SET_TX, a, 3);
}

int sx1262_rx_continuous(sx1262_t *r)
{
    int rc;
    if ((rc = set_standby_rc(r))) return rc;
    if ((rc = sx1262_irq_clear(r, 0xFFFF))) return rc;
    uint8_t a[3] = { 0xFF, 0xFF, 0xFF };         /* continuous */
    return cmd(r, OP_SET_RX, a, 3);
}

int sx1262_rx_duty_cycle(sx1262_t *r, uint32_t rx_ms, uint32_t sleep_ms)
{
    int rc;
    if ((rc = set_standby_rc(r))) return rc;
    if ((rc = sx1262_irq_clear(r, 0xFFFF))) return rc;

    /* Keep the timer running across a preamble: with StopTimerOnPreamble enabled,
     * noise that looks like a preamble holds the receiver open and the duty cycle
     * — and with it the whole power budget — quietly stops being what you set. */
    uint8_t stop = 0x00;
    if ((rc = cmd(r, OP_STOP_TIMER_ON_PREAMB, &stop, 1))) return rc;

    uint32_t rxp = to_steps(rx_ms), slp = to_steps(sleep_ms);
    uint8_t a[6] = { (uint8_t)(rxp >> 16), (uint8_t)(rxp >> 8), (uint8_t)rxp,
                     (uint8_t)(slp >> 16), (uint8_t)(slp >> 8), (uint8_t)slp };
    return cmd(r, OP_SET_RX_DUTY_CYCLE, a, 6);
}

int sx1262_sleep(sx1262_t *r, int warm_start)
{
    /* Warm start keeps the configuration in retention RAM: waking costs a few
     * hundred microseconds instead of a full re-init. Cold start drops to ~160 nA
     * but every wake pays for the whole bring-up above. */
    uint8_t a = (uint8_t)(warm_start ? 0x04 : 0x00);
    return cmd(r, OP_SET_SLEEP, &a, 1);
}

uint16_t sx1262_irq(sx1262_t *r)
{
    uint8_t v[2] = {0, 0};
    if (cmd_read(r, OP_GET_IRQ_STATUS, v, 2)) return 0;
    return (uint16_t)((v[0] << 8) | v[1]);
}

int sx1262_irq_clear(sx1262_t *r, uint16_t mask)
{
    uint8_t a[2] = { (uint8_t)(mask >> 8), (uint8_t)mask };
    return cmd(r, OP_CLR_IRQ_STATUS, a, 2);
}

int sx1262_read(sx1262_t *r, uint8_t *out, size_t max, int16_t *rssi, int8_t *snr)
{
    uint16_t irq = sx1262_irq(r);
    /* A CRC error is only meaningful when a CRC was requested. In Tier 0.5 there
     * is no CRC and no header, so there is nothing to reject — which is the entire
     * reason that tier exists. */
    if (r->cfg.crc_on && (irq & SX_IRQ_CRC_ERR)) { sx1262_irq_clear(r, 0xFFFF); return SX_E_CRC; }
    if (irq & SX_IRQ_TIMEOUT) { sx1262_irq_clear(r, 0xFFFF); return SX_E_TIMEOUT; }

    uint8_t st[2] = {0, 0};
    if (cmd_read(r, OP_GET_RX_BUFFER_STATUS, st, 2)) return SX_E_BUS;
    uint8_t len = st[0], offset = st[1];
    if (len > max) len = (uint8_t)max;

    uint8_t tx[4 + 255] = {0}, rx[4 + 255] = {0};
    tx[0] = OP_READ_BUFFER; tx[1] = offset; tx[2] = 0x00;
    r->bus.wait_busy(r->bus.ctx);
    if (r->bus.xfer(r->bus.ctx, tx, rx, (size_t)len + 3)) return SX_E_BUS;
    memcpy(out, rx + 3, len);

    uint8_t ps[3] = {0, 0, 0};
    if (cmd_read(r, OP_GET_PACKET_STATUS, ps, 3) == SX_OK) {
        if (rssi) *rssi = (int16_t)(-(int16_t)ps[0] / 2);
        if (snr)  *snr  = (int8_t)((int8_t)ps[1] / 4);
    }
    sx1262_irq_clear(r, 0xFFFF);
    return len;
}

uint16_t sx1262_device_errors(sx1262_t *r)
{
    uint8_t v[2] = {0, 0};
    if (cmd_read(r, OP_GET_DEVICE_ERRORS, v, 2)) return 0xFFFF;
    return (uint16_t)((v[0] << 8) | v[1]);
}

uint8_t sx1262_status(sx1262_t *r)
{
    /* GetStatus is the one read command with no payload byte: the answer is the
     * status byte returned while the NOP is clocked out, i.e. rx[1], not rx[2].
     * Every other Get* command inserts a status byte BEFORE its payload, which is
     * why cmd_read() skips two. Using cmd_read here would read one byte too far
     * and report the chip as dead. */
    uint8_t tx[2] = { OP_GET_STATUS, 0x00 }, rx[2] = {0, 0};
    r->bus.wait_busy(r->bus.ctx);
    if (r->bus.xfer(r->bus.ctx, tx, rx, 2)) return 0xFF;
    return rx[1];
}

int16_t sx1262_rssi_inst(sx1262_t *r)
{
    uint8_t v[1] = {0};
    if (cmd_read(r, OP_GET_RSSI_INST, v, 1)) return 0;
    return (int16_t)(-(int16_t)v[0] / 2);
}

int sx1262_selftest(sx1262_t *r, char *report, size_t n)
{
    char *p = report;
    size_t left = n;
    int fails = 0;
#define SAY(...) do { int k = snprintf(p, left, __VA_ARGS__); \
                      if (k > 0 && (size_t)k < left) { p += k; left -= (size_t)k; } } while (0)

    /* 1. SPI round trip through a register. If this fails, the pin map is wrong —
     *    and on a LilyGO board the pin map is the single most likely fault,
     *    because several T3-S3 revisions ship with different BUSY and DIO1 pins. */
    uint8_t saved[2] = {0, 0}, probe[2] = { 0x5A, 0xA5 }, back[2] = {0, 0};
    reg_read(r, REG_LORA_SYNC_MSB, saved, 2);
    reg_write(r, REG_LORA_SYNC_MSB, probe, 2);
    reg_read(r, REG_LORA_SYNC_MSB, back, 2);
    if (back[0] == probe[0] && back[1] == probe[1]) {
        SAY("  [ok]   SPI read/write round trip\n");
    } else {
        SAY("  [FAIL] SPI round trip: wrote 5A A5, read %02X %02X\n"
            "         -> check NSS / SCK / MOSI / MISO / BUSY in board_t3s3.h.\n"
            "         A wrong BUSY pin looks exactly like a dead radio.\n", back[0], back[1]);
        fails++;
    }
    reg_write(r, REG_LORA_SYNC_MSB, saved, 2);

    /* 2. Oscillator and calibration. XOSC_START_ERR here means the TCXO settings
     *    are wrong — the most common bring-up mistake on these modules, and the
     *    fix is one flag in board_t3s3.h. */
    uint16_t err = sx1262_device_errors(r);
    if (err == 0) {
        SAY("  [ok]   no device errors (oscillator started, all blocks calibrated)\n");
    } else {
        SAY("  [FAIL] device errors 0x%04X:%s%s%s%s\n", err,
            (err & SX_ERR_XOSC_START) ? " XOSC_START(TCXO config!)" : "",
            (err & SX_ERR_PLL_LOCK)   ? " PLL_LOCK(frequency out of band?)" : "",
            (err & SX_ERR_IMG_CALIB)  ? " IMG_CALIB" : "",
            (err & SX_ERR_PA_RAMP)    ? " PA_RAMP" : "");
        if (err & SX_ERR_XOSC_START)
            SAY("         -> set BOARD_HAS_TCXO to 0 (or fix the voltage) and retry.\n");
        fails++;
    }

    /* 3. The chip must report a sane mode. 0xFF means MISO is not being driven. */
    uint8_t st = sx1262_status(r);
    if (st != 0xFF && st != 0x00) {
        SAY("  [ok]   status byte 0x%02X (chip mode %u, command status %u)\n",
            st, (st >> 4) & 7, (st >> 1) & 7);
    } else {
        SAY("  [FAIL] status byte 0x%02X — MISO idle or chip held in reset\n", st);
        fails++;
    }

    /* 4. Receiver noise floor. A radio in RX on a quiet 915 MHz band reads about
     *    -100 to -125 dBm. Exactly 0 means RX never started; -1 or -2 means the
     *    front end is saturated or the LNA is unpowered. */
    sx1262_rx_continuous(r);
    r->bus.delay_ms(r->bus.ctx, 10);
    int16_t noise = sx1262_rssi_inst(r);
    if (noise <= -60 && noise >= -140) {
        SAY("  [ok]   receiver noise floor %d dBm (plausible)\n", noise);
    } else {
        SAY("  [FAIL] noise floor %d dBm — RX did not start, or the front end is\n"
            "         saturated. Check DIO2_AS_RF_SWITCH and the antenna path.\n", noise);
        fails++;
    }
    set_standby_rc(r);

    SAY("  %s\n", fails ? "SELFTEST FAILED — fix this before walking anywhere."
                        : "SELFTEST PASSED — the radio is alive and configured.");
#undef SAY
    return fails ? SX_E_BUS : SX_OK;
}
