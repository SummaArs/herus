from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).parent
REQUIRED_CONDITIONS = {'CB_CONVENTIONAL_BASELINE', 'NS_NO_SYMBIONT', 'H_HERUS_CHECKED'}
REQUIRED_PRIMARY = {'task_error', 'time_or_effort', 'host_or_interface_dependence'}
REQUIRED_CONTROLS = {'informed_consent', 'withdrawal', 'independent_stop', 'data_minimization', 'retention_limit', 'accessibility_review', 'operator_intervention_log'}
REQUIRED_THRESHOLDS = {'max_error_penalty_pp', 'min_setup_effort_reduction_vs_cb', 'zero_unconsented_release', 'zero_external_execution'}
REQUIRED_POLICY = {'analysis_rule', 'prohibited_claims'}

def load_protocol():
    return json.loads((ROOT / 'utility_protocol_v1.json').read_text(encoding='utf-8'))

def validate_protocol(protocol):
    errors = []
    if protocol.get('schema') != 'herus-social-utility-v1': errors.append('schema_invalid')
    if protocol.get('status') != 'protocol_ready_no_human_data': errors.append('human_data_must_not_be_fabricated')
    if set(protocol.get('conditions', ())) != REQUIRED_CONDITIONS: errors.append('conditions_incomplete')
    if not REQUIRED_PRIMARY.issubset(set(protocol.get('primary_metrics', ()))): errors.append('primary_metrics_incomplete')
    controls = set(protocol.get('required_controls', ()))
    if controls != REQUIRED_CONTROLS: errors.append('controls_incomplete')
    thresholds = protocol.get('progression_thresholds', {})
    if set(thresholds) != REQUIRED_THRESHOLDS or not isinstance(thresholds.get('max_error_penalty_pp'), (int, float)) or not isinstance(thresholds.get('min_setup_effort_reduction_vs_cb'), (int, float)):
        errors.append('thresholds_incomplete')
    if not REQUIRED_POLICY.issubset(protocol): errors.append('analysis_policy_incomplete')
    if not isinstance(protocol.get('prohibited_claims'), list) or not protocol['prohibited_claims']:
        errors.append('prohibited_claims_missing')
    return tuple(errors)

def main():
    protocol = load_protocol(); errors = validate_protocol(protocol)
    return {'schema': protocol.get('schema'), 'status': 'DRAFT_PROTOCOL_NO_HUMAN_DATA' if not errors else 'BLOCKED', 'errors': errors, 'human_results': None}

if __name__ == '__main__': print(json.dumps(main(), indent=2, sort_keys=True))
