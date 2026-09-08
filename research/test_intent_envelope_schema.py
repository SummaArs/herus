import json
from pathlib import Path

ROOT = Path(__file__).parents[1]
schema = json.loads((ROOT / "research/intent_envelope_v0.schema.json").read_text(encoding="utf-8"))

assert schema["$id"].endswith("/intent-envelope/v0")
assert schema["properties"]["schema"]["const"] == "herus-intent-envelope-v0"
expected = {
    "schema", "identity", "domain", "proposal", "purpose", "authority",
    "context", "conditions", "effects", "delegation", "evidence",
}
assert set(schema["required"]) == expected
assert schema["additionalProperties"] is False
assert schema["properties"]["delegation"]["properties"]["max_depth"]["maximum"] == 16
assert schema["properties"]["proposal"]["properties"]["arguments"]["maxItems"] == 16
assert schema["properties"]["context"]["properties"]["expires_at"]["minimum"] == 1
print("INTENT ENVELOPE SCHEMA: PASS")
