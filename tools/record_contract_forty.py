#!/usr/bin/env python3
"""Audit independent real40 checkpoints, full award pixels and four entire cross-restores."""
import argparse
import datetime
import json
from pathlib import Path
from PIL import Image, ImageChops
from audit_contract_streak_state import audit
from counter_save_audit import exact_restored_binary
from current_production_regression import current_regression
from record_live_production_verification import ROOT, digest, read, write
from validate_counter_supply_state import props


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--checkpoints', type=Path, required=True)
    parser.add_argument('--interop-report', type=Path, required=True)
    args = parser.parse_args()
    descriptor_path = args.checkpoints.resolve(strict=True)
    descriptor = read(descriptor_path)
    assert descriptor['status'] == 'verified_actual_contract_streak_checkpoint_profiles'
    assert descriptor['requested_target'] == 40 and not descriptor['profile_or_counter_fields_written']
    assert len(descriptor['profile_sets']) == 2
    branches, contexts, pictures, directories, sessions = [], [], [], [], []
    implementations = None
    for kind in ['native', 'source']:
        row = next(s for s in descriptor['profile_sets'] if s['kind'] == kind)
        path = ROOT / row['source_report']
        assert digest(path) == row['source_report_sha256']
        raw, audited = audit(path, allow_retained_failure=True)
        index = row['source_session_index']
        session = raw['sessions'][index]
        branch = next(b for b in audited if b['kind'] == kind)
        assert session['kind'] == kind
        assert ((session.get('status') == 'completed' and session['exit_code'] == 0) or
            (branch.get('completed_target_saved_after_driver_deadline') and session['failure_cleanup_exit_code'] == 0))
        assert branch['final_streak'] == 40 and branch['requested_target_reached']
        directory = ROOT / row['profile_directory']
        assert {p.name: digest(p) for p in directory.iterdir() if p.is_file()} == row['files']
        assert digest(directory / '00000001.save') == branch['complete_saved_sha256']
        before = props(ROOT / session['seed_profile_directory'] / '00000001.xml')
        after = props(directory / '00000001.xml')
        assert before['Award05Make40Task'] == before['awardShow5'] == ('Bool', '0')
        assert after['Award05Make40Task'] == after['awardShow5'] == ('Bool', '1')
        assert after['Award04Make20Task'] == after['awardShow4'] == ('Bool', '1')
        matching = [(n, c) for n, c in enumerate(session['runtime_award_contexts']) if c['streak'] == 40]
        assert len(matching) == 1
        award_index, context = matching[0]
        assert context['task20_owned'] and not context['task40_owned']
        assert context['money'] == branch['final_money']
        waits = [w for w in session['threshold_award_waits'] if w['ordinal'] == 5]
        assert len(waits) == 1 and waits[0]['streak'] == 40
        if branch.get('completed_target_saved_after_driver_deadline'):
            assert not waits[0]['before_owned'] and not waits[0]['before_shown']
            assert 'after_owned' not in waits[0] and 'after_shown' not in waits[0]
        else:
            assert waits[0]['after_owned'] and waits[0]['after_shown']
        pictures.append(ROOT / raw['work_directory'] / f'{index}-award-{award_index}.png')
        assert digest(pictures[-1]) == raw['screenshots'][pictures[-1].name]
        if implementations is None:
            implementations = raw['implementation_files']
        else:
            assert implementations == raw['implementation_files']
        for name, sha in raw['implementation_files'].items():
            assert digest(ROOT / raw['work_directory'] / 'runtime' / name) == sha
            current = 'Dobr\u00fd farm\u00e1\u0159.exe' if name == 'FarmarSourceInterop.exe' else name
            assert digest(ROOT / 'build/windows/Release' / current) == sha
        branches.append(branch)
        contexts.append(context)
        directories.append(directory)
        sessions.append(session)
    controlled = sessions[0]['input_profile_files'] == sessions[1]['input_profile_files']
    if controlled:
        assert len(branches[0]['actual_receipts']) == len(branches[1]['actual_receipts']) == 1
        for name in ['actual_receipts', 'supply_commands', 'market_transfers', 'final_money',
                     'additions', 'transferred', 'consumed', 'final_stock', 'final_market']:
            assert branches[0][name] == branches[1][name], name
    a, b = [Image.open(p).convert('RGB') for p in pictures]
    assert a.size == b.size == (800, 600)
    assert a.tobytes() == b.tobytes(), ImageChops.difference(a, b).getbbox()
    panel = dict(award_index=5, compared_rgb_pixels=480000, different_pixels=0,
        difference_bounds=ImageChops.difference(a, b).getbbox(), exact=True,
        files={p.relative_to(ROOT).as_posix(): digest(p) for p in pictures}, masks=[], normalizations=[])
    interop_path = args.interop_report.resolve(strict=True)
    interop = read(interop_path)
    assert interop['status'] == 'passed_actual_contract_streak_and_four_whole_bidirectional_saves_only'
    assert interop['expected_streak'] == 40
    assert interop['fixture_report']['sha256'] == interop['fixture_report_sha256'] == digest(descriptor_path)
    assert interop['implementation_files'] == implementations
    work = ROOT / interop['work_directory']
    for name, sha in interop['source_files'].items():
        archived = work / 'driver.py' if name == 'tools/validate_game_contract_streak_interop.py' else ROOT / name
        assert digest(archived) == sha, name
    for name, sha in interop['screenshots'].items():
        assert digest(work / name) == sha
    def location(index):
        return directories[index] if index < 2 else work / f'{index}-persisted/Profiles'
    transfers = interop['comparison']['transfer_comparisons']
    assert len(transfers) == len(interop['sessions']) == 4
    saved_sizes = []
    for index, row in enumerate(transfers):
        session = interop['sessions'][index]
        assert session['kind'] == ['source', 'native', 'native', 'source'][index]
        assert session['status'] == 'completed' and session['exit_code'] == 0
        restored = session['restored_expanded_farm']
        assert restored['streak'] == 40 and restored['task20_owned'] and restored['task40_owned']
        assert restored['awards'][4] and restored['awards'][5]
        before, after = row['before'], row['after']
        actual = exact_restored_binary(location(before) / '00000001.save', location(after) / '00000001.save')
        assert actual == row['binary'] and actual['whole_binary_unchanged'] and not row['normalizations']
        saved_sizes.append(actual['complete_expected_output_bytes'])
    traces = {}
    for path in work.glob('*-crash-trace.jsonl'):
        events = [json.loads(line) for line in path.read_text(encoding='utf8').splitlines()]
        assert events[-1] == dict(event='exit', exit_code=0)
        assert not any(e['event'] == 'access_violation' for e in events)
        traces[path.name] = dict(sha256=digest(path), events=events)
    assert len(traces) == 2
    full_name, _ = current_regression()
    for row in read(ROOT / 'analysis/inventory.json')['files']:
        assert digest(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159' / row['path']) == row['sha256']
    state_name = 'contract-streak-forty-state-validation.json'
    state = dict(status='passed_independently_completed_real40_receipts_ledgers_and_award5_full_pixels_only',
        checkpoints=dict(path=descriptor_path.relative_to(ROOT).as_posix(), sha256=digest(descriptor_path)),
        branches=branches, actual_threshold_award_panel=panel, original_and_source_contexts=contexts,
        identical_unmodified_input_profiles=controlled, controlled_final_receipt_and_all19_ledgers_exact=controlled,
        ordinary_farming_supply_claimed=False, original_memory_or_profile_written=False, whole_game_complete=False)
    write(ROOT / 'analysis' / state_name, state)
    interop_name = 'contract-streak-forty-interop-validation.json'
    interop.update(recorder_sha256=digest(__file__), independent_normal_exit_traces=traces,
        source_file_snapshots={'tools/validate_game_contract_streak_interop.py': (work / 'driver.py').relative_to(ROOT).as_posix()})
    write(ROOT / 'analysis' / interop_name, interop)
    regression_name = 'contract-streak-forty-regression.json'
    result = dict(status='passed_real40_award5_full_pixels_and_four_whole_cross_restores_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        reports={n: digest(ROOT / 'analysis' / n) for n in [state_name, interop_name]}, final_streak=40,
        exact_award_rgb_pixels=480000, actual_receipts_in_final_processes=[len(b['actual_receipts']) for b in branches],
        identical_unmodified_input_profiles=controlled, controlled_final_receipt_and_all19_ledgers_exact=controlled,
        final_money=[b['final_money'] for b in branches], four_cross_restore_bytes=saved_sizes,
        source_restore_processes_without_access_violation=2, original_files_unchanged=17,
        current_full_ctest_source_hashes_match=True, current_full_ctest_report=full_name,
        new_full_ctest_run=False, production_changed=False, whole_game_complete=False,
        pending=['Current final all-awards lifecycle comparison', 'Recorded native undefined tracker allocation fidelity decision'])
    write(ROOT / 'analysis' / regression_name, result)
    verification = read(ROOT / 'analysis/verification.json')
    for name in [state_name, interop_name, regression_name]:
        if 'analysis/' + name not in verification['reports']:
            verification['reports'].append('analysis/' + name)
    for name in [state_name, interop_name]:
        if 'analysis/' + name not in verification['original_game_runtime']['scope_reports']:
            verification['original_game_runtime']['scope_reports'].append('analysis/' + name)
    verification['latest_scoped_regression'] = dict(report='analysis/' + regression_name,
        recorded_at_utc=result['recorded_at_utc'], whole_game_complete=False)
    verification['current_contract_streak40_gate'] = dict(report='analysis/' + regression_name,
        complete=True, exact_award_rgb_pixels=480000, whole_cross_restores=4)
    write(ROOT / 'analysis/verification.json', verification)
    print(json.dumps(result))


if __name__ == '__main__':
    main()
