#!/usr/bin/env python3
"""Audit deferred warehouse and immediate field input in both actual complete EXEs."""
import datetime
import hashlib
from record_live_production_verification import ROOT, digest, read, write, checked
from validate_counter_supply_state import props, audit_timeline, seconds
from validate_factory_history_state import resource
from audit_squall_native_arguments import instructions
from pe import PE


def main():
    report = read(ROOT/'.local/paused-deep-click-validation.json')
    work = checked(report)
    assert report['status'] == 'explored_actual_deferred_warehouse_and_immediate_field_clicks_only'
    assert [s['kind'] for s in report['sessions']] == ['native','source']
    fixture = ROOT/'analysis/no-dry-award-state-validation.json'
    baseline = ROOT/read(fixture)['work_directory']/'0-persisted/Profiles'
    original = props(baseline/'00000001.xml')
    stock = [int(original[f'WGoodCount{n}'][1]) for n in range(19)]
    assert stock[0] == 1 and sum(stock) == 40
    events = list(resource('Gameplay/gameplay.xml').find('Events'))
    events.sort(key=seconds)
    branches = []
    for index,session in enumerate(report['sessions']):
        assert session['exit_code'] == 0 and session['status'] == 'completed'
        assert session['fixture_report']['sha256'] == digest(fixture)
        assert session['xml_input_policy'] == dict(warehouse='1',field='0')
        rows = session['input_observations']
        assert len(rows) == 8
        assert [r['label'] for r in rows] == [
            'paused-before-deferred-warehouse','paused-after-deferred-warehouse',
            'deferred-warehouse-executed-after-unpause','paused-before-abandoned-warehouse',
            'paused-second-deferred-warehouse','paused-immediate-field-replaces-active-warehouse',
            'abandoned-warehouse-did-not-store-harvest','complete-input-policy-comparison']
        assert [len(r['markers']) for r in rows] == [0,0,0,0,0,1,0,0]
        if index == 0:
            assert [r['actual_native_queue_size'] for r in rows] == [0,0,0,0,0,1,0,0]
        assert rows[0]['game_time'] == rows[1]['game_time']
        assert rows[3]['game_time'] == rows[4]['game_time'] == rows[5]['game_time']
        assert [r['collected'] for r in rows] == [44]*6+[45]*2
        assert [r['carried'] for r in rows] == [[12]+[0]*7]*2+[[0]*8]*4+[[1]+[0]*7,[0]*8]
        for row,added in zip(rows,[0,0,12,12,12,12,12,13]):
            expected = stock.copy();expected[0] += added
            assert row['stock'] == expected and row['money'] == 2504
        values = props(work/f'{index}-persisted/Profiles/00000001.xml')
        assert values['awQCollect'] == ('UInt','45') and values['Money'] == ('Int','2504')
        assert all(values[f'PCC{n}'] == ('UInt','0') for n in range(8))
        assert [int(values[f'WGoodCount{n}'][1]) for n in range(19)] == rows[-1]['stock']
        prices,notes,wraps,rounding = audit_timeline([float(values['GameTime'][1])],original,events)
        assert not notes and not wraps and not rounding and all(values[k] == v for k,v in prices.items())
        saved = work/f'{index}-persisted/Profiles/00000001.save'
        assert saved.read_bytes() == (baseline/'00000001.save').read_bytes() and saved.stat().st_size == 15808
        branches.append(dict(kind=session['kind'], actual_input_snapshots=8,
            queued_markers_at_snapshots=[0,0,0,0,0,1,0,0],
            deferred_click_executed_without_second_click=True,
            moving_to_immediate_field_abandoned_pending_warehouse_click=True,
            actual_harvest_actions=1, actual_store_operations=2, actual_grass_stored=13,
            final_collected=45, final_carried=0, final_warehouse_occupied=53,
            money_unchanged=2504, entire_binary_save_unchanged_bytes=15808,normalizations=[]))
    native = ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159.exe'
    path = ROOT/'analysis/paused-deep-click-native-instructions.txt'
    ins = instructions(path,PE(native))
    expected = [(0x4f2653,'mov','al,byte ptr [esi+0000026Ah]'),
        (0x4f265b,'je','004F2674'),(0x4f265d,'cmp','dword ptr ds:[006604DCh],esi'),
        (0x4f2665,'mov','dword ptr ds:[006604E0h],2'),
        (0x4f2678,'call','dword ptr [edx+00000150h]'),
        (0x4f28fd,'cmp','byte ptr [esi+0000026Ah],bl'),
        (0x4f2905,'cmp','dword ptr ds:[006604DCh],esi'),
        (0x4f290d,'mov','eax,dword ptr ds:[006604E0h]'),
        (0x4f2912,'dec','eax'),(0x4f2915,'dec','eax'),
        (0x4f291c,'call','dword ptr [eax+00000150h]'),
        (0x4f292e,'mov','dword ptr ds:[006604E0h],ebx')]
    evidence = []
    for address,op,operands in expected:
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == operands
        evidence.append(dict(address=f'{address:08x}',bytes=row['raw'].hex(),op=op,operands=operands))
    full = read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name) == sha for name,sha in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(native.parent/row['path']) == row['sha256']
    report.update(status='passed_actual_deferred_warehouse_pause_resume_and_immediate_field_input_only',
        auditor_sha256=digest(__file__),runtime_report_sha256=digest(ROOT/'.local/paused-deep-click-validation.json'),
        comparison=dict(branches=branches,original_input_instructions=evidence,
            original_deep_processing_byte_offset='26a',original_active_button='006604dc',original_pending_input='006604e0',
            original_input_xml=dict(warehouse_deep_processing=1,field_deep_processing=0),
            planning_queue_earlier_warehouse_observations_explained=True,
            actual_time_advanced_while_paused=False,original_profile_fields_modified_by_probe=False,
            source_native_clock_equality_claimed=False,production_changed=False,normalizations=[]),
        whole_game_complete=False)
    state_name = 'paused-deep-click-state-validation.json'
    write(ROOT/'analysis'/state_name,report)
    result = dict(status='passed_original_deferred_input_observation_resolved_in_both_complete_executables',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reports={state_name:digest(ROOT/'analysis'/state_name)},
        instruction_evidence_file=path.relative_to(ROOT).as_posix(),instruction_evidence_sha256=digest(path),
        actual_input_snapshots_per_executable=8,actual_harvests_per_executable=1,
        actual_store_operations_per_executable=2,actual_grass_stored_per_executable=13,
        final_collected=45,final_warehouse_occupied=53,final_carried=0,
        entire_binary_save_unchanged_bytes=15808,original_pending_cause_and_source_comparison_now_verified=True,
        production_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Natural20/40 contract streaks and final all-awards condition',
                 'Remaining77 native Squall ABI signatures and broader behavior',
                 'Remaining JNG/uninstaller and whole original goal scopes'])
    name = 'paused-deep-click-regression.json'
    write(ROOT/'analysis'/name,result)
    verification = read(ROOT/'analysis/verification.json')
    for entry in [state_name,name]:
        if 'analysis/'+entry not in verification['reports']:
            verification['reports'].append('analysis/'+entry)
    if 'analysis/'+state_name not in verification['original_game_runtime']['scope_reports']:
        verification['original_game_runtime']['scope_reports'].append('analysis/'+state_name)
    verification['latest_scoped_regression'] = dict(report='analysis/'+name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths = [ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows = [p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Verified original deferred warehouse input in both EXEs,16 actual snapshots,26 stored grass and unchanged15808-byte saves.')


if __name__ == '__main__':
    main()
