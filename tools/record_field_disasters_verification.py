#!/usr/bin/env python3
"""Record natural world events and transfers of both unmodified random outcomes."""
import datetime
from record_live_production_verification import ROOT,digest,read,write,checked


def main():
    state=read(ROOT/'.local/field-disasters-state-validation.json');checked(state)
    assert state['status']=='passed_for_actual_later_world_timeline_stochastic_money_and_first_crop_destruction_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_field_disasters_state.py')
    interop=read(ROOT/'.local/field-disasters-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_both_actual_crop_disaster_outcomes_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/field-disasters-state-validation.json')
    transfers=interop['comparison']['transfer_comparisons'];assert len(transfers)==len(interop['sessions'])==4
    assert [t['identical_complete_save_bytes'] for t in transfers]==[6943,7784,6943,6943]
    assert [t['money'] for t in transfers]==[2904,2804,2904,2904]
    for t in transfers:assert not t['normalizations']
    for s in interop['sessions']:
        assert s['status']=='completed' and s['exit_code']==0 and s['tutorial_step']==-1
        assert len(s['live_fields'])==60 and all(f['active'] for f in s['live_fields'])
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[__import__('json').loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    failures=[]
    scopes=[('20261101T164329730594Z','Suspension caught an in-progress scene-tree rotation; rejected snapshot'),
            ('20261101T164621601131Z','Owned window lost foreground while returning from fast clock'),
            ('20261101T165225056632Z','83 guarded foreground attempts exhausted the diagnostic deadline'),
            ('20261101T170218566342Z','Source Start pointer guard rejected move before mouse button; first three completed sessions retained')]
    for stamp,description in scopes:
        folder=ROOT/'.local/field-disasters'/stamp;failed=read(folder/'report.json');assert failed['status']=='failed'
        failures.append(dict(work_directory=failed['work_directory'],report_sha256=digest(folder/'report.json'),
            driver_sha256=digest(folder/'driver.py'),error=failed['error'],description=description,
            completed_sessions=[i for i,s in enumerate(failed['sessions']) if s.get('status')=='completed'],
            recorded_as_game_failure=False))
    observation=dict(status='diagnostic_snapshot_foreground_and_pointer_failures_preserved',failures=failures,
        resolution='Bounded read-only tree retry; ordinary save/restart returns to actual default1x clock; guarded pointer move retry',
        retained_completed_segments=state['retained_completed_segments'],game_state_memory_written=False,production_implementation_changed=False,whole_game_complete=False)
    reports={}
    for name,value in [('field-disasters-state-validation.json',state),('field-disasters-interop-validation.json',interop),('field-disasters-initial-observation.json',observation)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    regression=dict(status='passed_actual_later_world_timeline_first_crop_destruction_and_four_cross_restores_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        runtime_processes=4,further_bidirectional_restores=4,timeline_checkpoints_per_executable=6,
        money_event_ledger=state['comparison']['money_event_ledger'],explicit_stochastic_money=[2904,2804],
        destruction_event_indices=[112,113,114],destroy_fields_native_address='0042f080',
        destroyed_crop_ids=[14,15,16,17,18,23,27],surviving_ready_crop_id=34,
        matched_rng_and_particle_timing_verified=False,identical_whole_award_rgb_pixels=480000,
        branch_save_bytes=[6943,7784],complete_cross_restore_bytes=[t['identical_complete_save_bytes'] for t in transfers],normalizations=[],
        source_restore_debugger_traces=traces,production_implementation_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Guaranteed later crop destruction and remaining natural world events','Remaining natural awards and nonzero contract streak','Remaining complete Squall/JNG/uninstaller scopes'])
    write(ROOT/'analysis/field-disasters-regression.json',regression)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['field-disasters-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['field-disasters-state-validation.json','field-disasters-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/field-disasters-regression.json',recorded_at_utc=regression['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded natural seven-crop destruction, independent eleven-event money ledgers and four byte-exact branch restores.')


if __name__=='__main__':main()
