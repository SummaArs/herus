#include "composed_dialogue.h"

#include <string.h>

static void clear_transient(cdh_t *dialogue)
{
    memset(&dialogue->route, 0, sizeof(dialogue->route));
    memset(&dialogue->request, 0, sizeof(dialogue->request));
    memset(&dialogue->generated, 0, sizeof(dialogue->generated));
    memset(&dialogue->scratch, 0, sizeof(dialogue->scratch));
}

static int lifecycle_to_composed(cdh_t *dialogue)
{
    switch (dialogue->lifecycle.state) {
    case GDL_CONFIRMATION_PENDING:
        dialogue->state = CDH_CONFIRMATION_PENDING;
        break;
    case GDL_PRESENTED:
        dialogue->state = dialogue->lifecycle.signal.abstained != 0u ?
                          CDH_ABSTAINED : CDH_PRESENTED;
        break;
    case GDL_CONFIRMED:
        dialogue->state = CDH_CONFIRMED;
        break;
    case GDL_ABORTED:
        dialogue->state = CDH_ABORTED;
        break;
    case GDL_TIMED_OUT:
        dialogue->state = CDH_TIMED_OUT;
        break;
    case GDL_CLEARED:
        dialogue->state = CDH_CLEARED;
        break;
    default:
        break;
    }
    return CDH_OK;
}

static void abstention_for_route(cdh_t *dialogue)
{
    memset(&dialogue->generated, 0, sizeof(dialogue->generated));
    dialogue->generated.status = GC_STATUS_ABSTAIN;
    dialogue->generated.authority = GC_AUTH_PRESENTATION_ONLY;
    if (dialogue->route.intent == INTENT_ROUTER_CONFLICT_QUERY) {
        dialogue->generated.kind = GC_KIND_CONTRADICTED;
        dialogue->generated.abstain_reason = GC_ABSTAIN_CONFLICT;
    } else {
        dialogue->generated.kind = GC_KIND_UNKNOWN;
        dialogue->generated.abstain_reason = GC_ABSTAIN_NO_EVIDENCE;
    }
}

void cdh_init(cdh_t *dialogue, const cdh_config_t *config)
{
    if (dialogue == NULL) return;
    memset(dialogue, 0, sizeof(*dialogue));
    if (config != NULL) dialogue->cfg = *config;
    gdl_init(&dialogue->lifecycle, config == NULL ? NULL :
             &dialogue->cfg.lifecycle_config);
    dialogue->state = CDH_IDLE;
}

int cdh_start(cdh_t *dialogue, const char *text, size_t length,
              uint32_t physical_session_id, uint32_t now_ms)
{
    intent_router_status_t route_status;
    int lifecycle_status;
    if (dialogue == NULL || text == NULL) return CDH_E_ARG;
    if (physical_session_id == 0u) return CDH_E_PHYSICAL;
    if (dialogue->memory_quarantined != 0u) return CDH_E_STATE;
    if (dialogue->state != CDH_IDLE && dialogue->state != CDH_CLEARED &&
        dialogue->state != CDH_ABORTED && dialogue->state != CDH_TIMED_OUT)
        return CDH_E_STATE;
    route_status = intent_router_route(
        text, length, dialogue->cfg.router_memories,
        dialogue->cfg.router_memory_count, &dialogue->route);
    if (route_status != INTENT_ROUTER_OK) return CDH_E_ROUTE;
    lifecycle_status = gdl_begin(&dialogue->lifecycle,
                                 physical_session_id, now_ms);
    if (lifecycle_status != GDL_OK) return lifecycle_status == GDL_E_PHYSICAL ?
                                      CDH_E_PHYSICAL : CDH_E_STATE;
    dialogue->state = CDH_ROUTED;
    return CDH_OK;
}

