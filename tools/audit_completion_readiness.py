#!/usr/bin/env python3
"""Recheck existing completion evidence without loading a PE or opening a device.

This is a readiness audit, not the final six-criterion completion certificate.
It deliberately preserves outstanding runtime gates and bounded DLL evidence.
"""
import datetime
import hashlib
import re
from current_production_regression import current_regression
from pe import PE
from record_live_production_verification import ROOT, digest, read, write
from validate_exports import export_contract


def source_export_definitions():
    name = 'src/squall/runtime/SoundEngine/Squall.cpp'
    source = (ROOT / name).read_text(encoding='utf-8')
    # Ignore braces in comments and literals, retaining offsets and line numbers.
    literals = r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    masked = re.sub(literals, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]),
                    source, flags=re.S)
    signature = r'DLL_API\s+\w+(?:\s*\*)?\s+(SQUALL_\w+)\s*\([^;{}]*\)\s*(?:#endif\s*)?\{'
    rows = {}
    for match in re.finditer(signature, masked):
        start = match.end() - 1
        depth = 1
        end = start + 1
        while depth and end < len(masked):
            depth += (masked[end] == '{') - (masked[end] == '}')
            end += 1
        assert depth == 0 and match[1] not in rows, match[1]
        assert masked[start + 1:end - 1].strip(), match[1]
        rows[match[1]] = dict(source=name, line=source.count('\n', 0, match.start()) + 1,
            body_sha256=hashlib.sha256(source[start:end].encode('utf-8')).hexdigest())
    assert len(rows) == 97
    return rows


