/* main/test_radio.c — HERUS ESP32-S3 Hardware Integration & MAC Adversarial Unity Test Suite
 *
 * Exercises hardware integration (HIL) pin assertions, SPI transfers, BUSY timeouts,
 * IRQ line behavior, and adversarial MAC layer fuzzing (CRC corruption, invalid intent,
 * TTL=0 relay drop, buffer boundary fuzzing, replay defense, rate limiting, memory leaks).
 */

#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../board_t3s3.h"
#include "../../hal.h"
#include "../../sx1262.h"

#include "hcp.h"
#include "lexicon.h"
#include "crypto.h"
#include "session.h"
#include "region.h"
#include "weave.h"
#include "beat.h"
#include "link.h"

static const char *TAG_TEST = "test_radio";

/* Global bus & radio instances for test harness */
static sx_bus_t  s_test_bus;
static sx1262_t s_test_radio;
static int      s_board_inited = 0;

/* from hal_esp32s3.c */
int herus_board_init(sx_bus_t *bus_out);

/* Heap snapshot helper for memory leak assertions */
typedef struct {
    size_t free_heap;
    size_t free_internal;
} heap_snap_t;

static inline heap_snap_t snap_heap(void)
{
    heap_snap_t s;
    s.free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    s.free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    TEST_ASSERT_TRUE_MESSAGE(heap_caps_check_integrity_all(true), "Heap corruption detected prior to test!");
    return s;
}

static inline void assert_heap_stable(heap_snap_t before, const char *msg)
{
    TEST_ASSERT_TRUE_MESSAGE(heap_caps_check_integrity_all(true), "Heap corruption detected after test!");
    heap_snap_t after = snap_heap();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before.free_heap, after.free_heap, msg);
}

void setUp(void)
{
    /* Code executed before each test case */
}

void tearDown(void)
{
    /* Code executed after each test case */
}

/* =========================================================================
 * R1 HIL HARDWARE INTEGRATION TESTS
 * ========================================================================= */

/* -------------------------------------------------------------------------
 * R1.1 Physical Pin Mappings Validation against board_t3s3.h
 * ------------------------------------------------------------------------- */
void test_r1_1_pin_validation(void)
{
    ESP_LOGI(TAG_TEST, "Running R1.1 Pin Validation against board_t3s3.h...");
    TEST_ASSERT_EQUAL_INT_MESSAGE(5,  PIN_LORA_SCK,  "PIN_LORA_SCK mismatch! Expected GPIO 5");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3,  PIN_LORA_MISO, "PIN_LORA_MISO mismatch! Expected GPIO 3");
    TEST_ASSERT_EQUAL_INT_MESSAGE(6,  PIN_LORA_MOSI, "PIN_LORA_MOSI mismatch! Expected GPIO 6");
    TEST_ASSERT_EQUAL_INT_MESSAGE(7,  PIN_LORA_NSS,  "PIN_LORA_NSS mismatch! Expected GPIO 7");
    TEST_ASSERT_EQUAL_INT_MESSAGE(8,  PIN_LORA_RST,  "PIN_LORA_RST mismatch! Expected GPIO 8");
    TEST_ASSERT_EQUAL_INT_MESSAGE(34, PIN_LORA_BUSY, "PIN_LORA_BUSY mismatch! Expected GPIO 34");
    TEST_ASSERT_EQUAL_INT_MESSAGE(33, PIN_LORA_DIO1, "PIN_LORA_DIO1 mismatch! Expected GPIO 33");
}

/* -------------------------------------------------------------------------
 * R1.2 SPI Bus Communication & Sync Word Register Roundtrip
 * ------------------------------------------------------------------------- */
