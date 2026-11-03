#!/usr/bin/env python3
"""Audit preserved actual WMA outputs and read-only traces without hiding native failures."""
import datetime,json,struct
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write

INITIAL=ROOT/'.local/squall-wma/20261103T211155549288Z'
TRACE=ROOT/'.local/squall-wma-crash/20261103T212353468219Z'

def main():
    initial=read(INITIAL/'report.json');assert initial['failed_cases']==60 and len(initial['cases'])==125
    for name,value in initial['source_files'].items():
        assert digest(INITIAL/'source-snapshots'/name)==value,name
        if name.startswith('src/'):assert digest(ROOT/name)==value,name
    for name,value in initial['build_artifacts'].items():assert digest(INITIAL/'build-artifacts'/name)==value,name
    full=partial=failed=returned=destinations=metadata=0;observations=[]
    for row in initial['cases']:
        runs=row['runs'];assert len(runs)==2
        for run in runs:
            folder=ROOT/run['directory']
            for name,value in run['artifacts'].items():assert digest(folder/name)==value,name
        if row['phase']=='metadata_baseline':
            assert all(run['exit_code']==0 for run in runs) and runs[0]['result']==runs[1]['result'];metadata+=1;continue
        result=dict(fixture=row['fixture'],precision=row['precision'],memory=row['memory'],mode=row['mode'],count=row['count'])
        if all(run.get('exit_code')==0 for run in runs):
            assert runs[0]['result']==runs[1]['result'];a=(ROOT/runs[0]['directory']/'decoded-pcm.bin').read_bytes();b=(ROOT/runs[1]['directory']/'decoded-pcm.bin').read_bytes();value=runs[0]['result'];assert a==b and value['guards_intact']
            declared=row['count']*(2 if row['mode']=='mono' else 4);assert len(a)==declared+32 and value['returned_bytes']<=declared
            if value['returned_bytes']==declared:full+=1
            else:assert value['returned_bytes']==0;partial+=1
            returned+=value['returned_bytes'];destinations+=declared;result.update(status='exact_return_and_complete_guarded_destination',returned_bytes=value['returned_bytes'],declared_bytes=declared,destination_sha256=digest(ROOT/runs[0]['directory']/'decoded-pcm.bin'),partial_output_return=value['returned_bytes']<declared)
        else:
            assert all(run.get('exit_code')==0xc0000005 for run in runs) and row['mode']=='mono' and row['count']>2048
            for run in runs:assert (ROOT/run['directory']/'operations.log').read_text(encoding='utf-8').splitlines()[-1]=='decode'
            failed+=1;result.update(status='same_unhandled_AV_code_recorded_not_pcm_equivalence',native_exception=0xc0000005,source_exception=0xc0000005)
        observations.append(result)
    assert (metadata,full,partial,failed,returned,destinations)==(5,60,36,24,343120,1947440)
    left_observations=[]
    for name in ['wma-1-wmav1-44100-2.wma','wma-3-wmav2-44100-2.wma','wma-4-wmav2-48000-2.wma']:
        def pcm(mode):
            row=next(row for row in initial['cases'] if row['phase']=='pcm' and row['fixture']==name and row['mode']==mode and row['count']==128 and row['precision']=='PC53' and row['memory']==0);a=(ROOT/row['runs'][0]['directory']/'decoded-pcm.bin').read_bytes()[16:-16];return struct.unpack('<'+'h'*(len(a)//2),a)
        mono,stereo=pcm('mono'),pcm('stereo');left,right=stereo[::2],stereo[1::2];assert mono==left
        averages=[(a+b)//2 if a+b>=0 else -(-(a+b)//2) for a,b in zip(left,right)];different=sum(a!=b for a,b in zip(mono,averages));assert different>=127
        left_observations.append(dict(fixture=name,samples=128,mono_equals_left=True,mono_differs_from_signed_average_samples=different))
    trace=read(TRACE/'report.json');assert len(trace['cases'])==2
    for row in trace['cases']:
        assert row['exit_code']==0xc0000005 and row['trace_exit']==0 and len(row['faults'])==2
        first=row['faults'][0];assert first['access_kind']==1
        if row['backend']=='native':assert first['eip']-row['state']['module_base']==0x155b5 and first['edi']>0xffff0000 and first['ebp']==1
        else:assert first['eip']-row['state']['module_base']==0x13656 and first['eax']>0xffff0000
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path'])==row['sha256']
    write(ROOT/'analysis/squall-wma-initial-process-observations.json',initial)
    write(ROOT/'analysis/squall-wma-read-only-crash-traces.json',dict(status='actual_native_and_source_zero_count_underflow_write_faults_on_owned_test_inputs',trace=trace,artifacts={path.relative_to(ROOT).as_posix():digest(path) for path in TRACE.rglob('*') if path.is_file()},debugger_memory_written=False,exceptions_suppressed=False,production_implementation_changed=False))
    out=ROOT/'analysis/squall-wma-native-data-instructions.txt';out.write_bytes((ROOT/'.local/squall-wma-data-native.txt').read_bytes())
    prior=read(ROOT/'analysis/squall-mpeg-regression.json')
    for name,value in prior['production_source_files'].items():assert digest(ROOT/name)==value,name
    result=dict(status='actual_wma96_successful_pairs_exact_24_shared_process_failures_not_claimed_complete',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),initial_report_sha256=digest(INITIAL/'report.json'),initial_work_directory=INITIAL.relative_to(ROOT).as_posix(),candidate_sha256=initial['rebuilt_sha256'],reference_sha256=initial['reference_sha256'],source_files=initial['source_files'],metadata_pairs=metadata,full_return_pairs=full,matching_partial_return_pairs=partial,matched_guarded_destinations=full+partial,exact_returned_pcm_bytes=returned,exact_destination_bytes=destinations,shared_unhandled_AV_pairs=failed,observations=observations,left_channel_observations=left_observations,
        initial_test_error='Initial validator incorrectly required every read to return full requested bytes;36 of its60 failures are in fact exact native/source partial-output return0 behavior. Raw report is preserved unchanged.',
        native_instruction_evidence=dict(mono='100153e0',left_channel_only='1001558e MOVSX [ECX]; SHL1; signed DIV2; ECX+=4',zero_count_underflow='1001558e..100155c0 DO loop executes before decrementing EDI even when GetInfo actual/blockalign is0',stereo='10015180',duration='100155f0'),
        raw_Ghidra_export_missing_functions=['10015180','100153e0','100155f0'],raw_Ghidra_original_export_preserved=True,
        source_clarification_pending='Public WMA source uses unsequenced *src++ + *src++; current pinned MSVC binary demonstrably implements left only. Replace with explicit observed behavior after preserving baseline.',
        full_current_regression_unchanged='analysis/squall-mpeg-regression.json',production_implementation_changed=False,original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Stateful WMA start/loop/EOF and real physical buffers','Broader COM error/lifetime branches and general input behavior','Explicit left-channel source reconstruction and remaining original goal'])
    write(ROOT/'analysis/squall-wma-initial-observations.json',result)
    print('Recorded96 exact WMA return/destination pairs,24 shared AV cases and actual read-only underflow traces; no production changes.')
if __name__=='__main__':main()
