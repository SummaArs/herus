#!/usr/bin/env python3
"""build_web.py — monta web/herus-aberto.html a partir das pecas provadas.

A pagina NAO e escrita a mao com os dados dentro. Ela e GERADA a partir do
pacote que passou no portao do Loom e dos vetores que as sondas C exportaram.
Isso fecha o unico furo que sobraria: uma pagina editada a mao poderia divergir
do firmware sem ninguem notar. Aqui, se o pacote muda, a pagina muda com ele —
e se algum vetor divergir, ela diz em vermelho na propria carga.
"""
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
WEB = ROOT / "web"


def main() -> int:
    tpl = (WEB / "template.html").read_text(encoding="utf-8")
    herus = (WEB / "herus.js").read_text(encoding="utf-8")
    app = (WEB / "app.js").read_text(encoding="utf-8")
    pack = (ROOT / "research" / "loom" / "core.compiled.json").read_text(encoding="utf-8")
    vec = (ROOT / "research" / "loom" / "web_vectors.json").read_text(encoding="utf-8")

    for blob, name in ((pack, "pacote"), (vec, "vetores")):
        json.loads(blob)                      # nao publica JSON quebrado
        if "</script" in blob:
            print(f"build_web: FAIL {name} contem '</script'")
            return 1

    out = (tpl.replace("/*__HERUS_JS__*/", herus)
              .replace("/*__APP_JS__*/", app)
              .replace("/*__PACK__*/", pack)
              .replace("/*__VEC__*/", vec))
    # Procura o PLACEHOLDER, nao o nome: window.__PACK__ continua no codigo de
    # proposito, e e a variavel que o app le.
    for marker in ("/*__HERUS_JS__*/", "/*__APP_JS__*/", "/*__PACK__*/", "/*__VEC__*/"):
        if marker in out:
            print(f"build_web: FAIL marcador {marker} nao foi substituido")
            return 1
    dest = WEB / "herus-aberto.html"
    dest.write_text(out, encoding="utf-8")
    print(f"build_web: OK web/herus-aberto.html ({dest.stat().st_size:,} bytes) "
          f"= template + herus.js + app.js + pacote + "
          f"{len(json.loads(vec)['babel']) + len(json.loads(vec)['aether'])} vetores"
          .replace(",", "."))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