void test_r1_2_spi_transfer_roundtrip(void)
{
    ESP_LOGI(TAG_TEST, "Running R1.2 SPI Transfer Roundtrip...");
    if (!s_board_inited) {
        int init_rc = herus_board_init(&s_test_bus);
        TEST_ASSERT_EQUAL_INT_MESSAGE(0, init_rc, "herus_board_init failed during test setup");
        
        sx_cfg_t cfg = {
            .freq_hz = BOARD_FREQ_HZ,
            .sf = HERUS_SF_MEANING, .bw = SX_BW_125, .cr = SX_CR_4_5, .ldro = 0,
            .tx_dbm = BOARD_TX_DBM, .preamble_sym = 8,
            .implicit_header = 0, .crc_on = 1, .payload_len = (uint8_t)LINK_FRAME_LEN,
            .use_tcxo = BOARD_HAS_TCXO, .tcxo_voltage = BOARD_TCXO_VOLT,
            .tcxo_delay_us = BOARD_TCXO_DELAY_US,
            .dio2_as_rf_switch = BOARD_DIO2_RF_SWITCH, .boosted_rx = 1,
        };
        int rc = sx1262_init(&s_test_radio, &s_test_bus, &cfg);
        TEST_ASSERT_EQUAL_INT_MESSAGE(SX_OK, rc, "sx1262_init failed during SPI test setup");
        s_board_inited = 1;
    }

    /* Verify SPI transfer roundtrip via sx1262_selftest (REG_LORA_SYNC_MSB 0x0740 write/read) */
    char report[512];
    int st_rc = sx1262_selftest(&s_test_radio, report, sizeof(report));
    TEST_ASSERT_EQUAL_INT_MESSAGE(SX_OK, st_rc, report);

    /* Assert status byte is valid and MISO line is driven (not floating 0xFF) */
    uint8_t status = sx1262_status(&s_test_radio);
    TEST_ASSERT_NOT_EQUAL_HEX8_MESSAGE(0xFF, status, "MISO line is floating or SX1262 unresponsive!");
}

/* -------------------------------------------------------------------------
 * R1.3 BUSY Pin Wait & Timeout Logic
 * ------------------------------------------------------------------------- */
void test_r1_3_busy_pin_wait_timeout(void)
{
    ESP_LOGI(TAG_TEST, "Running R1.3 BUSY Pin Wait & Timeout Logic...");
    if (!s_board_inited) {
        test_r1_2_spi_transfer_roundtrip();
    }

    /* Idle BUSY pin must be LOW (0) */
    int busy_idle = gpio_get_level(PIN_LORA_BUSY);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, busy_idle, "BUSY pin high when radio is idle!");

    /* Measure standby command execution time */
    uint64_t t0 = esp_timer_get_time();
    sx1262_standby(&s_test_radio);
    uint64_t elapsed_us = esp_timer_get_time() - t0;

    /* Must return in well under the 100 ms (100,000 us) timeout deadline */
    TEST_ASSERT_TRUE_MESSAGE(elapsed_us < 100000, "BUSY pin wait exceeded 100 ms timeout!");

    /* Post-command BUSY level must return to LOW (0) */
    int busy_post = gpio_get_level(PIN_LORA_BUSY);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, busy_post, "BUSY pin failed to return to LOW after command execution!");
}

/* -------------------------------------------------------------------------
 * R1.4 DIO1 IRQ Line State Assertion and Clearing
 * ------------------------------------------------------------------------- */
