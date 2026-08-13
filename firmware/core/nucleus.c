/* nucleus.c — bounded, opt-in semantic transition memory for the Herus Nucleus. */
#include "nucleus.h"
#include <string.h>
#include <limits.h>

/* This fingerprint is an in-memory index, not a security primitive and never
 * crosses a trust boundary. The complete template comparison below remains the
 * authority for rule identity. */
static uint64_t fnv_step(uint64_t h, uint8_t b)
{
    return (h ^ b) * 1099511628211ull;
}

static int semantic_valid(const hcp_msg_t *m)
{
    if (!m) return 0;
    if (m->tier != HCP_TIER_GLYPH && m->tier != HCP_TIER_COMPOSED) return 0;
    if (m->intent >= HCP_INTENT_N || m->nslot > HCP_MAX_SLOT) return 0;
    for (unsigned i = 0; i < m->nslot; i++) {
        if (m->slot[i].role >= HCP_ROLE_N || m->slot[i].filler >= HCP_FILLER_N)
            return 0;
        if (m->slot[i].role == 0 && m->slot[i].filler == 0) return 0;
    }
    return 1;
}

/* Strip every transport-mutated field. A learned template means only intent,
 * tier, flags and ordered role/filler bindings. `pos` is reconstructed by the
 * canonical HCP encoder, so retaining it would preserve stale wire state. */
static void semantic_template(hcp_msg_t *out, const hcp_msg_t *in)
{
    memset(out, 0, sizeof(*out));
    out->ver = HCP_VERSION;
    out->tier = in->tier;
    out->flags = in->flags;
    out->intent = in->intent;
    out->nslot = in->nslot;
    for (unsigned i = 0; i < in->nslot; i++) out->slot[i] = in->slot[i];
}

static uint64_t semantic_fingerprint(const hcp_msg_t *m)
{
    uint64_t h = 1469598103934665603ull;
    h = fnv_step(h, m->tier);
    h = fnv_step(h, m->flags);
    h = fnv_step(h, (uint8_t)m->intent);
    h = fnv_step(h, (uint8_t)(m->intent >> 8));
    h = fnv_step(h, m->nslot);
    for (unsigned i = 0; i < m->nslot; i++) {
        h = fnv_step(h, m->slot[i].role);
        h = fnv_step(h, (uint8_t)m->slot[i].filler);
        h = fnv_step(h, (uint8_t)(m->slot[i].filler >> 8));
    }
    return h ? h : 1ull; /* zero is reserved for "no pending context". */
}

static int semantic_equal(const hcp_msg_t *a, const hcp_msg_t *b)
{
    if (a->tier != b->tier || a->flags != b->flags ||
        a->intent != b->intent || a->nslot != b->nslot) return 0;
    for (unsigned i = 0; i < a->nslot; i++)
        if (a->slot[i].role != b->slot[i].role ||
            a->slot[i].filler != b->slot[i].filler) return 0;
    return 1;
}

static int older_than(uint32_t now, uint32_t then, uint32_t max_age)
{
    return (uint32_t)(now - then) > max_age;
}

static nucleus_rule_t *find_rule(nucleus_t *n, uint64_t context, const hcp_msg_t *next)
{
    for (unsigned i = 0; i < NUC_RULE_CAP; i++)
        if (n->rule[i].used && n->rule[i].context == context &&
            semantic_equal(&n->rule[i].next, next)) return &n->rule[i];
    return NULL;
}

static nucleus_rule_t *free_or_oldest(nucleus_t *n)
{
    nucleus_rule_t *victim = &n->rule[0];
    for (unsigned i = 0; i < NUC_RULE_CAP; i++) {
        if (!n->rule[i].used) return &n->rule[i];
        if (n->rule[i].last_seen < victim->last_seen) victim = &n->rule[i];
    }
    return victim;
}

void nucleus_init(nucleus_t *n)
{
    if (n) memset(n, 0, sizeof(*n));
}

void nucleus_set_learning(nucleus_t *n, int enabled)
{
    if (!n) return;
    n->learning_enabled = enabled ? 1u : 0u;
    if (!enabled) {
        n->context = 0;
        n->context_seen = 0;
    }
}

void nucleus_clear_context(nucleus_t *n)
{
    if (n) {
        n->context = 0;
        n->context_seen = 0;
    }
}

