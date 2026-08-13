/* test_memory_capture.c — explicit memory-capture session invariants. */
#include "memory_capture.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

typedef struct {
    unsigned calls;
    uint32_t seen_session;
    size_t seen_len;
    uint8_t first_byte;
    int result;
} fake_adapter_t;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static int all_zero(const uint8_t *p, size_t n)
{
    while (n-- > 0u) if (*p++ != 0u) return 0;
    return 1;
}

static int consume(void *ctx, uint32_t session, uint8_t *bytes, size_t len)
{
    fake_adapter_t *a = ctx;
    a->calls++;
    a->seen_session = session;
    a->seen_len = len;
    a->first_byte = len ? bytes[0] : 0u;
    return a->result;
}

static memory_capture_t fresh(fake_adapter_t *adapter)
{
    memory_capture_t m;
    memory_capture_config_t cfg;
    memory_capture_adapter_t port;
    memory_capture_config_default(&cfg);
    cfg.window_ms = 100u;
    memset(&port, 0, sizeof(port));
    port.ctx = adapter;
    port.consume_transient = consume;
    memory_capture_init(&m, &cfg, &port);
    return m;
}

int main(void)
{
    fake_adapter_t adapter;
    memory_capture_t m;
    uint8_t bytes[8];
    uint8_t oversized[MEMORY_CAPTURE_MAX_TRANSIENT_BYTES + 1u];
    uint32_t first;
    uint32_t second;

    printf("\n== M2 explicit memory capture is physical, one-shot and transient ==\n");
    memset(&adapter, 0, sizeof(adapter));
    m = fresh(&adapter);

    ok(memory_capture_begin(&m, 0u, 10u) == MEMORY_CAPTURE_E_PHYSICAL &&
       m.state == MEMORY_CAPTURE_IDLE && m.metrics.sessions_started == 0u,
       "M2 no capture starts without a physical gesture");

    ok(memory_capture_begin(&m, 41u, 10u) == MEMORY_CAPTURE_OK &&
       m.state == MEMORY_CAPTURE_CAPTURING &&
       memory_capture_actions(&m)->start_capture == 1u &&
       memory_capture_actions(&m)->memory_indicator == 1u,
       "M2 physical gesture opens a visible bounded local capture window");
    first = memory_capture_session_id(&m);
    ok(first != 0u && memory_capture_begin(&m, 42u, 11u) == MEMORY_CAPTURE_E_STATE,
       "M2 a live capture cannot be re-entered or replaced by another gesture");

    memset(bytes, 0xA5, sizeof(bytes));
    ok(memory_capture_deliver(&m, first + 1u, bytes, sizeof(bytes), 20u) ==
       MEMORY_CAPTURE_E_SESSION && adapter.calls == 0u && all_zero(bytes, sizeof(bytes)) &&
       m.state == MEMORY_CAPTURE_CAPTURING,
       "M2 stale or mismatched source data is never consumed and is scrubbed immediately");

    memset(bytes, 0x5A, sizeof(bytes));
    ok(memory_capture_deliver(&m, first, bytes, sizeof(bytes), 25u) == MEMORY_CAPTURE_OK &&
       adapter.calls == 1u && adapter.seen_session == first && adapter.seen_len == sizeof(bytes) &&
       adapter.first_byte == 0x5Au && all_zero(bytes, sizeof(bytes)) &&
       m.state == MEMORY_CAPTURE_DELIVERED && memory_capture_session_id(&m) == 0u &&
       memory_capture_actions(&m)->stop_capture == 1u &&
       memory_capture_actions(&m)->discard_transient == 1u &&
       memory_capture_actions(&m)->memory_indicator == 0u,
       "M2 one synchronous delivery closes capture and zeroizes its buffer");

    ok(memory_capture_begin(&m, 41u, 30u) == MEMORY_CAPTURE_OK,
       "M2 a completed capture permits a new physical session");
    second = memory_capture_session_id(&m);
    memset(bytes, 0x3C, sizeof(bytes));
    ok(second != first && memory_capture_deliver(&m, first, bytes, sizeof(bytes), 35u) ==
       MEMORY_CAPTURE_E_SESSION && all_zero(bytes, sizeof(bytes)) && adapter.calls == 1u,
       "M2 a late buffer from an earlier capture cannot enter a later session");
    ok(memory_capture_cancel(&m, 41u, 40u) == MEMORY_CAPTURE_OK &&
       m.state == MEMORY_CAPTURE_CANCELLED && memory_capture_session_id(&m) == 0u &&
       memory_capture_actions(&m)->discard_transient == 1u,
       "M2 physical cancellation removes authorization and requests source discard");

    ok(memory_capture_begin(&m, 51u, 100u) == MEMORY_CAPTURE_OK &&
       memory_capture_tick(&m, 199u) == MEMORY_CAPTURE_OK &&
       m.state == MEMORY_CAPTURE_CAPTURING &&
       memory_capture_tick(&m, 200u) == MEMORY_CAPTURE_E_STATE &&
       m.state == MEMORY_CAPTURE_TIMED_OUT && memory_capture_session_id(&m) == 0u &&
       memory_capture_actions(&m)->discard_transient == 1u,
       "M2 expiry closes the window even if the source adapter remains silent");

    memset(oversized, 0x2Du, sizeof(oversized));
    ok(memory_capture_begin(&m, 61u, 300u) == MEMORY_CAPTURE_OK &&
       memory_capture_deliver(&m, memory_capture_session_id(&m), oversized,
                              sizeof(oversized), 305u) == MEMORY_CAPTURE_E_SIZE &&
       all_zero(oversized, sizeof(oversized)) && m.state == MEMORY_CAPTURE_FAILED &&
       memory_capture_session_id(&m) == 0u && memory_capture_actions(&m)->discard_transient == 1u,
       "M2 an oversized source buffer is scrubbed and terminates authorization");

    adapter.result = -1;
    ok(memory_capture_begin(&m, 71u, 310u) == MEMORY_CAPTURE_OK,
       "M2 a new session can start after a rejected source without reviving old data");
    memset(bytes, 0x7E, sizeof(bytes));
    ok(memory_capture_deliver(&m, memory_capture_session_id(&m), bytes, sizeof(bytes), 315u) ==
       MEMORY_CAPTURE_E_ADAPTER && all_zero(bytes, sizeof(bytes)) &&
       m.state == MEMORY_CAPTURE_FAILED && memory_capture_session_id(&m) == 0u &&
       memory_capture_actions(&m)->discard_transient == 1u,
       "M2 adapter failure still scrubs the buffer and fails closed");

    if (FAILED) {
        printf("MEMORY CAPTURE TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY CAPTURE INVARIANTS HOLD — capture is physical, bounded, one-shot and transient.\n");
    return 0;
}
