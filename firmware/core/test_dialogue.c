/* test_dialogue.c — executable contract for Advance 8 conversational boundary. */
#include "dialogue.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned calls;
    unsigned live_cards;
    size_t utterance_len;
    int mode;
} fake_model_t;

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static int contains_bytes(const void *haystack, size_t haystack_len,
                          const void *needle, size_t needle_len)
{
    const uint8_t *h = haystack;
    const uint8_t *n = needle;
    if (!needle_len || needle_len > haystack_len) return 0;
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (!memcmp(h + i, n, needle_len)) return 1;
    }
    return 0;
}

static int zeroed(const void *p, size_t n)
{
    const uint8_t *b = p;
    uint8_t any = 0;
    for (size_t i = 0; i < n; i++) any |= b[i];
    return any == 0;
}

static int fake_generate(void *ctx, const dialogue_request_t *request,
                         dialogue_model_reply_t *out)
{
    fake_model_t *m = ctx;
    m->calls++;
    m->utterance_len = request->utterance_len;
    for (unsigned i = 0; i < DIALOGUE_CONTEXT_CAP; i++) {
        if (request->context->card[i].topic != DIALOGUE_TOPIC_NONE) m->live_cards++;
    }
    memset(out, 0, sizeof(*out));
    if (m->mode == 1) return -1;
    if (m->mode == 2) {
        out->topic = DIALOGUE_TOPIC_NONE;
        strcpy(out->reply, "resposta que nao deve ser aceita");
        out->reply_len = (uint8_t)strlen(out->reply);
        return 0;
    }
    if (m->mode == 3) {
        strcpy(out->reply, "enviar ajuda agora");
        out->reply_len = (uint8_t)strlen(out->reply);
        out->topic = DIALOGUE_TOPIC_SAFETY;
        return 0;
    }
    strcpy(out->reply, "Entendi. Posso explicar o estado local.");
    out->reply_len = (uint8_t)strlen(out->reply);
    out->topic = DIALOGUE_TOPIC_GUIDANCE;
    return 0;
}

int main(void)
{
    dialogue_t d;
    dialogue_model_t model;
    dialogue_config_t cfg;
    fake_model_t fake;
    dialogue_topic_t topic;
    char reply[DIALOGUE_REPLY_MAX];
    static const char utterance[] = "QUASAR-91 conte o estado sem enviar nada";

    memset(&fake, 0, sizeof(fake));
    model.ctx = &fake;
    model.generate_local = fake_generate;
    dialogue_config_default(&cfg);
    cfg.turn_timeout_ms = 100u;
    cfg.context_timeout_ms = 50u;
    dialogue_init(&d, &cfg, &model);

    printf("\n== D1  local conversational boundary ==\n");
    ok(dialogue_begin_turn(&d, 0, 1) == DIALOGUE_E_PHYSICAL &&
       d.state == DIALOGUE_IDLE,
       "D1 no conversation turn begins without a nonzero physical session");

    ok(dialogue_note_topic(&d, DIALOGUE_TOPIC_STATUS, 2) == DIALOGUE_OK &&
       dialogue_note_topic(&d, DIALOGUE_TOPIC_GUIDANCE, 3) == DIALOGUE_OK &&
       dialogue_begin_turn(&d, 77, 4) == DIALOGUE_OK &&
       dialogue_submit_utterance(&d, utterance, strlen(utterance), 10) == DIALOGUE_OK &&
       fake.calls == 1 && fake.live_cards == 2 && fake.utterance_len == strlen(utterance) &&
       d.state == DIALOGUE_REPLY_READY,
       "D1 a local model receives one bounded utterance and typed topic cards only");
    ok(!contains_bytes(&d, sizeof(d), utterance, strlen(utterance)) &&
       dialogue_take_reply(&d, reply, sizeof(reply), &topic) == DIALOGUE_OK &&
       topic == DIALOGUE_TOPIC_GUIDANCE && zeroed(&d.pending, sizeof(d.pending)) &&
       d.state == DIALOGUE_IDLE,
       "D1 the runtime retains no transcript and zeroizes its local UX reply after delivery");

    fake.mode = 3;
    ok(dialogue_begin_turn(&d, 78, 20) == DIALOGUE_OK &&
       dialogue_submit_utterance(&d, utterance, strlen(utterance), 21) == DIALOGUE_OK &&
       dialogue_take_reply(&d, reply, sizeof(reply), &topic) == DIALOGUE_OK &&
       !strcmp(reply, "enviar ajuda agora") && d.state == DIALOGUE_IDLE,
       "D1 even action-looking model text is only a local reply, not a sendable semantic draft");

    fake.mode = 2;
    ok(dialogue_begin_turn(&d, 79, 30) == DIALOGUE_OK &&
       dialogue_submit_utterance(&d, utterance, strlen(utterance), 31) == DIALOGUE_E_REPLY &&
       d.state == DIALOGUE_REJECTED && zeroed(&d.pending, sizeof(d.pending)),
       "D1 malformed or unauthorized model output is rejected without a retained reply");

    fake.mode = 1;
    ok(dialogue_begin_turn(&d, 80, 40) == DIALOGUE_OK &&
       dialogue_submit_utterance(&d, utterance, strlen(utterance), 41) == DIALOGUE_E_MODEL &&
       d.state == DIALOGUE_MODEL_FAILED && dialogue_metrics(&d)->model_failed == 1,
       "D1 unavailable local inference fails closed and permits no fallback network path");

    fake.mode = 0;
    ok(dialogue_begin_turn(&d, 81, 50) == DIALOGUE_OK &&
       dialogue_tick(&d, 150) == DIALOGUE_E_TIMEOUT && d.state == DIALOGUE_TIMED_OUT &&
       dialogue_begin_turn(&d, 82, 151) == DIALOGUE_OK,
       "D1 an expired turn is cleared and a new turn still requires a fresh physical session");
    ok(dialogue_tick(&d, 200) == DIALOGUE_OK && dialogue_metrics(&d)->context_expired == 2,
       "D1 typed context expires independently of a new conversation turn");

    dialogue_forget(&d);
    ok(d.state == DIALOGUE_IDLE && zeroed(&d.context, sizeof(d.context)) &&
       zeroed(&d.pending, sizeof(d.pending)) && dialogue_metrics(&d)->privacy_clears == 1,
       "D1 privacy erase removes all bounded context and pending response without touching transport");

    if (FAILED) {
        printf("DIALOGUE TESTS FAILED\n");
        return 1;
    }
    printf("DIALOGUE INVARIANTS HOLD — local conversation remains bounded, private and non-autonomous.\n");
    return 0;
}
