#!/usr/bin/env python3
"""Record seven actual simultaneous placements, natural award9 and whole-process saves."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    state=read(ROOT/'.local/seven-factories-state-validation.json');checked(state)
    assert state['status']=='passed_for_actual_seven_simultaneous_level0_buildings_and_natural_award9_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_seven_factories_state.py')
    c=state['comparison'];assert c['business_snapshots_per_executable']==9 and c['actual_alias_word_snapshots']==56
    assert c['non_alias_profile_discretes_equal'] and c['complete_identical_save_bytes']==4089 and not c['normalizations']
    assert c['actual_award_panel_pixels']['exact'] and c['actual_award_panel_pixels']['compared_pixels']==480000
    interop=read(ROOT/'.local/seven-factories-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_seven_simultaneous_buildings_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/seven-factories-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['exit_code']==0 and row['status']=='completed'
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4089 and not row['normalizations']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    plan=read(ROOT/'.local/seven-factory-placement-plan.json');assert plan['driver_sha256']==digest(ROOT/'tools/plan_factory_placement.py')
    initial=read(ROOT/'.local/seven-factories-interop/20261031T100827282334Z/report.json')
    reports={}
    for name,value in [('seven-factories-state-validation.json',state),('seven-factories-interop-validation.json',interop),('seven-factory-placement-plan.json',plan),
        ('seven-factories-initial-probe.json',dict(status='diagnostic_driver_failed_before_start',report_sha256=digest(ROOT/initial['work_directory']/'report.json'),work_directory=initial['work_directory'],error=initial['error'],traceback=initial['traceback'],cause='Incorrect crash-tracer path; cleanup NameError obscured FileNotFoundError. Path now resolved before process creation; cleanup guards trace creation.',recorded_as_game_failure=False,whole_game_complete=False))]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/profile-floating-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_seven_simultaneous_level0_buildings_natural_award9_and_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        simultaneous_factory_count=7,factory_kind=2,all_levels=0,paid_cost=1050,positions=c['positions'],business_snapshots_per_executable=9,actual_alias_word_snapshots=56,alias_padding_claimed_equal=False,
        all_current_profile_discretes_equal=c['all_profile_discretes_equal'],actual_award_panel_exact_rgb_pixels=480000,complete_identical_save_bytes=4089,full_process_bidirectional_transfers=4,source_restore_debugger_traces=traces,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/profile-floating-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Simultaneous active production, movement/collision limits and remaining maximum-input kinds','Remaining crops and natural award predicates','Remaining Squall97, JNG and uninstaller scopes'])
    write(ROOT/'analysis/seven-factories-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['seven-factories-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['seven-factories-state-validation.json','seven-factories-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/seven-factories-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded seven simultaneous level0 buildings, natural award9,56 alias words,480000 RGB pixels and four complete4089-byte transfers')
if __name__=='__main__':main()