def audit_evidence():
    supporting = {}

    def report(name):
        path = ROOT / name
        supporting[name] = digest(path)
        return read(path)

    def references(rows, parent=''):
        for name, sha in rows.items():
            path = ROOT / parent / name
            assert digest(path) == sha, str(path)

    inventory = report('analysis/inventory.json')
    originals = ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    assert len(inventory['files']) == 17
    for row in inventory['files']:
        assert digest(originals / row['path']) == row['sha256'], row['path']
    original_by_name = {row['path']: row for row in inventory['files']}

    build = report('analysis/production-source-only-build-v4.json')
    assert build['configuration'] == dict(BUILD_TESTING=False, BUILD_SQUALL_CANDIDATE=True,
                                         BUILD_SQUALL_REFERENCE_LAYOUT=False)
    stage = build['staged_source_manifest']
    stage_root = ROOT / stage['directory']
    assert digest(stage_root / 'source-stage.json') == build['staged_source_manifest_sha256']
    assert not stage['original_game_binaries_present'] and not stage['original_reference_source_tree_exposed']
    references(stage['copied_source_files'], stage['directory'])
    current_staged = {name: sha for name, sha in stage['copied_source_files'].items()
                      if name.startswith(('src/', 'tools/cmake/')) or name == 'CMakeLists.txt'}
    references(current_staged)
    references(build['logs'])
    references(build['reports'])
    references(build['isolated_uninstaller_effects_transferred_by_exact_binary_sha256'])

    definitions = source_export_definitions()
    dll_exports = {}
    assert len(build['artifacts']) == 4
    for name, artifact in build['artifacts'].items():
        path = ROOT / artifact['path']
        assert digest(path) == artifact['sha256'] and path.stat().st_size == artifact['bytes']
        assert artifact['sha256'] != original_by_name[name]['sha256'], name
        pe = PE(path)
        assert pe.bits == 32 and pe.machine == 0x14c
        if not name.endswith('.dll'):
            continue
        exports = pe.exports()
        assert pe.characteristics & 0x2000
        assert export_contract(exports) == export_contract(original_by_name[name]['pe']['exports'])
        expected_count = 97 if name == 'Squall.dll' else 2
        assert len(exports) == expected_count
        for row in exports:
            assert not row.get('forwarder') and row['rva'] > 0
            pe.offset(row['rva'], 1)  # A real file-backed entry, not an empty EAT slot.
        if name == 'Squall.dll':
            assert {n for row in exports for n in row['names']} == set(definitions)
        dll_exports[name] = dict(count=len(exports), exact_original_contract=True,
            forwarded_exports=0, sha256=artifact['sha256'])

    current_name, current = current_regression()
    report(current_name)
    jng = report('analysis/jng-loader-regression.json')
    references(jng['source_files'])
    assert jng['game_images_per_build'] == 416 and jng['actual_error_API_process_pairs_per_build'] == 40
    export_audit = report('analysis/squall-export-audit.json')
    assert export_audit['native_signature_semantics_verified_count'] == 97
    assert export_audit['rebuilt_sha256'] == digest(ROOT / 'build/windows/Release/Squall.dll')
    # Bind every indexed source entry to its independent native argument/return evidence.
    for row in export_audit['exports']:
        assert row['name'] in definitions and row['native_signature_fully_verified']
        evidence = row['native_signature_semantic_evidence']
        native = report(evidence['report'])
        assert native and not evidence['whole_behavior_verified']
        definitions[row['name']]['native_signature_report'] = evidence['report']
        definitions[row['name']]['native_address'] = row['original_address']

    uninstall_hash = build['artifacts']['Uninstall.exe']['sha256']
    silent = report('analysis/uninstaller-runtime-validation.json')
    ui = report('analysis/uninstaller-ui-validation.json')
    bootstrap = report('analysis/uninstaller-bootstrap-validation.json')
    assert ui['source_uninstaller_sha256'] == bootstrap['source_uninstaller_sha256'] == uninstall_hash
    assert silent['import_audits']['source']['sha256'] == uninstall_hash
    assert silent['isolated_silent_removal_effects_verified'] and silent['uninstall_processes'] == 20
    assert ui['uninstall_processes'] == 16 and bootstrap['actual_uninstaller_processes'] == 32
    assert bootstrap['actual_reboot_not_tested'] and not ui['pixel_equality_verified']

    contracts = report('analysis/contract-streak-forty-regression.json')
    references(contracts['reports'], 'analysis')
    assert contracts['final_streak'] == 40 and contracts['four_cross_restore_bytes'] == [17369] * 4
    awards = report('analysis/current-awards-runtime-regression.json')
    assert awards['exact_panel_roi_rgb_pixels'] == 598500 and awards['saved_owned_and_shown_flags'] == 42
    assert digest(ROOT / awards['runtime_report']['path']) == awards['runtime_report']['sha256']
    warnings = report('analysis/tutorial-warning-first-pair-regression.json')
    assert set(warnings['verified_steps']) == {40, 42}
    callers = report('analysis/tutorial-native-callers-audit.json')
    coverage = report('analysis/current-tutorial-runtime-coverage.json')
    report('analysis/tutorial-warning-native-warehouse-observation.json')
    report('analysis/native-game-loopback-audio-observation.json')
    startup = report('analysis/monitored-source-startup-validation.json')
    assert not startup['revised_scene_tree_runtime_validated'] and not startup['incident_resolved']
    decision = report('analysis/native-tracker-fidelity-decision.json')
    assert decision['status'] == 'user_accepted_documented_sufficient_tracker_allocation_exception'
    v = report('analysis/verification.json')

    remaining = [
        'Complete a short independently monitored farm, physical input and scene observer probe with verified game-window focus.',
        'Complete source warning41 against retained completed native evidence and actual warning44 in both programs,including pixels,ledgers and normal exits.',
        'Verify nonzero process-only Windows audio output from the actual source game;native-only capture and exact decoded PCM are separate evidence.',
        'Assess window responsiveness during bounded runtime; DWM counters alone do not verify physical display output.',
        'Finish the six-criterion requirement audit and update documentation after those actual gates pass;do not equate97 implemented exports with all-path equivalence.'
    ]
    result = dict(status='current_source_build_and_existing_evidence_rechecked_original_goal_incomplete',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        runtime_programs_or_devices_opened=False, new_build_or_full_ctest_run=False,
        current_source_files_matching_staged_production=len(current_staged),
        staged_files_rehashed=len(stage['copied_source_files']), artifacts=build['artifacts'], dll_exports=dll_exports,
        squall_source_export_definitions=definitions,
        source_index_scope='Nonempty source bodies,exact exported names and native signature evidence only;no generic body completeness or all-path behavior claim.',
        existing_full_regression=dict(report=current_name,passed=current['tests_passed'],failed=current['tests_failed'],
                                      current_production_source_files=len(current['production_source_files'])),
        isolated_uninstaller_processes=dict(silent=20,ui=16,bootstrap=32,exact_current_source_artifact=True,
                                           actual_host_reboot=False,pixel_identity_claimed=False),
        remaining_actual_gates=remaining,supporting_reports=supporting,
        criteria=[dict(index=1,scope='Four real source-only Win32 production targets',evidence_rechecked=True),
            dict(index=2,scope='Actual complete game systems,graphics,input and Windows audio',evidence_rechecked=True,remaining_actual_gates=[0,1,2,3]),
            dict(index=3,scope='Original comparison,whole bidirectional saves and documented deviations',evidence_rechecked=True,remaining_actual_gates=[1,3]),
            dict(index=4,scope='Both DLL interfaces and all exported source bodies',evidence_rechecked=True,all_path_equivalence_claimed=False),
            dict(index=5,scope='Buildable uninstaller and bounded isolated effects',evidence_rechecked=True),
            dict(index=6,scope='Editable source,reproducible build and final current documentation',evidence_rechecked=True,remaining_actual_gates=[4])],
        original_files_unchanged=17,production_changed=False,whole_game_complete=False)
    return result


def main():
    result = audit_evidence()
    write(ROOT / 'analysis/original-goal-readiness-audit.json', result)
    print('Rechecked four PE32 targets, DLL exports and isolated uninstall evidence.')


if __name__ == '__main__':
    main()
