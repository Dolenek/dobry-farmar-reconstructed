#!/usr/bin/env python3
"""Record real twenty-contract award, full pixels and four whole cross-restores."""
import datetime
import hashlib
import json
from PIL import Image, ImageChops
from audit_contract_streak_state import audit
from record_live_production_verification import ROOT, digest, read, write
from validate_counter_supply_state import props
from counter_save_audit import exact_restored_binary


def main():
    raw_path = ROOT / '.local/contract-streak/20261106T184740071405Z/report.json'
    raw, branches = audit(raw_path)
    work = ROOT / raw['work_directory']
    assert [b['final_streak'] for b in branches] == [20, 20]
    assert [b['final_money'] for b in branches] == [14214, 13854]
    assert [b['supply_commands'] for b in branches] == [50, 49]
    assert [b['market_transfers'] for b in branches] == [865, 846]
    assert [b['complete_expected_saved_bytes'] for b in branches] == [16580, 16597]
    contexts = []
    for index, session in enumerate(raw['sessions']):
        assert session['status'] == 'completed' and session['exit_code'] == 0
        assert len(session['runtime_award_contexts']) == 1
        context = session['runtime_award_contexts'][0]
        assert context['streak'] == 20 and context['money'] == branches[index]['final_money']
        assert context['task20_owned'] is False and context['task40_owned'] is False
        waits = session['threshold_award_waits']
        assert len(waits) == 1 and waits[0]['ordinal'] == 4 and waits[0]['streak'] == 20
        assert waits[0]['before_owned'] is waits[0]['before_shown'] is False
        assert waits[0]['after_owned'] is waits[0]['after_shown'] is True
        before = props(ROOT / session['seed_profile_directory'] / '00000001.xml')
        after = props(work / f'{index}-persisted/Profiles/00000001.xml')
        assert before['Award04Make20Task'] == before['awardShow4'] == ('Bool', '0')
        assert after['Award04Make20Task'] == after['awardShow4'] == ('Bool', '1')
        assert after['Award05Make40Task'] == after['awardShow5'] == ('Bool', '0')
        assert not session.get('pointer_move_retries') and not session.get('acceptance_input_retries') and not session.get('confirmation_input_retries')
        contexts.append(context)
    pictures = [work / f'{n}-award-0.png' for n in range(2)]
    a, b = [Image.open(p).convert('RGB') for p in pictures]
    assert a.size == b.size == (800, 600)
    left, right = a.tobytes(), b.tobytes()
    assert left == right
    panel = dict(award_index=4, compared_rgb_pixels=480000, different_pixels=0,
        difference_bounds=ImageChops.difference(a, b).getbbox(), exact=True,
        files={p.relative_to(ROOT).as_posix(): digest(p) for p in pictures}, masks=[], normalizations=[])
    interop_path = ROOT / '.local/contract-streak-interop-validation.json'
    interop = read(interop_path);iw = ROOT / interop['work_directory']
    assert interop['status'] == 'passed_actual_contract_streak_and_four_whole_bidirectional_saves_only'
    assert interop['expected_streak'] == 20 and interop['fixture_report']['sha256'] == digest(raw_path)
    assert interop['fixture_report_sha256'] == digest(raw_path)
    # The actual driver was archived before launch. The only subsequent source
    # edit corrected its origin comment; retain its exact executed code hash.
    executed = 'tools/validate_game_contract_streak_interop.py'
    for name, sha in interop['source_files'].items():
        source = iw / 'driver.py' if name == executed else ROOT / name
        assert digest(source) == sha, name
    interop['source_file_snapshots'] = {executed: (iw / 'driver.py').relative_to(ROOT).as_posix()}
    for name, sha in interop['screenshots'].items():assert digest(iw / name) == sha
    transfers = interop['comparison']['transfer_comparisons']
    assert len(transfers) == len(interop['sessions']) == 4
    for index, row in enumerate(transfers):
        session = interop['sessions'][index]
        assert session['kind'] == ['source', 'native', 'native', 'source'][index]
        assert session['status'] == 'completed' and session['exit_code'] == 0
        observed = session['restored_expanded_farm']
        assert observed['streak'] == 20 and observed['task20_owned'] and observed['awards'][4]
        assert not observed['task40_owned'] and not observed['awards'][5]
        before, after = row['before'], row['after']
        location = lambda n: (work if n < 2 else iw) / f'{n}-persisted/Profiles'
        binary = exact_restored_binary(location(before) / '00000001.save', location(after) / '00000001.save')
        assert binary == row['binary'] and binary['whole_binary_unchanged'] and not row['normalizations']
        assert binary['complete_expected_output_bytes'] == [16580, 16597, 16580, 16580][index]
    traces = {}
    for path in iw.glob('*-crash-trace.jsonl'):
        events = [json.loads(line) for line in path.read_text(encoding='utf8').splitlines()]
        assert events[-1] == dict(event='exit', exit_code=0)
        assert not any(event['event'] == 'access_violation' for event in events)
        traces[path.name] = dict(sha256=digest(path), events=events)
    assert len(traces) == 2
    state_name = 'contract-streak-twenty-state-validation.json'
    state = read(ROOT / 'analysis' / state_name)
    assert state['runtime_report_sha256'] == digest(raw_path)
    state['comparison'].update(actual_threshold_award_panel=panel, original_and_source_contexts=contexts)
    write(ROOT / 'analysis' / state_name, state)
    interop_name = 'contract-streak-twenty-interop-validation.json'
    interop.update(recorder_sha256=digest(__file__), independent_normal_exit_traces=traces, whole_game_complete=False)
    write(ROOT / 'analysis' / interop_name, interop)
    full = read(ROOT / 'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT / n) == sha for n, sha in full['production_source_files'].items())
    for row in read(ROOT / 'analysis/inventory.json')['files']:
        assert digest(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159' / row['path']) == row['sha256']
    regression_name = 'contract-streak-twenty-regression.json'
    result = dict(status='passed_real_contract_streak20_award4_full_pixels_and_four_whole_cross_restores_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        reports={n: digest(ROOT / 'analysis' / n) for n in [state_name, interop_name]},
        actual_receipts_per_executable=5, actual_supply_commands=[50, 49], actual_market_transfers=[865, 846],
        final_streak=20, final_money=[14214, 13854], exact_award_rgb_pixels=480000,
        original_and_source_normal_exit_codes=[0, 0], four_cross_restore_bytes=[16580, 16597, 16580, 16580],
        source_restore_processes_without_access_violation=2, original_memory_or_profile_written=False,
        ordinary_farming_supply_claimed=False, production_changed=False, original_files_unchanged=17,
        current_full_ctest_source_hashes_match=True, current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',
        new_full_ctest_run=False, whole_game_complete=False,
        pending=['Continue actual40-contract threshold and final all-awards condition',
                 'Remaining19 native Squall signatures and wider DLL/game/JNG/uninstaller scopes'])
    write(ROOT / 'analysis' / regression_name, result)
    verification = read(ROOT / 'analysis/verification.json')
    for name in [state_name, interop_name, regression_name]:
        if 'analysis/' + name not in verification['reports']:verification['reports'].append('analysis/' + name)
    if 'analysis/' + interop_name not in verification['original_game_runtime']['scope_reports']:
        verification['original_game_runtime']['scope_reports'].append('analysis/' + interop_name)
    verification['latest_scoped_regression'] = dict(report='analysis/' + regression_name, recorded_at_utc=result['recorded_at_utc'], whole_game_complete=False)
    paths = [ROOT / 'CMakeLists.txt'] + [p for folder in ['src', 'tools', 'tests'] for p in (ROOT / folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    lines = [p.relative_to(ROOT).as_posix() + '\0' + digest(p) + '\n' for p in sorted(paths, key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(lines).encode('utf8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/' + regression_name
    write(ROOT / 'analysis/verification.json', verification)
    print('Verified actual20-contract award4,480000 exact RGB pixels,all16580/16597 bytes and4 whole cross-restores;2 source exits0 withoutAV.')


if __name__ == '__main__':main()
