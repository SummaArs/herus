from __future__ import annotations

import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "firmware" / "core" / "semantic_compiler.c"
TEST = ROOT / "firmware" / "core" / "test_semantic_compiler_language_ood.c"
SOURCES = [
    "firmware/core/hv.c",
    "firmware/core/resonator.c",
    "firmware/core/resonator_bridge.c",
    "firmware/core/symbol_registry.c",
    "firmware/core/symbolic_reasoner.c",
    "firmware/core/symbolic_planner.c",
    "firmware/core/symbolic_dialogue.c",
]

MUTATIONS = [
    {
        "name": "locative-alias-bypass",
        "old": "               word_eq(&tokens[verb], \"fica\")) {\n",
        "new": "               0) { /* REDTEAM */\n",
        "why": "the controlled locative paraphrase must preserve its canonical relation",
    },
    {
        "name": "interposed-question-bypass",
        "old": "        if (count >= 6u && word_any(&tokens[2], \"e\", \"é\") &&\n            word_eq(&tokens[3], \"que\")) {\n",
        "new": "        if (0) { /* REDTEAM */\n",
        "why": "interposed question words must not fall back to the wrong token positions",
    },
    {
        "name": "quantifier-bypass",
        "old": "    if (word_any(token, \"todos\", \"todas\") ||\n        word_any(token, \"todo\", \"toda\") ||\n        word_eq(token, \"qualquer\"))\n        return SC_E_UNSUPPORTED;\n",
        "new": "    if (0) return SC_E_UNSUPPORTED; /* REDTEAM */\n",
        "why": "quantified entities must not become concrete hashed symbols",
    },
    {
        "name": "negation-bypass",
        "old": "    if (word_any(&tokens[verb], \"nao\", \"não\")) {\n",
        "new": "    if (0) { /* REDTEAM */\n",
        "why": "negation must remain explicit in the typed IR",
    },
    {
        "name": "question-mark-bypass",
        "old": "    if (question || (count >= 2u && word_eq(&tokens[0], \"o\") &&\n",
        "new": "    if (0 || (count >= 2u && word_eq(&tokens[0], \"o\") &&\n",
        "why": "yes/no punctuation must select a read-only query rather than a fact",
    },
]


def main() -> int:
    original = SOURCE.read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory(prefix="herus-language-ood-redteam-") as raw:
        work = pathlib.Path(raw)
        for spec in MUTATIONS:
            if original.count(spec["old"]) != 1:
                print(f"FAIL {spec['name']}: control text is not unique")
                continue
            mutated = work / f"{spec['name']}.c"
            binary = work / f"{spec['name']}.bin"
            mutated.write_text(original.replace(spec["old"], spec["new"], 1), encoding="utf-8")
            build = subprocess.run(
                [
                    "cc", "-O2", "-Wall", "-Wextra", "-Werror", "-std=c11",
                    "-Ifirmware/core", *SOURCES, str(mutated), str(TEST),
                    "-o", str(binary),
                ], cwd=ROOT, text=True, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, check=False,
            )
            if build.returncode != 0:
                print(f"FAIL {spec['name']}: mutant does not compile")
                print(build.stdout[-1600:])
                continue
            run = subprocess.run([str(binary)], cwd=ROOT, text=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 check=False)
            if run.returncode != 0:
                killed += 1
                print(f"PASS {spec['name']}: {spec['why']}")
            else:
                print(f"FAIL {spec['name']}: surviving mutant")
                print(run.stdout[-1600:])
    print(f"SEMANTIC LANGUAGE OOD REDTEAM: {killed}/{len(MUTATIONS)} critical mutants killed")
    return 0 if killed == len(MUTATIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
