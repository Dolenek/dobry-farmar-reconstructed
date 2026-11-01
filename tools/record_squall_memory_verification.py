#!/usr/bin/env python3
"""Record loaded-DLL memory callback behavior without claiming whole Squall equivalence."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    reports={}
    for suffix in ['', '-fresh']:
        name=f'squall-memory{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_for_actual_memory_callbacks_reader_ownership_and_forced_allocation_failure_scope'
        assert report['result_comparisons']==1568 and report['callback_event_snapshots']==672
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256']
        assert digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        work=ROOT/report['work_directory']
        for path,value in report['logs'].items():assert digest(work/path)==value,path
        for path,value in report['sample_files'].items():assert digest(work/path)==value,path
        for row in report['precision_contexts']:
            assert row['result_comparisons']==784 and row['guarded_sample_snapshots']==114 and row['callback_event_snapshots']==336
            events=row['callback_events'];assert len(events)==row['callback_events_per_dll']==147
            live={};failures=0
            for operation,generation,size,serial in events:
                assert generation in [0,1] and size in [11,12,172]
                if operation==0:assert serial not in live;live[serial]=(generation,size)
                elif operation==1:assert live.pop(serial)==(generation,size)
                else:assert operation==2 and serial==0;failures+=1
            assert not live and failures==9
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    instructions=(ROOT/'.local/squall-memory-callbacks-native-disassembly.txt').read_text(encoding='utf-8')
    instructions=instructions[:instructions.index('  10003C0B:')]
    assert '10003BFC:' in instructions and 'call        100085E0' in instructions and '10003C0A: C3' in instructions
    write_path=ROOT/'analysis/squall-memory-native-instructions.txt';write_path.write_text(instructions,encoding='utf-8');reports[write_path.name]=digest(write_path)
    audit=read(ROOT/'analysis/squall-export-audit.json');addresses={}
    descriptions={
        'SetMemoryCallbacks':'Actual callback A/B/null/1/-1/readable/noaccess pointer pairs, pre-Init/DirectSound/post-Free, retained reader ownership, guarded sizes/order, filename/file-buffer allocation failure and successful retry; two builds and PC53/PC24',
        'Sample_UnloadAll':'Actual two reader generations and duplicate file references; all owned filename/file-buffer allocations freed exactly once through retained callbacks; repeated empty unload; two builds and PC53/PC24',
        'Sample_LoadFile':'Actual mono PCM16 WAV64frames/8000Hz/8ms, memory flags0/1/-1/2, shared duplicate file ownership, forced allocation failures and successful retry; two builds and PC53/PC24'}
    for name,description in descriptions.items():
        row=next(row for row in audit['exports'] if row['name']=='SQUALL_'+name);addresses[row['name']]=row['original_address']
        row['status']='partially_compared';row['loaded_DLL_memory_verified_scope']=description
    audit['memory_callback_reports']=['analysis/squall-memory-validation.json','analysis/squall-memory-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_memory_callback_ownership_and_failure_scope_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,original_export_addresses=addresses,api_results_per_build=1568,
        guarded_sample_snapshots_per_build=228,callback_event_snapshots_per_build=672,callback_events_per_precision_per_dll=147,
        pointer_pairs_per_build=228,actual_loads_per_build_per_dll=102,forced_failed_loads_per_build_per_dll=18,
        original_unconditional_setter_store_instruction='10003bfc calls100085e0 after validation, including error result-4',
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-memory-config.log','squall-memory-build.log']},
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',
        original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Invalid installed callbacks invoked in isolated fault child','Concurrent mutation and arbitrary codec allocation failures','Remaining complete97-export and full-game scopes'])
    write(ROOT/'analysis/squall-memory-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-memory-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-memory-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both memory callback builds:1568 API results,228 guarded sample snapshots,672 callback snapshots each.')


if __name__=='__main__':main()
