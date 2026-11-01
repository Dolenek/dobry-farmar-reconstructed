#!/usr/bin/env python3
"""Record actual callback observations; never equate independently scheduled counts."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    reports={}
    for suffix in ['', '-fresh']:
        name=f'squall-channel-worker{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_for_actual_channel_worker_userdata_reentrancy_retention_return_zero_and_signed_periods_scope'
        assert report['result_comparisons']==456 and report['actual_created_channels']==52
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256'] and digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        work=ROOT/report['work_directory']
        for path,value in {**report['logs'],**report['sample_files']}.items():assert digest(work/path)==value,path
        for context in report['precision_contexts']:
            assert context['pointer_cases']==216 and context['actual_created_channels']==26 and context['observations']==24
            rows=context['callback_observations'];assert len(rows)==24
            for dll in [0,1]:
                branch=[row for row in rows if row[0]==dll];assert len(branch)==12
                retained=[row for row in branch if row[3]==10];assert len(retained)==6
                assert {(row[1],row[2]) for row in retained}=={(spatial,stage) for spatial in [0,1] for stage in [1,2,3]}
                assert all(row[4]>=2 and row[5]==0 for row in retained)
                retired=[row for row in branch if row[5]==1];assert len(retired)==2 and all(row[3]==row[4]==1 for row in retired)
                high={row[3]:row[4] for row in branch if row[3]>10}
                assert high[0x7fffffff]==0 and all(high[value]>0 for value in [0xffffffff,0xfffffffe,0x80000000])
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    path=ROOT/'analysis/squall-channel-worker-native-instructions.txt'
    path.write_bytes((ROOT/'.local/squall-channel-worker-native-disassembly.txt').read_bytes());reports[path.name]=digest(path)
    audit=read(ROOT/'analysis/squall-export-audit.json');row=next(row for row in audit['exports'] if row['name']=='SQUALL_Channel_SetWorker')
    row['status']='partially_compared';row['loaded_DLL_channel_worker_verified_scope']='Actual prepared/playing/paused2D/3D callbacks, channel ID/userdata/thread/reentrant getter, rejected code1/zero-period retention, synchronous clear, return0 deletes after one call, signed high-bit periods; two builds and PC53/PC24. Independent scheduling counts preserved.'
    audit['channel_worker_reports']=['analysis/squall-channel-worker-validation.json','analysis/squall-channel-worker-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_channel_worker_callback_semantics_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,original_export_address='10005b10',api_results_per_build=456,
        pointer_cases_per_build=432,actual_created_channels_per_build=52,callback_observations_per_build=48,
        observation_ms_after_clear=35,independent_callback_counts_claimed_equal=False,
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-channel-worker-config.log','squall-channel-worker-build.log']},
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',
        original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Callback self-mutation and other reentrant APIs','Concurrent teardown and timer wraparound','Remaining complete97-export ABI/behavior and full-game scopes'])
    write(ROOT/'analysis/squall-channel-worker-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-channel-worker-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-channel-worker-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded actual channel-worker ID/userdata/reentrancy/return0 and signed high-period behavior in both builds.')


if __name__=='__main__':main()
