/* herus.js — Babel e Aether em JavaScript, porta fiel do C11 do firmware.
 *
 * Duas regras governaram esta porta e explicam as escolhas estranhas:
 *
 * 1. TUDO EM BYTES. O compilador do firmware trabalha sobre bytes UTF-8, e as
 *    regras de fronteira (palavra em idioma com espaco, transicao de escrita em
 *    japones e chines) sao definidas em bytes. Uma porta que usasse as strings
 *    UTF-16 do JavaScript concordaria com o C na maioria dos casos e divergiria
 *    em alguns — e "a maioria dos casos" nao e um contrato. Entao aqui tambem e
 *    Uint8Array de ponta a ponta.
 *
 * 2. O NAVEGADOR PROVA. herusSelfTest() reproduz os vetores exportados pelas
 *    sondas C. A pagina roda isso na carga, na frente de quem estiver olhando.
 *    Se um vetor divergir, a pagina diz — em vez de funcionar quase certo.
 */
(function (global) {
  'use strict';

  // ------------------------------------------------------------- constantes
  var OK = 'OK', E_ARG = 'ARG', E_EMPTY = 'EMPTY', E_OVERFLOW = 'OVERFLOW',
      E_GAP = 'GAP', E_AMBIGUOUS = 'AMBIGUOUS', E_SENSITIVE = 'SENSITIVE',
      E_AUTHORITY = 'AUTHORITY', E_INCOMPLETE = 'INCOMPLETE', E_BYTE = 'BYTE',
      E_LANG = 'LANG';
  var SEVERITY = { SENSITIVE: 90, AUTHORITY: 80, AMBIGUOUS: 70, OVERFLOW: 60,
                   INCOMPLETE: 50, GAP: 40, EMPTY: 30, BYTE: 20, ARG: 10 };
  var TEXT_MAX = 160, TOKEN_MAX = 24, PATH_MAX = 64, DEPTH_MAX = 48,
      EDGE_MAX = 512, MEANING_MAX = 4;
  var LX_STOP = 0, LX_OP = 1, LX_FILL = 2, LX_NEG = 3, LX_URG = 4,
      LX_QVAR = 5, LX_SENS = 6, LX_AUTH = 7, LX_FILLNEG = 8;
  var ROLE_NONE = 0, ROLE_QUEM = 1, ROLE_O_QUE = 2, ROLE_QUANDO = 3,
      ROLE_ONDE = 4, ROLE_QUANTO = 5, ROLE_ESTADO = 6;
  var OP_NONE = 0, OP_COMUNICAR = 1, OP_PERGUNTAR = 2, OP_LEMBRAR = 3,
      OP_PLANEJAR = 4, OP_SOCORRO = 5, OP_CONFIRMAR = 6, OP_CANCELAR = 7;
  var OP_NAME = { 1: 'COMUNICAR', 2: 'PERGUNTAR', 3: 'LEMBRAR', 4: 'PLANEJAR',
                  5: 'SOCORRO', 6: 'CONFIRMAR', 7: 'CANCELAR' };
  var ROLE_NAME = { 1: 'QUEM', 2: 'O_QUE', 3: 'QUANDO', 4: 'ONDE',
                    5: 'QUANTO', 6: 'ESTADO' };
  var FILLER_VAR = 0, PER_TODOS = 261;

  // ------------------------------------------------------------- dobramento
  var ACC = {};
  (function () {
    var i;
    for (i = 0xa0; i <= 0xa5; i++) ACC[i] = 97;
    for (i = 0x80; i <= 0x85; i++) ACC[i] = 97;
    ACC[0xa7] = ACC[0x87] = 99;
    for (i = 0xa8; i <= 0xab; i++) ACC[i] = 101;
    for (i = 0x88; i <= 0x8b; i++) ACC[i] = 101;
    for (i = 0xac; i <= 0xaf; i++) ACC[i] = 105;
    for (i = 0x8c; i <= 0x8f; i++) ACC[i] = 105;
    ACC[0xb1] = ACC[0x91] = 110;
    for (i = 0xb2; i <= 0xb6; i++) ACC[i] = 111;
    for (i = 0x92; i <= 0x96; i++) ACC[i] = 111;
    for (i = 0xb9; i <= 0xbc; i++) ACC[i] = 117;
    for (i = 0x99; i <= 0x9c; i++) ACC[i] = 117;
  }());
  var ASCII_SPACE = ' \t,.!?;:\n\r-\''.split('').map(function (c) { return c.charCodeAt(0); });
  /* pontuacao de largura plena e CJK que conta como separador */
  var WIDE = ['　', '、', '。', '・', '，', '！',
              '？', '：', '；', '…'];
  var WIDE_BYTES = WIDE.map(function (c) { return utf8(c); });

  function utf8(s) { return new TextEncoder().encode(s); }
  function fromUtf8(b) { return new TextDecoder().decode(b); }
  function hex(b) {
    var s = '', i;
    for (i = 0; i < b.length; i++) s += (b[i] < 16 ? '0' : '') + b[i].toString(16);
    return s;
  }
  function unhex(s) {
    var b = new Uint8Array(s.length >> 1), i;
    for (i = 0; i < b.length; i++) b[i] = parseInt(s.substr(i * 2, 2), 16);
    return b;
  }
  function eqBytes(a, b) {
    if (a.length !== b.length) return false;
    for (var i = 0; i < a.length; i++) if (a[i] !== b[i]) return false;
    return true;
  }

  function foldBytes(input, cap) {
    var data = (input instanceof Uint8Array) ? input : utf8(String(input));
    cap = cap || TEXT_MAX + 1;
    if (data.length + 1 > cap) return { status: E_OVERFLOW, bytes: null };
    var out = new Uint8Array(data.length), n = 0, i = 0, lastSpace = true;
    while (i < data.length) {
      var c = data[i], piece = null, plen = 1, ch = 0;
      if (c === 0) return { status: E_BYTE, bytes: null };
      if (c < 0x80) {
        if (c >= 65 && c <= 90) { ch = c + 32; i++; }
        else if ((c >= 97 && c <= 122) || (c >= 48 && c <= 57)) { ch = c; i++; }
        else if (ASCII_SPACE.indexOf(c) >= 0) { ch = 32; i++; }
        else return { status: E_BYTE, bytes: null };
      } else if (c === 0xc3) {
        if (i + 1 >= data.length) return { status: E_BYTE, bytes: null };
        var m = ACC[data[i + 1]];
        if (m === undefined) return { status: E_BYTE, bytes: null };
        ch = m; i += 2;
      } else {
        var w;
        if (c >= 0xc2 && c <= 0xdf) w = 2;
        else if (c >= 0xe0 && c <= 0xef) w = 3;
        else if (c >= 0xf0 && c <= 0xf4) w = 4;
        else return { status: E_BYTE, bytes: null };
        if (i + w > data.length) return { status: E_BYTE, bytes: null };
        for (var k = 1; k < w; k++) {
          if (data[i + k] < 0x80 || data[i + k] > 0xbf)
            return { status: E_BYTE, bytes: null };
        }
        var slice = data.subarray(i, i + w), isWide = false;
        for (var q = 0; q < WIDE_BYTES.length; q++) {
          if (eqBytes(slice, WIDE_BYTES[q])) { isWide = true; break; }
        }
        if (isWide) { ch = 32; }
        else { piece = slice; plen = w; }
        i += w;
      }
      if (piece === null && ch === 32) {
        if (lastSpace) continue;
        lastSpace = true;
      } else lastSpace = false;
      if (piece === null) out[n++] = ch;
      else { out.set(piece, n); n += plen; }
    }
    while (n > 0 && out[n - 1] === 32) n--;
    return { status: OK, bytes: out.subarray(0, n) };
  }

  // -------------------------------------------------------------- SHA-256
  var K256 = [
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2];
  function sha256(msg) {
    var H = [0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
             0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19];
    var len = msg.length, blocks = (((len + 9) / 64) | 0) + 1;
    var buf = new Uint8Array(blocks * 64);
    buf.set(msg); buf[len] = 0x80;
    var bits = len * 8;
    buf[buf.length - 4] = (bits >>> 24) & 255; buf[buf.length - 3] = (bits >>> 16) & 255;
    buf[buf.length - 2] = (bits >>> 8) & 255;  buf[buf.length - 1] = bits & 255;
    var w = new Int32Array(64), b, i;
    for (b = 0; b < blocks; b++) {
      for (i = 0; i < 16; i++) {
        w[i] = (buf[b * 64 + i * 4] << 24) | (buf[b * 64 + i * 4 + 1] << 16) |
               (buf[b * 64 + i * 4 + 2] << 8) | buf[b * 64 + i * 4 + 3];
      }
      for (i = 16; i < 64; i++) {
        var s0 = ((w[i-15] >>> 7) | (w[i-15] << 25)) ^ ((w[i-15] >>> 18) | (w[i-15] << 14)) ^ (w[i-15] >>> 3);
        var s1 = ((w[i-2] >>> 17) | (w[i-2] << 15)) ^ ((w[i-2] >>> 19) | (w[i-2] << 13)) ^ (w[i-2] >>> 10);
        w[i] = (w[i-16] + s0 + w[i-7] + s1) | 0;
      }
      var a = H[0], bb = H[1], c = H[2], d = H[3], e = H[4], f = H[5], g = H[6], h = H[7];
      for (i = 0; i < 64; i++) {
        var S1 = ((e >>> 6) | (e << 26)) ^ ((e >>> 11) | (e << 21)) ^ ((e >>> 25) | (e << 7));
        var ch = (e & f) ^ (~e & g);
        var t1 = (h + S1 + ch + K256[i] + w[i]) | 0;
        var S0 = ((a >>> 2) | (a << 30)) ^ ((a >>> 13) | (a << 19)) ^ ((a >>> 22) | (a << 10));
        var maj = (a & bb) ^ (a & c) ^ (bb & c);
        var t2 = (S0 + maj) | 0;
        h = g; g = f; f = e; e = (d + t1) | 0;
        d = c; c = bb; bb = a; a = (t1 + t2) | 0;
      }
      H[0] = (H[0]+a)|0; H[1] = (H[1]+bb)|0; H[2] = (H[2]+c)|0; H[3] = (H[3]+d)|0;
      H[4] = (H[4]+e)|0; H[5] = (H[5]+f)|0; H[6] = (H[6]+g)|0; H[7] = (H[7]+h)|0;
    }
    var out = new Uint8Array(32);
    for (i = 0; i < 8; i++) {
      out[i*4] = (H[i] >>> 24) & 255; out[i*4+1] = (H[i] >>> 16) & 255;
      out[i*4+2] = (H[i] >>> 8) & 255; out[i*4+3] = H[i] & 255;
    }
    return out;
  }

  // ------------------------------------------------------------------ HIR
  function Hir() {
    this.op = OP_NONE; this.polarity = 0; this.urgency = 0;
    this.persistence = 0; this.slots = [];
  }
  Hir.prototype.put = function (role, filler) {
    if (role === ROLE_NONE || role > ROLE_ESTADO) return 'RANGE';
    if (filler > 2047) return 'RANGE';
    for (var i = 0; i < this.slots.length; i++) {
      if (this.slots[i][0] === role) {
        /* espelha hir.c: mesmo preenchimento e idempotente, diferente e conflito */
        return this.slots[i][1] === filler ? 'OK' : 'DUPLICATE_ROLE';
      }
    }
    if (this.slots.length >= 9) return 'FULL';
    this.slots.push([role, filler]);
    this.slots.sort(function (a, b) { return a[0] - b[0] || a[1] - b[1]; });
    return 'OK';
  };
  Hir.prototype.get = function (role) {
    for (var i = 0; i < this.slots.length; i++)
      if (this.slots[i][0] === role) return this.slots[i][1];
    return null;
  };
  Hir.prototype.canon = function () {
    var out = [72, 1, this.op,
               this.polarity | (this.urgency << 1) | (this.persistence << 3),
               this.slots.length, 0];
    for (var i = 0; i < this.slots.length; i++)
      out.push(this.slots[i][0], (this.slots[i][1] >> 8) & 255, this.slots[i][1] & 255);
    return new Uint8Array(out);
  };
  Hir.prototype.digest = function () { return sha256(this.canon()).subarray(0, 8); };
  Hir.prototype.wire = function () {
    var out = new Uint8Array(24), off = 3, i;
    out[0] = (1 << 4) | (this.op & 15);
    out[1] = this.polarity | (this.urgency << 1) | (this.persistence << 3);
    out[2] = this.slots.length;
    for (i = 0; i < this.slots.length; i++) {
      var packed = ((this.slots[i][0] & 31) << 11) | (this.slots[i][1] & 2047);
      out[off++] = (packed >> 8) & 255; out[off++] = packed & 255;
    }
    return out;
  };
  Hir.prototype.key = function () {
    return [this.op, this.polarity, this.urgency, this.persistence,
            JSON.stringify(this.slots)].join('|');
  };
  Hir.prototype.clone = function () {
    var h = new Hir();
    h.op = this.op; h.polarity = this.polarity; h.urgency = this.urgency;
    h.persistence = this.persistence;
    h.slots = this.slots.map(function (s) { return [s[0], s[1]]; });
    return h;
  };
  function hirFromWire(w) {
    var h = new Hir(), n, i;
    h.op = w[0] & 15;
    h.polarity = w[1] & 1; h.urgency = (w[1] >> 1) & 3; h.persistence = (w[1] >> 3) & 3;
    n = w[2];
    for (i = 0; i < n; i++) {
      var packed = (w[3 + i * 2] << 8) | w[4 + i * 2];
      h.slots.push([(packed >> 11) & 31, packed & 2047]);
    }
    h.slots.sort(function (a, b) { return a[0] - b[0] || a[1] - b[1]; });
    return h;
  }
  function hirValidate(h) {
    if (h.op === OP_NONE || h.op >= 8) return 'RANGE';
    if (h.polarity > 1 || h.urgency > 3 || h.persistence > 2) return 'RANGE';
    if (!h.slots.length) return 'EMPTY';
    var seen = {};
    for (var i = 0; i < h.slots.length; i++) {
      var r = h.slots[i][0];
      if (r === ROLE_NONE || r > ROLE_ESTADO || h.slots[i][1] > 2047) return 'RANGE';
      if (seen[r]) return 'DUPLICATE_ROLE';
      seen[r] = 1;
    }
    return 'OK';
  }

  global.HERUS = {
    OK: OK, statuses: { E_ARG: E_ARG, E_EMPTY: E_EMPTY, E_OVERFLOW: E_OVERFLOW,
      E_GAP: E_GAP, E_AMBIGUOUS: E_AMBIGUOUS, E_SENSITIVE: E_SENSITIVE,
      E_AUTHORITY: E_AUTHORITY, E_INCOMPLETE: E_INCOMPLETE, E_BYTE: E_BYTE,
      E_LANG: E_LANG },
    ROLE_NAME: ROLE_NAME, OP_NAME: OP_NAME, FILLER_VAR: FILLER_VAR,
    ROLE_QUEM: ROLE_QUEM, ROLE_O_QUE: ROLE_O_QUE, ROLE_QUANDO: ROLE_QUANDO,
    ROLE_ONDE: ROLE_ONDE, ROLE_QUANTO: ROLE_QUANTO, ROLE_ESTADO: ROLE_ESTADO,
    OP_COMUNICAR: OP_COMUNICAR, OP_PERGUNTAR: OP_PERGUNTAR,
    OP_LEMBRAR: OP_LEMBRAR, OP_PLANEJAR: OP_PLANEJAR, OP_SOCORRO: OP_SOCORRO,
    OP_CONFIRMAR: OP_CONFIRMAR, OP_CANCELAR: OP_CANCELAR,
    Hir: Hir, hirFromWire: hirFromWire, hirValidate: hirValidate,
    fold: foldBytes, sha256: sha256, hex: hex, unhex: unhex,
    utf8: utf8, fromUtf8: fromUtf8, _internals: {}
  };
}(typeof window !== 'undefined' ? window : globalThis));