int cdh_generate_present(cdh_t *dialogue, uint32_t now_ms)
{
    gc_status_t generation_status;
    int lifecycle_status;
    if (dialogue == NULL) return CDH_E_ARG;
    if (dialogue->state != CDH_ROUTED) return CDH_E_STATE;
    dialogue->state = CDH_GENERATING;
    memset(&dialogue->request, 0, sizeof(dialogue->request));
    dialogue->request.mode = GC_MODE_ANSWER;
    dialogue->request.query = dialogue->cfg.default_query;
    dialogue->request.plan_problem = dialogue->cfg.plan_problem;
    dialogue->request.derivation_budget = 32u;
    dialogue->request.max_plan_nodes = 8u;
    dialogue->request.max_plan_depth = 4u;
    dialogue->request.memory = dialogue->cfg.memory;
    dialogue->request.current_generation = dialogue->cfg.current_generation;
    dialogue->request.personal_profile = dialogue->cfg.personal_profile;

    switch (dialogue->route.intent) {
    case INTENT_ROUTER_UNKNOWN:
    case INTENT_ROUTER_CONFLICT_QUERY:
        abstention_for_route(dialogue);
        break;
    case INTENT_ROUTER_ACTION_REQUEST:
    case INTENT_ROUTER_CAPTURE_MEMORY:
    case INTENT_ROUTER_FORGET_MEMORY:
    case INTENT_ROUTER_SHARE_MEMORY:
        if (dialogue->cfg.plan_problem == NULL) {
            abstention_for_route(dialogue);
            dialogue->generated.abstain_reason = GC_ABSTAIN_NO_PLAN;
        } else {
            dialogue->request.mode = GC_MODE_PLAN;
            generation_status = gc_generate(dialogue->cfg.reasoner,
                                            dialogue->cfg.lexicon,
                                            &dialogue->request,
                                            &dialogue->scratch,
                                            &dialogue->generated);
            if (generation_status != GC_STATUS_OK) goto generation_failed;
        }
        break;
    default:
        generation_status = gc_generate(dialogue->cfg.reasoner,
                                        dialogue->cfg.lexicon,
                                        &dialogue->request,
                                        &dialogue->scratch,
                                        &dialogue->generated);
        if (generation_status != GC_STATUS_OK &&
            generation_status != GC_STATUS_ABSTAIN &&
            generation_status != GC_STATUS_LIMIT) goto generation_failed;
        break;
    }
    lifecycle_status = gdl_present(&dialogue->lifecycle,
                                   &dialogue->generated, now_ms);
    if (lifecycle_status != GDL_OK) goto generation_failed;
    lifecycle_to_composed(dialogue);
    return CDH_OK;

generation_failed:
    dialogue->state = CDH_ABORTED;
    (void)gdl_abort(&dialogue->lifecycle);
    return CDH_E_GENERATION;
}

int cdh_confirm(cdh_t *dialogue, uint32_t physical_session_id, uint32_t now_ms)
{
    int result;
    if (dialogue == NULL) return CDH_E_ARG;
    result = gdl_confirm(&dialogue->lifecycle, physical_session_id, now_ms);
    if (result == GDL_E_PHYSICAL) return CDH_E_PHYSICAL;
    if (result != GDL_OK) return CDH_E_CONFIRMATION;
    lifecycle_to_composed(dialogue);
    return CDH_OK;
}

int cdh_deny(cdh_t *dialogue, uint32_t physical_session_id, uint32_t now_ms)
{
    int result;
    if (dialogue == NULL) return CDH_E_ARG;
    result = gdl_deny(&dialogue->lifecycle, physical_session_id, now_ms);
    if (result == GDL_E_PHYSICAL) return CDH_E_PHYSICAL;
    if (result != GDL_OK) return CDH_E_CONFIRMATION;
    lifecycle_to_composed(dialogue);
    return CDH_OK;
}

int cdh_tick(cdh_t *dialogue, uint32_t now_ms)
{
    int result;
    if (dialogue == NULL) return CDH_E_ARG;
    result = gdl_tick(&dialogue->lifecycle, now_ms);
    if (result == GDL_E_TIMEOUT) {
        lifecycle_to_composed(dialogue);
        return CDH_E_TIMEOUT;
    }
    if (result != GDL_OK) return CDH_E_STATE;
    lifecycle_to_composed(dialogue);
    return CDH_OK;
}

int cdh_abort(cdh_t *dialogue)
{
    int result;
    if (dialogue == NULL) return CDH_E_ARG;
    result = gdl_abort(&dialogue->lifecycle);
    if (result != GDL_OK) return CDH_E_STATE;
    clear_transient(dialogue);
    dialogue->state = CDH_ABORTED;
    return CDH_OK;
}

int cdh_forget(cdh_t *dialogue)
{
    int result;
    if (dialogue == NULL) return CDH_E_ARG;
    result = gdl_forget(&dialogue->lifecycle);
    if (result != GDL_OK) return CDH_E_STATE;
    clear_transient(dialogue);
    dialogue->state = CDH_CLEARED;
    return CDH_OK;
}

int cdh_reboot(cdh_t *dialogue, uint32_t recovered_generation)
{
    int result;
    if (dialogue == NULL || recovered_generation == 0u) return CDH_E_ARG;
    result = gdl_forget(&dialogue->lifecycle);
    if (result != GDL_OK) return CDH_E_STATE;
    clear_transient(dialogue);
    dialogue->memory_quarantined = 1u;
    dialogue->recovered_generation = recovered_generation;
    dialogue->state = CDH_CLEARED;
    return CDH_OK;
}

int cdh_rearm(cdh_t *dialogue,
              const intent_router_memory_t *router_memories,
              size_t router_memory_count,
              const mse_index_t *memory,
              uint32_t current_generation)
{
    if (dialogue == NULL || router_memories == NULL || memory == NULL ||
        router_memory_count == 0u || dialogue->memory_quarantined == 0u ||
        dialogue->state != CDH_CLEARED ||
        current_generation <= dialogue->recovered_generation ||
        memory->generation_floor != dialogue->recovered_generation ||
        mse_validate(memory) != MSE_OK)
        return CDH_E_STATE;
    dialogue->cfg.router_memories = router_memories;
    dialogue->cfg.router_memory_count = router_memory_count;
    dialogue->cfg.memory = memory;
    dialogue->cfg.current_generation = current_generation;
    dialogue->memory_quarantined = 0u;
    dialogue->state = CDH_IDLE;
    return CDH_OK;
}
