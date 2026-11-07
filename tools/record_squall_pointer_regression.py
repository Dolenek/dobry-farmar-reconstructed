#!/usr/bin/env python3
"""Record the narrow ordered3D repair, both DLLs and a new complete102 run."""
import datetime
import hashlib
import subprocess
import xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write
from record_squall_pcm_eof_verification import check

CHANGED='src/squall/runtime/SoundEngine/Squall.cpp'


def main():
    pipeline=read(ROOT/'.local/squall-pointer-regression-pipeline.json')
    assert pipeline['status']=='passed_both_builds_full102_pointer_play_stack_pcm_and_game_audio_pipeline_only'
    driver_snapshot=ROOT/'.local/squall-pointer-resume/driver-executed-resume.py'
    assert pipeline['driver_sha256']==digest(driver_snapshot)
    assert driver_snapshot.read_bytes().replace(b'\r\n',b'\n')==(ROOT/'tools/run_squall_pointer_regression.py').read_bytes().replace(b'\r\n',b'\n')
    assert len(pipeline['steps'])==17
    for step in pipeline['steps']:
        assert step['return_code']==0 and digest(ROOT/step['log'])==step['log_sha256']
    before=read(ROOT/'analysis/squall-pointer-faults-before.json')
    assert len(before['deviations'])==64 and before['isolated_processes']==270
    old=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    names=subprocess.check_output(['git','ls-files','src/game','src/squall'],cwd=ROOT,text=True,encoding='utf8').splitlines()
    current={name:digest(ROOT/name) for name in names}
    assert set(current)==set(old['production_source_files'])
    assert [p for p,sha in current.items() if sha!=old['production_source_files'][p]]==[CHANGED]
    raw=read(ROOT/'.local/squall-pointer-faults-validation.json');work=ROOT/raw['work_directory']
    assert raw['status']=='passed_actual_isolated_3d_cone_pointer_outcomes_only' and not raw['deviations']
    assert len(raw['cases'])==90 and raw['isolated_processes']==270
    assert raw['fault_processes']==120 and raw['normal_processes']==150
    for name,sha in raw['source_files'].items():assert digest(ROOT/name)==sha
    for name,sha in raw['logs'].items():assert digest(work/name)==sha
    for row in raw['cases']:
        for value in row['outcomes'].values():
            assert value['before_precision_bits']==value['after_precision_bits']==(0x20000 if row['precision']==24 else 0x10000)
            if value['exception']:assert value['fault_address']==(0 if row['method']==4 else 8)
    reports={'squall-pointer-faults-validation.json':raw};abi_names=[];hashes=[]
    for suffix,key in [('', 'source'),('-fresh','fresh')]:
        sha=raw['binaries'][key]['sha256'];hashes.append(sha)
        for stem in ['abi','play','stack','dll','game-audio','person-sounds']:
            name=f'squall-pointer-{stem}{suffix}-validation.json';value=read(ROOT/'.local'/name)
            if stem=='abi':
                assert value['status']=='matches_export_contract' and value['rebuilt_sha256']==sha
                abi_names.append('analysis/'+name)
            else:
                check(value);assert value['status'].startswith('passed')
                assert value.get('rebuilt_sha256',value.get('candidate_sha256'))==sha,name
            if stem=='play':
                assert len(value['precision_contexts'])==2
                assert all(r['actual_play_cases']==387 and r['guarded_channel_snapshots']==418 and r['result_comparisons']==6120 for r in value['precision_contexts'])
            elif stem=='stack':
                assert len(value['native_return_paths'])==97
                assert sum(len(r['return_addresses']) for r in value['native_return_paths'])==223
            elif stem=='dll':
                assert len(value['precision_contexts'])==2
                assert sum(r['dll_pcm_bytes'] for r in value['precision_contexts'])==122527752
            reports[name]=value
    junit=ROOT/'.local/squall-pointer-ctest.xml';suite=ET.parse(junit).getroot();tests=suite.findall('.//testcase')
    assert len(tests)==102 and not any(suite.findall('.//'+kind) for kind in ['failure','error','skipped'])
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/row['path'])==row['sha256']
    for name,value in reports.items():write(ROOT/'analysis'/name,value)
    report_name='analysis/squall-pointer-regression.json'
    result=dict(status='passed_ordered3d_reads_both_builds_new_full102_and_actual_public_pointer_play_audio_scope_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        base_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        production_source_files=current,candidate_sha256=hashes,changed_production_files=[CHANGED],
        prior_full_regression='analysis/squall-tracker-loaders-regression.json',
        tests_passed=102,tests_failed=0,sum_test_seconds=sum(float(t.get('time','0')) for t in tests),
        junit_sha256=digest(junit),pipeline=pipeline,pipeline_report_sha256=digest(ROOT/'.local/squall-pointer-regression-pipeline.json'),
        executed_driver_snapshot=dict(path=driver_snapshot.relative_to(ROOT).as_posix(),sha256=digest(driver_snapshot),only_subsequent_difference='Canonical LF line endings'),
        auditor_observation=dict(failed_runtime_report='.local/squall-pointer-faults/20261106T201034530041Z/report.json',
            failed_runtime_report_sha256=digest(ROOT/'.local/squall-pointer-faults/20261106T201034530041Z/report.json'),
            cause='Python audit expected wrong CRT precision constants;native C++ probe already measured correct values',
            correct_crt_precision_bits=dict(PC24=0x20000,PC53=0x10000),production_change_for_auditor_error=False),
        abi_reports=abi_names,all45_game_sound_pcm_bytes_per_build=122527752,
        actual_sample_play_cases_per_build=774,actual_pointer_processes=270,resolved_fault_byte_deviations=64,
        fault_processes_explicitly_terminated=120,normal_pointer_processes=150,
        reports={name:digest(ROOT/'analysis'/name) for name in reports},
        original_files_unchanged=17,original_program_binaries_used_in_implementation=False,
        whole_squall_complete=False,whole_game_complete=False,
        pending=['Current whole-game saves after this DLL repair','Continue actual40-contract award and final all-awards condition',
            'Other malformed/device/concurrent paths and remaining original goal scopes'])
    write(ROOT/report_name,result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json')
    provenance['instruction_backed_patches']=[p for p in provenance['instruction_backed_patches'] if p.get('method')!='Preserve original observable Position read order in CreateChannel3D']
    provenance['instruction_backed_patches'].append(dict(path='SoundEngine/Squall.cpp',
        method='Preserve original observable Position read order in CreateChannel3D',
        original_addresses=['10005021','10005039','10005053'],
        source_difference='Three ordered volatile binary32 input reads Z/Y/X;subsequent distance arithmetic retained',
        current_sha256=current[CHANGED],verification_report=report_name))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json')
    for name in ['analysis/'+n for n in reports]+[report_name]:
        if name not in verification['reports']:verification['reports'].append(name)
    verification['tests'].update(ctest_passed=102,last_full_ctest_report=report_name,full_ctest_repeated_for_ordered3d_reads=True)
    verification['squall_pointer_faults']=dict(report=report_name,resolved=True,actual_processes=270,observed_deviations=0,whole_behavior_complete=False)
    verification['latest_scoped_regression']=dict(report=report_name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    values=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(values).encode('utf8')).hexdigest()
    verification['source_tree_hash_as_of_report']=report_name;write(ROOT/'analysis/verification.json',verification)
    audit=read(ROOT/'analysis/squall-export-audit.json');audit['rebuilt_sha256']=hashes[0];audit['current_contract_reports']=abi_names
    for row in audit['exports']:
        if row['name'] in ['SQUALL_Sample_Play3D','SQUALL_Sample_Play3DEx','SQUALL_SampleGroup_Play3D','SQUALL_SampleGroup_Play3DEx','SQUALL_Channel_GetConeParameters']:
            row['loaded_DLL_pointer_fault_scope']='Both builds,PC53/PC24;actual null/bad/guard/extent/readonly3D inputs and cone outputs;exact fault byte,access,exception and remaining output words;owned fault probes explicitly terminated'
    write(ROOT/'analysis/squall-export-audit.json',audit)
    print('Recorded ordered3D repair,current102/102,both full DLL play/audio/ABI scopes and270 exact pointer processes;whole goal remains active.')


if __name__=='__main__':main()
