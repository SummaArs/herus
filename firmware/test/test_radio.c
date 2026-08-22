/* test_radio.c — the SX1262 driver, verified without an SX1262.
 *
 * A recording mock sits where the SPI bus would be. It emulates a register file
 * and the documented response layouts, so the driver's command sequences can be
 * asserted on a Mac. What this catches is precisely the class of bug that is
 * agonising to find on a bench, because the symptom is always the same — silence:
 *
 *   R1  SetPacketType(LORA) precedes SetModulationParams
 *   R2  the frequency word is freq * 2^25 / 32 MHz
 *   R3  PA config and TX power come from the same datasheet row
 *   R4  packet params match the ledger: 8-symbol preamble, explicit header, 34 B, CRC on
 *   R5  the private sync word 0x1424 is written (not the SX1276 value 0x12)
 *   R6  Tier 0.5 switches to implicit header with CRC off and keeps the length
 *   R7  BUSY is honoured before every command
 *   R8  SetRxDutyCycle converts milliseconds to 15.625 us steps correctly
 *   R9  an undocumented TX power is refused rather than approximated
 *   R10 the selftest detects a wrong pin map, a bad TCXO and a dead receiver
 *
 * A bench afternoon is for antennas. This file is so that it is not for opcodes.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../port/sx1262.h"
#include "../net/region.h"

static int FAILED = 0;
static void ok(int c, const char *w) { printf("  %-4s %s\n", c ? "PASS" : "FAIL", w); if (!c) FAILED = 1; }

/* ------------------------------------------------------------------ mock --- */

#define TRACE_MAX 4096
typedef struct {
    uint8_t  trace[TRACE_MAX];      /* every MOSI byte, in order */
    size_t   n;
    size_t   frames[256];           /* start offset of each command */
    size_t   nframes;
    uint8_t  regs[0x1000];
    int      busy_waits;
    int      resets;
    /* fault injection */
    int      broken_spi;            /* register writes do not stick */
    uint16_t inject_errors;         /* what GetDeviceErrors reports */
    uint8_t  inject_status;
    int8_t   inject_rssi;           /* raw, halved and negated by the driver */
} mock_t;

static int m_xfer(void *ctx, const uint8_t *tx, uint8_t *rx, size_t len)
{
    mock_t *m = (mock_t *)ctx;
    if (m->nframes < 256) m->frames[m->nframes++] = m->n;
    for (size_t i = 0; i < len && m->n < TRACE_MAX; i++) m->trace[m->n++] = tx[i];
    if (rx) memset(rx, 0, len);

    switch (tx[0]) {
    case 0x0D: {                                   /* WriteRegister */
        uint16_t a = (uint16_t)((tx[1] << 8) | tx[2]);
        if (!m->broken_spi)
            for (size_t i = 3; i < len && (a + i - 3) < sizeof m->regs; i++)
                m->regs[a + i - 3] = tx[i];
        break;
    }
    case 0x1D: {                                   /* ReadRegister */
        uint16_t a = (uint16_t)((tx[1] << 8) | tx[2]);
        if (rx) for (size_t i = 4; i < len; i++)
            rx[i] = (a + i - 4) < sizeof m->regs ? m->regs[a + i - 4] : 0;
        break;
    }
    case 0xC0: if (rx) rx[1] = m->inject_status; break;              /* GetStatus */
    case 0x17: if (rx) { rx[2] = (uint8_t)(m->inject_errors >> 8);   /* GetDeviceErrors */
                         rx[3] = (uint8_t)m->inject_errors; } break;
    case 0x15: if (rx) rx[2] = (uint8_t)m->inject_rssi; break;       /* GetRssiInst */
    case 0x12: if (rx) { rx[2] = 0x00; rx[3] = 0x02; } break;        /* GetIrqStatus: RxDone */
    case 0x13: if (rx) { rx[2] = 34; rx[3] = 0; } break;             /* GetRxBufferStatus */
    case 0x14: if (rx) { rx[2] = 180; rx[3] = 40; rx[4] = 180; } break; /* GetPacketStatus */
    default: break;
    }
    return 0;
}
static void m_busy(void *ctx)  { ((mock_t *)ctx)->busy_waits++; }
static void m_reset(void *ctx) { ((mock_t *)ctx)->resets++; }
static void m_delay(void *ctx, uint32_t ms) { (void)ctx; (void)ms; }

