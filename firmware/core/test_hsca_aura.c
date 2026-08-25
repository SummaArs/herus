/* test_hsca_aura.c — presence without a server, an account or a location. */
#include "aura.h"
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

static void fill_key(uint8_t k[AURA_KEY_BYTES], uint8_t seed)
{
    unsigned i;
    for (i = 0; i < AURA_KEY_BYTES; i++) k[i] = (uint8_t)(seed * 137u + (uint8_t)(i * 7));
}

static void run_recognition(void)
{
    aura_book_t book;
    uint8_t shared[AURA_KEY_BYTES], emitter[AURA_KEY_BYTES];
    uint8_t tok[AURA_TOKEN_BYTES];
    uint8_t idx = 0xffu, ahead = 0xffu;

    aura_book_init(&book);
    fill_key(shared, 5u);
    memcpy(emitter, shared, AURA_KEY_BYTES);
    check(aura_pair(&book, shared, &idx) == AURA_OK && idx == 0u, "a peer is paired");

    aura_token(emitter, tok);
    check(aura_recognize(&book, tok, &idx, &ahead) == AURA_OK && idx == 0u && ahead == 0u,
          "the current epoch of a paired peer is recognised");
    check(aura_recognize(&book, tok, &idx, &ahead) == AURA_E_UNKNOWN,
          "the same beacon replayed is no longer recognised: the epoch was consumed");

    {
        unsigned w, ok = 0;
        for (w = 0; w < 5u; w++) {
            aura_step(emitter);
            aura_token(emitter, tok);
            if (aura_recognize(&book, tok, &idx, &ahead) == AURA_OK) ok++;
        }
        check(ok == 5u, "an emitter that ran ahead is still recognised inside the window");
    }
    {
        unsigned w;
        for (w = 0; w < AURA_WINDOW + 4u; w++) aura_step(emitter);
        aura_token(emitter, tok);
        check(aura_recognize(&book, tok, &idx, &ahead) == AURA_E_UNKNOWN,
              "an emitter far past the window is not recognised: the window is bounded");
    }
}

static void run_forward_secrecy(void)
{
    uint8_t k[AURA_KEY_BYTES], past_token[AURA_TOKEN_BYTES];
    aura_book_t stolen;
    uint8_t idx, ahead;

    fill_key(k, 9u);
    aura_token(k, past_token);
    aura_step(k);
    aura_step(k);
    aura_step(k);

    aura_book_init(&stolen);
    (void)aura_pair(&stolen, k, &idx);   /* an attacker captures today's key */
    check(aura_recognize(&stolen, past_token, &idx, &ahead) == AURA_E_UNKNOWN,
          "a key captured today does not recognise yesterday's beacons");
}

static void run_revocation(void)
{
    aura_book_t book;
    uint8_t key[AURA_KEY_BYTES], emitter[AURA_KEY_BYTES], tok[AURA_TOKEN_BYTES];
    uint8_t idx = 0xffu, ahead = 0xffu;

    aura_book_init(&book);
    fill_key(key, 13u);
    memcpy(emitter, key, AURA_KEY_BYTES);
    (void)aura_pair(&book, key, &idx);

    aura_token(emitter, tok);
    check(aura_recognize(&book, tok, &idx, &ahead) == AURA_OK, "recognised before revocation");
    check(aura_revoke(&book, 0u) == AURA_OK, "the peer is revoked");

    aura_step(emitter);
    aura_token(emitter, tok);
    check(aura_recognize(&book, tok, &idx, &ahead) == AURA_E_REVOKED,
          "after revocation the same person is simply not there any more");
    {
        uint8_t zero[AURA_KEY_BYTES];
        memset(zero, 0, sizeof zero);
        check(memcmp(book.peer[0].key, zero, AURA_KEY_BYTES) == 0,
              "revocation scrubs the key rather than flagging it");
    }
    check(aura_revoke(&book, 7u) == AURA_E_ARG, "revoking a peer that does not exist is refused");
}

static unsigned forge_attempts = 100000u;

