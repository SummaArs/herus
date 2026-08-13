/* dialogue.c — bounded local conversational intelligence, never a send authority. */
#include "dialogue.h"
#include "crypto.h"
#include <string.h>

static int deadline_reached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

static uint32_t elapsed(uint32_t now, uint32_t then)
{
    return (uint32_t)(now - then);
}

static int topic_valid(dialogue_topic_t topic)
{
    return topic > DIALOGUE_TOPIC_NONE && topic < DIALOGUE_TOPIC_COUNT;
}

static void context_expire(dialogue_t *d, uint32_t now_ms)
{
    for (unsigned i = 0; i < DIALOGUE_CONTEXT_CAP; i++) {
        if (d->context.card[i].topic != DIALOGUE_TOPIC_NONE &&
            deadline_reached(now_ms, d->context.card[i].expires_ms)) {
            memset(&d->context.card[i], 0, sizeof(d->context.card[i]));
            d->metrics.context_expired++;
        }
    }
}

static int reply_valid(const dialogue_model_reply_t *reply)
{
    if (!reply || !topic_valid(reply->topic) || !reply->reply_len ||
        reply->reply_len >= DIALOGUE_REPLY_MAX ||
        reply->reply[reply->reply_len] != '\0') return 0;
    for (unsigned i = 0; i < reply->reply_len; i++) {
        unsigned char c = (unsigned char)reply->reply[i];
        if (c == 0 || c < 0x20u || c == 0x7fu) return 0;
    }
    return 1;
}

void dialogue_config_default(dialogue_config_t *out)
{
    if (!out) return;
    out->turn_timeout_ms = DIALOGUE_DEFAULT_TURN_MS;
    out->context_timeout_ms = DIALOGUE_DEFAULT_CONTEXT_MS;
}

void dialogue_init(dialogue_t *d, const dialogue_config_t *cfg,
                   const dialogue_model_t *model)
{
    dialogue_config_t defaults;
    if (!d) return;
    dialogue_config_default(&defaults);
    memset(d, 0, sizeof(*d));
    d->cfg = cfg ? *cfg : defaults;
    if (!d->cfg.turn_timeout_ms) d->cfg.turn_timeout_ms = defaults.turn_timeout_ms;
    if (!d->cfg.context_timeout_ms) d->cfg.context_timeout_ms = defaults.context_timeout_ms;
    if (model) d->model = *model;
    d->state = DIALOGUE_IDLE;
}

int dialogue_begin_turn(dialogue_t *d, uint32_t physical_session_id, uint32_t now_ms)
{
    if (!d) return DIALOGUE_E_ARG;
    if (!physical_session_id) return DIALOGUE_E_PHYSICAL;
    if (d->state == DIALOGUE_TIMED_OUT || d->state == DIALOGUE_MODEL_FAILED ||
        d->state == DIALOGUE_REJECTED) {
        secure_zero(&d->pending, sizeof(d->pending));
        d->physical_session_id = 0;
        d->state = DIALOGUE_IDLE;
    }
    if (d->state != DIALOGUE_IDLE) return DIALOGUE_E_STATE;
    context_expire(d, now_ms);
    d->physical_session_id = physical_session_id;
    d->turn_started_ms = now_ms;
    d->state = DIALOGUE_LISTENING;
    d->metrics.turns_started++;
    return DIALOGUE_OK;
}

int dialogue_note_topic(dialogue_t *d, dialogue_topic_t topic, uint32_t now_ms)
{
    dialogue_card_t *card;
    if (!d || !topic_valid(topic)) return DIALOGUE_E_ARG;
    context_expire(d, now_ms);
    for (unsigned i = 0; i < DIALOGUE_CONTEXT_CAP; i++) {
        if (d->context.card[i].topic == topic) {
            d->context.card[i].expires_ms = now_ms + d->cfg.context_timeout_ms;
            return DIALOGUE_OK;
        }
    }
    card = &d->context.card[d->next_card % DIALOGUE_CONTEXT_CAP];
    card->topic = topic;
    card->expires_ms = now_ms + d->cfg.context_timeout_ms;
    d->next_card = (d->next_card + 1u) % DIALOGUE_CONTEXT_CAP;
    return DIALOGUE_OK;
}

