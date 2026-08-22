/* generative_dialogue_loop.h — bounded local multi-step generation lifecycle. */
#ifndef HERUS_GENERATIVE_DIALOGUE_LOOP_H
#define HERUS_GENERATIVE_DIALOGUE_LOOP_H

#include "generative_core.h"
#include "generative_haptic_bridge.h"

#include <stdint.h>

typedef enum {
    GDL_IDLE = 0,
    GDL_GENERATING,
    GDL_PRESENTED,
    GDL_CONFIRMATION_PENDING,
    GDL_CONFIRMED,
    GDL_ABORTED,
    GDL_TIMED_OUT,
    GDL_CLEARED
} gdl_state_t;

typedef enum {
    GDL_OK = 0,
    GDL_E_ARG = -1,
    GDL_E_STATE = -2,
    GDL_E_PHYSICAL = -3,
    GDL_E_TIMEOUT = -4,
    GDL_E_GENERATION = -5,
    GDL_E_CONFIRMATION = -6
} gdl_status_t;

typedef struct {
    uint32_t turn_timeout_ms;
    uint32_t confirmation_timeout_ms;
} gdl_config_t;

typedef struct {
    uint32_t turns_started;
    uint32_t candidates_presented;
    uint32_t confirmations;
    uint32_t confirmation_denied;
    uint32_t generation_failed;
    uint32_t timed_out;
    uint32_t interrupted;
    uint32_t privacy_clears;
} gdl_metrics_t;

typedef struct {
    gdl_config_t cfg;
    gdl_state_t state;
    uint32_t physical_session_id;
    uint32_t started_ms;
    uint32_t presented_ms;
    gc_result_t candidate;
    gh_signal_t signal;
    gdl_metrics_t metrics;
} gdl_t;

void gdl_config_default(gdl_config_t *out);
void gdl_init(gdl_t *loop, const gdl_config_t *cfg);

/* A nonzero session must be supplied by a separately validated physical gesture. */
gdl_status_t gdl_begin(gdl_t *loop, uint32_t physical_session_id, uint32_t now_ms);

/* Candidate ownership stays in the loop only until present/clear; caller input is copied. */
gdl_status_t gdl_present(gdl_t *loop, const gc_result_t *candidate, uint32_t now_ms);

/* Only an explicit physical confirmation can move a pending plan to CONFIRMED. */
gdl_status_t gdl_confirm(gdl_t *loop, uint32_t physical_session_id, uint32_t now_ms);

gdl_status_t gdl_deny(gdl_t *loop, uint32_t physical_session_id, uint32_t now_ms);
gdl_status_t gdl_abort(gdl_t *loop);
gdl_status_t gdl_tick(gdl_t *loop, uint32_t now_ms);

/* Erases candidate, haptic signal and live session. */
gdl_status_t gdl_forget(gdl_t *loop);

const gdl_metrics_t *gdl_metrics(const gdl_t *loop);

#endif /* HERUS_GENERATIVE_DIALOGUE_LOOP_H */
