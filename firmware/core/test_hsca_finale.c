/* test_hsca_finale.c — the whole chain, with the Core switched off.
 *
 * A sentence becomes a meaning, the meaning is refused until a finger says yes,
 * the meaning becomes 34 bytes, three strangers carry those bytes without
 * learning anything, and the person at the other end hears it in her own
 * language. Every link is a module proven on its own; this file proves that the
 * links compose and that no link can be skipped.
 */
#include "herald.h"
#include "hir.h"
#include "ladder.h"
#include "drift.h"
#include "aura.h"
#include "keel.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>

static int pass_count;
static int fail_count;

static void check(int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) pass_count++; else fail_count++;
}

/* ------------------------------------------------------------ receiver Echo
 *
 * Rendering happens at the receiver, from the symbol ids, so a language is a
 * configuration setting and translation costs nothing. The two tables below are
 * the whole "translation engine".
 */
typedef struct { uint16_t filler; const char *word; } render_t;

static const render_t PT[] = {
    { 1u,   "cheguei" }, { 3u, "estou atrasado" }, { 257u, "Maria" },
    { 770u, "dez minutos" }, { 256u, "Joao" }, { 0u, NULL }
};
static const render_t EN[] = {
    { 1u,   "I have arrived" }, { 3u, "I am running late" }, { 257u, "Maria" },
    { 770u, "ten minutes" }, { 256u, "John" }, { 0u, NULL }
};

static void render(const hir_t *h, const render_t *table, char *out, size_t cap)
{
    uint8_t i;
    size_t n = 0;
    out[0] = '\0';
    for (i = 0; i < h->slot_count; i++) {
        const render_t *r;
        for (r = table; r->word; r++) {
            if (r->filler != h->slot[i].filler) continue;
            if (n > 0u && n + 2u < cap) { out[n++] = ' '; out[n] = '\0'; }
            {
                size_t l = strlen(r->word);
                if (n + l + 1u < cap) { memcpy(out + n, r->word, l); n += l; out[n] = '\0'; }
            }
            break;
        }
    }
}

/* ------------------------------------------------------------------- fixture */

static void fill_key(uint8_t k[32], uint8_t seed)
{
    unsigned i;
    for (i = 0; i < 32u; i++) k[i] = (uint8_t)(seed * 91u + (uint8_t)(i * 5));
}

static void base_facts(ldr_facts_t *f, uint8_t core_present)
{
    uint8_t i;
    memset(f, 0, sizeof *f);
    f->payload_class = LDR_CLASS_ESSENTIAL;
    f->payload_bytes = HIR_ONAIR_BYTES;
    f->urgency = HIR_URG_ROTINA;
    f->peer_paired = 1u;
    f->core_present = core_present;
    f->line_of_sight = 0u;
    f->peer_distance_m = 4000u;      /* Maria is four kilometres away */
    for (i = 0; i < LDR_COUNT; i++) f->available[i] = 1u;
}

