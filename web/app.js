/* app.js — a interface. Um objetivo: uma crianca consegue usar, e por baixo
 * roda exatamente o compilador que embarca no relogio.
 */
(function () {
  'use strict';
  var H = window.HERUS, A = H.aether;
  var pack = new H.Pack(window.__PACK__);
  var $ = function (id) { return document.getElementById(id); };

  /* ------------------------------------------------- prova, na sua frente */
  (function () {
    var el = $('parity');
    try {
      var r = H.selfTest(pack, window.__VEC__);
      if (r.fails.length) {
        el.className = 'badge bad';
        el.innerHTML = '<b>' + r.fails.length + ' de ' + r.total +
          ' vetores divergiram</b><br>' + r.fails[0].slice(0, 90);
      } else {
        el.innerHTML = '<b>' + r.total.toLocaleString('pt-BR') +
          ' vetores do firmware</b> reproduzidos aqui · 0 divergências';
      }
    } catch (e) {
      el.className = 'badge bad';
      el.textContent = 'a verificação falhou: ' + e.message;
    }
  }());
  $('fPack').textContent = 'core v' + (window.__PACK__.version || 1) +
    ' · ' + pack.raw.syms.length + ' conceitos · ' + pack.langs.length + ' idiomas';

  /* ------------------------------------------------------------- estado */
  var st = {
    send: 'pt', recv: 'ja', op: H.OP_COMUNICAR, polarity: 0, urgency: 0,
    slots: {}   /* role -> sym */
  };
  var ROLE_LABEL = { 1: 'quem', 2: 'o quê', 3: 'quando', 4: 'onde',
                     5: 'quanto', 6: 'estado' };
  var OP_LABEL = { 1: 'avisar', 2: 'perguntar', 3: 'lembrar', 4: 'planejar',
                   5: 'socorro', 6: 'confirmar', 7: 'cancelar' };
  var byRole = {};
  pack.raw.syms.forEach(function (s) {
    (byRole[s.role] = byRole[s.role] || []).push(s);
  });

  function buildHir() {
    var h = new H.Hir();
    h.op = st.op; h.polarity = st.polarity; h.urgency = st.urgency;
    Object.keys(st.slots).forEach(function (r) {
      if (st.slots[r] !== null && st.slots[r] !== undefined)
        h.put(parseInt(r, 10), st.slots[r]);
    });
    if (h.op === H.OP_SOCORRO) { h.urgency = 3; h.put(H.ROLE_QUEM, 261); }
    if (h.op === H.OP_LEMBRAR) h.persistence = 1;
    return h;
  }

  /* --------------------------------------------------------- construcao */
  function langRow(host, which) {
    host.innerHTML = '';
    pack.langs.forEach(function (l) {
      var b = document.createElement('button');
      b.className = 'chip'; b.type = 'button';
      b.textContent = l.endonym;
      b.setAttribute('aria-pressed', String(st[which] === l.tag));
      b.title = l.name + ' · ~' + l.speakers_m + ' milhões de falantes';
      b.onclick = function () { st[which] = l.tag; langRow(host, which); paint(); };
      host.appendChild(b);
    });
  }

  function opRow() {
    var host = $('opRow');
    host.innerHTML = '';
    [1, 2, 3, 4, 6, 7, 5].forEach(function (code) {
      var b = document.createElement('button');
      b.className = 'chip'; b.type = 'button';
      b.textContent = OP_LABEL[code];
      b.setAttribute('aria-pressed', String(st.op === code));
      b.onclick = function () { st.op = code; opRow(); paint(); };
      host.appendChild(b);
    });
  }

  function modRow() {
    var host = $('modRow');
    host.innerHTML = '';
    var neg = document.createElement('button');
    neg.className = 'chip'; neg.type = 'button'; neg.textContent = 'não';
    neg.setAttribute('aria-pressed', String(!!st.polarity));
    neg.onclick = function () { st.polarity = st.polarity ? 0 : 1; modRow(); paint(); };
    host.appendChild(neg);
    [[1, 'rápido'], [2, 'urgente']].forEach(function (u) {
      var b = document.createElement('button');
      b.className = 'chip'; b.type = 'button'; b.textContent = u[1];
      b.setAttribute('aria-pressed', String(st.urgency === u[0]));
      b.onclick = function () {
        st.urgency = (st.urgency === u[0]) ? 0 : u[0]; modRow(); paint();
      };
      host.appendChild(b);
    });
  }

  function palette() {
    var host = $('palette');
    host.innerHTML = '';
    [1, 2, 4, 3, 5, 6].forEach(function (role) {
      var list = byRole[role] || [];
      var g = document.createElement('div'); g.className = 'group';
      var head = document.createElement('div'); head.className = 'grouphead';
      var lab = document.createElement('p'); lab.className = 'eyebrow';
      lab.textContent = ROLE_LABEL[role] + ' · ' + list.length;
      head.appendChild(lab);
      if (st.slots[role] !== undefined && st.slots[role] !== null) {
        var clr = document.createElement('button');
        clr.className = 'clear'; clr.type = 'button'; clr.textContent = 'limpar';
        clr.onclick = function () { delete st.slots[role]; palette(); paint(); };
        head.appendChild(clr);
      }
      g.appendChild(head);
      var words = document.createElement('div'); words.className = 'words';
      list.forEach(function (s) {
        var form = s.render[st.send];
        if (!form) return;
        var b = document.createElement('button');
        b.className = 'chip'; b.type = 'button'; b.textContent = form;
        b.title = s.id + ' · ' + s.gloss + ' · símbolo ' + s.sym;
        b.setAttribute('aria-pressed', String(st.slots[role] === s.sym));
        b.onclick = function () {
          if (st.slots[role] === s.sym) delete st.slots[role];
          else st.slots[role] = s.sym;
          palette(); paint();
        };
        words.appendChild(b);
      });
      g.appendChild(words);
      host.appendChild(g);
    });
  }

  /* ------------------------------------------------------------- pintura */
  var byteCells = [];
  (function () {
    var grid = $('byteGrid');
    for (var i = 0; i < 24; i++) {
      var d = document.createElement('div'); d.className = 'by';
      d.innerHTML = '<span class="hx">00</span><span class="tick"></span>';
      grid.appendChild(d); byteCells.push(d);
    }
  }());

  var currentFrame = null;

  function paint() {
    var h = buildHir();
    var valid = H.hirValidate(h) === 'OK';
    var sendText = valid ? H.render(pack, h, st.send) : null;
    var recvText = valid ? H.render(pack, h, st.recv) : null;
    $('sendTag').textContent = st.send;
    $('recvTag').textContent = st.recv;
    var recvLang = pack.byTag[st.recv];
    $('recvSaid').setAttribute('dir', recvLang.dir || 'ltr');

    /* o compilador julga a propria frase: se nao fecha, a interface diz */
    var back = sendText ? H.compile(pack, sendText, st.send) : { status: 'EMPTY' };
    var sane = back.status === H.OK && back.hir.key() === h.key();

    $('sendSaid').textContent = sendText || '—';
    $('recvSaid').textContent = recvText || '—';

    var gloss = [];
    Object.keys(st.slots).forEach(function (r) {
      var s = pack.symById[st.slots[r]];
      if (s) gloss.push(ROLE_LABEL[r] + ': ' + s.gloss);
    });
    $('recvGloss').textContent = gloss.join(' · ');

    var rs = $('recvStatus');
    if (!valid) {
      rs.className = 'status no';
      rs.innerHTML = '<span class="tag">INCOMPLETO</span>' +
        '<span class="why">' + missingHint(h) + '</span>';
      wipeBytes(); currentFrame = null; drawLadder(null); drawSeal(null);
      $('kDigest').textContent = '—'; $('kOp').textContent = '—';
      $('kSlots').textContent = '—'; $('kFrame').textContent = '—';
      return;
    }
    if (!sane) {
      rs.className = 'status no';
      rs.innerHTML = '<span class="tag">' + back.status + '</span>' +
        '<span class="why">a volta não fechou — este significado não sobrevive à própria frase</span>';
    } else {
      var rback = H.compile(pack, recvText, st.recv);
      var okBoth = rback.status === H.OK && rback.hir.key() === h.key();
      rs.className = 'status ' + (okBoth ? 'ok' : 'no');
      rs.innerHTML = '<span class="tag">' + (okBoth ? 'IDA E VOLTA OK' : rback.status) +
        '</span><span class="why">' + (okBoth
          ? 'recompilado em ' + st.recv + ' → os mesmos 24 bytes'
          : 'divergiu no idioma de quem recebe') + '</span>';
    }

    var wire = h.wire();
    var roleOf = {};
    h.slots.forEach(function (s, i) { roleOf[3 + i * 2] = s[0]; roleOf[4 + i * 2] = s[0]; });
    for (var i = 0; i < 24; i++) {
      var cell = byteCells[i];
      cell.querySelector('.hx').textContent =
        (wire[i] < 16 ? '0' : '') + wire[i].toString(16);
      var tick = i === 0 ? 'v/op' : i === 1 ? 'flag' : i === 2 ? 'n' :
                 (roleOf[i] ? ROLE_LABEL[roleOf[i]].slice(0, 4) : '');
      cell.querySelector('.tick').textContent = tick;
      cell.className = 'by' + ((i < 3 || roleOf[i]) ? ' hot' : '');
    }
    $('kDigest').textContent = H.hex(h.digest());
    $('kOp').textContent = H.OP_NAME[h.op] +
      (h.polarity ? ' · negado' : '') +
      (h.urgency ? ' · urgência ' + h.urgency : '') +
      (h.persistence ? ' · lembrar' : '');
    $('kSlots').textContent = h.slots.map(function (s) {
      var sym = pack.symById[s[1]];
      return ROLE_LABEL[s[0]] + '=' + (s[1] === 0 ? '?' : (sym ? sym.id : s[1]));
    }).join(' ');

    currentFrame = A.pack(wire, 0);
    $('kFrame').textContent = H.hex(currentFrame);
    drawLadder(currentFrame); drawSeal(currentFrame);
  }

  function missingHint(h) {
    if (h.op === H.OP_COMUNICAR) {
      if (h.get(H.ROLE_QUEM) === null) return 'avisar precisa de um destinatário: escolha em "quem"';
      if (h.get(H.ROLE_O_QUE) === null) return 'avisar precisa do que dizer: escolha em "o quê"';
    }
    if (h.op === H.OP_PERGUNTAR) return 'perguntar precisa de uma pergunta e de um assunto';
    if (!h.slots.length) return 'escolha pelo menos uma palavra';
    return 'esta combinação de papéis não forma um significado';
  }

  function wipeBytes() {
    byteCells.forEach(function (c) {
      c.querySelector('.hx').textContent = '00';
      c.querySelector('.tick').textContent = '';
      c.className = 'by';
    });
  }

  /* ---------------------------------------------------- escada de tons */
  function drawLadder(frame) {
    var cv = $('ladder'), g = cv.getContext('2d');
    var css = getComputedStyle(document.body);
    var ink = css.getPropertyValue('--ink-faint').trim();
    var send = css.getPropertyValue('--send').trim();
    var line = css.getPropertyValue('--line').trim();
    g.clearRect(0, 0, cv.width, cv.height);
    var cols = A.TOTAL_SYMS, cw = cv.width / cols, rh = cv.height / A.TONES;
    g.strokeStyle = line; g.lineWidth = 1;
    for (var t = 0; t <= A.TONES; t += 4) {
      g.beginPath(); g.moveTo(0, t * rh); g.lineTo(cv.width, t * rh); g.stroke();
    }
    if (!frame) {
      g.fillStyle = ink; g.font = '13px ' + css.fontFamily;
      g.fillText('escolha um significado', 12, cv.height / 2);
      return;
    }
    var stream = A.toneStream(frame);
    for (var i = 0; i < stream.length; i++) {
      var y = (A.TONES - 1 - stream[i]) * rh;
      g.fillStyle = (i < 4) ? ink : send;
      g.fillRect(i * cw + .5, y + 1, Math.max(1.5, cw - 1), rh - 2);
    }
  }

  function drawSeal(frame) {
    var cv = $('seal'), g = cv.getContext('2d');
    var css = getComputedStyle(document.body);
    var ground = css.getPropertyValue('--ground').trim();
    var ink = css.getPropertyValue('--ink').trim();
    var side = A.GLYPH_SIDE, s = cv.width / side;
    g.fillStyle = ground; g.fillRect(0, 0, cv.width, cv.height);
    if (!frame) return;
    var cells = A.glyphEncode(frame);
    g.fillStyle = ink;
    for (var r = 0; r < side; r++) {
      for (var c = 0; c < side; c++) {
        if (cells[r * side + c]) g.fillRect(c * s, r * s, s, s);
      }
    }
  }

  /* -------------------------------------------------------------- o ar */
  var ctx = null;
  function audio() {
    if (!ctx) {
      var C = window.AudioContext || window.webkitAudioContext;
      try { ctx = new C({ sampleRate: A.SR }); } catch (e) { ctx = new C(); }
    }
    if (ctx.state === 'suspended') ctx.resume();
    return ctx;
  }
  function say(cls, tag, why) {
    var el = $('airStatus');
    el.className = 'status ' + cls;
    el.innerHTML = '<span class="tag">' + tag + '</span><span class="why">' + why + '</span>';
  }

  $('play').onclick = function () {
    if (!currentFrame) { say('no', 'SEM SIGNIFICADO', 'escolha algo para dizer'); return; }
    var c = audio();
    var wave = A.soundEncode(currentFrame, c.sampleRate);
    var buf = c.createBuffer(1, wave.length, c.sampleRate);
    buf.copyToChannel(wave, 0);
    var src = c.createBufferSource();
    src.buffer = buf; src.connect(c.destination); src.start();
    say('ok', 'TOCANDO', '1,49 s · 70 símbolos · ' + A.FRAME +
        ' bytes a ' + c.sampleRate + ' Hz. Deixe outro aparelho ouvir.');
  };

  $('loop').onclick = function () {
    if (!currentFrame) { say('no', 'SEM SIGNIFICADO', 'escolha algo para dizer'); return; }
    var wave = A.soundEncode(currentFrame, A.SR);
    var dec = A.soundDecode(wave, A.SR);
    if (dec.status !== 'OK') { say('no', dec.status, 'o modem não achou o preâmbulo'); return; }
    var un = A.unpack(dec.frame);
    if (un.status !== 'OK') { say('no', un.status, 'o quadro não fechou'); return; }
    var h = H.hirFromWire(un.payload);
    say('ok', 'VOLTA COMPLETA',
        'som → ' + A.FRAME + ' bytes → 24 bytes → "' +
        H.render(pack, h, st.recv) + '" · ' + un.corrected + ' byte(s) corrigido(s)');
  };

  var listening = false;
  $('listen').onclick = function () {
    if (listening) return;
    if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
      say('no', 'SEM MICROFONE', 'este navegador não expõe o microfone aqui — use "provar aqui"');
      return;
    }
    listening = true;
    $('listen').disabled = true;
    say('ok', 'OUVINDO', 'toque o som no outro aparelho — 5 s');
    navigator.mediaDevices.getUserMedia({
      audio: { echoCancellation: false, noiseSuppression: false, autoGainControl: false }
    }).then(function (stream) {
      var c = audio();
      var src = c.createMediaStreamSource(stream);
      var proc = c.createScriptProcessor(4096, 1, 1);
      var chunks = [], total = 0, want = Math.ceil(c.sampleRate * 5);
      proc.onaudioprocess = function (ev) {
        var d = ev.inputBuffer.getChannelData(0);
        chunks.push(new Float32Array(d)); total += d.length;
        if (total >= want) finish();
      };
      src.connect(proc); proc.connect(c.destination);
      var done = false;
      function finish() {
        if (done) return;
        done = true;
        proc.onaudioprocess = null;
        try { src.disconnect(); proc.disconnect(); } catch (e) {}
        stream.getTracks().forEach(function (t) { t.stop(); });
        listening = false; $('listen').disabled = false;
        var all = new Float32Array(total), at = 0;
        chunks.forEach(function (ch) { all.set(ch, at); at += ch.length; });
        var dec = A.soundDecode(all, c.sampleRate);
        if (dec.status !== 'OK') {
          say('no', 'NÃO OUVI NADA', 'nenhum preâmbulo HERUS nos 5 s — aproxime e tente de novo');
          return;
        }
        var un = A.unpack(dec.frame);
        if (un.status !== 'OK') {
          say('no', un.status, 'ouvi um quadro e ele chegou quebrado além do corrigível — repita');
          return;
        }
        var h = H.hirFromWire(un.payload);
        st.op = h.op; st.polarity = h.polarity;
        st.urgency = (h.op === H.OP_SOCORRO) ? 0 : h.urgency;
        st.slots = {};
        h.slots.forEach(function (s) { if (s[1] !== 0) st.slots[s[0]] = s[1]; });
        opRow(); modRow(); palette(); paint();
        say('ok', 'RECEBIDO DO AR',
            '"' + H.render(pack, h, st.recv) + '" · ' + un.corrected +
            ' byte(s) corrigido(s) pelo Reed-Solomon');
        buzz(h);
      }
      setTimeout(finish, 6000);
    }).catch(function (e) {
      listening = false; $('listen').disabled = false;
      say('no', 'MICROFONE NEGADO', e.message + ' — use "provar aqui"');
    });
  };

  function buzz(h) {
    if (!navigator.vibrate) return;
    var urg = h.urgency;
    var pat = urg >= 3 ? [90, 60, 90, 60, 90, 60, 240]
            : urg === 2 ? [120, 70, 120]
            : urg === 1 ? [70, 60, 70]
            : [180];
    try { navigator.vibrate(pat); } catch (e) {}
  }
  $('vibrate').onclick = function () { buzz(buildHir()); };

  /* ------------------------------------------------------------- recusas */
  var REFUSALS = [
    ['pt', 'avisa joao pizza',
     'a palavra não existe no vocabulário fechado. Não é erro: é um pedido de capacidade, e o quadro diz qual lexema faltou.'],
    ['pt', 'avisa a equipe a senha',
     'classe protegida nomeada na entrada. Recusa antes de qualquer significado ser construído, em qualquer posição da frase.'],
    ['pt', 'guarda isso automaticamente',
     'a frase tentou se conceder autoridade. Nenhuma sentença pode remover a confirmação humana.'],
    ['pt', 'avisa joao maria cheguei',
     'dois destinatários em conflito. Duas leituras válidas que se contradizem: recusa as duas em vez de escolher uma.'],
    ['pt', 'cheguei',
     'palavras válidas que não formam significado. Avisar exige destinatário, e a recusa diz qual papel falta.'],
    ['pt', 'avisa joao em casaco',
     '"casa" nunca casa dentro de "casaco". A fronteira de palavra é regra, não heurística.'],
    ['de', 'abbrechen komme spater',
     '"komme spater" é ATRASADO e também "komme"+"spater". Ambiguidade genuína do alemão: nenhuma leitura contém a outra.'],
    ['pt', 'avisa joao to aqui to aqui to aqui to aqui to aqui to aqui',
     'sessenta e quatro leituras, todas concordando — e ainda assim recusa: dentro do orçamento não dá para PROVAR unicidade.']
  ];
  (function () {
    var host = $('refusals');
    REFUSALS.forEach(function (row) {
      var r = H.compile(pack, row[1], row[0]);
      var d = document.createElement('div'); d.className = 'rcard';
      d.innerHTML = '<span class="tag">' + r.status +
        (r.gapText ? ' · «' + r.gapText + '»' : '') + '</span>' +
        '<p class="said">' + row[0] + ' · ' + row[1] + '</p>' +
        '<p class="why">' + row[2] + '</p>';
      host.appendChild(d);
    });
  }());

  /* ---------------------------------------------------------- texto livre */
  $('free').oninput = function () {
    var v = this.value, el = $('freeStatus');
    if (!v.trim()) { el.className = 'status'; el.innerHTML = ''; return; }
    var r = H.compile(pack, v, st.send);
    if (r.status === H.OK) {
      st.op = r.hir.op; st.polarity = r.hir.polarity;
      st.urgency = (r.hir.op === H.OP_SOCORRO) ? 0 : r.hir.urgency;
      st.slots = {};
      r.hir.slots.forEach(function (s) { if (s[1] !== 0) st.slots[s[0]] = s[1]; });
      opRow(); modRow(); palette(); paint();
      el.className = 'status ok';
      el.innerHTML = '<span class="tag">OK</span><span class="why">' +
        H.hex(r.hir.digest()) + ' · ' + r.readings + ' leitura(s) com significado</span>';
    } else {
      el.className = 'status no';
      el.innerHTML = '<span class="tag">' + r.status +
        (r.gapText ? ' · «' + r.gapText + '»' : '') + '</span>' +
        '<span class="why">' + explain(r) + '</span>';
    }
  };
  function explain(r) {
    switch (r.status) {
      case 'GAP': return 'este lexema não está no vocabulário fechado — lacuna tipada, não aproximação';
      case 'SENSITIVE': return 'classe protegida: recusado antes de construir significado';
      case 'AUTHORITY': return 'a frase tentou se conceder autoridade';
      case 'AMBIGUOUS': return 'duas leituras válidas que se contradizem — recusa as duas';
      case 'INCOMPLETE': return 'palavras válidas que não formam um significado completo';
      case 'BYTE': return 'byte fora da codificação aceita';
      case 'OVERFLOW': return 'passou do orçamento de texto ou de leituras';
      default: return 'recusa tipada';
    }
  }

  /* ------------------------------------------------------------- o tema */
  $('theme').onclick = function () {
    var cur = document.documentElement.getAttribute('data-theme');
    var next = cur === 'dark' ? 'light' : cur === 'light' ? null : 'dark';
    if (next) document.documentElement.setAttribute('data-theme', next);
    else document.documentElement.removeAttribute('data-theme');
    drawLadder(currentFrame); drawSeal(currentFrame);
  };

  /* ------------------------------------------------------------- arranque */
  langRow($('sendLangs'), 'send');
  langRow($('recvLangs'), 'recv');
  opRow(); modRow();
  /* abre num estado de trabalho de verdade, nao numa casca vazia */
  st.slots[H.ROLE_QUEM] = pack.raw.syms.filter(function (s) { return s.id === 'PER.MARIA'; })[0].sym;
  st.slots[H.ROLE_O_QUE] = pack.raw.syms.filter(function (s) { return s.id === 'EV.CHEGUEI'; })[0].sym;
  st.slots[H.ROLE_ONDE] = pack.raw.syms.filter(function (s) { return s.id === 'LOC.CASA'; })[0].sym;
  st.slots[H.ROLE_QUANDO] = pack.raw.syms.filter(function (s) { return s.id === 'TIME.AGORA'; })[0].sym;
  palette(); paint();
  say('ok', 'PRONTO', 'toque para ouvir o significado, ou "provar aqui" para ver a volta completa sem microfone');
}());
