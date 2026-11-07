#!/usr/bin/env python3
"""Retain actual input failures separately from independently verified saved results."""
import datetime
import xml.etree.ElementTree as ET
from record_live_production_verification import ROOT, digest, read, write


def properties(path):
    return {n.get('Name'): (n.get('Type'), n.get('Val')) for n in ET.parse(path).getroot()}


def main():
    cases = []
    for stamp, mode, slow_count in [('20261107T154907753248Z', 'rejected_batched_motion', 2),
                        ('20261107T155141727228Z', 'precise_motion_without_extra_navigation_move', 2),
                        ('20261107T155351262076Z', 'precise_motion_navigation_retries', 6),
                        ('20261107T155726217033Z', 'held_menu_snapshot_before_game_manager_existed', 1)]:
        path = ROOT / '.local/contract-streak' / stamp / 'report.json'
        raw = read(path)
        assert raw['status'] == 'failed'
        expected = 'ReadProcessMemory' if mode == 'held_menu_snapshot_before_game_manager_existed' else 'Actual navigation did not reach its requested scene'
        assert expected in raw['error']
        assert len(raw['sessions']) == 1
        session = raw['sessions'][0]
        assert session['kind'] == 'source' and session['failure_cleanup_exit_code'] == 0
        work = ROOT / raw['work_directory']
        before = properties(ROOT / session['seed_profile_directory'] / '00000001.xml')
        after = properties(work / '0-failure-persisted/Profiles/00000001.xml')
        keys = ['Money', 'awQTask', 'task0'] + [f'{prefix}{n}' for prefix in ['WGoodCount', 'MGoodCount'] for n in range(19)]
        assert all(before[k] == after[k] for k in keys)
        assert not session.get('contract_receipts') and not session.get('market_transfers') and not session.get('supply_commands')
        for name, sha in raw['source_files'].items():
            assert digest(ROOT / raw.get('source_file_snapshots', {}).get(name, name)) == sha, name
        for name, sha in raw['screenshots'].items():
            assert digest(work / name) == sha
        slow = [row for row in raw['input_api_timings'] if row['seconds'] >= 1]
        assert len(slow) == slow_count and all(row['api'] == 'SendInput' and 19 < row['seconds'] < 21 for row in slow)
        if mode != 'rejected_batched_motion':
            assert all(row['input_types'] == [0] and row['input_flags'] == [2] for row in slow)
        cases.append(dict(mode=mode, runtime_report=path.relative_to(ROOT).as_posix(), sha256=digest(path),
            original_driver_status=raw['status'], error=raw['error'], normal_cleanup_exit_code=0,
            slow_driver_api_calls=slow, actual_receipts=0, all19_stock_and_market_words_unchanged=True,
            seed_money_and_streak_unchanged=True, production_defect_inferred=False))
    native_path = ROOT / '.local/contract-streak/20261107T153240356025Z/report.json'
    native = read(native_path)
    assert native['status'] == 'failed' and 'Actual contract threshold award did not finish' in native['error']
    session = native['sessions'][0]
    assert session['failure_cleanup_exit_code'] == 0
    assert session['contract_receipts'][-1]['after_completion']['streak'] == 40
    state_path = ROOT / 'analysis/contract-streak-forty-native-state-validation.json'
    state = read(state_path)
    assert state['runtime_report_sha256'] == digest(native_path)
    branch = state['comparison']['branches'][0]
    assert branch['completed_target_saved_after_driver_deadline'] and branch['requested_target_reached']
    values = properties(ROOT / branch['persisted_profile_directory'] / '00000001.xml')
    assert values['awQTask'] == ('UInt', '40')
    assert values['Award05Make40Task'] == values['awardShow5'] == ('Bool', '1')
    interop_path=ROOT/'.local/contract-streak-interop/20261107T161452437163Z/report.json'
    partial=read(interop_path);partial_work=ROOT/partial['work_directory']
    assert partial['status']=='failed' and 'Main menu Start did not construct game' in partial['error']
    assert len(partial['sessions'])==4
    descriptor=read(ROOT/partial['fixture_report']['path'])
    assert digest(ROOT/partial['fixture_report']['path'])==partial['fixture_report']['sha256']
    def location(index):
        if index<2:
            return ROOT/next(row['profile_directory'] for row in descriptor['profile_sets'] if row['kind']==['native','source'][index])
        return partial_work/f'{index}-persisted/Profiles'
    completed=[]
    for session,(before,after) in zip(partial['sessions'][:3],[(0,2),(1,3),(2,4)]):
        assert session['status']=='completed' and session['exit_code']==0
        assert session['restored_expanded_farm']['streak']==40 and session['restored_expanded_farm']['money']==25714
        left=location(before)/'00000001.save';right=location(after)/'00000001.save'
        assert left.read_bytes()==right.read_bytes() and left.stat().st_size==17369
        completed.append(dict(before=before,after=after,complete_identical_binary_bytes=17369,
            input_sha256=digest(left),output_sha256=digest(right)))
    assert partial['sessions'][3]['kind']=='source' and 'restored_expanded_farm' not in partial['sessions'][3]
    traces={}
    import json
    for trace_path in partial_work.glob('*-crash-trace.jsonl'):
        events=[json.loads(line) for line in trace_path.read_text(encoding='utf8').splitlines()]
        assert events[-1]==dict(event='exit',exit_code=0) and not any(e['event']=='access_violation' for e in events)
        traces[trace_path.name]=dict(sha256=digest(trace_path),exit_code=0,access_violations=0)
    assert len(traces)==2
    result = dict(status='recorded_native_saved40_input_failures_and_partial_cross_restores_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        source_failures=cases, native_report=dict(path=native_path.relative_to(ROOT).as_posix(), sha256=digest(native_path),
            original_driver_status='failed', normal_cleanup_exit_code=0, actual_completed_streak=40,
            saved_owned_and_shown_flags=True, complete_independently_expected_saved_bytes=17369,
            independent_audit=dict(path=state_path.relative_to(ROOT).as_posix(), sha256=digest(state_path))),
        partial_cross_restore_attempt=dict(runtime_report=interop_path.relative_to(ROOT).as_posix(),sha256=digest(interop_path),
            original_driver_status='failed',three_actual_restores=completed,
            fourth_source_stayed_in_menu=True,fourth_game_restore_claimed=False,source_process_traces=traces),
        production_changed=False, rejected_batched_motion_retained_in_immutable_raw_snapshot=True,
        subsequent_source_completion_claimed=False, whole_game_complete=False)
    write(ROOT / 'analysis/contract-streak-input-observations.json', result)
    print('Preserved native deadline failure,saved40,4 rejected input runs and3 exact partial restores;fourth restore not claimed.')


if __name__ == '__main__':
    main()