/* ------------------------------------------------- Babel: compilar e render */
(function (global) {
  'use strict';
  var H = global.HERUS;
  var S = H.statuses;
  var LX_STOP = 0, LX_OP = 1, LX_FILL = 2, LX_NEG = 3, LX_URG = 4,
      LX_QVAR = 5, LX_SENS = 6, LX_AUTH = 7, LX_FILLNEG = 8;
  var SEVERITY = { SENSITIVE: 90, AUTHORITY: 80, AMBIGUOUS: 70, OVERFLOW: 60,
                   INCOMPLETE: 50, GAP: 40, EMPTY: 30, BYTE: 20, ARG: 10 };
  var TEXT_MAX = 160, TOKEN_MAX = 24, PATH_MAX = 64, DEPTH_MAX = 48,
      MEANING_MAX = 4;
  var PER_TODOS = 261;

  function Pack(raw) {
    var self = this;
    this.raw = raw;
    this.langs = raw.langs;
    this.byTag = {};
    raw.langs.forEach(function (l, i) { l.index = i; self.byTag[l.tag] = l; });
    this.tags = raw.langs.map(function (l) { return l.tag; });
    this.symById = {};
    raw.syms.forEach(function (s) { self.symById[s.sym] = s; });
    /* Baldes por primeiro byte, entradas da mais longa para a mais curta —
     * exatamente a ordem que LOOM_BUCKET gera no C. */
    this.buckets = {};
    this.tags.forEach(function (tag) {
      var buckets = new Array(256);
      raw.lex[tag].forEach(function (row) {
        var bytes = H.utf8(row[0]);
        var e = { bytes: bytes, len: bytes.length, cls: row[1],
                  role: row[2], val: row[3], form: row[0] };
        var b = bytes[0];
        if (!buckets[b]) buckets[b] = [];
        buckets[b].push(e);
      });
      for (var b = 0; b < 256; b++) {
        if (buckets[b]) buckets[b].sort(function (x, y) { return y.len - x.len; });
      }
      self.buckets[tag] = buckets;
    });
  }
  Pack.prototype.renderForm = function (sym, tag) {
    var s = this.symById[sym];
    return s ? (s.render[tag] || null) : null;
  };
  Pack.prototype.negForm = function (sym, tag) {
    var s = this.symById[sym];
    return (s && s.neg && s.neg[tag]) ? s.neg[tag] : null;
  };

  function isAlnum(c) {
    return (c >= 97 && c <= 122) || (c >= 48 && c <= 57);
  }
  function boundaryOk(s, n, i, j, spaced) {
    if (spaced) {
      if (i > 0 && s[i - 1] !== 32) return false;
      if (j < n && s[j] !== 32) return false;
      return true;
    }
    if (i > 0 && isAlnum(s[i - 1]) && isAlnum(s[i])) return false;
    if (j < n && isAlnum(s[j - 1]) && isAlnum(s[j])) return false;
    return true;
  }
  function skipSpaces(s, n, i) { while (i < n && s[i] === 32) i++; return i; }

  function matchesAt(pack, tag, s, n, pos, spaced) {
    var bucket = pack.buckets[tag][s[pos]] || [], out = [], k, e, j, m, ok;
    for (k = 0; k < bucket.length; k++) {
      e = bucket[k]; j = pos + e.len;
      if (j > n) continue;
      ok = true;
      for (m = 0; m < e.len; m++) { if (s[pos + m] !== e.bytes[m]) { ok = false; break; } }
      if (!ok) continue;
      if (!boundaryOk(s, n, pos, j, spaced)) continue;
      out.push(e);
    }
    return out;
  }

  function buildMeaning(reading) {
    var h = new H.Hir(), op = 0, qcount = 0, i, e, st, para = 0;
    for (i = 0; i < reading.length; i++) {
      e = reading[i];
      if (e.cls === LX_SENS) return { status: S.E_SENSITIVE };
      if (e.cls === LX_AUTH) return { status: S.E_AUTHORITY };
      if (e.cls === LX_STOP) continue;
      if (e.form.indexOf(' ') >= 0) para++;
      if (e.cls === LX_NEG) { h.polarity = 1; continue; }
      if (e.cls === LX_URG) {
        if (h.urgency !== 0 && h.urgency !== e.val) return { status: S.E_AMBIGUOUS };
        h.urgency = e.val; continue;
      }
      if (e.cls === LX_OP) {
        if (op !== 0 && op !== e.val) return { status: S.E_AMBIGUOUS };
        op = e.val; continue;
      }
      if (e.cls === LX_QVAR) {
        st = h.put(e.role, H.FILLER_VAR);
        if (st === 'DUPLICATE_ROLE') return { status: S.E_AMBIGUOUS };
        if (st === 'FULL') return { status: S.E_OVERFLOW };
        if (st !== 'OK') return { status: S.E_ARG };
        qcount++; continue;
      }
      if (e.cls === LX_FILL || e.cls === LX_FILLNEG) {
        st = h.put(e.role, e.val);
        if (st === 'DUPLICATE_ROLE') return { status: S.E_AMBIGUOUS };
        if (st === 'FULL') return { status: S.E_OVERFLOW };
        if (st !== 'OK') return { status: S.E_ARG };
        if (e.cls === LX_FILLNEG) h.polarity = 1;
        continue;
      }
      return { status: S.E_ARG };
    }
    if (op === 0) {
      if (qcount > 0) op = H.OP_PERGUNTAR;
      else if (h.slots.length) op = H.OP_COMUNICAR;
      else return { status: S.E_INCOMPLETE };
    }
    if (qcount > 0 && op !== H.OP_PERGUNTAR) return { status: S.E_AMBIGUOUS };
    h.op = op;
    if (op === H.OP_SOCORRO) { h.urgency = 3; h.put(H.ROLE_QUEM, PER_TODOS); }
    if (op === H.OP_LEMBRAR) h.persistence = 1;

    var roles = {}, hasVar = false;
    h.slots.forEach(function (s2) {
      roles[s2[0]] = 1;
      if (s2[1] === H.FILLER_VAR) hasVar = true;
    });
    if (op === H.OP_COMUNICAR) {
      if (!roles[H.ROLE_QUEM]) return { status: S.E_INCOMPLETE, missing: H.ROLE_QUEM };
      if (!roles[H.ROLE_O_QUE]) return { status: S.E_INCOMPLETE, missing: H.ROLE_O_QUE };
    } else if (op === H.OP_PERGUNTAR) {
      if (qcount > 1) return { status: S.E_AMBIGUOUS };
      if (!hasVar || qcount !== 1 || h.slots.length < 2) return { status: S.E_INCOMPLETE };
    } else if (op === H.OP_LEMBRAR) {
      if (!roles[H.ROLE_O_QUE] && !roles[H.ROLE_ONDE] && !roles[H.ROLE_QUANDO])
        return { status: S.E_INCOMPLETE, missing: H.ROLE_O_QUE };
    } else if (op === H.OP_CONFIRMAR || op === H.OP_CANCELAR) {
      if (!roles[H.ROLE_O_QUE]) return { status: S.E_INCOMPLETE, missing: H.ROLE_O_QUE };
    } else if (op === H.OP_PLANEJAR) {
      if (!roles[H.ROLE_O_QUE] && !roles[H.ROLE_ONDE])
        return { status: S.E_INCOMPLETE, missing: H.ROLE_O_QUE };
    } else if (op !== H.OP_SOCORRO) {
      return { status: S.E_INCOMPLETE };
    }
    if (hasVar && op !== H.OP_PERGUNTAR) return { status: S.E_AMBIGUOUS };
    if (H.hirValidate(h) !== 'OK') return { status: S.E_INCOMPLETE };
    return { status: H.OK, hir: h, paraphrase: para };
  }

  /* a ≼ b: mesma moldura e todo slot de `a` esta em `b` com o mesmo valor */
  function dominates(a, b) {
    if (a.op !== b.op || a.polarity !== b.polarity ||
        a.urgency !== b.urgency || a.persistence !== b.persistence) return false;
    for (var i = 0; i < a.slots.length; i++) {
      var found = false;
      for (var j = 0; j < b.slots.length; j++) {
        if (b.slots[j][0] !== a.slots[i][0]) continue;
        if (b.slots[j][1] !== a.slots[i][1]) return false;
        found = true; break;
      }
      if (!found) return false;
    }
    return true;
  }
  function dominant(cands) {
    var best = null, i, k, top;
    for (i = 0; i < cands.length; i++) {
      top = true;
      for (k = 0; k < cands.length; k++) {
        if (!dominates(cands[k], cands[i])) { top = false; break; }
      }
      if (!top) continue;
      if (best) return null;
      best = cands[i];
    }
    return best;
  }

  function compile(pack, text, tag) {
    var lang = pack.byTag[tag];
    if (!lang) return { status: S.E_LANG };
    var raw = (text instanceof Uint8Array) ? text : H.utf8(String(text));
    if (!raw.length) return { status: S.E_EMPTY };
    if (raw.length > TEXT_MAX) return { status: S.E_OVERFLOW };
    var f = H.fold(raw);
    if (f.status !== H.OK) return { status: f.status };
    var s = f.bytes, n = s.length;
    if (!n) return { status: S.E_EMPTY };
    var spaced = !!lang.spaced;
    if (spaced) {
      var ntok = 0, inTok = false, i;
      for (i = 0; i < n; i++) {
        if (s[i] === 32) { inTok = false; continue; }
        if (!inTok) { inTok = true; ntok++; }
      }
      if (ntok > TOKEN_MAX) return { status: S.E_OVERFLOW };
    }

    /* passo um: classe protegida e autoridade em qualquer posicao */
    for (var p = 0; p < n; p++) {
      if (s[p] === 32) continue;
      if (spaced && p > 0 && s[p - 1] !== 32) continue;
      var hits = matchesAt(pack, tag, s, n, p, spaced);
      for (var q = 0; q < hits.length; q++) {
        if (hits[q].cls === LX_SENS) return { status: S.E_SENSITIVE };
        if (hits[q].cls === LX_AUTH) return { status: S.E_AUTHORITY };
      }
    }

    /* busca em profundidade sobre todas as leituras, com orcamento */
    var cands = [], keys = {}, paths = 0, truncated = false, tooMany = false;
    var worst = S.E_INCOMPLETE, worstSev = 0, worstMissing = 0, paraphrase = 0;
    var stack = [{ pos: skipSpaces(s, n, 0), list: null, idx: 0 }];
    var path = [];
    while (stack.length) {
      var fr = stack[stack.length - 1];
      if (fr.list === null) fr.list = matchesAt(pack, tag, s, n, fr.pos, spaced);
      if (fr.idx >= fr.list.length) { stack.pop(); path.pop(); continue; }
      var e = fr.list[fr.idx++];
      path[stack.length - 1] = e;
      var npos = skipSpaces(s, n, fr.pos + e.len);
      if (npos >= n) {
        var r = buildMeaning(path.slice(0, stack.length));
        paths++;
        if (r.status === H.OK) {
          var key = r.hir.key();
          if (!keys[key]) {
            if (cands.length >= MEANING_MAX) { tooMany = true; break; }
            keys[key] = 1; cands.push(r.hir);
            if (cands.length === 1) paraphrase = r.paraphrase;
          }
        } else if ((SEVERITY[r.status] || 0) > worstSev) {
          worstSev = SEVERITY[r.status] || 0; worst = r.status;
          worstMissing = r.missing || 0;
        }
        if (paths >= PATH_MAX) { truncated = true; break; }
        continue;
      }
      if (stack.length >= DEPTH_MAX) return { status: S.E_OVERFLOW };
      stack.push({ pos: npos, list: null, idx: 0 });
    }

    if (tooMany) return { status: S.E_AMBIGUOUS, readings: cands.length + 1 };
    if (paths === 0) {
      /* alcance para frente: onde a lacuna comeca */
      var seen = new Uint8Array(n + 2), best = 0;
      seen[skipSpaces(s, n, 0)] = 1;
      for (var a = 0; a <= n; a++) {
        if (!seen[a]) continue;
        if (a > best) best = a;
        if (a === n) continue;
        var hs = matchesAt(pack, tag, s, n, a, spaced);
        for (var b = 0; b < hs.length; b++) seen[skipSpaces(s, n, a + hs[b].len)] = 1;
      }
      var at = skipSpaces(s, n, best), end = at;
      if (spaced) { while (end < n && s[end] !== 32) end++; }
      else { end = at < n ? at + 1 : at; }
      if (end === at && at < n) end = at + 1;
      return { status: S.E_GAP, gapAt: at, gapLen: end - at,
               gapText: H.fromUtf8(s.subarray(at, end)) };
    }
    if (cands.length) {
      if (truncated) return { status: S.E_AMBIGUOUS, readings: cands.length };
      var pick = cands.length === 1 ? cands[0] : dominant(cands);
      if (!pick) return { status: S.E_AMBIGUOUS, readings: cands.length };
      return { status: H.OK, hir: pick, readings: cands.length, paths: paths,
               paraphrase: paraphrase };
    }
    return { status: worst, missing: worstMissing, paths: paths };
  }

  function render(pack, h, tag) {
    var lang = pack.byTag[tag];
    if (!lang) return null;
    var tmplTable = pack.raw.templates[tag];
    var name = H.OP_NAME[h.op];
    if (!tmplTable || !name || !tmplTable[name]) return null;
    var tmpl = tmplTable[name];
    var varRole = 0, lexicalNeg = false, i;
    for (i = 0; i < h.slots.length; i++) {
      if (h.slots[i][1] === H.FILLER_VAR) varRole = h.slots[i][0];
      if (h.polarity && h.slots[i][0] === H.ROLE_O_QUE &&
          pack.negForm(h.slots[i][1], tag)) lexicalNeg = true;
    }
    var values = {};
    Object.keys(H.ROLE_NAME).forEach(function (code) {
      var c = parseInt(code, 10), nm = H.ROLE_NAME[c], f = h.get(c);
      if (f === null || c === varRole) { values[nm] = ''; return; }
      var form = null;
      if (h.polarity && c === H.ROLE_O_QUE) form = pack.negForm(f, tag);
      if (!form) form = pack.renderForm(f, tag);
      values[nm] = form || '';
    });
    values.OP = (pack.raw.op_render[tag] || {})[String(h.op)] || '';
    values.NEG = (!h.polarity || lexicalNeg) ? '' : (pack.raw.neg_render[tag] || '');
    if (h.urgency === 1 || h.urgency === 2)
      values.URG = (pack.raw.urg_render[tag] || {})[String(h.urgency)] || '';
    else if (h.urgency === 3 && h.op !== H.OP_SOCORRO)
      values.URG = (pack.raw.urg_render[tag] || {})['2'] || '';
    else values.URG = '';
    values.QVAR = varRole ? ((pack.raw.qw_render[tag] || {})[String(varRole)] || '') : '';

    var pieces = [];
    tmpl.forEach(function (seg) {
      if (seg.indexOf('%') < 0) { pieces.push(seg); return; }
      var out = seg, drop = false;
      Object.keys(values).forEach(function (k2) {
        var token = '%' + k2 + '%';
        if (drop || out.indexOf(token) < 0) return;
        if (!values[k2]) { drop = true; return; }
        out = out.split(token).join(values[k2]);
      });
      if (!drop && out) pieces.push(out);
    });
    var joined = '';
    pieces.forEach(function (piece) {
      if (!piece) return;
      if (joined) {
        if (lang.spaced) joined += ' ';
        else {
          var a = H.utf8(joined), b = H.utf8(piece);
          if (isAlnum(a[a.length - 1]) && isAlnum(b[0])) joined += ' ';
        }
      }
      joined += piece;
    });
    return lang.spaced ? joined.split(/\s+/).filter(Boolean).join(' ') : joined.trim();
  }

  H.Pack = Pack;
  H.compile = compile;
  H.render = render;
  H._internals.dominant = dominant;
}(typeof window !== 'undefined' ? window : globalThis));

