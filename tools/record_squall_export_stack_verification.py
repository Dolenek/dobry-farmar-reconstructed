#!/usr/bin/env python3
"""Record calling-convention proof separately from full argument semantics."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    reports={};returns=None
    for suffix in ['', '-fresh']:
        name=f'squall-export-stack{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_all97_export_caller_cleanup_paths_and_measured_stack_scope_only'
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256'] and digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        work=ROOT/report['work_directory']
        for path,value in report['logs'].items():assert digest(work/path)==value,path
        assert digest(work/'sample.wav')==report['sample_sha256']
        rows=report['native_return_paths'];assert len(rows)==97 and sum(len(row['return_addresses']) for row in rows)==223
        assert all(row['all_returns_caller_cleanup'] for row in rows)
        assert sum(row['reachable_instructions'] for row in rows)==5569
        if returns is None:returns=rows
        else:assert returns==rows
        for context in report['precision_contexts']:
            assert context['export_count']==97 and not context['native_argument_count_statically_proved']
            for entry in context['measured_exports']:
                assert entry['source_stack_delta']==entry['native_stack_delta']==0
                assert entry['declared_stack_argument_bytes']==entry['argument_count']*4 and entry['stack_canaries_per_call']==16
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    instructions=ROOT/'analysis/squall-exports-native-instructions.txt';instructions.write_bytes((ROOT/'.local/squall-exports-native-disassembly.txt').read_bytes());reports[instructions.name]=digest(instructions)
    audit=read(ROOT/'analysis/squall-export-audit.json');context=read(ROOT/'.local/squall-export-stack-validation.json')['precision_contexts'][0]
    for row in audit['exports']:
        entry=next(entry for entry in context['measured_exports'] if entry['name']==row['name'])
        row['caller_cleanup_abi_verified']=dict(declared_argument_count=entry['argument_count'],declared_stack_bytes=entry['declared_stack_argument_bytes'],declared_return_bytes=entry['declared_return_bytes'],native_and_source_measured_stack_delta=0,
            scope='All reachable native returns are C3; two builds/PC53/PC24 each execute every typed export with16 stack canaries. Declared count/type alone is not independent native argument proof.')
        assert not row['native_signature_fully_verified']
    audit['export_stack_reports']=['analysis/squall-export-stack-validation.json','analysis/squall-export-stack-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_all97_export_reachable_caller_cleanup_and_actual_stack_measurements_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        exports=97,aggregate_native_cfg_instructions=5569,aggregate_native_return_sites=223,resolved_switch_tables=2,measured_calls_per_build_per_dll=194,stack_canaries_per_build_per_dll=3104,
        original_argument_count_independently_proved=False,whole_native_signature_semantics_verified=False,
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-export-stack-config.log','squall-export-stack-build.log']},
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Independent native argument-count/stack-read audit','Remaining pointer/return semantics and complete97-export behavior','Remaining full original goal scopes'])
    write(ROOT/'analysis/squall-export-stack-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-export-stack-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-export-stack-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded all97 measured cdecl exports and223 reachable C3 returns; native argument-count audit remains separate.')


if __name__=='__main__':main()
