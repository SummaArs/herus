/*
 * HERUS aura — private presence without a server and without an identity.
 *
 * The social feature of every connected product is "who is around", and every
 * implementation of it works by telling a company where you are. Aura does the
 * same job with no account, no server, no location and no stable identifier.
 *
 * Each paired peer holds a ratcheting epoch key. Every epoch a device emits four
 * bytes: HMAC(key_n, "aura"). Someone you have paired with recognises it by
 * advancing their own copy of the ratchet up to a bounded window. Anybody else
 * sees four fresh pseudorandom bytes that do not repeat and do not link to the
 * previous epoch or the next one.
 *
 * The ratchet is what makes the past safe: key_{n+1} = HMAC(key_n, "step") is
 * one-way, so a key captured today does not recognise yesterday's beacons.
 * Recognition also consumes the epoch, so a captured beacon cannot be replayed.
 *
 * Honest limits. Four bytes give a 2^-32 false-accept probability per candidate
 * per attempt; the suite reports the measured count against a declared number of
 * attempts rather than claiming zero by construction. Unlinkability here is the
 * observable property that tokens are distinct and unpredictable without the
 * key; it is not a proof of indistinguishability, and it says nothing about
 * radio-layer fingerprinting, which is a hardware question.
 */
#ifndef HERUS_AURA_H
#define HERUS_AURA_H

#include <stdint.h>

#define AURA_KEY_BYTES    32u
#define AURA_TOKEN_BYTES   4u
#define AURA_MAX_PEERS     8u
#define AURA_WINDOW        8u   /* epochs of lookahead accepted */

typedef enum {
    AURA_OK          = 0,
    AURA_E_ARG       = 1,
    AURA_E_FULL      = 2,
    AURA_E_UNKNOWN   = 3,   /* nobody I know is emitting this */
    AURA_E_REVOKED   = 4,
    AURA_E_STALE     = 5    /* older than the window, or already consumed */
} aura_status_t;

typedef struct {
    uint8_t  key[AURA_KEY_BYTES];
    uint32_t epoch;
    uint8_t  active;
} aura_peer_t;

typedef struct {
    uint8_t     count;
    aura_peer_t peer[AURA_MAX_PEERS];
} aura_book_t;

void aura_book_init(aura_book_t *b);
aura_status_t aura_pair(aura_book_t *b, const uint8_t key[AURA_KEY_BYTES], uint8_t *index);
aura_status_t aura_revoke(aura_book_t *b, uint8_t index);

/* Emit: the token for the current epoch of `key`. Does not advance. */
void aura_token(const uint8_t key[AURA_KEY_BYTES], uint8_t out[AURA_TOKEN_BYTES]);
/* Advance one epoch, in place, irreversibly. */
void aura_step(uint8_t key[AURA_KEY_BYTES]);

/* Observe: does anybody I have paired with match this token within the window?
 * On success the peer's ratchet is advanced past the matched epoch, so the same
 * token can never be recognised twice. */
aura_status_t aura_recognize(aura_book_t *b, const uint8_t token[AURA_TOKEN_BYTES],
                             uint8_t *peer_index, uint8_t *epochs_ahead);

#endif /* HERUS_AURA_H */
