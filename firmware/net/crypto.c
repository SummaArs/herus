/* crypto.c — see crypto.h. Differentially tested against Python `cryptography`
 * on 400 random cases per primitive (tools/gen_vectors.py -> test_net.c V*). */
#include "crypto.h"
#include <string.h>

/* ============================== helpers ================================== */

static uint32_t ld32le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void st32le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}
static void st64le(uint8_t *p, uint64_t v)
{
    st32le(p, (uint32_t)v); st32le(p + 4, (uint32_t)(v >> 32));
}

int ct_eq(const void *a, const void *b, size_t len)
{
    const uint8_t *x = (const uint8_t *)a, *y = (const uint8_t *)b;
    uint8_t d = 0;
    for (size_t i = 0; i < len; i++) d |= (uint8_t)(x[i] ^ y[i]);
    return d == 0;
}

void secure_zero(void *p, size_t len)
{
    volatile uint8_t *v = (volatile uint8_t *)p;
    while (len--) *v++ = 0;
}

/* ============================== SHA-256 ================================== */

static const uint32_t K256[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,
    0x923f82a4u,0xab1c5ed5u,0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,
    0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,0xe49b69c1u,0xefbe4786u,
    0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,
    0x06ca6351u,0x14292967u,0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,
    0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,0xa2bfe8a1u,0xa81a664bu,
    0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,
    0x5b9cca4fu,0x682e6ff3u,0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,
    0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

#define ROR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void sha256_block(uint32_t h[8], const uint8_t *p)
{
    uint32_t w[64], a, b, c, d, e, f, g, hh, t1, t2;
    for (int i = 0; i < 16; i++)
        w[i] = ((uint32_t)p[i * 4] << 24) | ((uint32_t)p[i * 4 + 1] << 16) |
               ((uint32_t)p[i * 4 + 2] << 8) | (uint32_t)p[i * 4 + 3];
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = ROR32(w[i - 15], 7) ^ ROR32(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = ROR32(w[i - 2], 17) ^ ROR32(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    a = h[0]; b = h[1]; c = h[2]; d = h[3];
    e = h[4]; f = h[5]; g = h[6]; hh = h[7];
    for (int i = 0; i < 64; i++) {
        uint32_t S1 = ROR32(e, 6) ^ ROR32(e, 11) ^ ROR32(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        t1 = hh + S1 + ch + K256[i] + w[i];
        uint32_t S0 = ROR32(a, 2) ^ ROR32(a, 13) ^ ROR32(a, 22);
        uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
        t2 = S0 + mj;
        hh = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

void sha256_init(sha256_ctx *c)
{
    c->h[0] = 0x6a09e667u; c->h[1] = 0xbb67ae85u;
    c->h[2] = 0x3c6ef372u; c->h[3] = 0xa54ff53au;
    c->h[4] = 0x510e527fu; c->h[5] = 0x9b05688cu;
    c->h[6] = 0x1f83d9abu; c->h[7] = 0x5be0cd19u;
    c->len = 0; c->n = 0;
}

void sha256_update(sha256_ctx *c, const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    c->len += len;
    if (c->n) {
        size_t take = 64 - c->n;
        if (take > len) take = len;
        memcpy(c->buf + c->n, p, take);
        c->n += take; p += take; len -= take;
        if (c->n == 64) { sha256_block(c->h, c->buf); c->n = 0; }
    }
    while (len >= 64) { sha256_block(c->h, p); p += 64; len -= 64; }
    if (len) { memcpy(c->buf, p, len); c->n = len; }
}

void sha256_final(sha256_ctx *c, uint8_t out[SHA256_LEN])
{
    uint64_t bits = c->len * 8;
    uint8_t pad = 0x80;
    sha256_update(c, &pad, 1);
    pad = 0;
    while (c->n != 56) sha256_update(c, &pad, 1);
    uint8_t lb[8];
    for (int i = 0; i < 8; i++) lb[i] = (uint8_t)(bits >> (56 - 8 * i));
    sha256_update(c, lb, 8);
    for (int i = 0; i < 8; i++) {
        out[i * 4]     = (uint8_t)(c->h[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(c->h[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(c->h[i] >> 8);
        out[i * 4 + 3] = (uint8_t)(c->h[i]);
    }
}

void sha256(const void *data, size_t len, uint8_t out[SHA256_LEN])
{
    sha256_ctx c; sha256_init(&c); sha256_update(&c, data, len); sha256_final(&c, out);
}

/* ============================== HMAC ===================================== */

void hmac_sha256(const uint8_t *key, size_t keylen,
                 const void *msg, size_t msglen, uint8_t out[SHA256_LEN])
{
    uint8_t k[64], ipad[64], opad[64], inner[SHA256_LEN];
    sha256_ctx c;

    memset(k, 0, sizeof k);
    if (keylen > 64) sha256(key, keylen, k);
    else if (keylen)  memcpy(k, key, keylen);

    for (int i = 0; i < 64; i++) { ipad[i] = (uint8_t)(k[i] ^ 0x36); opad[i] = (uint8_t)(k[i] ^ 0x5c); }

    sha256_init(&c); sha256_update(&c, ipad, 64);
    sha256_update(&c, msg, msglen); sha256_final(&c, inner);

    sha256_init(&c); sha256_update(&c, opad, 64);
    sha256_update(&c, inner, SHA256_LEN); sha256_final(&c, out);

    secure_zero(k, sizeof k); secure_zero(ipad, sizeof ipad);
    secure_zero(opad, sizeof opad); secure_zero(inner, sizeof inner);
}

/* ============================== HKDF ===================================== */

void hkdf_extract(const uint8_t *salt, size_t saltlen,
                  const uint8_t *ikm, size_t ikmlen, uint8_t prk[SHA256_LEN])
{
    uint8_t zeros[SHA256_LEN];
    if (!salt || !saltlen) { memset(zeros, 0, sizeof zeros); salt = zeros; saltlen = SHA256_LEN; }
    hmac_sha256(salt, saltlen, ikm, ikmlen, prk);
}

int hkdf_expand(const uint8_t prk[SHA256_LEN], const void *info, size_t infolen,
                uint8_t *okm, size_t okmlen)
{
    if (okmlen > 255u * SHA256_LEN) return -1;
    uint8_t t[SHA256_LEN], buf[SHA256_LEN + 256 + 1];
    size_t tlen = 0, done = 0;
    uint8_t ctr = 1;

    if (infolen > 256) return -1;   /* bounded so the scratch stays on stack */

    while (done < okmlen) {
        size_t n = 0;
        if (tlen) { memcpy(buf, t, tlen); n = tlen; }
        if (infolen) { memcpy(buf + n, info, infolen); n += infolen; }
        buf[n++] = ctr++;
        hmac_sha256(prk, SHA256_LEN, buf, n, t);
        tlen = SHA256_LEN;
        size_t take = okmlen - done < SHA256_LEN ? okmlen - done : SHA256_LEN;
        memcpy(okm + done, t, take);
        done += take;
    }
    secure_zero(t, sizeof t); secure_zero(buf, sizeof buf);
    return 0;
}

int hkdf(const uint8_t *salt, size_t saltlen, const uint8_t *ikm, size_t ikmlen,
         const void *info, size_t infolen, uint8_t *okm, size_t okmlen)
{
    uint8_t prk[SHA256_LEN];
    hkdf_extract(salt, saltlen, ikm, ikmlen, prk);
    int r = hkdf_expand(prk, info, infolen, okm, okmlen);
    secure_zero(prk, sizeof prk);
    return r;
}

/* ============================== ChaCha20 ================================= */

#define ROL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROL32(d, 16), \
    c += d, b ^= c, b = ROL32(b, 12), \
    a += b, d ^= a, d = ROL32(d, 8),  \
    c += d, b ^= c, b = ROL32(b, 7))

static void chacha20_block(const uint32_t in[16], uint8_t out[64])
{
    uint32_t x[16];
    memcpy(x, in, sizeof x);
    for (int i = 0; i < 10; i++) {
        QR(x[0], x[4], x[8],  x[12]);
        QR(x[1], x[5], x[9],  x[13]);
        QR(x[2], x[6], x[10], x[14]);
        QR(x[3], x[7], x[11], x[15]);
        QR(x[0], x[5], x[10], x[15]);
        QR(x[1], x[6], x[11], x[12]);
        QR(x[2], x[7], x[8],  x[13]);
        QR(x[3], x[4], x[9],  x[14]);
    }
    for (int i = 0; i < 16; i++) st32le(out + i * 4, x[i] + in[i]);
}

int chacha20_xor(const uint8_t key[32], const uint8_t nonce[12],
                 uint32_t counter, const uint8_t *in, uint8_t *out, size_t len)
{
    uint32_t st[16];
    uint8_t  ks[64];

    /* Blocks available before the 32-bit counter would wrap. See the header for
     * why crossing it is refused rather than defined. */
    uint64_t need = ((uint64_t)len + 63u) / 64u;
    uint64_t avail = 0x100000000ull - (uint64_t)counter;
    if (need > avail) return -1;

    st[0] = 0x61707865u; st[1] = 0x3320646eu;
    st[2] = 0x79622d32u; st[3] = 0x6b206574u;
    for (int i = 0; i < 8; i++) st[4 + i] = ld32le(key + i * 4);
    st[12] = counter;
    for (int i = 0; i < 3; i++) st[13 + i] = ld32le(nonce + i * 4);

    while (len) {
        chacha20_block(st, ks);
        size_t n = len < 64 ? len : 64;
        for (size_t i = 0; i < n; i++) out[i] = (uint8_t)(in[i] ^ ks[i]);
        in += n; out += n; len -= n;
        st[12]++;                      /* cannot wrap: bounded above */
    }
    secure_zero(ks, sizeof ks);
    secure_zero(st, sizeof st);
    return 0;
}

/* ============================== Poly1305 ================================= */
/* 5 x 26-bit limbs, the standard portable ("donna-32") decomposition: it keeps
 * every partial product inside a uint64 with no 128-bit type, which the Xtensa
 * and Cortex-M33 both lack. */

typedef struct {
    uint32_t r[5], h[5], pad[4];
    size_t   leftover;
    uint8_t  buffer[16];
    uint8_t  final;
} poly_ctx;

static void poly_init(poly_ctx *st, const uint8_t key[32])
{
    /* r &= 0xffffffc0ffffffc0ffffffc0fffffff — the clamp is part of the design,
     * it bounds the carries so the 26-bit limb arithmetic cannot overflow. */
    st->r[0] = (ld32le(&key[0])      ) & 0x3ffffffu;
    st->r[1] = (ld32le(&key[3]) >> 2 ) & 0x3ffff03u;
    st->r[2] = (ld32le(&key[6]) >> 4 ) & 0x3ffc0ffu;
    st->r[3] = (ld32le(&key[9]) >> 6 ) & 0x3f03fffu;
    st->r[4] = (ld32le(&key[12]) >> 8) & 0x00fffffu;
    for (int i = 0; i < 5; i++) st->h[i] = 0;
    for (int i = 0; i < 4; i++) st->pad[i] = ld32le(&key[16 + i * 4]);
    st->leftover = 0;
    st->final = 0;
}

static void poly_blocks(poly_ctx *st, const uint8_t *m, size_t bytes)
{
    const uint32_t hibit = st->final ? 0 : (1u << 24);
    const uint32_t r0 = st->r[0], r1 = st->r[1], r2 = st->r[2],
                   r3 = st->r[3], r4 = st->r[4];
    const uint32_t s1 = r1 * 5, s2 = r2 * 5, s3 = r3 * 5, s4 = r4 * 5;
    uint32_t h0 = st->h[0], h1 = st->h[1], h2 = st->h[2], h3 = st->h[3], h4 = st->h[4];

    while (bytes >= 16) {
        uint64_t d0, d1, d2, d3, d4;
        uint32_t c;

        h0 += (ld32le(m + 0)      ) & 0x3ffffffu;
        h1 += (ld32le(m + 3) >> 2 ) & 0x3ffffffu;
        h2 += (ld32le(m + 6) >> 4 ) & 0x3ffffffu;
        h3 += (ld32le(m + 9) >> 6 ) & 0x3ffffffu;
        h4 += (ld32le(m + 12) >> 8) | hibit;

        d0 = (uint64_t)h0 * r0 + (uint64_t)h1 * s4 + (uint64_t)h2 * s3 + (uint64_t)h3 * s2 + (uint64_t)h4 * s1;
        d1 = (uint64_t)h0 * r1 + (uint64_t)h1 * r0 + (uint64_t)h2 * s4 + (uint64_t)h3 * s3 + (uint64_t)h4 * s2;
        d2 = (uint64_t)h0 * r2 + (uint64_t)h1 * r1 + (uint64_t)h2 * r0 + (uint64_t)h3 * s4 + (uint64_t)h4 * s3;
        d3 = (uint64_t)h0 * r3 + (uint64_t)h1 * r2 + (uint64_t)h2 * r1 + (uint64_t)h3 * r0 + (uint64_t)h4 * s4;
        d4 = (uint64_t)h0 * r4 + (uint64_t)h1 * r3 + (uint64_t)h2 * r2 + (uint64_t)h3 * r1 + (uint64_t)h4 * r0;

        c = (uint32_t)(d0 >> 26); h0 = (uint32_t)d0 & 0x3ffffffu;
        d1 += c; c = (uint32_t)(d1 >> 26); h1 = (uint32_t)d1 & 0x3ffffffu;
        d2 += c; c = (uint32_t)(d2 >> 26); h2 = (uint32_t)d2 & 0x3ffffffu;
        d3 += c; c = (uint32_t)(d3 >> 26); h3 = (uint32_t)d3 & 0x3ffffffu;
        d4 += c; c = (uint32_t)(d4 >> 26); h4 = (uint32_t)d4 & 0x3ffffffu;
        h0 += c * 5;      c = h0 >> 26; h0 &= 0x3ffffffu;
        h1 += c;

        m += 16; bytes -= 16;
    }
    st->h[0] = h0; st->h[1] = h1; st->h[2] = h2; st->h[3] = h3; st->h[4] = h4;
}

static void poly_update(poly_ctx *st, const uint8_t *m, size_t bytes)
{
    if (st->leftover) {
        size_t want = 16 - st->leftover;
        if (want > bytes) want = bytes;
        memcpy(st->buffer + st->leftover, m, want);
        bytes -= want; m += want; st->leftover += want;
        if (st->leftover < 16) return;
        poly_blocks(st, st->buffer, 16);
        st->leftover = 0;
    }
    if (bytes >= 16) {
        size_t want = bytes & ~((size_t)15);
        poly_blocks(st, m, want);
        m += want; bytes -= want;
    }
    if (bytes) { memcpy(st->buffer + st->leftover, m, bytes); st->leftover += bytes; }
}

static void poly_finish(poly_ctx *st, uint8_t mac[16])
{
    uint32_t h0, h1, h2, h3, h4, c, g0, g1, g2, g3, g4, mask;
    uint64_t f;

    if (st->leftover) {
        size_t i = st->leftover;
        st->buffer[i++] = 1;
        for (; i < 16; i++) st->buffer[i] = 0;
        st->final = 1;
        poly_blocks(st, st->buffer, 16);
    }

    h0 = st->h[0]; h1 = st->h[1]; h2 = st->h[2]; h3 = st->h[3]; h4 = st->h[4];

    c = h1 >> 26; h1 &= 0x3ffffffu;
    h2 += c; c = h2 >> 26; h2 &= 0x3ffffffu;
    h3 += c; c = h3 >> 26; h3 &= 0x3ffffffu;
    h4 += c; c = h4 >> 26; h4 &= 0x3ffffffu;
    h0 += c * 5; c = h0 >> 26; h0 &= 0x3ffffffu;
    h1 += c;

    /* h + -p, then a constant-time select of whichever is the reduced value. */
    g0 = h0 + 5; c = g0 >> 26; g0 &= 0x3ffffffu;
    g1 = h1 + c; c = g1 >> 26; g1 &= 0x3ffffffu;
    g2 = h2 + c; c = g2 >> 26; g2 &= 0x3ffffffu;
    g3 = h3 + c; c = g3 >> 26; g3 &= 0x3ffffffu;
    g4 = h4 + c - (1u << 26);

    mask = (g4 >> 31) - 1;            /* 0 if h < p, all-ones if h >= p */
    g0 &= mask; g1 &= mask; g2 &= mask; g3 &= mask; g4 &= mask;
    mask = ~mask;
    h0 = (h0 & mask) | g0; h1 = (h1 & mask) | g1; h2 = (h2 & mask) | g2;
    h3 = (h3 & mask) | g3; h4 = (h4 & mask) | g4;

    /* pack the 130-bit value down to 128 bits */
    h0 = (h0 | (h1 << 26));
    h1 = ((h1 >> 6) | (h2 << 20));
    h2 = ((h2 >> 12) | (h3 << 14));
    h3 = ((h3 >> 18) | (h4 << 8));

    f = (uint64_t)h0 + st->pad[0]; h0 = (uint32_t)f;
    f = (uint64_t)h1 + st->pad[1] + (f >> 32); h1 = (uint32_t)f;
    f = (uint64_t)h2 + st->pad[2] + (f >> 32); h2 = (uint32_t)f;
    f = (uint64_t)h3 + st->pad[3] + (f >> 32); h3 = (uint32_t)f;

    st32le(mac + 0, h0); st32le(mac + 4, h1);
    st32le(mac + 8, h2); st32le(mac + 12, h3);
    secure_zero(st, sizeof *st);
}

void poly1305_mac(const uint8_t key[32], const void *msg, size_t len,
                  uint8_t tag[POLY1305_TAG_LEN])
{
    poly_ctx st;
    poly_init(&st, key);
    poly_update(&st, (const uint8_t *)msg, len);
    poly_finish(&st, tag);
}

/* ============================== AEAD ==================================== */

static void aead_tag(const uint8_t key[32], const uint8_t nonce[12],
                     const uint8_t *aad, size_t aadlen,
                     const uint8_t *ct, size_t ctlen,
                     uint8_t tag16[16])
{
    uint8_t otk[64] = {0}, zeros[16] = {0}, lens[16];
    poly_ctx st;

    /* One-time Poly1305 key = ChaCha20 block 0 under the same key and nonce. */
    chacha20_xor(key, nonce, 0, otk, otk, 64);
    poly_init(&st, otk);

    poly_update(&st, aad, aadlen);
    if (aadlen % 16) poly_update(&st, zeros, 16 - (aadlen % 16));
    poly_update(&st, ct, ctlen);
    if (ctlen % 16) poly_update(&st, zeros, 16 - (ctlen % 16));
    st64le(lens, (uint64_t)aadlen);
    st64le(lens + 8, (uint64_t)ctlen);
    poly_update(&st, lens, 16);

    poly_finish(&st, tag16);
    secure_zero(otk, sizeof otk);
}

void aead_encrypt(const uint8_t key[32], const uint8_t nonce[12],
                  const uint8_t *aad, size_t aadlen,
                  const uint8_t *pt, size_t ptlen,
                  uint8_t *ct, uint8_t *tag, size_t tag_len)
{
    uint8_t full[16];
    chacha20_xor(key, nonce, 1, pt, ct, ptlen);
    aead_tag(key, nonce, aad, aadlen, ct, ptlen, full);
    memcpy(tag, full, tag_len);          /* truncation is a caller decision */
}

int aead_decrypt(const uint8_t key[32], const uint8_t nonce[12],
                 const uint8_t *aad, size_t aadlen,
                 const uint8_t *ct, size_t ctlen,
                 const uint8_t *tag, size_t tag_len,
                 uint8_t *pt)
{
    uint8_t full[16];
    aead_tag(key, nonce, aad, aadlen, ct, ctlen, full);
    if (!ct_eq(full, tag, tag_len)) {
        memset(pt, 0, ctlen);            /* never hand back unverified bytes */
        return -1;
    }
    chacha20_xor(key, nonce, 1, ct, pt, ctlen);
    return 0;
}
