#!/usr/bin/env python3
"""Record native PCM saved-register/argument repair and complete current regression."""
import datetime,json,re,subprocess,xml.etree.ElementTree as ET
from pe import PE
from record_live_production_verification import ROOT,digest,read,write,checked as checked_game

PCM='src/squall/runtime/SoundFile/WAV/PcmDecoder.cpp'
INITIAL=ROOT/'.local/squall-wave-formats/20261101T182037416422Z'


def check(report,historical=False):
    rows=report['source_files'];rows=rows.items() if isinstance(rows,dict) else [(r['path'],r['sha256']) for r in rows]
    for name,value in rows:
        path=INITIAL/'source-snapshots'/name if historical and name==PCM else ROOT/name
        assert digest(path)==value,name
    for key,hash_key in [('candidate_path','rebuilt_sha256'),('oracle_path','oracle_sha256')]:
        if key in report:assert digest(ROOT/report[key])==report.get(hash_key,report.get('candidate_sha256')),key
    if 'source_map' in report:assert digest(ROOT/report['source_map']['path'])==report['source_map']['sha256']
    if 'work_directory' in report:
        work=ROOT/report['work_directory']
        for group in ['logs','fixture_files']:
            for name,value in report.get(group,{}).items():assert digest(work/name)==value,name


