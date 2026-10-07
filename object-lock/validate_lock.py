"""Minimal structural validator for the proposed Object Lock."""
from pathlib import Path
import json

ROOT = Path(__file__).parent

def validate_schema(path=ROOT / 'schema_episode.json'):
    data = json.loads(path.read_text(encoding='utf-8'))
    required = set(data.get('required', []))
    expected = {'example_id','entity_id','timestamp','context','current_state','action','outcome','provenance','verifier'}
    return required == expected and data.get('additionalProperties') is False

if __name__ == '__main__':
    if not validate_schema():
        raise SystemExit('OBJECT_LOCK_SCHEMA_INVALID')
    print('OBJECT_LOCK_SCHEMA_VALID — execution remains blocked until protocol review')
