#include "generative_dialogue_loop.h"

#include <string.h>

static void clear_payload(gdl_t *loop)
{
    memset(&loop->candidate, 0, sizeof(loop->candidate));
    memset(&loop->signal, 0, sizeof(loop->signal));
    loop->physical_session_id = 0u;
    loop->started_ms = 0u;
    loop->presented_ms = 0u;
}

static int is_terminal(gdl_state_t state)
{
    return state == GDL_ABORTED || state == GDL_TIMED_OUT ||
           state == GDL_CLEARED;
}

void gdl_config_default(gdl_config_t *out)
{
    if (out == NULL) return;
    out->turn_timeout_ms = 10000u;
    out->confirmation_timeout_ms = 5000u;
}

void gdl_init(gdl_t *loop, const gdl_config_t *cfg)
{
    gdl_config_t defaults;
    if (loop == NULL) return;
    memset(loop, 0, sizeof(*loop));
    gdl_config_default(&defaults);
    loop->cfg = cfg == NULL ? defaults : *cfg;
    loop->state = GDL_IDLE;
}

gdl_status_t gdl_begin(gdl_t *loop, uint32_t physical_session_id,
                       uint32_t now_ms)
{
    if (loop == NULL || physical_session_id == 0u) return GDL_E_ARG;
    if (loop->state != GDL_IDLE && !is_terminal(loop->state))
        return GDL_E_STATE;
    clear_payload(loop);
    loop->state = GDL_GENERATING;
    loop->physical_session_id = physical_session_id;
    loop->started_ms = now_ms;
    loop->metrics.turns_started++;
    return GDL_OK;
}

gdl_status_t gdl_present(gdl_t *loop, const gc_result_t *candidate,
                         uint32_t now_ms)
{
    if (loop == NULL || candidate == NULL) return GDL_E_ARG;
    if (loop->state != GDL_GENERATING) return GDL_E_STATE;
    if (candidate->status != GC_STATUS_OK &&
        candidate->status != GC_STATUS_ABSTAIN &&
        candidate->status != GC_STATUS_LIMIT) {
        clear_payload(loop);
        loop->state = GDL_ABORTED;
        loop->metrics.generation_failed++;
        return GDL_E_GENERATION;
    }
    loop->candidate = *candidate;
    if (gh_from_result(&loop->candidate, &loop->signal) != HL_OK) {
        clear_payload(loop);
        loop->state = GDL_ABORTED;
        loop->metrics.generation_failed++;
        return GDL_E_GENERATION;
    }
    loop->presented_ms = now_ms;
    loop->metrics.candidates_presented++;
    if (loop->signal.confirmation_required != 0u)
        loop->state = GDL_CONFIRMATION_PENDING;
    else
        loop->state = GDL_PRESENTED;
    return GDL_OK;
}

gdl_status_t gdl_confirm(gdl_t *loop, uint32_t physical_session_id,
                         uint32_t now_ms)
{
    (void)now_ms;
    if (loop == NULL || physical_session_id == 0u) return GDL_E_ARG;
    if (loop->state != GDL_CONFIRMATION_PENDING)
        return GDL_E_STATE;
    if (physical_session_id != loop->physical_session_id)
        return GDL_E_PHYSICAL;
    if (loop->signal.abstained != 0u ||
        loop->signal.confirmation_required == 0u)
        return GDL_E_CONFIRMATION;
    loop->state = GDL_CONFIRMED;
    loop->physical_session_id = 0u;
    loop->metrics.confirmations++;
    return GDL_OK;
}

gdl_status_t gdl_deny(gdl_t *loop, uint32_t physical_session_id,
                      uint32_t now_ms)
{
    (void)now_ms;
    if (loop == NULL || physical_session_id == 0u) return GDL_E_ARG;
    if (loop->state != GDL_CONFIRMATION_PENDING)
        return GDL_E_STATE;
    if (physical_session_id != loop->physical_session_id)
        return GDL_E_PHYSICAL;
    clear_payload(loop);
    loop->state = GDL_ABORTED;
    loop->metrics.confirmation_denied++;
    return GDL_OK;
}

gdl_status_t gdl_abort(gdl_t *loop)
{
    if (loop == NULL) return GDL_E_ARG;
    if (loop->state == GDL_IDLE || loop->state == GDL_CLEARED)
        return GDL_E_STATE;
    clear_payload(loop);
    loop->state = GDL_ABORTED;
    loop->metrics.interrupted++;
    return GDL_OK;
}

gdl_status_t gdl_tick(gdl_t *loop, uint32_t now_ms)
{
    uint32_t elapsed;
    uint32_t limit;
    if (loop == NULL) return GDL_E_ARG;
    if (loop->state != GDL_GENERATING &&
        loop->state != GDL_PRESENTED &&
        loop->state != GDL_CONFIRMATION_PENDING)
        return GDL_OK;
    if (loop->state == GDL_CONFIRMATION_PENDING) {
        elapsed = now_ms - loop->presented_ms;
        limit = loop->cfg.confirmation_timeout_ms;
    } else {
        elapsed = now_ms - loop->started_ms;
        limit = loop->cfg.turn_timeout_ms;
    }
    if (elapsed < limit) return GDL_OK;
    clear_payload(loop);
    loop->state = GDL_TIMED_OUT;
    loop->metrics.timed_out++;
    return GDL_E_TIMEOUT;
}

gdl_status_t gdl_forget(gdl_t *loop)
{
    if (loop == NULL) return GDL_E_ARG;
    clear_payload(loop);
    loop->state = GDL_CLEARED;
    loop->metrics.privacy_clears++;
    return GDL_OK;
}

const gdl_metrics_t *gdl_metrics(const gdl_t *loop)
{
    return loop == NULL ? NULL : &loop->metrics;
}