int dialogue_submit_utterance(dialogue_t *d, const char *utterance, size_t utterance_len,
                              uint32_t now_ms)
{
    dialogue_request_t request;
    dialogue_model_reply_t candidate;
    int rc;
    if (!d || !utterance || !utterance_len || utterance_len > DIALOGUE_UTTERANCE_MAX ||
        memchr(utterance, '\0', utterance_len) != NULL) return DIALOGUE_E_ARG;
    if (d->state != DIALOGUE_LISTENING) return DIALOGUE_E_STATE;
    context_expire(d, now_ms);
    if (elapsed(now_ms, d->turn_started_ms) >= d->cfg.turn_timeout_ms) {
        d->physical_session_id = 0;
        d->state = DIALOGUE_TIMED_OUT;
        d->metrics.timed_out++;
        return DIALOGUE_E_TIMEOUT;
    }
    if (!d->model.generate_local) {
        d->physical_session_id = 0;
        d->state = DIALOGUE_MODEL_FAILED;
        d->metrics.model_failed++;
        return DIALOGUE_E_MODEL;
    }
    request.utterance = utterance;
    request.utterance_len = utterance_len;
    request.context = &d->context;
    memset(&candidate, 0, sizeof(candidate));
    rc = d->model.generate_local(d->model.ctx, &request, &candidate);
    if (rc) {
        secure_zero(&candidate, sizeof(candidate));
        d->physical_session_id = 0;
        d->state = DIALOGUE_MODEL_FAILED;
        d->metrics.model_failed++;
        return DIALOGUE_E_MODEL;
    }
    if (!reply_valid(&candidate)) {
        secure_zero(&candidate, sizeof(candidate));
        d->physical_session_id = 0;
        d->state = DIALOGUE_REJECTED;
        d->metrics.model_rejected++;
        return DIALOGUE_E_REPLY;
    }
    d->pending = candidate;
    secure_zero(&candidate, sizeof(candidate));
    d->physical_session_id = 0;
    d->metrics.last_latency_ms = elapsed(now_ms, d->turn_started_ms);
    d->metrics.replies_ready++;
    d->state = DIALOGUE_REPLY_READY;
    return DIALOGUE_OK;
}

int dialogue_take_reply(dialogue_t *d, char *out, size_t out_cap,
                        dialogue_topic_t *out_topic)
{
    if (!d || !out || !out_topic) return DIALOGUE_E_ARG;
    if (d->state != DIALOGUE_REPLY_READY) return DIALOGUE_E_STATE;
    if (out_cap <= d->pending.reply_len) return DIALOGUE_E_ARG;
    memcpy(out, d->pending.reply, d->pending.reply_len + 1u);
    *out_topic = d->pending.topic;
    secure_zero(&d->pending, sizeof(d->pending));
    d->physical_session_id = 0;
    d->turn_started_ms = 0;
    d->state = DIALOGUE_IDLE;
    return DIALOGUE_OK;
}

int dialogue_tick(dialogue_t *d, uint32_t now_ms)
{
    if (!d) return DIALOGUE_E_ARG;
    context_expire(d, now_ms);
    if (d->state == DIALOGUE_LISTENING &&
        elapsed(now_ms, d->turn_started_ms) >= d->cfg.turn_timeout_ms) {
        d->physical_session_id = 0;
        secure_zero(&d->pending, sizeof(d->pending));
        d->state = DIALOGUE_TIMED_OUT;
        d->metrics.timed_out++;
        return DIALOGUE_E_TIMEOUT;
    }
    return DIALOGUE_OK;
}

void dialogue_forget(dialogue_t *d)
{
    if (!d) return;
    secure_zero(&d->context, sizeof(d->context));
    secure_zero(&d->pending, sizeof(d->pending));
    d->physical_session_id = 0;
    d->turn_started_ms = 0;
    d->next_card = 0;
    d->state = DIALOGUE_IDLE;
    d->metrics.privacy_clears++;
}

const dialogue_metrics_t *dialogue_metrics(const dialogue_t *d)
{
    return d ? &d->metrics : NULL;
}

const dialogue_context_t *dialogue_context(const dialogue_t *d)
{
    return d ? &d->context : NULL;
}