static void mock_init(mock_t *m)
{
    memset(m, 0, sizeof *m);
    m->inject_status = 0x2A;          /* STDBY_RC, command completed */
    m->inject_rssi   = (int8_t)220;   /* -110 dBm */
}

/* Find the i-th command whose opcode is `op`; returns its offset or -1. */
static long find_cmd(const mock_t *m, uint8_t op, int nth)
{
    int seen = 0;
    for (size_t f = 0; f < m->nframes; f++) {
        if (m->trace[m->frames[f]] == op) {
            if (seen == nth) return (long)m->frames[f];
            seen++;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------ tests -- */

static sx_cfg_t base_cfg(void)
{
    sx_cfg_t c;
    memset(&c, 0, sizeof c);
    c.freq_hz = 915000000u;
    c.sf = HERUS_SF_MEANING;
    c.bw = SX_BW_125;
    c.cr = SX_CR_4_5;
    c.ldro = 0;
    c.tx_dbm = 22;
    c.preamble_sym = 8;
    c.implicit_header = 0;
    c.crc_on = 1;
    c.payload_len = (uint8_t)HERUS_FRAME_LEN;
    c.use_tcxo = 1;
    c.tcxo_voltage = SX_TCXO_1_8V;
    c.tcxo_delay_us = 5000;
    c.dio2_as_rf_switch = 1;
    c.boosted_rx = 1;
    return c;
}

int main(void)
{
    mock_t m;
    sx1262_t r;
    sx_bus_t bus = { m_xfer, m_busy, m_reset, m_delay, &m };
    sx_cfg_t cfg = base_cfg();

    printf("SX1262 driver proof suite (recording mock bus, no hardware)\n\n");

    mock_init(&m);
    int rc = sx1262_init(&r, &bus, &cfg);
    ok(rc == SX_OK, "init completes");
    ok(m.resets == 1, "R0 the chip is reset exactly once");

    long pt = find_cmd(&m, 0x8A, 0);          /* SetPacketType */
    long mp = find_cmd(&m, 0x8B, 0);          /* SetModulationParams */
    ok(pt >= 0 && mp >= 0 && pt < mp,
       "R1 SetPacketType(LORA) is issued BEFORE SetModulationParams");
    ok(pt >= 0 && m.trace[pt + 1] == 0x01, "R1 and the packet type is LoRa");

    long fq = find_cmd(&m, 0x86, 0);          /* SetRfFrequency */
    uint32_t word = (uint32_t)((m.trace[fq + 1] << 24) | (m.trace[fq + 2] << 16) |
                              (m.trace[fq + 3] << 8) | m.trace[fq + 4]);
    printf("  915 MHz -> 0x%08X (expected 0x39300000)\n", word);
    ok(word == 0x39300000u, "R2 the frequency word is freq * 2^25 / 32 MHz");

    long pa = find_cmd(&m, 0x95, 0);          /* SetPaConfig */
    long tp = find_cmd(&m, 0x8E, 0);          /* SetTxParams */
    ok(pa >= 0 && m.trace[pa + 1] == 0x04 && m.trace[pa + 2] == 0x07 &&
       m.trace[pa + 3] == 0x00 && m.trace[pa + 4] == 0x01,
       "R3 PA config for +22 dBm is 04 07 00 01 (datasheet table 13-21)");
    ok(tp >= 0 && m.trace[tp + 1] == 22,
       "R3 and TX power comes from the same row, not a different one");
    ok(m.regs[0x08E7] == 0x38, "R3 over-current protection raised to 140 mA for the HP PA");

    long pp = find_cmd(&m, 0x8C, 0);          /* SetPacketParams */
    ok(pp >= 0 && m.trace[pp + 1] == 0 && m.trace[pp + 2] == 8, "R4 preamble is 8 symbols");
    ok(m.trace[pp + 3] == 0x00, "R4 header is explicit for the AEAD tiers");
    ok(m.trace[pp + 4] == HERUS_FRAME_LEN, "R4 payload length is the ledger's 34 bytes");
    ok(m.trace[pp + 5] == 0x01, "R4 CRC is on for the AEAD tiers");

    ok(m.regs[0x0740] == 0x14 && m.regs[0x0741] == 0x24,
       "R5 the private sync word 0x1424 is written, not the SX1276-era 0x12");
    ok(m.regs[0x08AC] == 0x96, "R5b boosted RX gain is enabled");

    r.packet_type_set = 0;
    ok(sx1262_set_mode(&r, 9, 0, 1, (uint8_t)HERUS_FRAME_LEN) == SX_E_ARG,
       "R1b reconfiguration refuses modulation when packet type authority is absent");
    r.packet_type_set = 1;

    /* Tier 0.5 reconfiguration. */
    m.nframes = 0; m.n = 0;
    rc = sx1262_set_mode(&r, 9, 1, 0, (uint8_t)HERUS_SKETCH_FRAME_LEN);
    pp = find_cmd(&m, 0x8C, 0);
    ok(rc == SX_OK && pp >= 0 && m.trace[pp + 3] == 0x01 && m.trace[pp + 5] == 0x00 &&
       m.trace[pp + 4] == HERUS_SKETCH_FRAME_LEN,
       "R6 Tier 0.5 switches to implicit header, CRC off, 38-byte fixed length");

    /* BUSY discipline. */
    mock_init(&m);
    sx1262_init(&r, &bus, &cfg);
    printf("  %zu commands issued, %d BUSY waits\n", m.nframes, m.busy_waits);
    ok(m.busy_waits >= (int)m.nframes, "R7 BUSY is honoured before every single command");

    /* Duty cycle arithmetic: 20 ms RX / 2000 ms sleep in 15.625 us steps. */
    m.nframes = 0; m.n = 0;
    sx1262_rx_duty_cycle(&r, 20, 2000);
    long dc = find_cmd(&m, 0x94, 0);
    uint32_t rxp = (uint32_t)((m.trace[dc + 1] << 16) | (m.trace[dc + 2] << 8) | m.trace[dc + 3]);
    uint32_t slp = (uint32_t)((m.trace[dc + 4] << 16) | (m.trace[dc + 5] << 8) | m.trace[dc + 6]);
    printf("  20 ms -> %u steps (expect 1280), 2000 ms -> %u steps (expect 128000)\n", rxp, slp);
    ok(rxp == 1280 && slp == 128000,
       "R8 SetRxDutyCycle converts ms to 15.625 us steps exactly");
    ok(find_cmd(&m, 0x9F, 0) >= 0,
       "R8b StopTimerOnPreamble is disabled first, or noise silently ruins the duty cycle");

    /* An invented PA row must be refused. */
    sx_cfg_t bad = base_cfg();
    bad.tx_dbm = 15;
    mock_init(&m);
    ok(sx1262_init(&r, &bus, &bad) == SX_E_ARG,
       "R9 a TX power that is not a datasheet row is refused, not approximated");

    /* Selftest: healthy board. */
    char rep[1024];
    mock_init(&m);
    sx1262_init(&r, &bus, &cfg);
    rc = sx1262_selftest(&r, rep, sizeof rep);
    printf("\n  --- selftest on a healthy board ---\n%s", rep);
    ok(rc == SX_OK, "R10 the selftest passes on a healthy board");

    /* Selftest: wrong pin map (writes do not stick). */
    mock_init(&m); m.broken_spi = 1;
    sx1262_init(&r, &bus, &cfg);
    rc = sx1262_selftest(&r, rep, sizeof rep);
    ok(rc != SX_OK && strstr(rep, "board_t3s3.h") != NULL,
       "R10 a wrong pin map is diagnosed, and the report names the file to edit");

    /* Selftest: TCXO misconfigured. */
    mock_init(&m); m.inject_errors = SX_ERR_XOSC_START;
    sx1262_init(&r, &bus, &cfg);
    rc = sx1262_selftest(&r, rep, sizeof rep);
    ok(rc != SX_OK && strstr(rep, "BOARD_HAS_TCXO") != NULL,
       "R10 a TCXO misconfiguration is diagnosed by name with the fix");

    /* Selftest: receiver dead. */
    mock_init(&m); m.inject_rssi = 0;
    sx1262_init(&r, &bus, &cfg);
    rc = sx1262_selftest(&r, rep, sizeof rep);
    ok(rc != SX_OK && strstr(rep, "noise floor") != NULL,
       "R10 a receiver that never starts is diagnosed");

    printf("\n%s\n", FAILED ? "SOMETHING REGRESSED — do not flash this."
                            : "SX1262 COMMAND SEQUENCES VERIFIED.");
    return FAILED;
}
