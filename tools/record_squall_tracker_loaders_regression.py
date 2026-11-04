#!/usr/bin/env python3
"""Record current full-game regression after the original tracker loader and nonzero mixer reconstruction."""
import datetime
import json
import hashlib
import re
import subprocess
import xml.etree.ElementTree as ET

from record_live_production_verification import ROOT,digest,read,write,checked as checked_game
from record_squall_pcm_eof_verification import check


def main():
    scope=read(ROOT/'analysis/squall-tracker-loaders-scoped-reconstruction.json')
    hashes=scope['candidate_sha256'];reports={}
    for path,value in scope['production_files'].items():assert digest(ROOT/path)==value,path
    for suffix,index in [('',0),('-fresh',1)]:
        expected=hashes[index]
        for stem in ['pcm-eof','wave-formats','dll','game-audio','person-sounds']:
            name=f'squall-tracker-loaders-{stem}{suffix}-validation.json';report=read(ROOT/'.local'/name);check(report)
            assert report.get('rebuilt_sha256',report.get('candidate_sha256'))==expected,name
            if stem=='pcm-eof':assert report['same_address_cases']==216 and report['source_mismatches']==0
            elif stem=='wave-formats':
                assert report['guarded_pcm_reads']==1952 and report['physical_buffers']==192 and report['result_comparisons']==2530
                assert not report['genuinely_uninitialized_pcm8_eof_claimed_equal']
            elif stem=='dll':
                assert sum(r['dll_pcm_bytes'] for r in report['precision_contexts'])==122527752
                assert report['physical_pcm_bytes']==564480
            elif stem=='game-audio':assert sum(r['state_comparisons'] for r in report['precision_contexts'])==378
            elif stem=='person-sounds':assert report['states']==332 and report['playback_progress_checks']==40
            reports[name]=report
        name=f'squall-tracker-loaders{suffix}-abi.json';report=read(ROOT/'.local'/name)
        assert report['status']=='matches_export_contract' and report['rebuilt_sha256']==expected and len(report['exports'])==97
        reports[name]=report
    game=read(ROOT/'.local/squall-tracker-loaders-game-interop-validation.json');work=checked_game(game)
    assert game['status']=='passed_for_both_actual_crop_disaster_outcomes_bidirectional_saves_only'
    assert game['implementation_files']['Squall.dll']==hashes[0]
    sizes=[r['identical_complete_save_bytes'] for r in game['comparison']['transfer_comparisons']]
    assert sizes==[6943,7784,6943,6943]
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        events=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert events[-1]==dict(event='exit',exit_code=0) and not any(r['event']=='access_violation' for r in events)
        traces[path.name]=dict(sha256=digest(path),events=events)
    assert len(traces)==2
    scope['latest_full_game_regression']='analysis/squall-tracker-loaders-regression.json'
    scope['pending']=[p for p in scope['pending'] if not p.startswith('Current whole-game regression')]
    scope['pending'].append('Remaining original whole-game/JNG/uninstaller goals')
    write(ROOT/'analysis/squall-tracker-loaders-scoped-reconstruction.json',scope)
    reports['squall-tracker-loaders-game-interop-validation.json']=game
    junit=ROOT/'.local/squall-tracker-loaders-ctest.xml';suite=ET.parse(junit).getroot()
    assert len(suite.findall('.//testcase'))==102
    assert not any(suite.findall('.//'+kind) for kind in ['failure','error','skipped'])
    log=ROOT/'.local/squall-tracker-loaders-ctest.log';text=log.read_text(encoding='utf-8-sig')
    assert '100% tests passed out of 102' in text
    elapsed=float(re.search(r'Total Test time \(real\) =\s*([\d.]+) sec',text)[1])
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    for name,report in reports.items():write(ROOT/'analysis'/name,report)
    current={name:digest(ROOT/name) for name in subprocess.check_output(['git','ls-files','src/game','src/squall'],cwd=ROOT,text=True,encoding='utf-8').splitlines()}
    result=dict(status='passed_current_tracker_loaders_and_nonzero_mixer_both_builds_full102_audio_and_four_whole_game_restores',
                recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                recorder_sha256=digest(__file__),base_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                production_source_files=current,candidate_sha256=hashes,implementation_files=game['implementation_files'],
                tests_passed=102,tests_failed=0,ctest_elapsed_seconds=elapsed,
                junit_sha256=digest(junit),ctest_log_sha256=digest(log),
                actual_agc_kernel_cases_per_build=1728,actual_nonzero_mixer_reads_per_build=960,
                recovery_steps_per_build=6400,actual_channel_stop_cases_per_build=288,
                actual_tracker_loader_order_pairs_per_build=12,safe_actual_tracker_constructor_pairs_per_build=32,
                all45_game_sound_pcm_bytes_per_build=122527752,game_audio_states_per_build=378,
                person_sound_states_per_build=332,whole_game_bidirectional_transfers=4,
                complete_identical_save_bytes=sizes,source_restore_debugger_traces=traces,
                original_files_unchanged=17,original_program_binaries_used_in_implementation=False,
                reports={**{name:digest(ROOT/'analysis'/name) for name in reports},
                         'squall-tracker-loaders-scoped-reconstruction.json':digest(ROOT/'analysis/squall-tracker-loaders-scoped-reconstruction.json')},
                build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-tracker-loaders-main-full-build.log','squall-tracker-loaders-fresh-full-build.log']},
                pipeline_logs={p.name:digest(p) for p in (ROOT/'.local').glob('squall-tracker-loaders-regression-[0-9][0-9].log')},
                whole_game_complete=False,whole_squall_complete=False,
                pending=['Broader remaining native tracker format bodies and active voice/effect cases',
                         'Native tracker factory/title invalid frees and Ogg uninitialized fallback are explicitly unsafe; no claim of general successful tracker playback',
                         'Later natural gameplay and remaining JNG/uninstaller and full export requirements'])
    assert len(result['pipeline_logs'])==13
    write(ROOT/'analysis/squall-tracker-loaders-regression.json',result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json')
    from audit_squall_tracker_loader_offsets import LOADERS
    names=sorted({row[1] for row in LOADERS if row[0] not in ['Mod','UMX']} | {'sndmix.cpp','fastmix.cpp'})
    paths=['SoundFile/track/'+name for name in names]
    provenance['instruction_backed_patches']=[r for r in provenance['instruction_backed_patches'] if r.get('path') not in paths]
    for path in paths:
        provenance['instruction_backed_patches'].append(dict(path=path,method='Original old loader fields and nonzero tracker mixer/AGC operations',
            original_addresses=[row[2] for row in LOADERS if path.endswith('/'+row[1])]+['10017fe0','10018510','100185a0','100185d0','10018670','100186d0','10018710','1002a590'],
            source_difference='Restore old field addresses for remaining loader translation units, original AGC call and limiter/recovery, negative preclip bias/full VU limits and decayed offset levels with explicit Win32 wrapping.',
            current_sha256=digest(ROOT/'src/squall/runtime'/path),verification_report='analysis/squall-tracker-loaders-regression.json'))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json');verification['recorded_at_utc']=result['recorded_at_utc']
    names=list(reports)+['squall-tracker-loaders-scoped-reconstruction.json','squall-tracker-loaders-regression.json']
    for name in names:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-tracker-loaders-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests'].update(ctest_passed=102,last_full_ctest_report='analysis/squall-tracker-loaders-regression.json')
    verification['squall-tracker-loaders-reconstruction']=dict(report='analysis/squall-tracker-loaders-scoped-reconstruction.json',complete=False,
                                                      actual_tracker_loader_and_nonzero_mixer_scopes_passed=True,unsafe_factory_open=True)
    name='analysis/squall-tracker-loaders-game-interop-validation.json'
    if name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append(name)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report']='analysis/squall-tracker-loaders-regression.json'
    verification['current_report_serialization']=dict(encoding='UTF-8',newlines='LF',cross_report_hashes_match_git_text_policy=True,
        scope='Current tracker-loaders reports; older records identify their original locally recorded bytes')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded current full102/102, both complete audio builds and four exact whole-game restores; tracker and original whole goal remain active.')


if __name__=='__main__':main()