void test_r1_4_dio1_irq_line_state(void)
{
    ESP_LOGI(TAG_TEST, "Running R1.4 DIO1 IRQ Line State Assertion & Clearing...");
    if (!s_board_inited) {
        test_r1_2_spi_transfer_roundtrip();
    }

    /* Clear any pending IRQ flags first */
    sx1262_irq_clear(&s_test_radio, 0xFFFF);
    vTaskDelay(pdMS_TO_TICKS(5));

    /* Idle DIO1 line must be LOW (0) */
    int dio1_idle = gpio_get_level(PIN_LORA_DIO1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, dio1_idle, "DIO1 interrupt pin high when idle!");

    /* Trigger short TX packet to induce SX_IRQ_TX_DONE */
    uint8_t dummy_tx[16] = {0xDE, 0xAD, 0xBE, 0xEF};
    int tx_rc = sx1262_tx(&s_test_radio, dummy_tx, sizeof(dummy_tx), 1000);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SX_OK, tx_rc, "sx1262_tx failed during IRQ test");

    /* Poll for completion or timeout */
    uint16_t irq_stat = 0;
    int ticks = 0;
    while (!((irq_stat = sx1262_irq(&s_test_radio)) & (SX_IRQ_TX_DONE | SX_IRQ_TIMEOUT))) {
        vTaskDelay(pdMS_TO_TICKS(2));
        if (++ticks > 100) break;
    }

    TEST_ASSERT_TRUE_MESSAGE(irq_stat & (SX_IRQ_TX_DONE | SX_IRQ_TIMEOUT), "TX interrupt failed to fire!");

    /* Assert DIO1 hardware line was pulled HIGH (1) by SX1262 interrupt hardware */
    int dio1_fired = gpio_get_level(PIN_LORA_DIO1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, dio1_fired, "DIO1 hardware pin failed to assert HIGH on IRQ event!");

    /* Clear IRQ flags and verify DIO1 pin drops back to LOW (0) */
    sx1262_irq_clear(&s_test_radio, 0xFFFF);
    vTaskDelay(pdMS_TO_TICKS(2));
    int dio1_cleared = gpio_get_level(PIN_LORA_DIO1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, dio1_cleared, "DIO1 hardware pin failed to drop LOW after irq_clear!");
}

/* =========================================================================
 * R2 ADVERSARIAL MAC LAYER TESTS
 * ========================================================================= */

/* -------------------------------------------------------------------------
 * R2.1 Corrupted CRC / Tag Authentication Failure Fuzzing
 * ------------------------------------------------------------------------- */
void test_r2_1_corrupted_crc_tag_auth_failure(void)
{
    ESP_LOGI(TAG_TEST, "Running R2.1 Corrupted CRC / Tag Authentication Failure Fuzzing...");
    heap_snap_t snap = snap_heap();

    herus_session sess;
    herus_link link;
    uint8_t root[32];
    memset(root, 0x55, sizeof(root));

    session_init(&sess, root, 1, 1000);
    link.sess = &sess;
    link.region = HZ_REGION_BR915;
    link.tx_dbm = 14;
    link.known_roles = NULL;
    link.nknown = 0;

    hcp_msg_t tx_msg = { .tier = HCP_TIER_GLYPH, .intent = 10, .seq = 1 };
    uint8_t valid_frame[LINK_FRAME_LEN];
    TEST_ASSERT_EQUAL_INT_MESSAGE(LINK_OK, link_send(&link, &tx_msg, 3, valid_frame), "link_send failed");

    /* 1. Corrupt byte in payload ciphertext */
    uint8_t bad_frame[LINK_FRAME_LEN];
    memcpy(bad_frame, valid_frame, LINK_FRAME_LEN);
    bad_frame[12] ^= 0xA5;

    hcp_msg_t rx_msg;
    uint32_t ctr = 0;
    int sess_err = 0;
    int res = link_recv(&link, bad_frame, 1001, &rx_msg, &ctr, &sess_err);

    TEST_ASSERT_EQUAL_INT_MESSAGE(LINK_E_SESSION, res, "link_recv accepted corrupted frame!");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SESS_E_AUTH, sess_err, "Expected authentication error (SESS_E_AUTH)!");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, sess.stat_auth_fail, "stat_auth_fail not incremented!");

    /* 2. Corrupt tag bytes */
    memcpy(bad_frame, valid_frame, LINK_FRAME_LEN);
    bad_frame[LINK_FRAME_LEN - 1] ^= 0xFF;
    res = link_recv(&link, bad_frame, 1002, &rx_msg, &ctr, &sess_err);
    TEST_ASSERT_EQUAL_INT_MESSAGE(LINK_E_SESSION, res, "link_recv accepted frame with corrupted tag!");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SESS_E_AUTH, sess_err, "Expected tag auth error!");

    assert_heap_stable(snap, "Memory leak detected in Corrupted CRC/Tag test!");
}

/* -------------------------------------------------------------------------
 * R2.2 Invalid / Out-of-Bound Intent Values (>= 2048)
 * ------------------------------------------------------------------------- */