int nucleus_observe(nucleus_t *n, const hcp_msg_t *m, uint32_t now)
{
    hcp_msg_t next;
    uint64_t fp;
    nucleus_rule_t *r;

    if (!n || !semantic_valid(m)) return NUC_REJECTED;
    if (!n->learning_enabled) return NUC_DISABLED;

    semantic_template(&next, m);
    fp = semantic_fingerprint(&next);

    /* Repeating a meaning refreshes the context but never teaches the predictor
     * to suggest the same action merely because a peer retried it. */
    if (n->context && n->context != fp) {
        r = find_rule(n, n->context, &next);
        if (!r) {
            r = free_or_oldest(n);
            memset(r, 0, sizeof(*r));
            r->used = 1;
            r->context = n->context;
            r->next = next;
            r->support = 1;
        } else if (r->support < UINT16_MAX) {
            r->support++;
        }
        r->last_seen = now;
    }

    n->context = fp;
    n->context_seen = now;
    return NUC_OK;
}

unsigned nucleus_suggest(const nucleus_t *n, const hcp_msg_t *context,
                         uint32_t now, uint32_t max_age,
                         nucleus_suggestion_t *out, unsigned cap)
{
    hcp_msg_t key;
    uint64_t fp;
    uint32_t total = 0;
    unsigned used = 0;

    if (!n || !out || !cap || !max_age || !semantic_valid(context)) return 0;
    if (cap > NUC_SUGGESTION_CAP) cap = NUC_SUGGESTION_CAP;

    semantic_template(&key, context);
    fp = semantic_fingerprint(&key);

    for (unsigned i = 0; i < NUC_RULE_CAP; i++)
        if (n->rule[i].used && n->rule[i].context == fp &&
            !older_than(now, n->rule[i].last_seen, max_age))
            total += n->rule[i].support;
    if (!total) return 0;

    for (unsigned i = 0; i < NUC_RULE_CAP; i++) {
        const nucleus_rule_t *r = &n->rule[i];
        nucleus_suggestion_t s;
        unsigned pos;

        if (!r->used || r->context != fp || r->support < NUC_MIN_SUPPORT ||
            older_than(now, r->last_seen, max_age)) continue;

        memset(&s, 0, sizeof(s));
        s.template = r->next;
        s.support = r->support;
        s.observations = (uint16_t)(total > UINT16_MAX ? UINT16_MAX : total);
        s.confidence_pct = (uint8_t)((100u * r->support) / total);

        if (used == cap && s.support <= out[used - 1].support) continue;
        pos = used < cap ? used++ : used - 1;
        while (pos && s.support > out[pos - 1].support) {
            out[pos] = out[pos - 1];
            pos--;
        }
        out[pos] = s;
    }
    return used;
}

void nucleus_forget(nucleus_t *n)
{
    if (n) memset(n, 0, sizeof(*n));
}

unsigned nucleus_expire(nucleus_t *n, uint32_t now, uint32_t max_age)
{
    unsigned expired = 0;
    if (!n) return 0;
    for (unsigned i = 0; i < NUC_RULE_CAP; i++)
        if (n->rule[i].used && (!max_age || older_than(now, n->rule[i].last_seen, max_age))) {
            memset(&n->rule[i], 0, sizeof(n->rule[i]));
            expired++;
        }
    if (n->context && (!max_age || older_than(now, n->context_seen, max_age))) {
        n->context = 0;
        n->context_seen = 0;
    }
    return expired;
}

unsigned nucleus_rule_count(const nucleus_t *n)
{
    unsigned count = 0;
    if (!n) return 0;
    for (unsigned i = 0; i < NUC_RULE_CAP; i++) if (n->rule[i].used) count++;
    return count;
}

void nucleus_govern(const nucleus_telemetry_t *in, nucleus_governance_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    if (!in) {
        out->state = NUC_BASE_CORE_UNREACHABLE;
        out->owner_alert = 1;
        return;
    }

    /* Charge is only a suggestion. The physical charger must still require an
     * identified cradle, battery authentication, temperature and current checks. */
    out->charge_recommended = (in->cradle_present && in->battery_permille < 950u) ? 1u : 0u;

    if (!in->core_link_ok) {
        out->state = NUC_BASE_CORE_UNREACHABLE;
        out->owner_alert = 1;
        return;
    }
    if (in->battery_permille < 200u) {
        out->state = NUC_BASE_SAVE_POWER;
        out->owner_alert = 1;
        return;
    }

    /* PDR is the primary truth. RSSI/SNR only make the recommendation more
     * explainable: move the puck upward, away from the body, or toward a window. */
    if (in->pdr_pct < 80u || in->rssi_dbm_x2 < -240 || in->snr_db_x4 < -40) {
        out->state = NUC_BASE_REPOSITION;
        out->relay_recommended = in->battery_permille >= 500u ? 1u : 0u;
        out->owner_alert = 1;
        return;
    }

    out->state = NUC_BASE_HEALTHY;
    out->relay_recommended = (in->battery_permille >= 350u && in->pdr_pct >= 90u) ? 1u : 0u;
}
