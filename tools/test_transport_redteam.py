#!/usr/bin/env python3
"""GAN red-team for haptic and radio transport adapters.

The campaign mutates production C sources and recompiles the existing host mocks.
No SPI, I2C, GPIO or RF hardware is opened by this tool.
"""
from __future__ import annotations

import os
import pathlib
import subprocess
import sys
import tempfile
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / "firmware"
CC = os.environ.get("CC", "cc")
FLAGS = ["-O2", "-Wall", "-Wextra", "-std=c11", "-DHV_LUT_POPCOUNT", "-Icore", "-Inet", "-Iport"]


@dataclass(frozen=True)
class Mutation:
    name: str
    source: str
    test_sources: tuple[str, ...]
    find: str
    replace: str
    extra_flags: tuple[str, ...] = ()


HAPTIC_SOURCES = (
    "core/haptic_language.c", "core/haptic_adapter.c",
    "core/test_haptic_adapter.c",
)
RADIO_SOURCES = ("test/test_radio.c", "port/sx1262.c", "net/region.c")
TARGET_SOURCES = (
    "core/haptic_language.c", "core/haptic_adapter.c",
    "port/esp32s3/haptic_drv2605l_esp32s3.c",
    "port/esp32s3/test_haptic_target.c",
)


def run_mutation(directory: pathlib.Path, mutation: Mutation) -> bool:
    original_path = ROOT / mutation.source
    original = original_path.read_text(encoding="utf-8")
    if original.count(mutation.find) != 1:
        print(f"  FAIL REDTEAM {mutation.name}: control text not unique")
        return False
    mutated_path = directory / pathlib.Path(mutation.source).name
    mutated_path.write_text(original.replace(mutation.find, mutation.replace),
                            encoding="utf-8")
    target_name = pathlib.Path(mutation.source).name
    sources = [str(mutated_path) if pathlib.Path(source).name == target_name else source
               for source in mutation.test_sources]
    binary = directory / f"{mutation.name}.bin"
    build = subprocess.run([CC, *FLAGS, *mutation.extra_flags, *sources,
                            "-o", str(binary)], cwd=FIRMWARE, text=True,
                           capture_output=True, check=False)
    if build.returncode != 0:
        print(f"  FAIL REDTEAM {mutation.name}: mutant did not compile")
        print(build.stderr)
        return False
    result = subprocess.run([str(binary)], cwd=FIRMWARE, text=True,
                            capture_output=True, check=False)
    if result.returncode == 0:
        print(f"  FAIL REDTEAM {mutation.name}: sabotage survived")
        return False
    print(f"  PASS REDTEAM {mutation.name}: sabotage was detected")
    return True


def main() -> int:
    mutations = (
        Mutation(
            "haptic-bus-fault-state",
            "firmware/core/haptic_adapter.c",
            HAPTIC_SOURCES,
            "    if (device->bus.write(device->bus.context, device->config.address7,\n                          reg, data, length) != 0) {\n        device->errors++;\n        device->state = HA_STATE_FAULT;\n        return HA_E_BUS;\n    }\n",
            "    if (device->bus.write(device->bus.context, device->config.address7,\n                          reg, data, length) != 0) {\n        device->errors++;\n        return HA_E_BUS; /* REDTEAM: fault state removed. */\n    }\n",
        ),
        Mutation(
            "haptic-busy-gate",
            "firmware/core/haptic_adapter.c",
            HAPTIC_SOURCES,
            "    if (device->state == HA_STATE_PLAYING) return HA_E_BUSY;\n",
            "    /* REDTEAM: concurrent playback gate removed. */\n",
        ),
        Mutation(
            "haptic-overtemperature-gate",
            "firmware/core/haptic_adapter.c",
            HAPTIC_SOURCES,
            "    if ((status & (HA_STATUS_DIAG_BIT | HA_STATUS_OVERTEMP_BIT |\n                   HA_STATUS_OVERCURRENT_BIT)) != 0u) {\n        device->state = HA_STATE_FAULT;\n        return HA_E_FAULT;\n    }\n",
            "    /* REDTEAM: device fault status ignored. */\n",
        ),
        Mutation(
            "radio-busy-discipline",
            "firmware/port/sx1262.c",
            RADIO_SOURCES,
            "    r->bus.wait_busy(r->bus.ctx);\n    return r->bus.xfer(r->bus.ctx, buf, NULL, n + 1) ? SX_E_BUS : SX_OK;\n",
            "    return r->bus.xfer(r->bus.ctx, buf, NULL, n + 1) ? SX_E_BUS : SX_OK; /* REDTEAM */\n",
        ),
        Mutation(
            "radio-packet-type-order-gate",
            "firmware/port/sx1262.c",
            RADIO_SOURCES,
            "    if (!r->packet_type_set) return SX_E_ARG;\n",
            "    /* REDTEAM: modulation can run without packet type. */\n",
        ),
        Mutation(
            "radio-undocumented-power-gate",
            "firmware/port/sx1262.c",
            RADIO_SOURCES,
            "        default: return SX_E_ARG;                /* refuse to invent a row */\n",
            "        default: break;                         /* REDTEAM */\n",
        ),
        Mutation(
            "radio-duty-stop-timer-gate",
            "firmware/port/sx1262.c",
            RADIO_SOURCES,
            "    uint8_t stop = 0x00;\n    if ((rc = cmd(r, OP_STOP_TIMER_ON_PREAMB, &stop, 1))) return rc;\n",
            "    /* REDTEAM: duty-cycle preamble timer guard removed. */\n",
        ),
        Mutation(
            "target-unwired-gate",
            "firmware/port/esp32s3/haptic_drv2605l_esp32s3.c",
            TARGET_SOURCES,
            "    if (!device || !BOARD_HAS_HAPTIC_I2C || !PIN_HAPTIC_ENABLE_VALID)\n        return HT_E_UNWIRED;\n",
            "    if (!device) return HT_E_UNWIRED; /* REDTEAM: unverified board gate removed. */\n",
            ("-Itest/idf_stub", "-Iport/esp32s3"),
        ),
    )

    print("\n== HERUS transport deterministic red-team campaign ==")
    passed = True
    with tempfile.TemporaryDirectory(prefix="herus-transport-redteam-") as raw:
        directory = pathlib.Path(raw)
        for mutation in mutations:
            passed = run_mutation(directory, mutation) and passed
    if not passed:
        print("TRANSPORT REDTEAM FAILED — at least one adapter mutant survived")
        return 1
    print("TRANSPORT REDTEAM: 8/8 critical adapter mutants killed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