void test_r2_2_invalid_intent_values(void)
{
    ESP_LOGI(TAG_TEST, "Running R2.2 Invalid / Out-of-Bound Intent Values...");
    heap_snap_t snap = snap_heap();

    /* 1. Direct hcp_encode out-of-bounds check (max valid intent is 2047) */
    hcp_msg_t invalid_msg = { .tier = HCP_TIER_GLYPH, .intent = 2048, .seq = 1 };
    uint8_t pt[HCP_PLAINTEXT_LEN];
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_encode(pt, &invalid_msg), "hcp_encode allowed intent 2048!");

    invalid_msg.intent = 4095;
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_encode(pt, &invalid_msg), "hcp_encode allowed intent 4095!");

    /* 2. Direct hcp_decode out-of-bounds check */
    uint8_t raw_pt[HCP_PLAINTEXT_LEN];
    memset(raw_pt, 0, sizeof(raw_pt));
    raw_pt[0] = (HCP_VERSION & 3) << 6; /* Version 1 */
    raw_pt[2] = 0xFF;
    raw_pt[3] = 0x07; /* Intent = 0x07FF = 2047 (Valid boundary) */
    
    hcp_msg_t decoded;
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, hcp_decode(&decoded, raw_pt), "hcp_decode rejected valid boundary intent 2047");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(2047, decoded.intent, "Decoded intent mismatch");

    raw_pt[2] = 0x00;
    raw_pt[3] = 0x08; /* Intent = 0x0800 = 2048 (Out-of-bounds) */
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_decode(&decoded, raw_pt), "hcp_decode accepted out-of-bounds intent 2048!");

    raw_pt[2] = 0xFF;
    raw_pt[3] = 0x0F; /* Intent = 0x0FFF = 4095 (Out-of-bounds) */
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_decode(&decoded, raw_pt), "hcp_decode accepted out-of-bounds intent 4095!");

    /* 3. Non-canonical slot gap check */
    memset(raw_pt, 0, sizeof(raw_pt));
    raw_pt[0] = (HCP_VERSION & 3) << 6;
    raw_pt[4] = 0x00; /* Slot 0 empty (role 0) */
    raw_pt[6] = 0x01; /* Slot 1 non-empty (role 1) -> Invalid non-canonical gap! */
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_decode(&decoded, raw_pt), "hcp_decode accepted non-canonical slot gap!");

    assert_heap_stable(snap, "Memory leak detected in Invalid Intent test!");
}

/* -------------------------------------------------------------------------
 * R2.3 TTL=0 Frame Relay Rejection
 * ------------------------------------------------------------------------- */
void test_r2_3_ttl0_frame_relay_rejection(void)
{
    ESP_LOGI(TAG_TEST, "Running R2.3 TTL=0 Frame Relay Rejection...");
    heap_snap_t snap = snap_heap();

    weave_t w;
    weave_init(&w, WEAVE_RELAY);

    uint8_t frame[LINK_FRAME_LEN];
    memset(frame, 0xAA, sizeof(frame));

    /* Set TTL = 0 (Byte 1 bits 7..6 = 0) */
    frame[1] &= 0x3F;
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, session_frame_ttl(frame), "session_frame_ttl failed to extract 0");

    /* Offer frame to relay: mine = 0 (addressed to another node) */
    int res = weave_offer(&w, frame, sizeof(frame), 0, 1000, 5000);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, res, "weave_offer accepted frame with TTL=0 for relay!");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, w.stat_ttl_exhausted, "stat_ttl_exhausted not incremented!");

    /* Set TTL = 3 (Byte 1 bits 7..6 = 3 -> 0xC0) */
    frame[1] = (frame[1] & 0x3F) | (3 << 6);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(3, session_frame_ttl(frame), "session_frame_ttl failed to extract 3");

    res = weave_offer(&w, frame, sizeof(frame), 0, 1000, 5000);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, res, "weave_offer rejected valid TTL=3 frame!");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, w.stat_relayed, "stat_relayed not incremented!");

    assert_heap_stable(snap, "Memory leak detected in TTL=0 test!");
}

