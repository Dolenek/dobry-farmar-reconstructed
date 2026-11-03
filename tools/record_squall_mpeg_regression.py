#!/usr/bin/env python3
"""Verify current whole-game/audio regression after the instruction-backed MPEG repair."""
import datetime,json,re,subprocess,xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write,checked as checked_game
from record_squall_pcm_eof_verification import check
from record_squall_mpeg_scope import checked as checked_mpeg

def main():
    scope=read(ROOT/'analysis/squall-mpeg-scoped-reconstruction.json');hashes=scope['candidate_sha256'];reports={}
    for name,value in scope['reports'].items():assert digest(ROOT/'analysis'/name)==value,name
    for suffix in ['', '-fresh']:
        checked_mpeg(f'squall-mpeg-safe-formats{suffix}-validation.json');checked_mpeg(f'squall-mpeg-kernels{suffix}-validation.json')
    for suffix,index in [('',0),('-fresh',1)]:
        expected=hashes[index]
        for stem in ['pcm-eof','wave-formats','dll','game-audio','person-sounds']:
            name=f'squall-mpeg-{stem}{suffix}-validation.json';report=read(ROOT/'.local'/name);check(report)
            assert report.get('rebuilt_sha256',report.get('candidate_sha256'))==expected,name
            if stem=='pcm-eof':assert report['same_address_cases']==216 and report['source_mismatches']==0 and report['status']=='passed_native_pcm_eof_saved_this_and_start_argument_bytes_at_same_addresses_only'
            elif stem=='wave-formats':
                assert report['guarded_pcm_reads']==1952 and report['physical_buffers']==192 and report['result_comparisons']==2530 and not report['genuinely_uninitialized_pcm8_eof_claimed_equal']
                for row in report['precision_contexts']:assert row['exact_equal_pcm_bytes']==62428 and row['exact_defined_pcm_and_guards_fnv1a64']=='cd64397592471269'
            elif stem=='dll':assert sum(row['dll_pcm_bytes'] for row in report['precision_contexts'])==122527752 and report['physical_pcm_bytes']==564480
            elif stem=='game-audio':assert sum(row['state_comparisons'] for row in report['precision_contexts'])==378
            elif stem=='person-sounds':assert report['states']==332 and report['playback_progress_checks']==40 and report['paused_cursor_checks']==8
            reports[name]=report
        name=f'squall-mpeg{suffix}-abi.json';report=read(ROOT/'.local'/name);assert report['status']=='matches_export_contract' and report['rebuilt_sha256']==expected and len(report['exports'])==97;reports[name]=report
    game=read(ROOT/'.local/squall-mpeg-game-interop-validation.json');work=checked_game(game)
    assert game['status']=='passed_for_both_actual_crop_disaster_outcomes_bidirectional_saves_only' and game['implementation_files']['Squall.dll']==hashes[0]
    sizes=[row['identical_complete_save_bytes'] for row in game['comparison']['transfer_comparisons']];assert sizes==[6943,7784,6943,6943]
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        events=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()];assert events[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in events)
        traces[path.name]=dict(sha256=digest(path),events=events)
    assert len(traces)==2;reports['squall-mpeg-game-interop-validation.json']=game
    failed_path=ROOT/'.local/field-disasters-interop/20261103T210325996856Z/report.json';failed=read(failed_path)
    assert failed['status']=='failed' and 'Injected pointer missed owned window target' in failed['error'] and failed['implementation_files']==game['implementation_files']
    failed_work=failed_path.parent
    for name,value in failed['screenshots'].items():assert digest(failed_work/name)==value,name
    reports['squall-mpeg-game-interop-initial-input-failure.json']=dict(status='test_input_missed_before_native_Start_unchanged_implementation_passed_full_retry',work_directory=failed_work.relative_to(ROOT).as_posix(),error=failed['error'],report_sha256=digest(failed_path),implementation_files=failed['implementation_files'],screenshots=failed['screenshots'],completed_source_restore_before_failure=True,retry_report='analysis/squall-mpeg-game-interop-validation.json',retry_log_sha256=digest(ROOT/'.local/squall-mpeg-game-interop-retry-run.log'),production_changed_between_attempts=False)
    junit=ROOT/'.local/squall-mpeg-ctest.xml'
    if not junit.exists():
        source=ROOT/'build/windows/.local/squall-mpeg-ctest.xml';assert source.exists();junit.write_bytes(source.read_bytes())
    suite=ET.parse(junit).getroot();cases=suite.findall('.//testcase');assert len(cases)==102 and not any(suite.findall('.//'+kind) for kind in ['failure','error','skipped'])
    log=ROOT/'.local/squall-mpeg-ctest.log';text=log.read_text(encoding='utf-8-sig');assert '100% tests passed out of 102' in text
    elapsed=float(re.search(r'Total Test time \(real\) =\s*([\d.]+) sec',text)[1])
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path'])==row['sha256']
    for name,report in reports.items():write(ROOT/'analysis'/name,report)
    names=subprocess.check_output(['git','ls-files','src/game','src/squall'],cwd=ROOT,text=True,encoding='utf-8').splitlines();current={name:digest(ROOT/name) for name in names}
    for name,value in scope['production_source_files'].items():assert current[name]==value,name
    result=dict(status='passed_current_mpeg_repair_both_builds_full102_regression_audio_and_four_whole_game_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),base_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),production_source_files=current,implementation_files=game['implementation_files'],candidate_sha256=hashes,
        tests_passed=102,tests_failed=0,ctest_elapsed_seconds=elapsed,junit_sha256=digest(junit),ctest_log_sha256=digest(log),kernel_cases_per_build=40960,guarded_mpeg_reads_per_build=640,exact_returned_mpeg_pcm_bytes_per_build=4421736,actual_safe_mpeg_physical_buffers_per_build=56,exact_mpeg_physical_pcm_bytes_per_build=1690416,
        corrected_same_address_pcm_eof_cases_per_build=216,guarded_wave_reads_per_build=1952,actual_wave_physical_buffers_per_build=192,exact_defined_wave_pcm_bytes_per_build=124856,all_45_game_sound_pcm_bytes_per_build=122527752,source_restore_debugger_traces=traces,whole_game_bidirectional_transfers=4,complete_identical_save_bytes=sizes,
        reports={**{name:digest(ROOT/'analysis'/name) for name in reports},'squall-mpeg-scoped-reconstruction.json':digest(ROOT/'analysis/squall-mpeg-scoped-reconstruction.json')},build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-mpeg-main-build.log','squall-mpeg-fresh-build.log']},pipeline_logs={path.name:digest(path) for path in (ROOT/'.local').glob('squall-mpeg-regression-[0-9][0-9].log')},original_files_unchanged=17,original_program_binaries_used_in_implementation=False,whole_squall_complete=False,whole_game_complete=False,
        pending=['Native MPEG2 Layer2 unsafe table/output behavior is separately proved; unsafe real playback is not claimed verified','MPEG layer1 and broader malformed inputs,WMA/tracker and other remaining97-export behavior','Later natural world progression and remaining JNG/uninstaller original-goal requirements'])
    result['pipeline_summary_log_sha256']=digest(ROOT/'.local/squall-mpeg-regression-pipeline.log')
    assert len(result['pipeline_logs'])==13
    write(ROOT/'analysis/squall-mpeg-regression.json',result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json')
    paths=['SoundFile/MP3/MpegDecoder.cpp','SoundFile/MP3/l3dec.cpp','SoundFile/MP3/NativeMpegMath.h']
    provenance['instruction_backed_patches']=[row for row in provenance['instruction_backed_patches'] if row.get('path') not in paths]
    for path in paths:provenance['instruction_backed_patches'].append(dict(path=path,method='MPEG x87 tables,IMDCT18/6,window accumulation and dequant order',original_addresses=['1000abd0','1000acc0','1000ad70','1000ba00','1000c350','1000c5cc','1000c798','1000e8f0','1000ea50','1000eee0','10010cf0','10010e60','1003cf60','1003f3b0'],instruction_evidence='analysis/squall-mpeg-native-instructions.txt',source_difference='Preserve original transcendental instructions,scalar register lifetimes,float spills and arithmetic order; finite internal MPEG math domain,not a generic CRT replacement.',current_sha256=digest(ROOT/'src/squall/runtime'/path),verification_report='analysis/squall-mpeg-regression.json'))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json');verification['recorded_at_utc']=result['recorded_at_utc']
    for name in list(reports)+['squall-mpeg-scoped-reconstruction.json','squall-mpeg-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-mpeg-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False);verification['tests'].update(ctest_passed=102,last_full_ctest_report='analysis/squall-mpeg-regression.json')
    if 'analysis/squall-mpeg-game-interop-validation.json' not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/squall-mpeg-game-interop-validation.json')
    verification['full_reconstruction_pending']=['Remaining full-game systems and later natural progression; main source EXE already builds','Complete Squall semantics,remaining MPEG/WMA/tracker/device cases','Remaining general JNG/MNG,concurrency and allocation/error scopes','Remaining isolated uninstaller reboot and host-launch/window verification']
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded full current102/102,MPEG/PCM/game audio on both builds and four exact whole-game restores.')
if __name__=='__main__':main()
