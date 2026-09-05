/* app_main.c — Herus on an ESP32-S3 + SX1262: bring-up console and radio task.
 *
 * This is the firmware you flash on day one. It exists to answer, in order:
 *   1. is the radio wired correctly?          `selftest`
 *   2. do two boards agree on a lexicon?      `lexcheck`
 *   3. does a meaning cross the air?          `listen` on one, `send` on the other
 *   4. what is the real range on a wrist?     `range 200` and tools/fieldlog.py
 *   5. what does the leaf duty cycle cost?    `beat` and a power analyser
 *
 * Everything it does above the radio is the same portable code the host proof
 * suite exercises, so a bug that appears here and not there is a hardware or a
 * timing bug — which is exactly the discrimination you want on a bench.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_heap_caps.h"

#include "../../hal.h"
#include "../../sx1262.h"
#include "../board_t3s3.h"

#include "hcp.h"
#include "lexicon.h"
#include "crypto.h"
#include "session.h"
#include "region.h"
#include "weave.h"
#include "beat.h"
#include "link.h"

/* from hal_esp32s3.c */
int  herus_board_init(sx_bus_t *bus_out);
int  herus_button_pressed(void);
void herus_led(int on);

/* ------------------------------------------------------------------ state -- */

static sx1262_t      radio;
static herus_session session;
static herus_link    s_link;
static weave_t       weave;
static beat_t        beat;
static lex_t         lexicon;

static uint64_t      domain_seed = 0x48455255530002ull;
static uint8_t       group_key[32];
static int           paired = 0;

/* The roles this build understands. A frame carrying anything else keeps its
 * intent and loses only the field we cannot name (P4). */
static const uint8_t KNOWN_ROLES[] = { 1, 2, 3, 4, 5 };
static const char   *ROLE_NAME[]   = { "?", "who", "what", "where", "when", "how" };

/* Not `mode_t`: POSIX and newlib both already define that, so the obvious name
 * breaks the build on the host AND on the target. Found by the stub syntax check
 * in one second instead of on a bench. */
typedef enum { MODE_IDLE, MODE_LISTEN, MODE_BEAT, MODE_RANGE } herus_mode_t;
static volatile herus_mode_t mode = MODE_IDLE;
static volatile int    range_left = 0;

typedef struct { uint8_t frame[HERUS_SKETCH_FRAME_LEN]; uint8_t len; } txreq_t;
static QueueHandle_t txq;

static const char *TAGA = "herus";

/* ------------------------------------------------------------- helpers ---- */

static void say(const char *s) { printf("%s\n", s); fflush(stdout); }

static int parse_hex(const char *s, uint8_t *out, size_t n)
{
    size_t i = 0;
    for (; i < n && s[i * 2] && s[i * 2 + 1]; i++) {
        char b[3] = { s[i * 2], s[i * 2 + 1], 0 };
        char *end;
        long v = strtol(b, &end, 16);
        if (*end) return -1;
        out[i] = (uint8_t)v;
    }
    return (i == n) ? 0 : -1;
}

static void print_msg(const hcp_msg_t *m, int16_t rssi, int8_t snr, uint32_t ctr)
{
    printf("  <- intent %u  seq %u  ttl %u  [rssi %d dBm, snr %d dB, counter %lu]\n",
           m->intent, m->seq, m->ttl, rssi, snr, (unsigned long)ctr);
    for (int i = 0; i < m->nslot; i++) {
        const char *rn = (m->slot[i].role < sizeof ROLE_NAME / sizeof ROLE_NAME[0])
                       ? ROLE_NAME[m->slot[i].role] : "unknown";
        printf("     %-6s = %u\n", rn, m->slot[i].filler);
    }
    fflush(stdout);
}

/* ---------------------------------------------------------- radio task ---- */

