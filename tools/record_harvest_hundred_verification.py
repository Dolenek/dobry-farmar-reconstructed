#!/usr/bin/env python3
"""Record real harvesting,capacity boundaries,natural award0 and cross-EXE saves."""
import datetime,hashlib,json
from record_live_production_verification import ROOT,digest,read,write,checked

def traces_for(work):
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2;return traces

def main():
    state=read(ROOT/'.local/harvest-hundred-state-validation.json');checked(state)
    assert state['status']=='passed_actual56_harvests_natural100_award_and_capacity_ledger_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_harvest_hundred_state.py')
    c=state['comparison'];assert c['business_snapshots_per_executable']==61 and c['actual_field_snapshots_per_executable']==3660
    assert c['actual_harvest_actions_per_executable']==56 and c['initial_collected']==44 and c['final_collected']==100
    assert c['complete_identical_save_bytes']==15808 and c['entire_save_unchanged_from_native_baseline'] and not c['normalizations']
    assert c['actual_award_panel_pixels']==dict(compared_pixels=480000,different_pixels=0,difference_bounds=None,exact=True)
    interop=read(ROOT/'.local/harvest-hundred-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_natural100_harvest_full_warehouse_carried8_and_four_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/harvest-hundred-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['exit_code']==0 and row['status']=='completed'
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==15808 and not row['normalizations']
    traces=traces_for(work)
    first_path=ROOT/'.local/harvest-hundred-interop/20261105T133448523283Z/report.json';first=read(first_path)
    assert first['status']=='failed' and 'observed[\'no_dry\']' in first['traceback']
    assert len(first['sessions'])==4 and all(s['status']=='completed' and s['exit_code']==0 and not s['restored_expanded_farm']['no_dry'] and s['restored_expanded_farm']['awards'][12] for s in first['sessions'])
    first_traces=traces_for(ROOT/first['work_directory'])
    observation=dict(status='completed_original_and_source_restores_followed_by_rejected_inherited_no_dry_assumption',
        initial_report=first_path.relative_to(ROOT).as_posix(),initial_report_sha256=digest(first_path),completed_processes=4,source_debugger_traces=first_traces,
        original_awNoDry=False,already_earned_award12=True,
        resolution='Compare transient awNoDry to its actual saved baseline;water below15 clears transient flag/time without revoking earned award12',
        recorded_as_game_failure=False,production_changed=False,
        actual_independent_regrowth_differences=c['regrowth_readiness_differences'],actual_independent_random_offer_difference=c['random_offer_difference'],
        matched_rng_stream_or_identical_world_timing_claimed=False,normalizations=[],whole_game_complete=False)
    reports={}
    for name,value in [('harvest-hundred-state-validation.json',state),('harvest-hundred-interop-validation.json',interop),('harvest-hundred-initial-observations.json',observation)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    for name,sha in full['production_source_files'].items():assert digest(ROOT/name)==sha,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual56_harvests_natural100_award_capacity_boundaries_and_bidirectional_saves_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        original_counter_increment='0045BFA0',original_award_predicate='004C8650:awQCollect>99',
        harvest_count_before=44,harvest_count_after=100,actual_harvests_per_executable=56,business_snapshots_per_executable=61,
        actual_field_snapshots_per_executable=3660,carried_capacity=16,warehouse_capacity=100,final_carried_grass=8,final_stored_grass=61,
        actual_award_panel_exact_rgb_pixels=480000,actual_panel_glyphs=40,
        complete_identical_save_bytes=15808,entire_save_unchanged_from_native_baseline=True,
        full_process_bidirectional_transfers=4,source_restore_debugger_traces=traces,
        independent_random_offer_and_regrowth_differences_preserved=True,matched_rng_stream_claimed=False,
        original_profile_fields_modified_by_probe=False,production_implementation_changed=False,
        current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Natural1000 harvest and100/1000 production counters,20/40 contract streaks and remaining awards','Whole Squall97/JNG/uninstaller and remaining original goal scopes'])
    name='harvest-hundred-regression.json';write(ROOT/'analysis'/name,result)
    verification=read(ROOT/'analysis/verification.json')
    for entry in list(reports)+[name]:
        if 'analysis/'+entry not in verification['reports']:verification['reports'].append('analysis/'+entry)
    for entry in ['harvest-hundred-state-validation.json','harvest-hundred-interop-validation.json']:
        if 'analysis/'+entry not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+entry)
    verification['latest_scoped_regression']=dict(report='analysis/'+name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded112 actual harvests,natural100 award,capacity boundaries and four complete15808-byte restores.')

if __name__=='__main__':main()
