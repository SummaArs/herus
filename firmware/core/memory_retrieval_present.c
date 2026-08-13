/* memory_retrieval_present.c — symbolic, one-shot human status presentation. */
#include "memory_retrieval_present.h"
#include <string.h>

#define MEMORY_RETRIEVAL_KNOWN_REASONS \
    (MEMORY_RETRIEVAL_REASON_KIND | MEMORY_RETRIEVAL_REASON_ORIGIN | \
     MEMORY_RETRIEVAL_REASON_EXPLICIT | MEMORY_RETRIEVAL_REASON_CONFIDENCE | \
     MEMORY_RETRIEVAL_REASON_NOVELTY | MEMORY_RETRIEVAL_REASON_FUTURE | \
     MEMORY_RETRIEVAL_REASON_CONSEQUENCE)

static int access_valid(const memory_consolidation_access_t *access)
{
    return access && access->physical_session_id != 0u &&
           access->physical_confirmed == 1u;
}

static int zero_winner(const memory_retrieval_result_t *result)
{
    return result->card_id == 0u && result->kind == MEMORY_KIND_NONE &&
           result->origin == MEMORY_EXTRACT_ORIGIN_NONE && result->reasons == 0u;
}

static int result_valid(const memory_retrieval_result_t *result)
{
    uint32_t delta;
    if (!result || result->score_pct > 100u || result->runner_up_score_pct > 100u ||
        result->runner_up_score_pct > result->score_pct)
        return 0;
    delta = (uint32_t)result->score_pct - (uint32_t)result->runner_up_score_pct;
    switch (result->status) {
    case MEMORY_RETRIEVAL_NO_MATCH:
        return zero_winner(result) && result->score_pct < MEMORY_RETRIEVAL_MIN_SCORE;
    case MEMORY_RETRIEVAL_AMBIGUOUS:
        return zero_winner(result) && result->score_pct >= MEMORY_RETRIEVAL_MIN_SCORE &&
               result->runner_up_score_pct != 0u &&
               delta < MEMORY_RETRIEVAL_MIN_MARGIN;
    case MEMORY_RETRIEVAL_MATCH:
        return result->card_id != 0u &&
               result->kind > MEMORY_KIND_NONE && result->kind < MEMORY_KIND_COUNT &&
               result->origin > MEMORY_EXTRACT_ORIGIN_NONE &&
               result->origin < MEMORY_EXTRACT_ORIGIN_COUNT &&
               result->score_pct >= MEMORY_RETRIEVAL_MIN_SCORE &&
               (result->runner_up_score_pct == 0u ||
                delta >= MEMORY_RETRIEVAL_MIN_MARGIN) &&
               result->reasons != 0u &&
               (result->reasons & ~MEMORY_RETRIEVAL_KNOWN_REASONS) == 0u;
    default:
        return 0;
    }
}

static void presentation_clear(memory_retrieval_present_t *p)
{
    memset(&p->pending, 0, sizeof(p->pending));
    p->active_physical_session_id = 0u;
}

static void blocked(memory_retrieval_present_t *p)
{
    presentation_clear(p);
    p->state = MEMORY_RETRIEVAL_PRESENT_BLOCKED;
}

static void set_haptic(memory_retrieval_presentation_t *out, voice_event_t event)
{
    voice_haptic_plan(event, &out->haptic);
    if (!haptic_plan_safe(&out->haptic)) memset(&out->haptic, 0, sizeof(out->haptic));
}

void memory_retrieval_present_init(memory_retrieval_present_t *p)
{
    if (!p) return;
    memset(p, 0, sizeof(*p));
    p->state = MEMORY_RETRIEVAL_PRESENT_IDLE;
}

int memory_retrieval_present_show(memory_retrieval_present_t *p,
                                  const memory_consolidation_access_t *access,
                                  const memory_retrieval_result_t *result,
                                  memory_retrieval_presentation_t *out)
{
    if (!p || !out) return MEMORY_RETRIEVAL_PRESENT_E_ARG;
    memset(out, 0, sizeof(*out));
    if (p->state != MEMORY_RETRIEVAL_PRESENT_IDLE) {
        p->metrics.rejected_state++;
        return MEMORY_RETRIEVAL_PRESENT_E_STATE;
    }
    if (!access_valid(access)) {
        p->metrics.rejected_access++;
        return MEMORY_RETRIEVAL_PRESENT_E_ACCESS;
    }
    if (!result_valid(result)) {
        p->metrics.rejected_result++;
        blocked(p);
        return MEMORY_RETRIEVAL_PRESENT_E_RESULT;
    }

    memset(&p->pending, 0, sizeof(p->pending));
    switch (result->status) {
    case MEMORY_RETRIEVAL_MATCH:
        p->pending.phrase = MEMORY_RETRIEVAL_PHRASE_MATCH_AVAILABLE;
        p->pending.kind = result->kind;
        p->pending.origin = result->origin;
        p->pending.reasons = result->reasons;
        set_haptic(&p->pending, VOICE_EVENT_DRAFT);
        p->metrics.shown_match++;
        break;
    case MEMORY_RETRIEVAL_NO_MATCH:
        p->pending.phrase = MEMORY_RETRIEVAL_PHRASE_NO_MATCH;
        set_haptic(&p->pending, VOICE_EVENT_CANCEL);
        p->metrics.shown_no_match++;
        break;
    case MEMORY_RETRIEVAL_AMBIGUOUS:
        p->pending.phrase = MEMORY_RETRIEVAL_PHRASE_AMBIGUOUS_REVIEW;
        set_haptic(&p->pending, VOICE_EVENT_UNKNOWN);
        p->metrics.shown_ambiguous++;
        break;
    default:
        p->metrics.rejected_result++;
        blocked(p);
        return MEMORY_RETRIEVAL_PRESENT_E_RESULT;
    }
    p->active_physical_session_id = access->physical_session_id;
    p->state = MEMORY_RETRIEVAL_PRESENT_SHOWN;
    *out = p->pending;
    return MEMORY_RETRIEVAL_PRESENT_OK;
}

int memory_retrieval_present_dismiss(memory_retrieval_present_t *p,
                                     const memory_consolidation_access_t *access)
{
    if (!p) return MEMORY_RETRIEVAL_PRESENT_E_ARG;
    if (p->state != MEMORY_RETRIEVAL_PRESENT_SHOWN) {
        p->metrics.rejected_state++;
        return MEMORY_RETRIEVAL_PRESENT_E_STATE;
    }
    if (!access_valid(access) ||
        access->physical_session_id != p->active_physical_session_id) {
        p->metrics.rejected_access++;
        return MEMORY_RETRIEVAL_PRESENT_E_ACCESS;
    }
    presentation_clear(p);
    p->state = MEMORY_RETRIEVAL_PRESENT_IDLE;
    p->metrics.dismissed++;
    return MEMORY_RETRIEVAL_PRESENT_OK;
}

const memory_retrieval_present_metrics_t *memory_retrieval_present_metrics(
    const memory_retrieval_present_t *p)
{
    return p ? &p->metrics : 0;
}
