#!/usr/bin/env python3
"""Record restored DLL FP startup, defined native fallback and current full regression."""
import datetime
import hashlib
import subprocess
import xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write
from record_squall_pcm_eof_verification import check


def checked_runtime(path,expected):
    value=read(path);assert value['status']==expected and not value['deviations']
    work=ROOT/value['work_directory']
    for name,sha in value['source_files'].items():assert digest(ROOT/name)==sha,name
    for name,sha in value['logs'].items():assert digest(work/name)==sha,name
    for item in value['binaries'].values():assert digest(ROOT/item['path'])==item['sha256']
    assert digest(ROOT/value['probe']['path'])==value['probe']['sha256']
    return value


def main():
    pipeline=read(ROOT/'.local/squall-loader-regression-pipeline.json')
    assert pipeline['status']=='passed_both_builds_full102_loader_fpu_preemption_pointers_and_core_audio_pipeline_only'
    assert pipeline['driver_sha256']==digest(ROOT/'tools/run_squall_loader_regression.py') and len(pipeline['steps'])==18
    for step in pipeline['steps']:assert step['return_code']==0 and digest(ROOT/step['log'])==step['log_sha256']
    prior_name='analysis/squall-pointer-regression.json';prior=read(ROOT/prior_name)
    names=subprocess.check_output(['git','ls-files','src/game','src/squall'],cwd=ROOT,text=True,encoding='utf8').splitlines()
    current={n:digest(ROOT/n) for n in names};added=set(current)-set(prior['production_source_files'])
    assert added=={'src/squall/runtime/Initialization.cpp'}
    changed=[n for n in current if n in prior['production_source_files'] and current[n]!=prior['production_source_files'][n]]
    assert changed==['src/squall/runtime/SoundEngine/Context.cpp']
    fpu=checked_runtime(ROOT/'.local/squall-loader-fpu-validation.json','passed_actual_DLL_load_unload_x87_and_sse_environments_only')
    preempt=checked_runtime(ROOT/'.local/squall-loader-preemption-validation.json','passed_actual_full_channel_preemption_and_priority_fallback_only')
    pointers=checked_runtime(ROOT/'.local/squall-loader-pointers-validation.json','passed_actual_isolated_3d_cone_pointer_outcomes_only')
    assert (fpu['isolated_processes'],fpu['actual_load_unload_cycles'],len(fpu['cases']))==(72,144,24)
    assert (preempt['isolated_processes'],len(preempt['cases']))==(528,176)
    assert (pointers['isolated_processes'],len(pointers['cases']))==(270,90)
    for case in preempt['cases']:
        assert all(v['loaded_precision_bits']==0x10000 for v in case['outcomes'].values())
    reports={'squall-loader-fpu-validation.json':fpu,'squall-loader-preemption-validation.json':preempt,'squall-loader-pointers-validation.json':pointers}
    hashes=[fpu['binaries'][key]['sha256'] for key in ['source','fresh']];abi_names=[]
    assert [preempt['binaries'][key]['sha256'] for key in ['source','fresh']]==hashes
    assert [pointers['binaries'][key]['sha256'] for key in ['source','fresh']]==hashes
    for suffix,index in [('',0),('-fresh',1)]:
        for stem in ['abi','play','stack','dll','game-audio','person-sounds']:
            name=f'squall-loader-{stem}{suffix}-validation.json';value=read(ROOT/'.local'/name)
            if stem=='abi':
                assert value['status']=='matches_export_contract' and value['rebuilt_sha256']==hashes[index]
                abi_names.append('analysis/'+name)
            else:
                check(value);assert value['status'].startswith('passed')
                assert value.get('rebuilt_sha256',value.get('candidate_sha256'))==hashes[index]
            if stem=='play':assert all(r['actual_play_cases']==387 and r['result_comparisons']==6120 for r in value['precision_contexts'])
            elif stem=='stack':assert len(value['native_return_paths'])==97 and sum(len(r['return_addresses']) for r in value['native_return_paths'])==223
            elif stem=='dll':assert sum(r['dll_pcm_bytes'] for r in value['precision_contexts'])==122527752
            reports[name]=value
    suite=ET.parse(ROOT/'.local/squall-loader-ctest.xml').getroot();tests=suite.findall('.//testcase')
    assert len(tests)==102 and not any(suite.findall('.//'+k) for k in ['failure','error','skipped'])
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    for item in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/item['path'])==item['sha256']
    before=read(ROOT/'analysis/squall-load-fpu-before.json');assert len(before['deviations'])==42
    before_preempt=read(ROOT/'analysis/squall-preemption-before.json');assert len(before_preempt['deviations'])==176
    for name,value in reports.items():write(ROOT/'analysis'/name,value)
    report_name='analysis/squall-loader-regression.json'
    result=dict(status='passed_native_FP_startup_preemption_both_builds_new_full102_and_core_audio_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        base_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        production_source_files=current,added_production_files=sorted(added),changed_production_files=changed,
        build_support_files={n:digest(ROOT/n) for n in ['CMakeLists.txt','tools/cmake/SquallCandidate.cmake']},
        candidate_sha256=hashes,abi_reports=abi_names,prior_full_regression=prior_name,tests_passed=102,tests_failed=0,
        sum_test_seconds=sum(float(t.get('time','0')) for t in tests),junit_sha256=digest(ROOT/'.local/squall-loader-ctest.xml'),
        pipeline=pipeline,pipeline_report_sha256=digest(ROOT/'.local/squall-loader-regression-pipeline.json'),
        reports={n:digest(ROOT/'analysis'/n) for n in reports},
        resolved_loader_environment_case_pairs=42,resolved_preemption_loader_precision_pairs=176,
        isolated_fpu_processes=72,actual_load_unload_cycles=144,actual_preemption_processes=528,
        retained_actual_pointer_processes=270,all45_game_sound_pcm_bytes_per_build=122527752,actual_play_cases_per_build=774,
        ordinary_preemption_behavior_already_matched_before_source_fallback_definition=True,
        original_files_unchanged=17,original_program_code_used_in_implementation=False,whole_game_complete=False,whole_squall_complete=False,
        pending=['Current whole-game restores after this DLL change','Known JNGLoad load-time sticky-flag difference','Actual40-contract threshold and remaining original goal scopes'])
    write(ROOT/report_name,result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json')
    methods=['Restore native pre-CPP x87 initialization and preserve SSE loader environment','Define original incoming-priority fallback index']
    provenance['instruction_backed_patches']=[p for p in provenance['instruction_backed_patches'] if p.get('method') not in methods]
    for path,method,addresses in [('Initialization.cpp',methods[0],['1003e814','1003cf10','1003eacb','1003cf24']),
        ('SoundEngine/Context.cpp',methods[1],['100024dc','10002504'])]:
        provenance['instruction_backed_patches'].append(dict(path=path,method=method,original_addresses=addresses,
            current_sha256=digest(ROOT/'src/squall/runtime'/path),verification_report=report_name))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json')
    for n in ['analysis/'+n for n in reports]+[report_name]:
        if n not in verification['reports']:verification['reports'].append(n)
    verification['tests'].update(ctest_passed=102,last_full_ctest_report=report_name,full_ctest_repeated_for_native_DLL_FP_startup=True)
    verification['squall_loader_fpu']=dict(report=report_name,resolved=True,processes=72,load_unload_cycles=144,remaining_environment_differences=0)
    verification['latest_scoped_regression']=dict(report=report_name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    values=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(values).encode('utf8')).hexdigest()
    verification['source_tree_hash_as_of_report']=report_name;write(ROOT/'analysis/verification.json',verification)
    audit=read(ROOT/'analysis/squall-export-audit.json');audit['rebuilt_sha256']=hashes[0];audit['current_contract_reports']=abi_names
    audit['loader_environment_reports']=['analysis/squall-loader-fpu-validation.json']
    for row in audit['exports']:
        if row['name'].startswith(('SQUALL_Sample_Play','SQUALL_SampleGroup_Play')):
            row['loaded_DLL_preemption_scope']='Both builds,PC53/PC24;four prepared occupied channels,ascending and tied priorities;all8 public play methods;incoming-priority fallback and last-minimum selection;528 exact isolated processes'
    write(ROOT/'analysis/squall-export-audit.json',audit)
    print('Recorded current102/102,both DLLs,72 exact loader processes,528 preemptions,270 pointer processes and full game audio scope.')


if __name__=='__main__':main()
