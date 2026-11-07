#!/usr/bin/env python3
"""Record native pre-constructor FNCLEX, exact asset pixels and error ABI."""
import datetime
import struct
from record_live_production_verification import ROOT,digest,read,write
from current_production_regression import current_regression
from pe import PE


def main():
    pipeline=read(ROOT/'.local/jng-loader-regression-pipeline.json')
    assert pipeline['status']=='passed_JNG_loader72_all416_pixels_two_builds_and40_API_pairs_per_build_only'
    assert pipeline['driver_sha256']==digest(ROOT/'tools/run_jng_loader_regression.py') and len(pipeline['steps'])==7
    for step in pipeline['steps']:assert step['return_code']==0 and digest(ROOT/step['log'])==step['log_sha256']
    for name,sha in pipeline['artifacts'].items():assert digest(ROOT/name)==sha,name
    before=read(ROOT/'.local/jng-load-fpu-before.json');assert len(before['deviations'])==24
    oldwork=ROOT/before['work_directory']
    for name,sha in before['source_files'].items():assert digest(oldwork/'snapshots'/name)==sha,name
    for backend in ['native','source','fresh']:assert digest(oldwork/(backend+'.dll'))==before['binaries'][backend]['sha256']
    for row in before['deviations']:
        assert row['sticky']==1
        assert all(d['field']=='status' and d['native']==0 and d['candidate']==4 for d in row['differences'])
    fpu=read(ROOT/'.local/jng-loader-fpu-validation.json');assert fpu['status']=='passed_actual_DLL_load_unload_x87_and_sse_environments_only'
    assert not fpu['deviations'] and fpu['isolated_processes']==72 and fpu['actual_load_unload_cycles']==144
    work=ROOT/fpu['work_directory']
    for name,sha in fpu['source_files'].items():assert digest(ROOT/name)==sha,name
    for name,sha in fpu['logs'].items():assert digest(work/name)==sha,name
    for row in fpu['cases']:
        for value in row['outcomes'].values():
            for cycle in value['cycles']:
                assert cycle['loaded']==cycle['unloaded']==dict(control=row['control'],status=0,mxcsr=cycle['before']['mxcsr'])
    reports={'jng-loader-fpu-before.json':before,'jng-loader-fpu-validation.json':fpu};hashes=[]
    for suffix,backend in [('', 'source'),('-fresh','fresh')]:
        sha=fpu['binaries'][backend]['sha256'];hashes.append(sha)
        for stem in ['abi','pixels','errors']:
            name=f'jng-loader-{stem}{suffix}-validation.json';v=read(ROOT/'.local'/name)
            if stem=='abi':assert v['status']=='matches_export_contract' and v['rebuilt_sha256']==sha and len(v['exports'])==2
            elif stem=='pixels':assert v['original_dll_compared'] and v['images']==v['compared_images']==416 and v['failed']==0
            else:
                assert v['status']=='passed_for_actual_jng_error_abi_png_and_selected_mng_inputs_only' and v['source_dll_sha256']==sha
                assert v['comparison']['case_process_pairs']==40 and v['comparison']['all_events_equal'] and v['comparison']['all_valid_pixels_equal']
                for name2,sha2 in v['source_files'].items():assert digest(ROOT/name2)==sha2,name2
                for case in v['cases']:
                    assert case['events_equal'] and case['valid_pixels_equal']
            reports[name]=v
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159';pe=PE(original/'JNGLoad.dll')
    assert pe.entry_rva==0x3ae44
    witnesses=[]
    for address,data in [(0x1003ad90,'6a00e81e020000'),(0x1003afb5,'a13842051085c07407ff742404ffd059'),
        (0x1003a8a2,'e8c3ffffffe823420000837c240400a3ec5305107405e8c0410000dbe2c3'),(0x10054238,'a2a80310')]:
        expected=bytes.fromhex(data);offset=pe.offset(address-pe.image_base,len(expected));assert pe.data[offset:offset+len(expected)]==expected
        witnesses.append(dict(address=f'{address:08x}',bytes_hex=data))
    assert digest(ROOT/'src/jngload/JNGLoad.cpp')==before['source_files']['src/jngload/JNGLoad.cpp']
    full_name,full=current_regression()
    for item in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/item['path'])==item['sha256']
    for name,value in reports.items():write(ROOT/'analysis'/name,value)
    result=dict(status='passed_JNG_native_FNCLEX_startup_both_builds_all416_assets_and40_API_pairs_per_build_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        pipeline=pipeline,pipeline_sha256=digest(ROOT/'.local/jng-loader-regression-pipeline.json'),
        source_files={n:digest(ROOT/n) for n in ['src/jngload/JNGLoad.cpp','src/jngload/JNGLoad.h','src/jngload/Initialization.cpp','CMakeLists.txt']},
        native_entry='1003ae44',native_instruction_witnesses=witnesses,candidate_sha256=hashes,
        reports={'analysis/'+n:digest(ROOT/'analysis'/n) for n in reports},resolved_loader_case_pairs=24,
        actual_loader_processes=72,actual_load_unload_cycles=144,game_images_per_build=416,actual_error_API_process_pairs_per_build=40,
        wrapper_decode_body_unchanged=True,new_full_ctest_run=False,unchanged_game_and_squall_full102_report=full_name,
        build_logs={n:digest(ROOT/'.local'/n) for n in ['jng-loader-main-final-build.log','jng-loader-fresh-final-build.log']},
        original_files_unchanged=17,original_program_code_used_in_implementation=False,whole_game_complete=False,
        pending=['Current integrated contract40 and whole-game restores','General malformed inputs,allocation failure,concurrency and unmasked FP input scopes remain explicitly unverified'])
    name='analysis/jng-loader-regression.json';write(ROOT/name,result)
    v=read(ROOT/'analysis/verification.json')
    for n in ['analysis/'+n for n in reports]+[name]:
        if n not in v['reports']:v['reports'].append(n)
    v['jng_loader_fpu']=dict(report=name,resolved=True,processes=72,load_unload_cycles=144,remaining_environment_differences=0)
    v['latest_scoped_regression']=dict(report=name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',v)
    print('Verified native JNG FNCLEX,72 processes,144 cycles,416 exact assets and40 API pairs in each build.')


if __name__=='__main__':main()