def main():
    reports={};hashes=[]
    before=read(ROOT/'.local/squall-pcm-eof-before-validation.json');check(before,True)
    assert before['status']=='observed_old_source_pcm_eof_mismatches_at_same_addresses' and before['same_address_cases']==before['source_mismatches']==216
    reports['squall-pcm-eof-before-validation.json']=before
    for suffix in ['', '-fresh']:
        name=f'squall-pcm-eof{suffix}-validation.json';report=read(ROOT/'.local'/name);check(report)
        assert report['status']=='passed_native_pcm_eof_saved_this_and_start_argument_bytes_at_same_addresses_only'
        assert report['same_address_cases']==216 and report['source_mismatches']==0
        for context in report['precision_contexts']:
            assert context['cases']==108 and context['native_expected_model_verified']
            for row in context['observations']:assert row['source_return']==row['native_return'] and row['source_hex']==row['native_hex']==row['expected_hex']
        hashes.append(report['rebuilt_sha256']);reports[name]=report
        name=f'squall-wave-formats{suffix}-validation.json';wave=read(ROOT/'.local'/name);check(wave)
        assert wave['status']=='passed_for_actual_nonzero_pcm_ima_msadpcm_wav_decode_seek_loop_and_physical_buffers_only'
        assert wave['rebuilt_sha256']==hashes[-1] and wave['result_comparisons']==2530 and wave['guarded_pcm_reads']==1952 and wave['physical_buffers']==192
        assert len(wave['fixtures'])==24 and not wave['all_raw_pcm_outputs_claimed_equal'] and not wave['genuinely_uninitialized_pcm8_eof_claimed_equal']
        for context in wave['precision_contexts']:
            assert context['exact_equal_pcm_bytes']==62428 and context['physical_pcm_bytes']==21096 and len(context['eof_observations'])==24
            assert context['exact_defined_pcm_and_guards_fnv1a64']=='cd64397592471269'
            for kind in ['saved_this_high_byte','saved_this_high_word','start_argument_word_reuse','genuinely_uninitialized_stack_bytes_not_claimed_equal']:
                assert sum(row['kind']==kind for row in context['eof_observations'])==6
            for row in context['eof_observations']:
                if row['kind']=='start_argument_word_reuse':assert row['observed_equal'] and row['source_raw_hex']==row['native_raw_hex']
        reports[name]=wave
        for stem in ['dll','game-audio','person-sounds']:
            name=f'squall-pcm-eof-{stem}{suffix}-validation.json';value=read(ROOT/'.local'/name);check(value)
            assert value.get('rebuilt_sha256',value.get('candidate_sha256'))==hashes[-1]
            if stem=='dll':assert sum(row['dll_pcm_bytes'] for row in value['precision_contexts'])==122527752 and value['physical_pcm_bytes']==564480
            if stem=='game-audio':assert sum(row['state_comparisons'] for row in value['precision_contexts'])==378
            if stem=='person-sounds':assert value['states']==332 and value['playback_progress_checks']==40 and value['paused_cursor_checks']==8
            reports[name]=value
        name=f'squall-pcm-eof{suffix}-abi.json';abi=read(ROOT/'.local'/name)
        assert abi['status']=='matches_export_contract' and abi['rebuilt_sha256']==hashes[-1] and len(abi['exports'])==97;reports[name]=abi
    game=read(ROOT/'.local/squall-pcm-eof-game-interop-validation.json');work=checked_game(game)
    assert game['status']=='passed_for_both_actual_crop_disaster_outcomes_bidirectional_saves_only' and game['implementation_files']['Squall.dll']==hashes[0]
    assert [row['identical_complete_save_bytes'] for row in game['comparison']['transfer_comparisons']]==[6943,7784,6943,6943]
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2;reports['squall-pcm-eof-game-interop-validation.json']=game
    failed=read(INITIAL/'report.json');assert failed['status']=='failed' and 'channels=2 start=256' in failed['error']
    snapshots=read(INITIAL/'source-snapshots.json')
    for name,row in snapshots.items():assert digest(ROOT/row['path'])==row['sha256']==failed['source_files'][name]
    reports['squall-pcm-eof-initial-observation.json']=dict(status='actual_pcm8_mono_to_stereo_eof_difference_reproduced_and_instruction_backed_repair_verified',
        initial_report_sha256=digest(INITIAL/'report.json'),work_directory=failed['work_directory'],error=failed['error'],preserved_source_snapshots=snapshots,
        raw_outputs={name:digest(INITIAL/name) for name in ['failed-source-pcm.bin','failed-native-pcm.bin']},
        explanation='Native PUSH ECX seeds local PCM8 byte/PCM16 word with upper bits of this; PCM16 mono reuses start argument DWORD. Source C++ had unrelated uninitialized locals.',
        genuinely_uninitialized_pcm8_stereo_to_mono_eof='Separate SUB ESP,8 local remains genuinely uninitialized; raw pairs preserved, equality not claimed.',whole_game_complete=False)
    junit=ROOT/'.local/squall-pcm-eof-ctest.xml';suite=ET.parse(junit).getroot();cases=suite.findall('.//testcase')
    assert len(cases)==102 and not any(suite.findall('.//'+kind) for kind in ['failure','error','skipped'])
    log=ROOT/'.local/squall-pcm-eof-ctest.log';text=log.read_text(encoding='utf-8-sig');assert '100% tests passed out of 102' in text
    elapsed=float(re.search(r'Total Test time \(real\) =\s*([\d.]+) sec',text)[1])
    byte_comparisons=[];ranges=[]
    for suffix in ['', '-fresh']:
        candidate=ROOT/reports[f'squall-pcm-eof{suffix}-validation.json']['candidate_path'];pe=PE(candidate);data=bytearray(pe.data)
        mask=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28 and pe.u32(mask[0][0])==pe.u32(mask[1][0])
        for first,last in mask:data[first:last]=b'\0'*(last-first)
        byte_comparisons.append(data);ranges.append(mask)
    assert byte_comparisons[0]==byte_comparisons[1] and ranges[0]==ranges[1]
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    for name,value in reports.items():write(ROOT/'analysis'/name,value)
    inst=ROOT/'analysis/squall-pcm-eof-native-instructions.txt';inst.write_bytes((ROOT/'.local/squall-pcm-eof-native-instructions.txt').read_bytes())
    names=subprocess.check_output(['git','ls-files','src/game','src/squall'],cwd=ROOT,text=True,encoding='utf-8').splitlines()
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');provenance['instruction_backed_patches']=[p for p in provenance['instruction_backed_patches'] if p.get('path')!='SoundFile/WAV/PcmDecoder.cpp']
    provenance['instruction_backed_patches'].append(dict(path='SoundFile/WAV/PcmDecoder.cpp',method='PCM8/PCM16 empty/partial input bytes',original_addresses=['100149f0','10014b70','10014c70'],
        instruction_evidence='PUSH ECX local allocation retains high this byte/word; PCM16 stereo-to-mono aliases start argument DWORD after seeking.',source_difference='Public C++ used unrelated uninitialized locals; initialize corresponding bytes explicitly without changing valid reads or the genuinely uninitialized PCM8 mono averaging branch.',current_sha256=digest(ROOT/PCM)))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    result=dict(status='passed_native_pcm_eof_repair_two_builds_complete_current102_regression_and_whole_game_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        production_source_files={name:digest(ROOT/name) for name in names},implementation_files=game['implementation_files'],candidate_sha256=hashes,
        reports={name:digest(ROOT/'analysis'/name) for name in list(reports)+[inst.name]},source_restore_debugger_traces=traces,
        before_same_address_cases=216,before_source_mismatches=216,corrected_same_address_cases_per_build=216,corrected_source_mismatches=0,
        wave_fixtures=24,guarded_wave_reads_per_build=1952,physical_wave_buffers_per_build=192,exact_defined_wave_pcm_bytes_per_build=124856,physical_wave_pcm_bytes_per_build=42192,
        raw_address_dependent_and_uninitialized_outputs_claimed_equal=False,raw_outputs_normalized=False,
        tests_passed=102,tests_failed=0,ctest_elapsed_seconds=elapsed,junit_sha256=digest(junit),ctest_log_sha256=digest(log),
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-pcm-eof-main-build.log','squall-pcm-eof-fresh-build.log','squall-wave-formats-build.log','squall-pcm-eof-build.log']},
        fresh_build_comparison=dict(bytes=len(byte_comparisons[0]),only_ignored_timestamp_ranges=ranges[0],files_modified=False),
        whole_game_bidirectional_transfers=4,complete_identical_save_bytes=[6943,7784,6943,6943],original_files_unchanged=17,original_program_code_used_in_implementation=False,whole_game_complete=False,
        pending=['Genuinely uninitialized PCM8 EOF bytes and additional malformed WAV cases','MPEG/WMA/tracker and other remaining Squall scopes','Later natural game progression,remaining JNG and isolated uninstaller scopes'])
    write(ROOT/'analysis/squall-pcm-eof-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(result['reports'])+['squall-pcm-eof-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-pcm-eof-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests'].update(ctest_passed=102,last_full_ctest_report='analysis/squall-pcm-eof-regression.json')
    if 'analysis/squall-pcm-eof-game-interop-validation.json' not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/squall-pcm-eof-game-interop-validation.json')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded PCM EOF repair:216 old differences, zero corrected; both24-WAV/192-buffer audits, full102/102 and four whole-game restores.')


if __name__=='__main__':main()
