from __future__ import annotations

import pathlib
import subprocess
import sys
import zlib

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(ROOT / "tools"))
import babel_ref as B  # noqa: E402

AE_DATA = 29
AE_PARITY = 4
AE_FRAME = 33
AE_PAYLOAD = 24


def gf_tables():
    exp = [0] * 512
    log = [0] * 256
    x = 1
    for i in range(255):
        exp[i] = x
        log[x] = i
        x <<= 1
        if x & 0x100:
            x ^= 0x11D
    for i in range(255, 512):
        exp[i] = exp[i - 255]
    return exp, log


GF_EXP, GF_LOG = gf_tables()


def gf_mul(a: int, b: int) -> int:
    if not a or not b:
        return 0
    return GF_EXP[GF_LOG[a] + GF_LOG[b]]


def rs_encode(data: bytes) -> bytes:
    g = [0] * (AE_PARITY + 1)
    g[0] = 1
    for i in range(AE_PARITY):
        root = GF_EXP[i]
        for j in range(i + 1, 0, -1):
            g[j] = gf_mul(g[j - 1], 1) ^ gf_mul(g[j], root)
        g[0] = gf_mul(g[0], root)
    parity = [0] * AE_PARITY
    for value in data:
        feedback = value ^ parity[0]
        for j in range(AE_PARITY - 1):
            parity[j] = parity[j + 1] ^ gf_mul(feedback, g[AE_PARITY - 1 - j])
        parity[AE_PARITY - 1] = gf_mul(feedback, g[0])
    return bytes(parity)


def ae_pack(payload: bytes, seq: int) -> bytes:
    assert len(payload) == AE_PAYLOAD
    header = bytes([(1 << 4) | (seq & 3)])
    body = header + payload
    crc = zlib.crc32(body) & 0xFFFFFFFF
    data = body + crc.to_bytes(4, "big")
    assert len(data) == AE_DATA
    return data + rs_encode(data)


def run_probe(requests: list[str]) -> list[str]:
    probe = ROOT / "firmware" / "build" / "meaning_pipeline_probe"
    if not probe.exists():
        raise AssertionError("meaning_pipeline_probe ausente; rode make -C firmware meaning-pipeline")
    completed = subprocess.run(
        [str(probe)], input=("\n".join(requests) + "\n").encode("utf-8"),
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
    )
    if completed.returncode != 0:
        raise AssertionError(f"probe terminou com {completed.returncode}: {completed.stderr!r}")
    return completed.stdout.decode("utf-8").splitlines()


def main() -> int:
    pack = B.Pack.load(ROOT / "research" / "loom" / "core")
    vectors = [
        ("pt", "ja", 7, "emergencia estou aqui"),
        ("pt", "en", 2, "cheguei em casa avisa joao"),
        ("en", "pt", 5, "tell maria urgent not arrived at home now"),
    ]
    requests = [f"P\t{src}\t{dst}\t{seq}\t{text}" for src, dst, seq, text in vectors]
    got = run_probe(requests)
    failures = []
    for vector, line in zip(vectors, got):
        src, dst, seq, text = vector
        unit = B.compile(text, src, pack)
        if not unit.ok:
            failures.append(f"referencia recusou vetor {vector!r}: {unit}")
            continue
        expected_wire = unit.hir.wire().hex()
        expected_frame = ae_pack(unit.hir.wire(), seq).hex()
        expected_render = B.render(unit.hir, dst, pack)
        parts = line.split(" ", 3)
        if len(parts) != 4 or parts[0] != "OK":
            failures.append(f"{vector!r}: probe={line!r}")
            continue
        _, frame, wire, rendered = parts
        if frame != expected_frame:
            failures.append(f"{vector!r}: frame C/Python divergiu")
        if wire != expected_wire:
            failures.append(f"{vector!r}: HIR C/Python divergiu")
        if rendered != expected_render:
            failures.append(f"{vector!r}: render C/Python divergiu")

    if failures:
        print(f"MEANING INTEROP: FAIL {len(failures)}")
        for failure in failures:
            print(f"  {failure}")
        return 1
    print(f"MEANING INTEROP: PASS {len(vectors)} vetores C/Python, frame, HIR e render")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
