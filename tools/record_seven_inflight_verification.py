#!/usr/bin/env python3
"""Record seven loaded factories through actual pause, four restores and two completions."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    fixture=read(ROOT/'.local/seven-inflight-state-validation.json');checked(fixture)
    assert fixture['status']=='passed_for_actual_seven_simultaneous_inflight_buildings_only'
    assert fixture['auditor_sha256']==digest(ROOT/'tools/validate_seven_inflight_state.py')
    c=fixture['comparison'];assert c['business_snapshots_per_executable']==18 and c['actual_loaded_grass']==14 and c['all_profile_discretes_equal'] and c['complete_identical_save_bytes']==4089
    cursors=[]
    for session in fixture['sessions']:
        points=session['actual_slider_release_cursors'];assert [r['slot'] for r in points]==list(range(7))
        assert all(r['position'][0]>=r['requested'][0]-1 for r in points);cursors.append(points)
    reports={};traces={}
    write(ROOT/'analysis/seven-inflight-state-validation.json',fixture);reports['seven-inflight-state-validation.json']=digest(ROOT/'analysis/seven-inflight-state-validation.json')
    for name,status,count in [('seven-inflight-interop-state-validation.json','passed_for_actual_seven_inflight_bidirectional_saves_only',4),('seven-completion-state-validation.json','passed_for_actual_seven_restored_completions_only',2)]:
        report=read(ROOT/'.local'/name);work=checked(report)
        assert report['status']==status and len(report['sessions'])==len(report['comparison']['transfer_comparisons'])==count
        assert report['fixture_report_sha256']==digest(ROOT/'.local/seven-inflight-state-validation.json') and report['auditor_sha256']==digest(ROOT/'tools/validate_seven_inflight_interop_state.py')
        for row in report['sessions']:assert row['status']=='completed' and row['exit_code']==0
        for row in report['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4089 and not row['normalizations']
        for path in work.glob('*-crash-trace.jsonl'):
            events=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
            assert events[-1]==dict(event='exit',exit_code=0) and not any(r['event']=='access_violation' for r in events)
            traces[path.relative_to(ROOT).as_posix()]=dict(sha256=digest(path),events=events)
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    assert len(traces)==3
    probes=[]
    for stamp,reason in [('20261031T102030640277Z','Driver incorrectly expected zero previous processed items; actual existing baseline has six. No gameplay modification.'),
        ('20261031T102117437399Z','Reference AddDepot input did not occur after slowed scene entry; recorded as driver failure, not a proved game difference.'),
        ('20261031T102323638051Z','Reference slider input did not occur after slowed scene entry. Current driver allows both0.1-speed scene transitions to finish.'),
        ('20261031T102642761652Z','Native completed; source slot4 accepted one unit at a nominal slider endpoint. Exact old cursor was not recorded. Current real drag crosses maximum and records all actual cursor positions; no inferred production-code fix.')]:
        path=ROOT/'.local/seven-inflight'/stamp/'report.json';value=read(path)
        assert value['status']=='failed';probes.append(dict(work_directory=value['work_directory'],report_sha256=digest(path),error=value['error'],reason=reason,recorded_as_pass=False))
    initial=dict(status='driver_counter_timing_endpoint_and_event_order_findings_preserved',probes=probes,
        completion_auditor_initial_mistake=dict(source_xml_index=75,original_and_source_profile_letter_index=37,resolution='Stable-sort all Gameplay events by time before interpreting profile letter IDs; original gameplay unchanged'),whole_game_complete=False)
    write(ROOT/'analysis/seven-inflight-initial-probes.json',initial);reports['seven-inflight-initial-probes.json']=digest(ROOT/'analysis/seven-inflight-initial-probes.json')
    full=read(ROOT/'analysis/profile-floating-regression.json')
    changed={name:[value,digest(ROOT/name)] for name,value in full['production_source_files'].items() if digest(ROOT/name)!=value};assert set(changed)<={'CMakeLists.txt'},changed
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_seven_simultaneous_inflight_factories_four_restores_and_two_fourteen_unit_completions',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        actual_processes=8,business_snapshots_per_executable=18,simultaneous_factories=7,loaded_units=14,real_slider_release_cursors=cursors,
        partial_progress_transfers=4,cross_executable_completion_processes=2,production_units_per_completion=14,final_milk=20,final_processed=20,final_warehouse=94,complete_identical_save_bytes=4089,normalizations=[],
        actual_new_letter=dict(time=1110,event_index=37),source_exception_traces=traces,production_implementation_changed=False,previous_full_ctest_report='analysis/profile-floating-regression.json',changes_since_full_ctest=changed,
        original_files_unchanged=17,whole_game_complete=False,pending=['Other factory-kind maximum loads and movement/collision limits','Five other planted crop cycles and remaining natural awards','Remaining Squall97, general JNG and isolated uninstaller scopes'])
    write(ROOT/'analysis/seven-inflight-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['seven-inflight-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in reports:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/seven-inflight-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification);print('Recorded seven active factories, fourteen real inputs, four whole-save restores and two fourteen-unit completions')
if __name__=='__main__':main()
