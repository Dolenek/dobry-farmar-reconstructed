#!/usr/bin/env python3
"""Record actual main-story completion and first late-world coefficient renewal."""
import datetime,hashlib,json
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    state=read(ROOT/'.local/campaign-loop-state-validation.json');checked(state)
    assert state['status']=='passed_actual_main_story_end_and_first_late_world_clock_loop_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_campaign_loop_state.py')
    assert state['coefficient_auditor_sha256']==digest(ROOT/'tools/gameplay_coefficient_audit.py')
    interop=read(ROOT/'.local/campaign-loop-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_both_post_campaign_clock_loop_outcomes_and_four_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/campaign-loop-state-validation.json')
    transfers=interop['comparison']['transfer_comparisons'];assert len(transfers)==len(interop['sessions'])==4
    assert all(s['status']=='completed' and s['exit_code']==0 and not s['restored_expanded_farm']['main_game'] for s in interop['sessions'])
    assert all(not t['normalizations'] and t['identical_complete_save_bytes']>14616 for t in transfers)
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(e['event']=='access_violation' for e in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    reports={}
    for name,value in [('campaign-loop-state-validation.json',state),('campaign-loop-interop-validation.json',interop)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert full['tests_passed']==102 and full['tests_failed']==0 and full['implementation_files']==state['implementation_files']==interop['implementation_files']
    assert all(digest(ROOT/name)==value for name,value in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_main_story_end_clock_wrap_regeneration_and_four_bidirectional_transfers_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        runtime_processes=2,further_bidirectional_restores=4,timeline_checkpoints_per_executable=13,
        main_game_end_event=312,main_game_end_time=11190,clock_wrap_from=14400,clock_wrap_to=11190,
        new_saved_coefficient_rng_audits=state['comparison']['new_saved_coefficient_rng_audits'],
        original_rng_start_high_bit_unobservable=True,matched_rng_claimed=False,
        money_event_ledger=state['comparison']['money_event_ledger'],
        branch_save_bytes=state['comparison']['complete_save_bytes'],
        complete_cross_restore_bytes=[t['identical_complete_save_bytes'] for t in transfers],normalizations=[],
        source_restore_debugger_traces=traces,production_implementation_changed=False,
        implementation_files=state['implementation_files'],current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Remaining natural awards and contract streak','Later repeated loops and cumulative contract requirements in actual dialogs','Remaining complete Squall/JNG/uninstaller scopes'])
    write(ROOT/'analysis/campaign-loop-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['campaign-loop-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in reports:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/campaign-loop-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report']='analysis/campaign-loop-regression.json'
    verification['current_report_serialization']['scope']='Current tracker-loaders,late-timeline and campaign-loop reports;older records identify their original locally recorded bytes'
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded actual main-story end,14400->11190 wrap,383 new RNG coefficients each and four complete save transfers.')

if __name__=='__main__':main()