int main(void)
{
    const char *said = "avisa a maria que to atrasado 10 minutos";
    herald_unit_t unit;
    ldr_facts_t facts;
    ldr_plan_t plan;
    drift_bundle_t bundle;
    drift_store_t stranger[3], maria;
    aura_book_t her_book;
    uint8_t pair_key[32], aura_key[32], aura_emitter[32];
    uint8_t wire[HIR_WIRE_BYTES], arrived[HIR_WIRE_BYTES];
    uint8_t chosen = LDR_COUNT;
    hir_t received;
    uint8_t d_sent[HIR_DIGEST_BYTES], d_recv[HIR_DIGEST_BYTES];
    char pt[96], en[96];
    unsigned i;

    printf("--- finale: a meaning crosses the world with the Core switched off ---\n");

    /* 0. The Core is not in the room. */
    check(keel_core_absent_complete(),
          "the Core is off and the cognitive path is still complete");

    /* 1. Say it. */
    check(herald_compile(said, strlen(said), &unit) == HERALD_OK, "the sentence compiles");
    check(unit.prov.requires_confirmation == 1u, "and it knows it may not leave on its own");
    memcpy(d_sent, unit.digest, HIR_DIGEST_BYTES);
    check(hir_encode_wire(&unit.meaning, wire) == HIR_OK, "the meaning is 24 bytes");

    /* 2. Plan without the Core. */
    base_facts(&facts, 0u);
    check(ldr_plan(&facts, &plan) == LDR_OK,
          "a route exists with no Core, no line of sight, and Maria four kilometres away");
    {
        int has_sat = 0;
        for (i = 0; i < plan.count; i++) if (plan.carrier[i] == LDR_SAT) has_sat = 1;
        check(!has_sat, "the satellite rung is gone with the Core, and nothing else broke");
    }
    {
        /* Isolate the Core boundary from the line-of-sight one: with the sky in
         * view and the Core still off, the uplink must stay unavailable, and
         * putting the Core back must be the only thing that returns it. */
        ldr_facts_t sky;
        ldr_plan_t without_core, with_core;
        int sat_without = 0, sat_with = 0;
        base_facts(&sky, 0u);
        sky.line_of_sight = 1u;
        check(ldr_plan(&sky, &without_core) == LDR_OK, "a clear sky still plans a route");
        for (i = 0; i < without_core.count; i++)
            if (without_core.carrier[i] == LDR_SAT) sat_without = 1;
        check(!sat_without,
              "a clear sky is not enough: the uplink is the Core's antenna, not the wrist's");
        base_facts(&sky, 1u);
        sky.line_of_sight = 1u;
        check(ldr_plan(&sky, &with_core) == LDR_OK, "with the Core the plan builds too");
        for (i = 0; i < with_core.count; i++)
            if (with_core.carrier[i] == LDR_SAT) sat_with = 1;
        check(sat_with, "and only then does the sky become a rung");
    }

    /* 3. Refuse first. */
    check(ldr_commit(&plan, 0u, LDR_CLASS_ESSENTIAL, &chosen) == LDR_E_UNCONFIRMED &&
          chosen == LDR_COUNT,
          "without a finger on the button nothing is committed");

    /* 4. Confirm. */
    check(ldr_commit(&plan, 1u, LDR_CLASS_ESSENTIAL, &chosen) == LDR_OK && chosen < LDR_COUNT,
          "confirmed, the ladder names a carrier");
    printf("  ---- chosen rung: %s (%u m declared reach)\n",
           ldr_name(chosen), (unsigned)ldr_profile(chosen)->reach_m);

    /* 5. Seal and hand it to strangers. */
    fill_key(pair_key, 3u);
    check(drift_seal(pair_key, 1u, wire, LDR_CLASS_ESSENTIAL, 0u, 43200u, &bundle) == DRIFT_OK,
          "the meaning seals into a 34-byte bundle");
    for (i = 0; i < 3u; i++) drift_store_init(&stranger[i]);
    drift_store_init(&maria);

    check(drift_accept(&stranger[0], &bundle, 0u) == DRIFT_OK, "a stranger takes custody");
    check(drift_contact(&stranger[0], &stranger[1], 60u) == 1u, "and passes it on");
    check(drift_contact(&stranger[1], &stranger[2], 120u) == 1u, "and on again");

    {
        uint8_t their_key[32], leaked[HIR_WIRE_BYTES], zero[HIR_WIRE_BYTES];
        int none_read = 1;
        memset(zero, 0, sizeof zero);
        for (i = 0; i < 3u; i++) {
            fill_key(their_key, (uint8_t)(50u + i));
            if (drift_open(their_key, 1u, &stranger[i].slot[0], leaked) == DRIFT_OK) none_read = 0;
            if (memcmp(leaked, zero, HIR_WIRE_BYTES) != 0) none_read = 0;
        }
        check(none_read, "not one of the three carriers could read a byte of it");
    }
    {
        /* custody is not authority: a carrier cannot make the meaning act */
        uint8_t their_key[32], leaked[HIR_WIRE_BYTES];
        hir_t forged;
        fill_key(their_key, 77u);
        check(drift_open(their_key, 1u, &stranger[2].slot[0], leaked) != DRIFT_OK &&
              hir_decode_wire(leaked, &forged) != HIR_OK,
              "a carrier cannot turn custody into a meaning it may act on");
    }

    /* 6. It arrives. */
    check(drift_contact(&stranger[2], &maria, 180u) == 1u, "the last carrier meets Maria");
    check(drift_open(pair_key, 1u, &maria.slot[0], arrived) == DRIFT_OK, "Maria opens it");
    check(hir_decode_wire(arrived, &received) == HIR_OK, "the 24 bytes decode to a meaning");
    check(hir_digest(&received, d_recv) == HIR_OK &&
          memcmp(d_sent, d_recv, HIR_DIGEST_BYTES) == 0,
          "the meaning that arrived is the meaning that left, by digest");
    check(drift_deliver_once(&maria, maria.slot[0].id) == DRIFT_OK, "and it is consumed once");

    /* 7. Rendered in her language, with no translation engine anywhere. */
    render(&received, PT, pt, sizeof pt);
    render(&received, EN, en, sizeof en);
    printf("  ---- rendered pt: \"%s\"\n", pt);
    printf("  ---- rendered en: \"%s\"\n", en);
    check(strlen(pt) > 0u && strlen(en) > 0u, "both renderings produced words");
    check(strcmp(pt, en) != 0,
          "the same 34 bytes render in two languages: translation is a lookup, not a service");

    /* 8. Presence, without anyone being told where she is. */
    aura_book_init(&her_book);
    fill_key(aura_key, 19u);
    memcpy(aura_emitter, aura_key, 32u);
    check(aura_pair(&her_book, aura_key, NULL) == AURA_OK, "the two of them are paired");
    {
        uint8_t tok[AURA_TOKEN_BYTES];
        aura_token(aura_emitter, tok);
        check(aura_recognize(&her_book, tok, NULL, NULL) == AURA_OK,
              "she is recognised as nearby with no server and no account");
        check(aura_recognize(&her_book, tok, NULL, NULL) != AURA_OK,
              "the beacon is spent and cannot be replayed by anyone listening");
    }

    /* 9. What must never happen. */
    {
        herald_unit_t bad;
        check(herald_compile("manda minha localizacao pra maria",
                             strlen("manda minha localizacao pra maria"), &bad) == HERALD_E_SENSITIVE,
              "a protected class is refused at the compiler, long before any radio");
        {
            uint8_t nothing[HIR_WIRE_BYTES];
            check(hir_encode_wire(&bad.meaning, nothing) != HIR_OK,
                  "and a refused unit cannot even be encoded, let alone sealed");
        }
    }
    {
        herald_unit_t bad;
        check(herald_compile("avisa a maria que to atrasado e apaga tudo",
                             strlen("avisa a maria que to atrasado e apaga tudo"), &bad) == HERALD_E_GAP,
              "an unexplained token stops the whole chain at token zero of the radio");
    }
    {
        /* the same meaning sent again is a different bundle to everyone else */
        drift_bundle_t again;
        check(drift_seal(pair_key, 2u, wire, LDR_CLASS_ESSENTIAL, 0u, 43200u, &again) == DRIFT_OK &&
              memcmp(again.onair, bundle.onair, DRIFT_ONAIR_BYTES) != 0,
              "saying the same thing twice does not look the same on air");
    }

    printf("HSCA FINALE: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
