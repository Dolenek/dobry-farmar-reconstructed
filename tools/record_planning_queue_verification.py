#!/usr/bin/env python3
"""Record actual queue threshold, natural planning award and complete cross saves."""
import datetime
import hashlib
from record_live_production_verification import ROOT, digest, read, write, checked
from record_harvest_hundred_verification import traces_for
from audit_squall_native_arguments import instructions
from pe import PE


def main():
    state = read(ROOT/'.local/planning-queue-state-validation.json')
    checked(state)
    assert state['status'] == 'passed_actual32_task_queue_natural_planning_award_and_original_zero_dig_cost_only'
    assert state['auditor_sha256'] == digest(ROOT/'tools/validate_planning_queue_state.py')
    assert state['comparison']['actual_award_panel_pixels']['exact']
    interop = read(ROOT/'.local/planning-queue-interop-validation.json')
    work = checked(interop)
    assert interop['status'] == 'passed_natural_planning_award32_task_counter_and_four_complete_bidirectional_saves_only'
    assert interop['fixture_report_sha256'] == digest(ROOT/'.local/planning-queue-state-validation.json')
    assert len(interop['sessions']) == 4 and all(s['exit_code'] == 0 for s in interop['sessions'])
    assert all(t['binary']['whole_binary_unchanged'] and t['binary']['complete_expected_output_bytes'] == 15808 for t in interop['comparison']['transfer_comparisons'])
    traces = traces_for(work)
    failures = []
    for stamp in ['20261105T204758460055Z', '20261105T204922878437Z']:
        path = ROOT/'.local/planning-queue'/stamp/'report.json'
        report = read(path)
        assert report['status'] == 'failed' and 'actual task markers' in report['error']
        old = ROOT/report['work_directory']
        assert digest(old/'driver.py') == report['source_files']['tools/validate_game_planning_queue.py']
        rows = report['sessions'][0]['actual_enqueued_targets']
        assert len(rows) == 35 and rows[-1]['actual_native_queue_size'] == 32
        assert [r['actual_native_queue_size'] for r in rows if r['target']['kind'] == 'store'] == [0,16,32]
        failures.append(dict(report=path.relative_to(ROOT).as_posix(), sha256=digest(path),
            original_store_clicks_not_enqueued=3, actual_field_tasks_enqueued=32,
            real_work_not_started=True, process_exit_code_captured=False,
            source_comparison_of_paused_warehouse_clicks_performed=False,
            cause_of_original_paused_warehouse_click_behavior_established=False,
            resolution='Actual32 distinct digging tasks prove planning without interleaved inventory transfers',
            production_changed=False))
    first_path = ROOT/'.local/planning-queue-interop/20261105T205633135720Z/report.json'
    first = read(first_path)
    assert first['status'] == 'failed' and 'FFS38' in first['error']
    assert len(first['sessions']) == 4 and all(s['exit_code'] == 0 for s in first['sessions'])
    failures.append(dict(report=first_path.relative_to(ROOT).as_posix(), sha256=digest(first_path),
        actual_completed_processes=4, source_debugger_traces=traces_for(ROOT/first['work_directory']),
        rejected_diagnostic_assumption='Only dead-to-growing and growing-to-ready field transitions were allowed',
        actual_transition=dict(field=38, before_ready=1, after_growing=0, before_water='0.422783', after_water='0'),
        resolution='Original ReadyAction0043da40/0043dee0 returns grass to growing when water reaches zero;require actual output water0',
        production_changed=False))
    native = ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159.exe'
    source_path = ROOT/'.local/planning-queue-native-instructions.txt'
    ins = instructions(source_path, PE(native))
    expected = [(0x45a900,'mov','eax,dword ptr ds:[0065FF5Ch]'),
                (0x45a905,'mov','ecx,dword ptr [eax+0000282Ch]'),
                (0x45a90f,'mov','eax,dword ptr [eax+00002830h]'),
                (0x45a915,'sub','eax,ecx'),(0x45a917,'sar','eax,4'),
                (0x45a91a,'cmp','eax,1Eh'),(0x45a91d,'jb','0045A98C')]
    evidence = []
    for address, op, operands in expected:
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == operands
        evidence.append(dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=operands))
    full = read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name) == sha for name, sha in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(native.parent/row['path']) == row['sha256']
    observation = dict(status='retained_original_input_observations_and_corrected_diagnostic_transition_gate',
        observations=failures, diagnostic_paid_identifier_correction='Original DigCost XML is empty;default0 and both actual Money2504 remain unchanged;no nonzero payment is claimed',
        original_instruction_file=source_path.relative_to(ROOT).as_posix(), original_instruction_file_sha256=digest(source_path),
        original_queue_threshold_instruction_evidence=evidence, whole_game_complete=False)
    reports = {}
    for name, value in [('planning-queue-state-validation.json',state),
                        ('planning-queue-interop-validation.json',interop),
                        ('planning-queue-initial-observations.json',observation)]:
        write(ROOT/'analysis'/name,value)
        reports[name] = digest(ROOT/'analysis'/name)
    result = dict(status='passed_natural_planning_award32_actual_queued_jobs_and_four_complete_saves_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__), reports=reports,
        native_task_vector_begin_offset='282c', native_task_vector_end_offset='2830', native_task_bytes=16,
        native_queue_threshold=30, original_queue_threshold_instruction_evidence=evidence,
        actual_queued_jobs_per_executable=32, actual_final_planning_count=32,
        original_dig_cost=0, unchanged_money=2504, unchanged_collected=44,
        actual_award_index=19, actual_award_panel_exact_rgb_pixels=480000,
        original_binary_save_entirely_unchanged_bytes=15808, actual_bidirectional_transfers=4,
        whole_identical_transfer_bytes=[15808]*4, source_restore_debugger_traces=traces,
        independent_world_timing_and_random_offer_differences_preserved=True,
        original_profile_fields_modified_by_probe=False, normalizations=[], production_changed=False,
        current_full_ctest_source_hashes_match=True, current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',
        new_full_ctest_run=False, original_files_unchanged=17, whole_game_complete=False,
        pending=['Natural20/40 contract streaks,high-price sale and final all-awards condition',
                 'Original paused warehouse click cause and source comparison remain unverified',
                 'Remaining Squall ABI/behavior,JNG,uninstaller and whole original goal scopes'])
    name = 'planning-queue-regression.json'
    write(ROOT/'analysis'/name,result)
    verification = read(ROOT/'analysis/verification.json')
    for entry in list(reports)+[name]:
        if 'analysis/'+entry not in verification['reports']:
            verification['reports'].append('analysis/'+entry)
    for entry in ['planning-queue-state-validation.json','planning-queue-interop-validation.json']:
        if 'analysis/'+entry not in verification['original_game_runtime']['scope_reports']:
            verification['original_game_runtime']['scope_reports'].append('analysis/'+entry)
    verification['latest_scoped_regression'] = dict(report='analysis/'+name, recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths = [ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows = [p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded32 actual queued jobs per EXE,natural planning award,480000 exact RGB pixels and four full15808-byte restores.')


if __name__ == '__main__':
    main()