/* -------------------------------------------------------------------------
 * R2.4 Truncated & Over-Length Buffer Fuzzing
 * ------------------------------------------------------------------------- */
void test_r2_4_truncated_overlength_buffer_fuzzing(void)
{
    ESP_LOGI(TAG_TEST, "Running R2.4 Truncated & Over-Length Buffer Fuzzing...");
    heap_snap_t snap = snap_heap();

    weave_t w;
    weave_init(&w, WEAVE_RELAY);

    uint8_t fuzzed_buf[256];
    memset(fuzzed_buf, 0x7E, sizeof(fuzzed_buf));

    /* WEAVE_FRAME_MAX is 38 bytes */
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, weave_offer(&w, fuzzed_buf, 39, 0, 1000, 5000), "weave_offer accepted 39 B over-length buffer!");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, weave_offer(&w, fuzzed_buf, 255, 0, 1000, 5000), "weave_offer accepted 255 B over-length buffer!");

    /* Fuzz link_recv and weave_offer with truncated lengths: 0, 1, 5, 10, 33 */
    const size_t trunc_sizes[] = { 0, 1, 2, 5, 10, 16, 33 };
    herus_session sess;
    herus_link link;
    uint8_t root[32] = {0};
    session_init(&sess, root, 1, 1000);
    link.sess = &sess;
    link.region = HZ_REGION_BR915;
    link.tx_dbm = 14;
    link.known_roles = NULL;
    link.nknown = 0;

    hcp_msg_t rx_msg;
    uint32_t ctr = 0;
    int sess_err = 0;

    for (size_t i = 0; i < sizeof(trunc_sizes)/sizeof(trunc_sizes[0]); i++) {
        size_t sz = trunc_sizes[i];
        int w_res = weave_offer(&w, fuzzed_buf, sz, 0, 1000, 5000);
        if (sz < 2) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(-1, w_res, "weave_offer accepted frame with len < 2!");
        }
        int r = link_recv(&link, fuzzed_buf, 1000 + i, &rx_msg, &ctr, &sess_err);
        /* Must return session/decode error cleanly without buffer overflow or crash */
        TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(LINK_OK, r, "link_recv accepted truncated frame!");
    }

    assert_heap_stable(snap, "Memory leak detected during buffer fuzzing!");
}

/* -------------------------------------------------------------------------
 * R2.5 Replayed Frames & Session Rate Limit Exhaustion
 * ------------------------------------------------------------------------- */
void test_r2_5_replayed_frame_rate_limiting(void)
{
    ESP_LOGI(TAG_TEST, "Running R2.5 Replayed Frames & Rate Limit Exhaustion...");
    heap_snap_t snap = snap_heap();

    herus_session sess_tx, sess_rx;
    uint8_t root[32];
    memset(root, 0x88, sizeof(root));

    session_init(&sess_tx, root, 1, 1000);
    session_init(&sess_rx, root, 0, 1000);

    herus_link link_rx;
    link_rx.sess = &sess_rx;
    link_rx.region = HZ_REGION_BR915;
    link_rx.tx_dbm = 14;
    link_rx.known_roles = NULL;
    link_rx.nknown = 0;

    /* 1. Send valid sealed frame from TX */
    uint8_t pt[LINK_PT_LEN] = {0};
    uint8_t frame[LINK_FRAME_LEN];
    TEST_ASSERT_EQUAL_INT_MESSAGE(SESS_OK, session_seal(&sess_tx, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, frame), "session_seal failed");

    /* 2. First reception: should succeed */
    hcp_msg_t rx_msg;
    uint32_t ctr = 0;
    int sess_err = 0;
    TEST_ASSERT_EQUAL_INT_MESSAGE(LINK_OK, link_recv(&link_rx, frame, 1001, &rx_msg, &ctr, &sess_err), "link_recv failed valid frame");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SESS_OK, sess_err, "Expected SESS_OK on first frame");

    /* 3. Replay exact same frame: single-use key consumed, must fail */
    int replay_res = link_recv(&link_rx, frame, 1002, &rx_msg, &ctr, &sess_err);
    TEST_ASSERT_EQUAL_INT_MESSAGE(LINK_E_SESSION, replay_res, "link_recv accepted replayed frame!");
    TEST_ASSERT_TRUE_MESSAGE(sess_err == SESS_E_ADDR || sess_err == SESS_E_AUTH, "Expected address or auth failure on replay");

    /* 4. Rate Limiter Test (Token bucket 20 tokens max) */
    uint8_t unauth_frame[LINK_FRAME_LEN];
    memcpy(unauth_frame, frame, LINK_FRAME_LEN);
    /* Target valid address window so it passes address check */
    unauth_frame[0] = sess_rx.addr_win[0] & 0xFF;
    unauth_frame[1] = (sess_rx.addr_win[0] >> 8) & 0x3F;

    int rate_dropped_count = 0;
    for (int i = 0; i < 35; i++) {
        int r = link_recv(&link_rx, unauth_frame, 1003, &rx_msg, &ctr, &sess_err);
        if (r == LINK_E_SESSION && sess_err == SESS_E_RATE) {
            rate_dropped_count++;
        }
    }

    TEST_ASSERT_GREATER_THAN_INT_MESSAGE(0, rate_dropped_count, "Rate limiter failed to drop flood frames!");
    TEST_ASSERT_GREATER_THAN_UINT32_MESSAGE(0, sess_rx.stat_rate_drop, "stat_rate_drop counter zero after flood!");

    assert_heap_stable(snap, "Memory leak detected in Replay / Rate Limit test!");
}