/* ------------------------------------- Aether: RS(33,29), CRC-32 e 16-FSK */
(function (global) {
  'use strict';
  var H = global.HERUS;
  var PAYLOAD = 24, HDR = 1, CRC = 4, DATA = 29, PARITY = 4, FRAME = 33;
  var VERSION = 1, PROFILE_OPEN = 0;
  var SR = 48000, SYM_LEN = 1024, TONES = 16, BIN_BASE = 40, SYNC_LEN = 4;
  var SYNC = [15, 0, 15, 0];
  var DATA_SYMS = FRAME * 2, TOTAL_SYMS = SYNC_LEN + DATA_SYMS;
  var SAMPLES = TOTAL_SYMS * SYM_LEN;
  var GLYPH_SIDE = 18, GLYPH_CELLS = GLYPH_SIDE * GLYPH_SIDE, GLYPH_BITS = FRAME * 8;

  var EXP = new Uint8Array(512), LOG = new Uint8Array(256);
  (function () {
    var x = 1, i;
    for (i = 0; i < 255; i++) { EXP[i] = x; LOG[x] = i; x <<= 1; if (x & 0x100) x ^= 0x11d; }
    for (i = 255; i < 512; i++) EXP[i] = EXP[i - 255];
  }());
  function mul(a, b) { return (a === 0 || b === 0) ? 0 : EXP[LOG[a] + LOG[b]]; }
  function inv(a) { return a === 0 ? 0 : EXP[255 - LOG[a]]; }
  function div(a, b) { return mul(a, inv(b)); }

  function generator() {
    var g = new Uint8Array(PARITY + 1), i, j;
    g[0] = 1;
    for (i = 0; i < PARITY; i++) {
      var root = EXP[i];
      for (j = i + 1; j > 0; j--) g[j] = g[j - 1] ^ mul(g[j], root);
      g[0] = mul(g[0], root);
    }
    return g;
  }
  function rsEncode(data) {
    var g = generator(), parity = new Uint8Array(PARITY), i, j;
    for (i = 0; i < DATA; i++) {
      var fb = data[i] ^ parity[0];
      for (j = 0; j < PARITY - 1; j++) parity[j] = parity[j + 1] ^ mul(fb, g[PARITY - 1 - j]);
      parity[PARITY - 1] = mul(fb, g[0]);
    }
    return parity;
  }
  function syndromes(r) {
    var s = new Uint8Array(PARITY), j, i;
    for (j = 0; j < PARITY; j++) {
      var acc = 0;
      for (i = 0; i < FRAME; i++) acc = mul(acc, EXP[j]) ^ r[i];
      s[j] = acc;
    }
    return s;
  }
  function rsCorrect(r) {
    var s = syndromes(r), check;
    if (!(s[0] | s[1] | s[2] | s[3])) return 0;
    if (s[0] !== 0) {
      var X = div(s[1], s[0]);
      if (X !== 0 && s[1] === mul(s[0], X) && s[2] === mul(s[0], mul(X, X)) &&
          s[3] === mul(s[0], mul(X, mul(X, X)))) {
        var loc = LOG[X];
        if (loc < FRAME) {
          r[FRAME - 1 - loc] ^= s[0];
          check = syndromes(r);
          if (!(check[0] | check[1] | check[2] | check[3])) return 1;
          r[FRAME - 1 - loc] ^= s[0];
        }
      }
    }
    var det = mul(s[1], s[1]) ^ mul(s[0], s[2]);
    if (det === 0) return -1;
    var l1 = div(mul(s[2], s[1]) ^ mul(s[3], s[0]), det);
    var l2 = div(mul(s[1], s[3]) ^ mul(s[2], s[2]), det);
    if (l2 === 0) return -1;
    var X1 = 0, X2 = 0, found = 0, i;
    for (i = 0; i < 255 && found < 2; i++) {
      var x = EXP[i];
      if ((1 ^ mul(l1, x) ^ mul(l2, mul(x, x))) !== 0) continue;
      if (found === 0) X1 = inv(x); else X2 = inv(x);
      found++;
    }
    if (found !== 2 || X1 === X2) return -1;
    var p1 = LOG[X1], p2 = LOG[X2];
    if (p1 >= FRAME || p2 >= FRAME) return -1;
    var d = X1 ^ X2;
    if (d === 0) return -1;
    var e1 = div(mul(s[0], X2) ^ s[1], d), e2 = s[0] ^ e1;
    r[FRAME - 1 - p1] ^= e1;
    r[FRAME - 1 - p2] ^= e2;
    check = syndromes(r);
    if (!(check[0] | check[1] | check[2] | check[3])) return 2;
    r[FRAME - 1 - p1] ^= e1;
    r[FRAME - 1 - p2] ^= e2;
    return -1;
  }

  function crc32(p) {
    var crc = 0xffffffff, i, k;
    for (i = 0; i < p.length; i++) {
      crc ^= p[i];
      for (k = 0; k < 8; k++) crc = (crc >>> 1) ^ (0xedb88320 & -(crc & 1));
    }
    return (~crc) >>> 0;
  }

  function pack(payload, seq) {
    var data = new Uint8Array(DATA), out = new Uint8Array(FRAME);
    data[0] = (VERSION << 4) | ((PROFILE_OPEN & 3) << 2) | (seq & 3);
    data.set(payload, HDR);
    var c = crc32(data.subarray(0, HDR + PAYLOAD));
    data[25] = (c >>> 24) & 255; data[26] = (c >>> 16) & 255;
    data[27] = (c >>> 8) & 255;  data[28] = c & 255;
    out.set(data, 0);
    out.set(rsEncode(data), DATA);
    return out;
  }
  function unpack(frame) {
    var work = new Uint8Array(frame), fixed = rsCorrect(work);
    if (fixed < 0) return { status: 'UNCORRECTED' };
    var want = ((work[25] << 24) | (work[26] << 16) | (work[27] << 8) | work[28]) >>> 0;
    if (crc32(work.subarray(0, HDR + PAYLOAD)) !== want) return { status: 'CRC' };
    if ((work[0] >> 4) !== VERSION) return { status: 'VERSION' };
    if (((work[0] >> 2) & 3) !== PROFILE_OPEN) return { status: 'PROFILE' };
    return { status: 'OK', payload: work.subarray(HDR, HDR + PAYLOAD),
             corrected: fixed, seq: work[0] & 3 };
  }

  function toneHz(t) { return Math.round((BIN_BASE + (t & 15)) * SR / SYM_LEN); }
  function toneStream(frame) {
    var out = new Uint8Array(TOTAL_SYMS), i;
    out.set(SYNC, 0);
    for (i = 0; i < FRAME; i++) {
      out[SYNC_LEN + i * 2] = frame[i] >> 4;
      out[SYNC_LEN + i * 2 + 1] = frame[i] & 15;
    }
    return out;
  }
  function soundEncode(frame, rate) {
    rate = rate || SR;
    var symLen = Math.round(SYM_LEN * rate / SR);
    var stream = toneStream(frame);
    var out = new Float32Array(stream.length * symLen), sym, i;
    for (sym = 0; sym < stream.length; sym++) {
      var w = 2 * Math.PI * (BIN_BASE + stream[sym]) / SYM_LEN * (SR / rate);
      for (i = 0; i < symLen; i++) out[sym * symLen + i] = 0.5 * Math.sin(w * i);
    }
    return out;
  }
  function goertzel(x, off, len, bin, scale) {
    var w = 2 * Math.PI * bin / SYM_LEN * scale;
    var coeff = 2 * Math.cos(w), s0 = 0, s1 = 0, s2 = 0, i;
    for (i = 0; i < len; i++) {
      s0 = x[off + i] + coeff * s1 - s2;
      s2 = s1; s1 = s0;
    }
    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
  }
  function demod(x, off, symLen, scale) {
    var best = -1, second = -1, arg = 0, t;
    for (t = 0; t < TONES; t++) {
      var m = goertzel(x, off, symLen, BIN_BASE + t, scale);
      if (m > best) { second = best; best = m; arg = t; }
      else if (m > second) second = m;
    }
    return { tone: arg, best: best, second: second };
  }
  function soundDecode(samples, rate) {
    rate = rate || SR;
    var symLen = Math.round(SYM_LEN * rate / SR);
    var scale = SR / rate;
    var need = TOTAL_SYMS * symLen;
    if (samples.length < need) return { status: 'SHORT' };
    var limit = samples.length - need, bestOff = 0, bestScore = -1, step, off, i;
    for (step = 0; step < 2; step++) {
      var from = 0, to = limit, grain = 64;
      if (step === 1) {
        from = Math.max(0, bestOff - 64);
        to = Math.min(limit, bestOff + 64);
        grain = 4; bestScore = -1;
      }
      for (off = from; off <= to; off += grain) {
        var score = 0, ok = true;
        for (i = 0; i < SYNC_LEN; i++) {
          var d = demod(samples, off + i * symLen, symLen, scale);
          if (d.tone !== SYNC[i]) { ok = false; break; }
          score += (d.best - d.second) / (d.best + d.second + 1e-12);
        }
        if (ok && score > bestScore) { bestScore = score; bestOff = off; }
      }
      if (bestScore < 0) return { status: 'NO_SYNC' };
    }
    var frame = new Uint8Array(FRAME), base = bestOff + SYNC_LEN * symLen, weak = 0;
    for (i = 0; i < FRAME; i++) {
      var a = demod(samples, base + (i * 2) * symLen, symLen, scale);
      var b = demod(samples, base + (i * 2 + 1) * symLen, symLen, scale);
      frame[i] = (a.tone << 4) | b.tone;
      if (a.best < 2 * a.second) weak++;
      if (b.best < 2 * b.second) weak++;
    }
    return { status: 'OK', frame: frame, offset: bestOff, weak: weak };
  }

  var CORNER = [[0, 0], [0, GLYPH_SIDE - 2], [GLYPH_SIDE - 2, 0],
                [GLYPH_SIDE - 2, GLYPH_SIDE - 2]];
  function isCorner(r, c) {
    for (var k = 0; k < 4; k++) {
      if (r >= CORNER[k][0] && r < CORNER[k][0] + 2 &&
          c >= CORNER[k][1] && c < CORNER[k][1] + 2) return true;
    }
    return false;
  }
  function glyphEncode(frame) {
    var cells = new Uint8Array(GLYPH_CELLS), bit = 0, k, dr, dc, r, c;
    for (k = 0; k < 4; k++) {
      for (dr = 0; dr < 2; dr++) for (dc = 0; dc < 2; dc++)
        cells[(CORNER[k][0] + dr) * GLYPH_SIDE + CORNER[k][1] + dc] = (k === 3) ? 0 : 1;
    }
    for (r = 0; r < GLYPH_SIDE; r++) {
      for (c = 0; c < GLYPH_SIDE; c++) {
        if (isCorner(r, c)) continue;
        if (bit >= GLYPH_BITS) continue;
        cells[r * GLYPH_SIDE + c] = (frame[bit >> 3] >> (7 - (bit & 7))) & 1;
        bit++;
      }
    }
    return cells;
  }

  H.aether = {
    FRAME: FRAME, PAYLOAD: PAYLOAD, SR: SR, SYM_LEN: SYM_LEN, TONES: TONES,
    SAMPLES: SAMPLES, TOTAL_SYMS: TOTAL_SYMS, GLYPH_SIDE: GLYPH_SIDE,
    pack: pack, unpack: unpack, crc32: crc32, toneHz: toneHz,
    toneStream: toneStream, soundEncode: soundEncode, soundDecode: soundDecode,
    glyphEncode: glyphEncode, rsCorrect: rsCorrect
  };
}(typeof window !== 'undefined' ? window : globalThis));

