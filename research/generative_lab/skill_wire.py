"""Tiny deterministic wire format for verified, effect-free arithmetic Skills.

This is a host-only preparation artifact. The wire format carries a bounded DSL
program, not Python code and not execution authority.
"""
from __future__ import annotations

from .skills import Skill, SkillState


MAGIC = b"HSK1"
MAX_PROGRAM = 12
MAX_WIRE_BYTES = 4 + 1 + MAX_PROGRAM

_TOKEN_TO_ID = {
    "INPUT": 1,
    "ADD": 2,
    "MUL": 3,
    "NEG": 4,
    "CONST:-2": 10,
    "CONST:-1": 11,
    "CONST:0": 12,
    "CONST:1": 13,
    "CONST:2": 14,
    "CONST:3": 15,
}
_ID_TO_TOKEN = {value: key for key, value in _TOKEN_TO_ID.items()}


def encode_verified_skill(skill: Skill) -> bytes:
    if skill.state != SkillState.VERIFIED:
        raise ValueError("only VERIFIED skills may be encoded")
    if skill.allowed_effects:
        raise ValueError("effectful skills cannot be encoded")
    if len(skill.program) > MAX_PROGRAM:
        raise ValueError("program exceeds embedded limit")
    try:
        opcodes = bytes(_TOKEN_TO_ID[token] for token in skill.program)
    except KeyError as exc:
        raise ValueError("program contains an opcode outside the embedded DSL") from exc
    wire = MAGIC + bytes([len(opcodes)]) + opcodes
    if len(wire) > MAX_WIRE_BYTES:
        raise ValueError("wire representation exceeds embedded limit")
    return wire


def decode_skill_program(wire: bytes) -> tuple[str, ...]:
    if not isinstance(wire, bytes) or len(wire) < 5 or len(wire) > MAX_WIRE_BYTES:
        raise ValueError("invalid wire length")
    if wire[:4] != MAGIC:
        raise ValueError("invalid wire magic")
    count = wire[4]
    opcodes = wire[5:]
    if count == 0 or count != len(opcodes) or count > MAX_PROGRAM:
        raise ValueError("invalid opcode count")
    try:
        return tuple(_ID_TO_TOKEN[opcode] for opcode in opcodes)
    except KeyError as exc:
        raise ValueError("unknown embedded opcode") from exc
