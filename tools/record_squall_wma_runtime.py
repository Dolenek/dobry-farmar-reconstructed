#!/usr/bin/env python3
"""Record actual WMA buffers and stateful output while preserving playback limits."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write

def checked(name):
    report=read(ROOT/'.local'/name);work=ROOT/report['work_directory']
    for path,value in report['source_files'].items():
        assert digest(work/'source-snapshots'/path)==value,path
        if path.startswith('src/'):assert digest(ROOT/path)==value,path
    assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256']
    for path,value in report['build_artifacts'].items():assert digest(work/'build-artifacts'/path)==value,path
    for row in report['cases']:
        for run in row['runs']:
            folder=ROOT/run['directory']
            for path,value in run['artifacts'].items():assert digest(folder/path)==value,path
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path'])==row['sha256']
    return report

def main():
    equivalence=read(ROOT/'analysis/squall-wma-explicit-left-validation.json');reports={};hashes=equivalence['candidate_sha256']
    for index,suffix in [(0,''),(1,'-fresh')]:
        name='squall-wma-buffers-status'+suffix+'-validation.json';report=checked(name)
        assert report['status']=='matched_forty_prepared_wma_buffer_pairs_playback_nonprogress_separately_recorded' and report['failed_cases']==0 and report['rebuilt_sha256']==hashes[index]
        assert len(report['cases'])==40 and report['exact_physical_pcm_bytes']==97104 and report['actual_playback_progress_observations']==72 and len(report['nonprogress_observations'])==8 and report['actual_pause_observations']==80
        for row in report['cases']:
            assert row['status']=='passed'
            for run in row['runs']:
                result=run['result'];assert result['file_exclusive_after_unload'] and result['actual_pause_stable'] and result['nonzero_bytes']>0
                raw=result['raw_cursors'];assert raw[2]==raw[3]
            a=(ROOT/row['runs'][0]['directory']/'physical-pcm.bin').read_bytes();b=(ROOT/row['runs'][1]['directory']/'physical-pcm.bin').read_bytes();assert a==b
        for row in report['nonprogress_observations']:assert row['fixture']=='wma-4-wmav2-48000-2.wma' and row['result']['directsound_playing_flag'] and row['result']['raw_channel_status']==1
        reports[name]=report
        name='squall-wma-sequences'+suffix+'-validation.json';report=checked(name);assert report['status']=='passed_twenty_stateful_wma_pairs_280_guarded_reads_only' and report['failed_cases']==0 and report['rebuilt_sha256']==hashes[index] and report['exact_destination_bytes']==92160 and len(report['cases'])==20
        for row in report['cases']:
            assert row['status']=='passed';a,b=[run['result'] for run in row['runs']];assert a==b and a['guarded_sequence_reads']==14
            assert a['sequence'][6]['start']==a['sequence'][7]['start']==0xffffffff and a['sequence'][10]['count']==a['sequence'][11]['count']==0
            assert (ROOT/row['runs'][0]['directory']/'sequence-pcm.bin').read_bytes()==(ROOT/row['runs'][1]['directory']/'sequence-pcm.bin').read_bytes()
        reports[name]=report
    name='squall-wma-buffers-48k-11ms-validation.json';report=checked(name);assert report['status']=='matched_prepared_wma_buffer_pairs_playback_nonprogress_separately_recorded' and report['failed_cases']==0 and len(report['cases'])==8 and report['buffer_milliseconds']==11 and report['actual_playback_progress_observations']==16 and not report['nonprogress_observations'] and report['exact_physical_pcm_bytes']==25344;reports[name]=report
    current=read(ROOT/'.local/squall-wma-current-validation.json');assert current['status']=='found_actual_wma_differences_or_failures' and current['failed_cases']==1 and current['rebuilt_sha256']==hashes[0]
    current_work=ROOT/current['work_directory']
    for name,value in current['source_files'].items():assert digest(current_work/'source-snapshots'/name)==value,name
    for row in current['cases']:
        for run in row['runs']:
            for name,value in run['artifacts'].items():assert digest(ROOT/run['directory']/name)==value,name
    differences=[row for row in current['cases'] if row.get('status')=='failed'];assert len(differences)==1
    row=differences[0];assert row['fixture']=='wma-2-wmav2-22050-1.wma' and row['mode']=='mono' and row['count']==16361 and row['memory']==1 and row['precision']=='PC53'
    a,b=[(ROOT/run['directory']/'decoded-pcm.bin').read_bytes() for run in row['runs']];assert a!=b and all(run['result']['returned_bytes']==0 and run['result']['guards_intact'] for run in row['runs'])
    actual_changed=[sum(x!=0x5a for x in data[16:-16]) for data in [a,b]];reported_changed=[run['result']['changed_bytes'] for run in row['runs']];assert actual_changed[1]!=reported_changed[1]
    reports['squall-wma-current-block-observations.json']=current
    trace_work=ROOT/'.local/squall-wma-async-callers/20261104T175556529960Z';trace=read(trace_work/'report.json');assert len(trace['cases'])==8
    for case in trace['cases']:
        matching=[call for call in case['info']['calls'] if call['caller_address']-case['state']['module_base']==(0x1548a if case['backend']=='native' else 0x1355f)]
        assert len(matching)==1 and matching[0]['HRESULT']==0 and matching[0]['actual_bytes']==2048 and case['result']['returned_bytes']==0 and case['result']['changed_bytes']>30000
    reports['squall-wma-async-com-observations.json']=dict(status='actual_system_GetInfo_returns_before_continuing_destination_writes',trace=trace,artifacts={path.relative_to(ROOT).as_posix():digest(path) for path in trace_work.rglob('*') if path.is_file()},original_mono_instructions='1001546a Update(SSUPDATE_ASYNC);10015487 GetInfo;no CompletionStatus wait',raw_unequal_case_directory=row['runs'][1]['directory'],stored_destination_changed_bytes=actual_changed,subsequent_changed_byte_counts=reported_changed,raw_destination_equality_claimed=False,controlled_COM_result_forced=False,diagnostic_GetInfo_vtable_temporarily_replaced=True,production_changed=False)
    for name,report in reports.items():write(ROOT/'analysis'/name,report)
    failed=[]
    for folder in ['20261104T173929189806Z','20261104T174113898915Z','20261104T174317774694Z']:
        work=ROOT/'.local/squall-wma-buffers'/folder;report=read(work/'report.json');assert report['status'] in ['found_actual_wma_buffer_or_playback_differences','failed']
        for path,value in report['build_artifacts'].items():assert digest(work/'build-artifacts'/path)==value,path
        failed.append(dict(work_directory=work.relative_to(ROOT).as_posix(),report_sha256=digest(work/'report.json'),status=report['status'],failed_cases=report.get('failed_cases'),error=report.get('error'),all_raw_outputs_preserved=True))
    result=dict(status='matched_both_wma_prepared_buffer_and_stateful_read_scopes_short48k10ms_nonprogress_still_open',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),candidate_sha256=hashes,reports={name:digest(ROOT/'analysis'/name) for name in reports},binary_equivalence_proof='analysis/squall-wma-explicit-left-validation.json',prepared_buffer_pairs_per_build=40,exact_physical_pcm_bytes_per_build=97104,observed_cursor_progress_processes_per_build=72,nonprogress_processes_per_build=8,pause_stability_observations_per_build=80,stateful_read_pairs_per_build=280,exact_stateful_destination_bytes_per_build=92160,
        latest_block_comparison_raw_destination_mismatches=1,async_destination_difference_resolved_as_exact_match=False,additional48k11ms_buffer_pairs=8,additional48k11ms_exact_pcm_bytes=25344,additional48k11ms_cursor_progress_processes=16,raw_cursors_compared_or_normalized=False,
        interpretation='All prepared buffers and stateful read outputs match exactly.10ms48kHz nonprogress occurs on both native and reconstructed processes;11ms shows progress in all16 probes. Dependence on fragment length is observed; general timing correctness and root cause are not yet proved.',preserved_exploration_failures=failed,original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,pending=['Async return0 raw destinations are unequal and not treated as verified byte equality','Resolve short48kHz10ms cursor observation/root cause,default200ms buffers and long refill/EOF','General COM error and lifetime branches,malformed WMA','Tracker and full original game/DLL/JNG/uninstaller requirements'])
    write(ROOT/'analysis/squall-wma-runtime-validation.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-wma-runtime-validation.json','squall-wma-explicit-left-validation.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-wma-runtime-validation.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False,latest_full_game_regression='analysis/squall-mpeg-regression.json',full_game_regression_transferred_by_program_byte_equality='analysis/squall-wma-explicit-left-validation.json')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both40-buffer/280-stateful-read WMA scopes with72 progress/8 nonprogress observations each;11ms48kHz16/16 progress is separate.')
if __name__=='__main__':main()