/* ----------------------------------------- a pagina prova, na frente de voce */
(function (global) {
  'use strict';
  var H = global.HERUS;
  H.selfTest = function (pack, vectors) {
    var fails = [], n = 0;
    vectors.babel.forEach(function (v) {
      n++;
      if (v.op === 'render') {
        var got = H.render(pack, H.hirFromWire(H.unhex(v.wire)), v.lang);
        if (got !== v.text) fails.push('render[' + v.lang + '] ' + v.wire +
          ' -> ' + JSON.stringify(got) + ' != ' + JSON.stringify(v.text));
      } else if (v.op === 'compile') {
        var r = H.compile(pack, v.text, v.lang);
        if (r.status !== H.OK) fails.push('compile[' + v.lang + '] ' +
          JSON.stringify(v.text) + ' -> ' + r.status);
        else {
          var d = H.hex(r.hir.digest()), w = H.hex(r.hir.wire());
          if (d !== v.digest || w !== v.wire)
            fails.push('compile[' + v.lang + '] ' + JSON.stringify(v.text) +
                       ' -> ' + d + '/' + w + ' != ' + v.digest + '/' + v.wire);
        }
      } else if (v.op === 'refuse') {
        var r2 = H.compile(pack, v.text, v.lang);
        if (r2.status !== v.status) fails.push('refuse[' + v.lang + '] ' +
          JSON.stringify(v.text) + ' -> ' + r2.status + ' != ' + v.status);
      }
    });
    vectors.aether.forEach(function (v) {
      n++;
      var A = H.aether;
      if (v.op === 'pack') {
        var got = H.hex(A.pack(H.unhex(v.payload), 0));
        if (got !== v.frame) fails.push('pack ' + v.payload + ' -> ' + got);
      } else if (v.op === 'unpack') {
        var r = A.unpack(H.unhex(v.frame));
        if (r.status !== 'OK' || H.hex(r.payload) !== v.payload ||
            r.corrected !== v.corrected)
          fails.push('unpack ' + v.frame + ' -> ' + r.status + '/' +
                     (r.payload ? H.hex(r.payload) : '-') + '/' + r.corrected);
      } else if (v.op === 'unpack_fail') {
        var r2 = A.unpack(H.unhex(v.frame));
        if (r2.status !== v.status) fails.push('unpack_fail ' + v.frame +
          ' -> ' + r2.status + ' != ' + v.status);
      } else if (v.op === 'tones') {
        var s = A.toneStream(H.unhex(v.frame)), got2 = '';
        for (var i = 0; i < s.length; i++) got2 += s[i].toString(16);
        if (got2 !== v.tones) fails.push('tones ' + v.frame + ' -> ' + got2);
      } else if (v.op === 'glyph') {
        var cells = A.glyphEncode(H.unhex(v.frame)), got3 = '';
        for (var j = 0; j < cells.length; j++) got3 += cells[j] ? '1' : '0';
        if (got3 !== v.cells) fails.push('glyph ' + v.frame + ' divergiu');
      } else if (v.op === 'crc32') {
        var c = A.crc32(H.unhex(v.data)).toString(16).padStart(8, '0');
        if (c !== v.crc) fails.push('crc32 ' + v.data + ' -> ' + c + ' != ' + v.crc);
      }
    });
    /* o par codificador/decodificador do modem fecha sobre si mesmo */
    var A2 = H.aether, loop = 0, loopFails = 0;
    for (var t = 0; t < 4; t++) {
      var pl = new Uint8Array(A2.PAYLOAD);
      for (var k = 0; k < pl.length; k++) pl[k] = (t * 37 + k * 61) & 255;
      var fr = A2.pack(pl, t & 3);
      var wave = A2.soundEncode(fr, A2.SR);
      var dec = A2.soundDecode(wave, A2.SR);
      loop++;
      if (dec.status !== 'OK' || H.hex(dec.frame) !== H.hex(fr)) loopFails++;
    }
    if (loopFails) fails.push('modem: ' + loopFails + '/' + loop +
                              ' voltas de audio falharam');
    return { total: n + loop, fails: fails };
  };
}(typeof window !== 'undefined' ? window : globalThis));
