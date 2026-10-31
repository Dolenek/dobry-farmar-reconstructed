#!/usr/bin/env python3
"""Record tested JNG error/exception and selected PNG/MNG behavior without generalizing it."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write

def main():
    reports={};runs=[]
    for label,file in [('main','jng-error-abi-validation.json'),('independent','jng-error-abi-fresh-validation.json')]:
        report=read(ROOT/'.local'/file);runs.append(report)
        assert report['status']=='passed_for_actual_jng_error_abi_png_and_selected_mng_inputs_only'
        for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
        assert digest(ROOT/report['probe_path'])==report['probe_sha256']
        assert digest(ROOT/report['source_dll_path'])==report['source_dll_sha256']
        assert report['original_files_unchanged']==17 and report['comparison']['case_process_pairs']==40
        assert report['comparison']['all_events_equal'] and report['comparison']['all_valid_pixels_equal']
        for case in report['cases']:
            assert case['events_equal'] and case['valid_pixels_equal']
            for fixture in case['inputs']:assert digest(ROOT/report['work_directory']/(fixture['name']+'.png'))==fixture['sha256']
            for run in case['runs']:
                folder=ROOT/report['work_directory']/(case['name']+'-'+run['kind'])
                events=[json.loads(line) for line in (folder/'stdout.jsonl').read_text(encoding='utf-8').splitlines()]
                assert events==run['events']
                for name,value in run['pixels'].items():assert digest(folder/name)==value
        write(ROOT/'analysis'/file,report);reports[file]=digest(ROOT/'analysis'/file)
    assert [c['runs'][1]['events'] for c in runs[0]['cases']]==[c['runs'][1]['events'] for c in runs[1]['cases']]
    exports=read(ROOT/'.local/jng-error-abi-exports.json');assert exports['status']=='matches_export_contract' and len(exports['exports'])==2
    write(ROOT/'analysis/jng-error-abi-exports.json',exports);reports['jng-error-abi-exports.json']=digest(ROOT/'analysis/jng-error-abi-exports.json')
    prior=read(ROOT/'analysis/profile-floating-regression.json')
    changed={name:[value,digest(ROOT/name)] for name,value in prior['production_source_files'].items() if digest(ROOT/name)!=value}
    assert set(changed)<= {'CMakeLists.txt'},changed
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_jng_error_exception_abi_and_selected_png_mng_inputs_in_two_win32_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        actual_process_pairs_per_build=40,independent_builds=2,export_names_and_ordinals_equal=True,original_const_char_pointer_exception_caught=True,
        error_outputs_preserve_caller_refs=True,free_null_and_nonnull_pointer_preservation=True,protected_truncation_cases_per_build=7,
        crc_fixtures_accepted_by_both=True,zero_width_accepted_by_both=True,mng_delay20_returns_first_frame=True,mng_delay0_returns_last_frame=True,
        selected_valid_raw_pixels_equal=True,all_invalid_pixels_claimed_equal=False,valid_error_valid_and_cross_format_sequence=True,
        production_implementation_changed=False,previous_full_ctest_report='analysis/profile-floating-regression.json',changes_since_full_ctest=changed,build_configuration_change='Adds only EXCLUDE_FROM_ALL jng-error-probe diagnostic target; production sources and binaries unchanged',
        build_log_sha256={name:digest(ROOT/'.local'/name) for name in ['jng-error-probe-build.log','jng-error-probe-fresh-build.log']},
        original_files_unchanged=17,whole_game_complete=False,pending=['Allocation/registration failure injection and general exception ABI','General MNG scripting, animation/callback timing and concurrent calls','Remaining Squall, game and isolated uninstaller scopes'])
    write(ROOT/'analysis/jng-error-abi-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['jng-error-abi-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/jng-error-abi-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded40 process pairs in each Win32 build, selected C++ exception ABI, guarded errors, exact valid pixels and unchanged originals')
if __name__=='__main__':main()
