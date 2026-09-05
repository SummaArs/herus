import unittest
from pathlib import Path


ROOT = Path(__file__).parents[1]


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        raise AssertionError(f"missing function: {signature}")
    brace = source.find("{", start)
    if brace < 0:
        raise AssertionError(f"missing body: {signature}")
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace:index + 1]
    raise AssertionError(f"unterminated body: {signature}")


class CriticalTransportBypassTests(unittest.TestCase):
    def test_raw_console_send_has_no_transport_effect(self) -> None:
        source = (ROOT / "firmware/port/esp32s3/main/app_main.c").read_text(encoding="utf-8")
        body = function_body(source, "static int cmd_send(")
        self.assertNotIn("link_send(", body)
        self.assertNotIn("xQueueSend(", body)
        self.assertIn("raw console send is disabled", body)
        self.assertIn("return 1;", body)

    def test_raw_interaction_rig_handoff_remains_fail_closed(self) -> None:
        source = (ROOT / "firmware/core/interaction_rig.c").read_text(encoding="utf-8")
        body = function_body(source, "int interaction_rig_take_send(")
        self.assertNotIn("interaction_take_send(", body)
        self.assertIn("return INTERACTION_E_UNTRUSTED;", body)


if __name__ == "__main__":
    unittest.main()