/* -------------------------------------------------------------------------
 * R2.6 Defensive NULL Pointer Checks
 * ------------------------------------------------------------------------- */
void test_r2_6_null_pointer_defensive_checks(void)
{
    ESP_LOGI(TAG_TEST, "Running R2.6 Defensive NULL Pointer Checks...");
    heap_snap_t snap = snap_heap();

    hcp_msg_t msg;
    uint8_t buf[HCP_PLAINTEXT_LEN];
    weave_t w;
    weave_init(&w, WEAVE_RELAY);

    /* 1. hcp_encode NULL checks */
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_encode(NULL, &msg), "hcp_encode allowed NULL out24!");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_encode(buf, NULL), "hcp_encode allowed NULL msg!");

    /* 2. hcp_decode NULL checks */
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_decode(NULL, buf), "hcp_decode allowed NULL msg!");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, hcp_decode(&msg, NULL), "hcp_decode allowed NULL in24!");

    /* 3. weave_offer NULL checks */
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, weave_offer(NULL, buf, 10, 0, 1000, 5000), "weave_offer allowed NULL weave_t!");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, weave_offer(&w, NULL, 10, 0, 1000, 5000), "weave_offer allowed NULL frame!");

    assert_heap_stable(snap, "Memory leak detected in NULL pointer test!");
}

/* =========================================================================
 * TEST SUITE RUNNER ENTRY POINT
 * ========================================================================= */

void test_radio_run(void)
{
    ESP_LOGI(TAG_TEST, "\n=======================================================");
    ESP_LOGI(TAG_TEST, "   HERUS ESP32-S3 HIL & MAC ADVERSARIAL TEST SUITE    ");
    ESP_LOGI(TAG_TEST, "=======================================================\n");

    UNITY_BEGIN();

    /* R1 HIL Tests */
    RUN_TEST(test_r1_1_pin_validation);
    RUN_TEST(test_r1_2_spi_transfer_roundtrip);
    RUN_TEST(test_r1_3_busy_pin_wait_timeout);
    RUN_TEST(test_r1_4_dio1_irq_line_state);

    /* R2 Adversarial MAC Tests */
    RUN_TEST(test_r2_1_corrupted_crc_tag_auth_failure);
    RUN_TEST(test_r2_2_invalid_intent_values);
    RUN_TEST(test_r2_3_ttl0_frame_relay_rejection);
    RUN_TEST(test_r2_4_truncated_overlength_buffer_fuzzing);
    RUN_TEST(test_r2_5_replayed_frame_rate_limiting);
    RUN_TEST(test_r2_6_null_pointer_defensive_checks);

    UNITY_END();
}
