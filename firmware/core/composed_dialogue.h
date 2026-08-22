/* composed_dialogue.h — local composition of routing, generation and haptics. */
#ifndef HERUS_COMPOSED_DIALOGUE_H
#define HERUS_COMPOSED_DIALOGUE_H

#include "generative_dialogue_loop.h"
#include "intent_router.h"

#include <stddef.h>
#include <stdint.h>

enum {
    CDH_OK = 0,
    CDH_E_ARG = -1,
    CDH_E_STATE = -2,
    CDH_E_ROUTE = -3,
    CDH_E_GENERATION = -4,
    CDH_E_CONFIRMATION = -5,
    CDH_E_PHYSICAL = -6,
    CDH_E_TIMEOUT = -7
};

typedef enum {
    CDH_IDLE = 0,
    CDH_ROUTED,
    CDH_GENERATING,
    CDH_PRESENTED,
    CDH_CONFIRMATION_PENDING,
    CDH_CONFIRMED,
    CDH_ABSTAINED,
    CDH_ABORTED,
    CDH_TIMED_OUT,
    CDH_CLEARED
} cdh_state_t;

typedef struct {
    const intent_router_memory_t *router_memories;
    size_t router_memory_count;
    const gc_lexicon_t *lexicon;
    const sr_reasoner_t *reasoner;
    const mse_index_t *memory;
    const pa_profile_t *personal_profile;
    sr_pattern_t default_query;
    const sp_problem_t *plan_problem;
    uint32_t current_generation;
    gdl_config_t lifecycle_config;
} cdh_config_t;

typedef struct {
    cdh_config_t cfg;
    cdh_state_t state;
    gdl_t lifecycle;
    sr_reasoner_t scratch;
    intent_router_result_t route;
    gc_request_t request;
    gc_result_t generated;
    uint8_t memory_quarantined;
    uint32_t recovered_generation;
} cdh_t;

void cdh_init(cdh_t *dialogue, const cdh_config_t *config);

/* Routes one borrowed text observation and opens a physical turn. */
int cdh_start(cdh_t *dialogue, const char *text, size_t length,
              uint32_t physical_session_id, uint32_t now_ms);

/* Generates and presents the typed candidate selected by the route. */
int cdh_generate_present(cdh_t *dialogue, uint32_t now_ms);

int cdh_confirm(cdh_t *dialogue, uint32_t physical_session_id, uint32_t now_ms);
int cdh_deny(cdh_t *dialogue, uint32_t physical_session_id, uint32_t now_ms);
int cdh_tick(cdh_t *dialogue, uint32_t now_ms);
int cdh_abort(cdh_t *dialogue);
int cdh_forget(cdh_t *dialogue);

/* Scrubs active semantic/session inputs and requires an explicit newer rearm. */
int cdh_reboot(cdh_t *dialogue, uint32_t recovered_generation);
int cdh_rearm(cdh_t *dialogue,
              const intent_router_memory_t *router_memories,
              size_t router_memory_count,
              const mse_index_t *memory,
              uint32_t current_generation);

#endif /* HERUS_COMPOSED_DIALOGUE_H */
