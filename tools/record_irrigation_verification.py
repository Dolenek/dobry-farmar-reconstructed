#!/usr/bin/env python3
"""Record actual sprinkler purchases, movement, active watering and complete restores."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write,checked


def main():
    state=read(ROOT/'.local/irrigation-state-validation.json');checked(state)
    interop=read(ROOT/'.local/irrigation-interop-validation.json');work=checked(interop)
    assert state['status']=='passed_for_actual_three_paid_sprinklers_watering_and_movement_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_irrigation_state.py')
    c=state['comparison'];assert c['business_snapshots_per_executable']==17 and c['automatically_watered_active_fields']==20 and c['paid_cost']==600
    assert c['complete_identical_save_bytes']==4089 and not c['normalizations']
    assert interop['status']=='passed_for_three_sprinklers_and_live_watering_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/irrigation-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['status']=='completed' and row['exit_code']==0
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4089 and not row['normalizations']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    initial=read(ROOT/'.local/irrigation-interop/20261031T153129591871Z/report.json')
    assert initial['status']=='failed' and initial['error']=="KeyError('active_fields')"
    failure=dict(status='diagnostic_metadata_lookup_error_only',work_directory=initial['work_directory'],report_sha256=digest(ROOT/initial['work_directory']/'report.json'),error=initial['error'],
        resolution='Read active field IDs from fixture sessions[0], not a nonexistent top-level key',recorded_as_game_failure=False,whole_game_complete=False)
    reports={}
    for name,value in [('irrigation-state-validation.json',state),('irrigation-interop-validation.json',interop),('irrigation-initial-observation.json',failure)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_three_paid_sprinklers_twenty_watered_fields_and_bidirectional_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        business_snapshots_per_executable=17,paid_sprinklers=3,paid_cost=600,active_automatically_watered_fields=20,affected_field_ids=c['affected_field_ids'],
        actual_warehouse_rejection_argb=c['actual_invalid_preview_argb'],invalid_release_keeps_moving=True,empty_radius_drop=[130,280],movement_cost=0,
        live_time_dependent_maturity=c['time_dependent_discrete_difference'],complete_identical_save_bytes=4089,full_process_bidirectional_restores=4,source_restore_debugger_traces=traces,
        controlled_whole_frame_live_particle_replay_verified=False,production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Actual contract failure/delayed acceptance, weather and remaining natural awards','Controlled whole-frame replay of live irrigation particles','Remaining Squall97, JNG and uninstaller scopes'])
    write(ROOT/'analysis/irrigation-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['irrigation-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['irrigation-state-validation.json','irrigation-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/irrigation-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded three actual paid sprinklers,20 watered fields,17 work checks per EXE and four exact4089-byte restores')


if __name__=='__main__':main()
