#!/usr/bin/env python3
"""Record partial production through four real transfers and two actual completions."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write,checked
def main():
    reports={};traces={}
    fixture=read(ROOT/'.local/inflight-production-state-validation.json');checked(fixture)
    assert fixture['status']=='passed_for_actual_paused_inflight_cowshed_fixture_only' and fixture['auditor_sha256']==digest(ROOT/'tools/validate_inflight_production_state.py')
    write(ROOT/'analysis/inflight-production-state-validation.json',fixture);reports['inflight-production-state-validation.json']=digest(ROOT/'analysis/inflight-production-state-validation.json')
    for name,status,count in [('inflight-interop-validation.json','passed_for_partial_production_bidirectional_saves_only',4),('inflight-completion-validation.json','passed_for_restored_partial_production_completion_only',2)]:
        report=read(ROOT/'.local'/name);work=checked(report)
        assert report['status']==status and report['fixture_report_sha256']==digest(ROOT/'.local/inflight-production-state-validation.json')
        assert len(report['sessions'])==len(report['comparison']['transfer_comparisons'])==count
        for session in report['sessions']:assert session['exit_code']==0 and session['status']=='completed'
        for row in report['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==3730 and not row['normalizations']
        for path in work.glob('*-crash-trace.jsonl'):
            rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
            assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
            traces[path.relative_to(ROOT).as_posix()]=dict(sha256=digest(path),events=rows)
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    assert len(traces)==3
    failed=ROOT/'.local/inflight-completion/20261029T214600590375Z';probe=read(failed/'report.json')
    assert probe['status']=='failed' and len(probe['sessions'])==2 and all(session['exit_code']==0 for session in probe['sessions'])
    write(ROOT/'analysis/inflight-completion-initial-probe.json',dict(status='actual_processes_completed_but_auditor_omitted_natural_grass_maturity',
        runtime_report_sha256=digest(failed/'report.json'),work_directory=failed.relative_to(ROOT).as_posix(),error=probe['error'],recorded_as_pass=False,
        resolution='Both games naturally complete grass24 while finishing milk; require FFS24=1,FFP24=0 and awGrowType|1 explicitly, then rerun all transfers/completions'))
    full=read(ROOT/'analysis/live-production-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_inflight_count_progress_four_transfers_and_two_cross_executable_completions',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,actual_processes=8,partial_progress_transfers=4,completion_processes=2,
        complete_save_bytes_per_transfer=3730,normalizations=[],production_units_per_completion=2,final_milk=4,final_processed=4,
        saved_progress_is_nonzero_and_monotonic_during_actual_frames=True,progress_claimed_bit_identical_across_independent_clocks=False,
        source_exception_traces=traces,implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/live-production-regression.json',
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Warehouse-full blocking and resource release; all factory levels and simultaneous buildings','Remaining whole-game, audio, JNG and uninstaller scopes'])
    write(ROOT/'analysis/inflight-production-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['inflight-completion-initial-probe.json','inflight-production-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in reports:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/inflight-production-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification);print('Recorded actual partial production, four exact saves and two completed cross-executable restores; whole goal incomplete')
if __name__=='__main__':main()
