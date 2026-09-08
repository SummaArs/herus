#!/usr/bin/env python3
"""Red team do Aether: tira uma barreira, exige que a suite perceba.

O Aether tem tres barreiras em serie e a ordem importa: Reed-Solomon conserta
ate dois bytes, recusa o que nao consegue consertar, e o CRC-32 e a ultima
palavra. Esta campanha desliga uma barreira por vez e cobra que test_aether.c
falhe. Barreira que pode ser desligada sem ninguem notar nao e barreira.
"""
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CORE = ROOT / "firmware" / "core"
SOURCES = ["aether.c", "test_aether.c"]

# CONTROLE REMOVIDO, NAO ESCONDIDO. Havia um decimo quarto mutante aqui,
# "RS: magnitude de erro nula deixa de ser recusada", e ele SOBREVIVEU. O
# diagnostico foi que o guard `if (e1 == 0 || e2 == 0)` era redundante: a
# reverificacao de sindromes logo abaixo recusa o mesmo caso. Em vez de
# inventar um teste para "matar" um guard que nao decide nada, o guard foi
# REMOVIDO do aether.c e o argumento ficou escrito la. Menos codigo e a
# resposta certa quando a campanha diz que uma linha nao decide nada.

MUTANTS = [
    ("RS: a paridade nao e anexada ao quadro", "aether.c",
     "    memcpy(out, data, AE_DATA);\n    rs_encode(data, out + AE_DATA);\n    return AE_OK;\n}\n\nae_status_t ae_unpack",
     "    memcpy(out, data, AE_DATA);\n    memset(out + AE_DATA, 0, AE_PARITY);\n    return AE_OK;\n}\n\nae_status_t ae_unpack"),

    ("RS: as sindromes ignoram os bytes de paridade", "aether.c",
     "        for (i = 0; i < AE_FRAME; i++)\n            acc = (uint8_t)(gf_mul(acc, GF_EXP[j]) ^ r[i]);",
     "        for (i = 0; i < AE_DATA; i++)\n            acc = (uint8_t)(gf_mul(acc, GF_EXP[j]) ^ r[i]);"),

    ("RS: o caminho de DOIS erros e desligado", "aether.c",
     "        if (det == 0u) return -1;",
     "        return -1;\n        if (det == 0u) return -1;"),

    ("RS: uma posicao de erro fora do bloco passa a ser aceita", "aether.c",
     "            if (p1 >= AE_FRAME || p2 >= AE_FRAME) return -1;",
     "            if (p1 >= 255u || p2 >= 255u) return -1;"),

    ("CRC: a ultima palavra e removida", "aether.c",
     "    if (crc != want) return AE_E_CRC;        /* a ultima palavra             */",
     "    if (0) return AE_E_CRC;"),

    ("quadro: o payload sai do quadro CRU em vez do corrigido", "aether.c",
     "    memcpy(out, work + AE_HDR, AE_PAYLOAD);",
     "    memcpy(out, frame + AE_HDR, AE_PAYLOAD);"),

    ("cabecalho: a versao deixa de ser verificada", "aether.c",
     "    if ((work[0] >> 4) != AE_VERSION) return AE_E_VERSION;",
     "    if (0) return AE_E_VERSION;"),

    ("cabecalho: o perfil deixa de ser verificado", "aether.c",
     "    if (((work[0] >> 2) & 3u) != AE_PROFILE_OPEN) return AE_E_PROFILE;",
     "    if (0) return AE_E_PROFILE;"),

    ("som: o decodificador usa outra raia base que o codificador", "aether.c",
     "        uint8_t hi = demod_symbol(data + (size_t)(i * 2u) * AE_SYM_LEN, &b1, &s1);",
     "        uint8_t hi = (uint8_t)((demod_symbol(data + (size_t)(i * 2u) * AE_SYM_LEN, &b1, &s1) + 1u) & 15u);"),

    ("som: o preambulo deixa de ser exigido", "aether.c",
     "                if (got != SYNC[i]) { ok = 0; break; }",
     "                if (got != SYNC[i]) { (void)got; }"),

    ("glifo: os cantos de orientacao deixam de ser verificados", "aether.c",
     "    if (turn == 4u) return AE_E_GLYPH;",
     "    if (turn == 4u) turn = 0u;"),

    ("glifo: as rotacoes deixam de ser tentadas", "aether.c",
     "        if (corners_ok(a)) break;\n        rot90(a, b);\n        memcpy(a, b, AE_GLYPH_CELLS);",
     "        if (1) break;"),
]


def build_and_run(work: Path) -> int:
    src = [str(work / f) for f in SOURCES]
    binary = work / "t_ae"
    cc = subprocess.run(["cc", "-Os", "-std=c11", "-I", str(work), *src,
                         "-o", str(binary), "-lm"], capture_output=True, text=True)
    if cc.returncode != 0:
        return 2                       # recusar compilar tambem e deteccao
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=300)
    if run.returncode != 0 or "FAIL" in run.stdout:
        return 1
    return 0


def stage(tmp: Path) -> Path:
    work = tmp / "core"
    work.mkdir(parents=True, exist_ok=True)
    for f in CORE.glob("*.h"):
        shutil.copy(f, work / f.name)
    for name in SOURCES:
        shutil.copy(CORE / name, work / name)
    return work


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        tmp = Path(td)
        if build_and_run(stage(tmp)) != 0:
            print("  FAIL  a suite base nao passa sem mutacao; nada abaixo significa nada")
            return 1
        print("  base: test_aether passa sem mutacao")

        killed, survived = 0, []
        for i, (name, filename, needle, replacement) in enumerate(MUTANTS):
            work = stage(tmp / f"m{i}")
            target = work / filename
            text = target.read_text()
            if needle not in text:
                print(f"  FAIL  mutacao obsoleta, a agulha nao existe mais: {name}")
                survived.append(name)
                continue
            target.write_text(text.replace(needle, replacement, 1))
            try:
                verdict = build_and_run(work)
            except subprocess.TimeoutExpired:
                verdict = 1            # travar tambem e deteccao
            if verdict != 0:
                killed += 1
            else:
                survived.append(name)
                print(f"  SOBREVIVEU  {name}")

        print(f"AETHER REDTEAM: {killed}/{len(MUTANTS)} mutantes detectados")
        if survived:
            print("  as barreiras a seguir podem ser desligadas sem ninguem notar:")
            for s in survived:
                print(f"    - {s}")
            return 1
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