static void radio_task(void *arg)
{
    (void)arg;
    uint8_t frame[HERUS_SKETCH_FRAME_LEN];
    hcp_msg_t msg;
    uint64_t next_range_tx = 0;
    uint16_t range_seq = 0;

    for (;;) {
        txreq_t req;

        /* --- transmit anything queued by the console --- */
        if (xQueueReceive(txq, &req, 0) == pdTRUE) {
            herus_led(1);
            uint64_t t0 = hal_micros();
            sx1262_tx(&radio, req.frame, req.len, 3000);
            while (!(sx1262_irq(&radio) & (SX_IRQ_TX_DONE | SX_IRQ_TIMEOUT)))
                vTaskDelay(pdMS_TO_TICKS(2));
            sx1262_irq_clear(&radio, 0xFFFF);
            printf("  -> %u B on air, %llu us measured (ledger says %u us)\n",
                   req.len, (unsigned long long)(hal_micros() - t0),
                   HERUS_AIRTIME_MEANING_US);
            fflush(stdout);
            herus_led(0);
            if (mode == MODE_LISTEN || mode == MODE_BEAT) sx1262_rx_continuous(&radio);
        }

        /* --- the Phase-0 range walk: a strict cadence is the distance record --- */
        if (mode == MODE_RANGE && range_left > 0 && hal_millis() >= next_range_tx) {
            hcp_msg_t m = {0};
            m.tier = HCP_TIER_GLYPH; m.intent = 1; m.seq = range_seq++;
            uint8_t f[LINK_FRAME_LEN];
            if (link_send(&s_link, &m, 3, f) == LINK_OK) {
                herus_led(1);
                sx1262_tx(&radio, f, LINK_FRAME_LEN, 3000);
                while (!(sx1262_irq(&radio) & (SX_IRQ_TX_DONE | SX_IRQ_TIMEOUT)))
                    vTaskDelay(pdMS_TO_TICKS(2));
                sx1262_irq_clear(&radio, 0xFFFF);
                herus_led(0);
                printf("tx,%u,%llu\n", m.seq, (unsigned long long)hal_millis());
                fflush(stdout);
            }
            next_range_tx = hal_millis() + 2000;   /* 2.000 s: elapsed time IS distance */
            range_left--;
            if (range_left == 0) { mode = MODE_IDLE; say("range walk finished"); }
        }

        /* --- receive --- */
        if (mode == MODE_LISTEN || mode == MODE_BEAT || mode == MODE_RANGE) {
            uint16_t irq = sx1262_irq(&radio);
            if (irq & SX_IRQ_RX_DONE) {
                int16_t rssi = 0; int8_t snr = 0;
                int n = sx1262_read(&radio, frame, sizeof frame, &rssi, &snr);
                if (n == (int)LINK_FRAME_LEN) {
                    int mine = session_addr_in_window(&session, frame);
                    uint32_t ctr = 0; int se = 0;
                    if (mine && link_recv(&s_link, frame, hal_millis(), &msg, &ctr, &se) == LINK_OK) {
                        print_msg(&msg, rssi, snr, ctr);
                        beat_resync(&beat, hal_millis(), 2);
                    } else if (!mine) {
                        /* Not ours: consider relaying. A relay never decrypts and
                         * never learns anything — it moves ciphertext. */
                        int r = weave_offer(&weave, frame, (size_t)n, 0, hal_millis(), 5000);
                        if (r == 1) {
                            txreq_t fw; size_t l;
                            if (weave_next_tx(&weave, hal_millis(), fw.frame, &l)) {
                                fw.len = (uint8_t)l;
                                xQueueSend(txq, &fw, 0);
                                printf("  ~~ relaying a frame that is not ours (ttl now %u)\n",
                                       session_frame_ttl(fw.frame));
                            }
                        }
                    } else {
                        printf("  !! address matched but the frame did not open (session %d)\n", se);
                    }
                    fflush(stdout);
                } else if (n >= 0) {
                    printf("  ?? %d bytes — not a Herus meaning frame\n", n);
                }
                sx1262_rx_continuous(&radio);
            } else if (irq & (SX_IRQ_CRC_ERR | SX_IRQ_HEADER_ERR)) {
                sx1262_irq_clear(&radio, 0xFFFF);
                sx1262_rx_continuous(&radio);
            }
        }

        /* --- Beat: sleep through the empty windows instead of polling --- */
        if (mode == MODE_BEAT) {
            uint32_t wait = beat_until_next_slot_ms(&beat, hal_millis());
            if (wait > 50) {
                sx1262_rx_duty_cycle(&radio, BEAT_RX_MS, BEAT_PERIOD_MS - BEAT_RX_MS);
                hal_sleep_until_radio(wait);
                continue;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* --------------------------------------------------------- console cmds --- */

static int cmd_selftest(int argc, char **argv)
{
    (void)argc; (void)argv;
    char rep[1024];

    say("radio:");
    sx1262_selftest(&radio, rep, sizeof rep);
    printf("%s", rep);

    /* Crypto: one RFC-anchored vector is enough to catch a miscompiled build. The
     * exhaustive differential testing happens on the host (test_net V1-V6); what
     * this proves is that THIS binary computes the same thing. */
    uint8_t md[32];
    static const uint8_t abc_sha[32] = {
        0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
        0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad };
    sha256("abc", 3, md);
    printf("  [%s]   SHA-256(\"abc\") on target\n", memcmp(md, abc_sha, 32) ? "FAIL" : "ok");

    /* Algebra: quasi-orthogonality is the load-bearing assumption of the whole
     * architecture, so measure it here rather than trusting the host run. */
    hv_t a, b;
    double sum = 0;
    for (int i = 0; i < 200; i++) {
        hv_gen(&a, domain_seed, (uint32_t)(2 * i));
        hv_gen(&b, domain_seed, (uint32_t)(2 * i + 1));
        sum += hv_dist(&a, &b);
    }
    double mean = sum / 200.0;
    printf("  [%s]   mean random distance %.1f bits (theory %.1f)\n",
           (mean > HV_BITS * 0.47 && mean < HV_BITS * 0.53) ? "ok" : "FAIL",
           mean, HV_BITS / 2.0);

    /* Timing on the real core, so the projections in the docs stop being
     * projections. */
    uint64_t t0 = hal_micros();
    for (int i = 0; i < 100; i++) { hv_gen(&a, domain_seed, (uint32_t)i); }
    printf("  hv_gen: %.1f us per code (%d bits)\n",
           (double)(hal_micros() - t0) / 100.0, HV_BITS);

    t0 = hal_micros();
    int dsum = 0;
    for (int i = 0; i < 100; i++) dsum += hv_dist(&a, &b);
    printf("  hv_dist: %.1f us per compare (%d bytes)\n",
           (double)(hal_micros() - t0) / 100.0, HV_BYTES);
    (void)dsum;

    printf("  free heap: %u B (internal %u B)\n",
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    printf("  session %u B, weave %u B, beat %u B, lexicon sketches %u B\n",
           (unsigned)sizeof(herus_session), (unsigned)sizeof(weave_t),
           (unsigned)sizeof(beat_t), (unsigned)(lexicon.n * sizeof(hv_sk_t)));
    fflush(stdout);
    return 0;
}

static int cmd_pair(int argc, char **argv)
{
    uint8_t root[32];
    int initiator = 1;

    if (argc >= 2 && strcmp(argv[1], "demo") == 0) {
        /* Two boards, one command, no key exchange: the fastest path to "a meaning
         * crossed the air". This key is PUBLIC — it is in the source. Anything sent
         * under it is not private, and the console says so every time. */
        memset(root, 0x42, sizeof root);
        initiator = (argc >= 3 && strcmp(argv[2], "b") == 0) ? 0 : 1;
        say("!! DEMO KEY (0x42 x 32) — this traffic is NOT private. Development only.");
    } else if (argc >= 2 && parse_hex(argv[1], root, 32) == 0) {
        initiator = (argc >= 3 && strcmp(argv[2], "b") == 0) ? 0 : 1;
    } else {
        say("usage: pair demo [a|b]   |   pair <64 hex chars> [a|b]");
        say("  The two ends must pick different roles: one 'a', one 'b'.");
        return 1;
    }

    session_init(&session, root, initiator, hal_millis());
    memcpy(group_key, root, 32);
    /* Not named `link`: POSIX declares link() and a global object of that name is a
     * type conflict the moment unistd.h lands in the translation unit. Second
     * collision caught by the stub check; both would have been bench time. */
    s_link.sess = &session;
    s_link.region = HZ_REGION_BR915;
    s_link.tx_dbm = BOARD_TX_DBM;
    s_link.known_roles = KNOWN_ROLES;
    s_link.nknown = (int)(sizeof KNOWN_ROLES);
    paired = 1;
    secure_zero(root, sizeof root);
    printf("paired as %s; first expected address 0x%04X\n",
           initiator ? "a (initiator)" : "b", session.addr_win[0]);
    return 0;
}

static int cmd_send(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    say("raw console send is disabled: no assurance snapshot is available");
    return 1;
}

static int cmd_listen(int argc, char **argv)
{
    (void)argc; (void)argv;
    if (!paired) say("not paired: you will see frames arrive but none will open");
    sx1262_set_mode(&radio, HERUS_SF_MEANING, 0, 1, (uint8_t)LINK_FRAME_LEN);
    sx1262_rx_continuous(&radio);
    mode = MODE_LISTEN;
    say("listening (continuous RX). `idle` to stop.");
    return 0;
}

static int cmd_beat(int argc, char **argv)
{
    (void)argc; (void)argv;
    beat_init(&beat, hal_millis(), BEAT_PERIOD_MS, 3);
    sx1262_set_mode(&radio, HERUS_SF_MEANING, 0, 1, (uint8_t)LINK_FRAME_LEN);
    mode = MODE_BEAT;
    printf("Beat: %u ms RX every %u ms, guard %u ms. Measure the average current now.\n",
           BEAT_RX_MS, BEAT_PERIOD_MS, BEAT_GUARD_MS);
    say("NOTE: this build uses light sleep (~240 uA), not deep sleep. The 74 uA");
    say("target needs ratchet state persisted first — see hal_sleep_until_radio().");
    return 0;
}

static int cmd_range(int argc, char **argv)
{
    if (!paired) { say("pair first"); return 1; }
    range_left = (argc >= 2) ? atoi(argv[1]) : 200;
    sx1262_set_mode(&radio, HERUS_SF_MEANING, 0, 1, (uint8_t)LINK_FRAME_LEN);
    mode = MODE_RANGE;
    printf("range walk: %d frames at a strict 2.000 s cadence.\n", range_left);
    say("Start your stopwatch NOW and note the time at each distance mark.");
    say("Log the receiving board's output and feed both to tools/fieldlog.py.");
    return 0;
}

static int cmd_sketch(int argc, char **argv)
{
    if (!paired) { say("pair first"); return 1; }
    uint32_t id = (argc >= 2) ? (uint32_t)strtoul(argv[1], NULL, 0) : 7;

    /* Tier 0.5: the sketch of a symbol's dense code, keystream-XORed, no MAC, no
     * CRC, implicit header — 38 bytes so the airtime matches a meaning frame. */
    hv_t v; hv_sk_t sk;
    lex_code(&lexicon, &v, id);
    hv_sk_make(&sk, &v, lexicon.pos);

    txreq_t req;
    static uint32_t epoch = 0;
    sketch_seal(group_key, epoch, (const uint8_t *)sk.w, HERUS_SKETCH_BYTES, req.frame);
    sketch_pad(group_key, epoch, HERUS_SKETCH_BYTES,
               req.frame + 2 + HERUS_SKETCH_BYTES, HERUS_SKETCH_PAD);
    req.len = (uint8_t)HERUS_SKETCH_FRAME_LEN;
    epoch++;

    sx1262_set_mode(&radio, HERUS_SF_MEANING, 1, 0, (uint8_t)HERUS_SKETCH_FRAME_LEN);
    xQueueSend(txq, &req, 0);
    printf("Tier 0.5 beacon for symbol %lu: %u B, implicit header, CRC off, %u us airtime\n",
           (unsigned long)id, (unsigned)HERUS_SKETCH_FRAME_LEN, HERUS_AIRTIME_SKETCH_US);
    say("Remember what this tier is for: broadcast where no ACK is possible.");
    return 0;
}

static int cmd_lexcheck(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* Two boards must generate byte-identical codebooks from one 64-bit seed. This
     * is the cheapest possible confidence check on "derived, never transmitted":
     * print a fingerprint, compare it by eye across two boards. */
    uint8_t h[32];
    sha256_ctx c; sha256_init(&c);
    hv_t v;
    for (uint32_t i = 0; i < 64; i++) { lex_code(&lexicon, &v, i); sha256_update(&c, v.w, sizeof v.w); }
    sha256_final(&c, h);
    printf("domain seed 0x%016llx\n", (unsigned long long)domain_seed);
    printf("codebook fingerprint (first 64 symbols): ");
    for (int i = 0; i < 8; i++) printf("%02x", h[i]);
    printf("\nThis string MUST be identical on every board in the domain.\n");
    return 0;
}

static int cmd_stats(int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("session: sent %lu, opened %lu, auth-fail %lu, rate-dropped %lu, recv counter %lu\n",
           (unsigned long)session.stat_sent, (unsigned long)session.stat_opened,
           (unsigned long)session.stat_auth_fail, (unsigned long)session.stat_rate_drop,
           (unsigned long)session.recv.n);
    printf("weave:   relayed %lu, duplicates %lu, expired %lu, ttl-exhausted %lu, role %d\n",
           (unsigned long)weave.stat_relayed, (unsigned long)weave.stat_dup,
           (unsigned long)weave.stat_expired, (unsigned long)weave.stat_ttl_exhausted,
           (int)weave.role);
    printf("beat:    drift since resync %lu us (guard %lu us)\n",
           (unsigned long)beat_drift_us(&beat, hal_millis()),
           (unsigned long)(BEAT_GUARD_MS * 1000u));
    printf("radio:   status 0x%02X, device errors 0x%04X, noise %d dBm\n",
           sx1262_status(&radio), sx1262_device_errors(&radio), sx1262_rssi_inst(&radio));
    return 0;
}

static int cmd_idle(int argc, char **argv)
{
    (void)argc; (void)argv;
    mode = MODE_IDLE;
    range_left = 0;
    sx1262_standby(&radio);
    say("idle");
    return 0;
}

/* test_radio.c */
void test_radio_run(void);

static int cmd_test_radio(int argc, char **argv)
{
    (void)argc; (void)argv;
    test_radio_run();
    return 0;
}

static void reg(const char *cmd, const char *help, esp_console_cmd_func_t fn)
{
    const esp_console_cmd_t c = { .command = cmd, .help = help, .hint = NULL, .func = fn };
    esp_console_cmd_register(&c);
}

/* ------------------------------------------------------------------ main --- */

void app_main(void)
{
    sx_bus_t bus;
    printf("\n\nHERUS  frame %u B  airtime %.1f ms  SF%u  %.3f MHz  D=%d bits\n",
           HERUS_FRAME_LEN, HERUS_AIRTIME_MEANING_US / 1000.0, HERUS_SF_MEANING,
           BOARD_FREQ_HZ / 1e6, HV_BITS);

    if (herus_board_init(&bus) != 0) { say("board init failed"); return; }

    sx_cfg_t cfg = {
        .freq_hz = BOARD_FREQ_HZ,
        .sf = HERUS_SF_MEANING, .bw = SX_BW_125, .cr = SX_CR_4_5, .ldro = 0,
        .tx_dbm = BOARD_TX_DBM, .preamble_sym = 8,
        .implicit_header = 0, .crc_on = 1, .payload_len = (uint8_t)LINK_FRAME_LEN,
        .use_tcxo = BOARD_HAS_TCXO, .tcxo_voltage = BOARD_TCXO_VOLT,
        .tcxo_delay_us = BOARD_TCXO_DELAY_US,
        .dio2_as_rf_switch = BOARD_DIO2_RF_SWITCH, .boosted_rx = 1,
    };
    int rc = sx1262_init(&radio, &bus, &cfg);
    printf("radio init: %s\n", rc == SX_OK ? "ok" : "FAILED — run selftest");

    if (lex_init(&lexicon, domain_seed, 512) != 0) say("lexicon alloc failed");
    weave_init(&weave, WEAVE_LEAF);
    beat_init(&beat, hal_millis(), BEAT_PERIOD_MS, 3);
    txq = xQueueCreate(4, sizeof(txreq_t));
    xTaskCreate(radio_task, "herus_radio", 8192, NULL, 5, NULL);

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t rcfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    rcfg.prompt = "herus>";
    rcfg.max_cmdline_length = 160;

#if defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t dev = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&dev, &rcfg, &repl));
#else
    esp_console_dev_uart_config_t dev = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&dev, &rcfg, &repl));
#endif

    esp_console_register_help_command();
    reg("selftest", "radio + crypto + algebra + RAM. RUN THIS FIRST.", cmd_selftest);
    reg("test",     "run Unity hardware & MAC adversarial tests", cmd_test_radio);
    reg("lexcheck", "codebook fingerprint — must match on every board", cmd_lexcheck);
    reg("pair",     "pair demo [a|b] | pair <64 hex> [a|b]", cmd_pair);
    reg("send",     "send <intent> [role:filler ...]", cmd_send);
    reg("listen",   "continuous receive", cmd_listen);
    reg("beat",     "duty-cycled receive (measure current here)", cmd_beat);
    reg("range",    "range <n>: n frames at a 2.000 s cadence for the walk", cmd_range);
    reg("sketch",   "sketch <id>: a Tier 0.5 beacon", cmd_sketch);
    reg("stats",    "counters for session, weave, beat and radio", cmd_stats);
    reg("idle",     "stop transmitting and receiving", cmd_idle);

    say("");
    say("Start with:  selftest");
    say("Then on board A:  pair demo a   / on board B:  pair demo b");
    say("B: listen        A: send 41 1:7 3:300");
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
    ESP_LOGI(TAGA, "console up");
}
