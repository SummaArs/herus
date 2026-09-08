from pathlib import Path

SOURCE = Path(__file__).parents[1] / "firmware/port/esp32s3/main/app_main.c"
source = SOURCE.read_text(encoding="utf-8")
start = source.index("static int cmd_send")
end = source.index("static int cmd_listen", start)
body = source[start:end]

for forbidden in ("link_send(", "xQueueSend(", "sx1262_tx(", "babel_compile(", "ae_pack("):
    assert forbidden not in body, f"raw console send bypass reintroduced: {forbidden}"
assert "send is disabled" in body
assert "Babel" in body and "HSCA" in body and "Aether" in body
print("MEANING TRANSPORT BYPASS: PASS")