static void run_false_accept(void)
{
    aura_book_t book;
    uint8_t key[AURA_KEY_BYTES];
    uint32_t state = 20260824u;
    unsigned attempts = forge_attempts, accepted = 0, i, p;

    aura_book_init(&book);
    for (p = 0; p < AURA_MAX_PEERS; p++) {
        fill_key(key, (uint8_t)(40u + p));
        (void)aura_pair(&book, key, NULL);
    }
    for (i = 0; i < attempts; i++) {
        uint8_t tok[AURA_TOKEN_BYTES];
        unsigned b;
        for (b = 0; b < AURA_TOKEN_BYTES; b++) {
            state = state * 1664525u + 1013904223u;
            tok[b] = (uint8_t)(state >> 17);
        }
        if (aura_recognize(&book, tok, NULL, NULL) == AURA_OK) accepted++;
    }
    printf("  ---- %u forged beacons against %u peers x %u epochs: %u accepted\n",
           attempts, (unsigned)AURA_MAX_PEERS, (unsigned)AURA_WINDOW, accepted);
    printf("       expected by chance: %u x %u x %u / 2^32 = %.4f\n",
           attempts, (unsigned)AURA_MAX_PEERS, (unsigned)AURA_WINDOW,
           (double)attempts * AURA_MAX_PEERS * AURA_WINDOW / 4294967296.0);
    check(accepted == 0u, "no forged beacon was accepted in the measured campaign");
}

static void run_unlinkability(void)
{
    uint8_t k[AURA_KEY_BYTES];
    static uint8_t tok[1024][AURA_TOKEN_BYTES];
    unsigned i, j, repeats = 0;
    unsigned ones = 0;

    fill_key(k, 17u);
    for (i = 0; i < 1024u; i++) {
        aura_token(k, tok[i]);
        aura_step(k);
    }
    for (i = 0; i < 1024u; i++)
        for (j = i + 1u; j < 1024u; j++)
            if (memcmp(tok[i], tok[j], AURA_TOKEN_BYTES) == 0) repeats++;
    for (i = 0; i < 1024u; i++) {
        unsigned b;
        for (b = 0; b < AURA_TOKEN_BYTES; b++) {
            uint8_t v = tok[i][b];
            while (v) { ones += (unsigned)(v & 1u); v = (uint8_t)(v >> 1); }
        }
    }
    printf("  ---- 1024 consecutive epochs: %u repeated beacons, bit density %.4f\n",
           repeats, (double)ones / (1024.0 * AURA_TOKEN_BYTES * 8.0));
    check(repeats == 0u, "an observer without the key never sees the same beacon twice");
    check(ones > 1024u * 16u - 400u && ones < 1024u * 16u + 400u,
          "beacons carry no visible structure: bit density sits at one half");
    {
        uint8_t a[AURA_KEY_BYTES], b[AURA_KEY_BYTES], ta[AURA_TOKEN_BYTES], tb[AURA_TOKEN_BYTES];
        fill_key(a, 21u);
        fill_key(b, 22u);
        aura_token(a, ta);
        aura_token(b, tb);
        check(memcmp(ta, tb, AURA_TOKEN_BYTES) != 0,
              "two different pairings do not emit the same beacon");
    }
}

static void run_boundaries(void)
{
    aura_book_t book;
    uint8_t key[AURA_KEY_BYTES], tok[AURA_TOKEN_BYTES];
    unsigned i;
    int filled = 1;

    aura_book_init(&book);
    for (i = 0; i < AURA_MAX_PEERS; i++) {
        fill_key(key, (uint8_t)i);
        if (aura_pair(&book, key, NULL) != AURA_OK) filled = 0;
    }
    check(filled, "the book fills to its declared capacity");
    fill_key(key, 99u);
    check(aura_pair(&book, key, NULL) == AURA_E_FULL, "one more pairing is refused, not squeezed in");
    check(aura_pair(NULL, key, NULL) == AURA_E_ARG, "a null book is refused");
    check(aura_recognize(&book, NULL, NULL, NULL) == AURA_E_ARG, "a null beacon is refused");
    {
        aura_book_t empty;
        aura_book_init(&empty);
        memset(tok, 0, sizeof tok);
        check(aura_recognize(&empty, tok, NULL, NULL) == AURA_E_UNKNOWN,
              "with nobody paired, nobody is recognised");
    }
    check(sizeof(aura_peer_t) == AURA_KEY_BYTES + 8u,
          "a peer record is a key, an epoch counter and a flag: no name, no place");
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--quick") == 0) forge_attempts = 4000u;
    printf("--- aura: private presence, no server and no identity ---\n");
    run_recognition();
    run_forward_secrecy();
    run_revocation();
    run_false_accept();
    run_unlinkability();
    run_boundaries();
    printf("HSCA AURA: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
