#!/usr/bin/env python3
"""Compare recovered game audio/UI flow through source and original physical DLLs."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
from pe import PE
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir',type=Path,default=ROOT/'build/windows')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/game-audio-validation.json')
    args=parser.parse_args();build=args.build_dir.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def originals_unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    originals_unchanged()
    oracle=build/'Release/game-audio-reference-test.exe'
    candidate=oracle.parent/'Squall.dll'
    assert digest(candidate)==digest(build/'squall-candidate/Release/Squall.dll')
    expected=next(row['sha256'] for row in inventory['files'] if row['path']=='Squall.dll')
    assert digest(candidate)!=expected
    exports={row['ordinal']:row['names'][0] for row in next(row['pe']['exports'] for row in inventory['files'] if row['path']=='Squall.dll')}
    imports=next(row['symbols'] for row in PE(oracle).metadata()['imports'] if row['library'].lower()=='squall.dll')
    names=sorted(row.get('name',exports.get(row.get('ordinal'))) for row in imports)
    required=sorted('SQUALL_'+name for name in ('Init','Free','SetFileCallbacks','Listener_EAX_SetPreset','Pause','Stop',
        'ChannelGroup_SetVolume','Sample_LoadFile','Sample_Unload','Sample_GetFileFrequency','Sample_PlayEx',
        'Channel_Stop','Channel_Status','Channel_Pause','Channel_Start'))
    assert names==required
    candidate_hash=digest(candidate);oracle_hash=digest(oracle);contexts=[]
    for precision,output in run_precision_cases(oracle,[original/'Squall.dll',original/'Data'],timeout=60):
        row=json.loads(output)
        assert row['state_comparisons']==189 and row['production_imports']==15
        assert row['cursor_progress_checks']==18 and row['paused_cursor_checks']==8
        assert row['source_position_progress']>0 and row['native_position_progress']>0
        assert row['playback_tested'] and row['silent_playback'] and not row['audible_output_verified']
        row['precision']=precision;contexts.append(row)
        assert digest(candidate)==candidate_hash and digest(oracle)==oracle_hash
        originals_unchanged()
    paths=sorted([path for directory in ('src/game','src/squall','tools/cmake') for path in (ROOT/directory).rglob('*') if path.is_file()]
                 +[ROOT/'tools/game_audio_reference_test.cpp',ROOT/'tools/loading_audio_cases.inc',ROOT/'tools/squall_exports.inc',Path(__file__).resolve()])
    report=dict(status='passed_for_integrated_game_audio_and_menu_control_scope',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        rebuilt_sha256=candidate_hash,oracle_sha256=oracle_hash,precision_contexts=contexts,
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        production_imports=names,reference_files_unchanged=len(inventory['files']),
        original_game_entry_point_executed=False,original_Squall_entry_point_executed=True,
        original_program_code_used_in_implementation=False,
        source_files=[dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path)) for path in paths],
        coverage=['Actual production SquallSoundApi imports all fifteen game functions from the source-built DLL; executable IAT pointers are checked at runtime',
            'Actual game SoundSetup with null HWND: each backend creates its real hidden audio window, workers and DirectSound buffers',
            'Recovered Settings -> SoundManager -> FileIndex/SoundFiles -> ResourceManager/cSound -> cChannel chain, cache reuse and exact lifetime cleanup',
            '378 observed state comparisons across PC53 and PC24: prepared/play/pause/resume/manager pause, six channel counts, volume/pan/frequency/loop/priority, Loading and Settings',
            '36 independent actual playback cursor progress checks and 16 frozen paused cursor checks; playback is silent, timing progress amounts are recorded but not compared',
            'Real original MainMenu/FixedBack/Options XML, recovered owned controls, real D3D8 texture/font loading and actual game cursor binding',
            'Actual MainMenu start-button pressed sound through Button -> SoundManager -> own DLL; Options Slider callbacks set both native volume groups, including fractional truncation',
            'Actual LoadingFromLog over shipped GameResLog.xml (539 entries) and MenuResLog.xml (148 entries), batch progress and resource loading through actual GPU/font/XML/sound factories',
            'Shared music replacement deletes old cChannel and releases cSound before playing selected GameMusic/MenuMusic; physical cursor, pause/resume and volume zero verified',
            'Both log completions queue and delete actual Loading control trees, release XML, retain playing music; absent-log constructor branch completes and destroys controls immediately',
            'Bounded actual worker retirement after Stop, channel wrapper destruction and resource sample unloading before SoundManager Free; no callback handles remain'],
        test_adapters=['Original Squall DLL is loaded normally as an independent physical audio oracle; production adapter never forwards to it',
            'Both runs use the previously native-compared recovered game controllers. This comparison is not a new native game-controller oracle',
            'Windows D3D8 HAL device uses diagnostic presentation settings and FPU preserve; full original graphics initialization/reset is separate',
            'Fullscreen reset boundary throws if exercised; this audio scenario never toggles fullscreen and cannot claim reset validation',
            'MainMenu content and actual LoadingFromLog are loaded; completion is a counted diagnostic callback, not a cGame implementation. The native EXE controller comparison is reported separately',
            'This comparison applies recovered LoadingFromLog to both audio DLLs, with native game instructions compared in validate_loading.py'],
        pending=['Audible or WASAPI loopback comparison and hardware output; silence deliberately used for cursor tests',
            'Timed worker/fragment refill PCM during playback, exact pause resume scheduling and whole menu focus/message/input lifecycle',
            'Native full ResourceManager UI trees, initial caller pause byte allocation and exceptional construction/destruction',
            'Complete MainMenu navigation/engine/new game and all gameplay/save compatibility',
            'All 97 Squall signatures/full behavior, other codecs, streaming and hardware extensions'])
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('Game audio/Loading: 378 states, 36 actual playback progress and 16 paused cursor checks; 17 originals unchanged')
if __name__=='__main__':main()
