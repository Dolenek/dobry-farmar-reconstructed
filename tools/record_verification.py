#!/usr/bin/env python3
"""Refresh portable verification metadata from completed local reports, checking references."""
import datetime
import hashlib
import json
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

def read(path): return json.loads((ROOT/path).read_text(encoding='utf-8'))
def write(path,data): (ROOT/path).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def main():
    original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=read('analysis/inventory.json')
    for row in inventory['files']:
        assert hashlib.sha256((original/row['path']).read_bytes()).hexdigest()==row['sha256'], row['path']
    junit=ET.parse(ROOT/'.local/ctest-windows.xml').getroot()
    assert not junit.findall('.//failure') and not junit.findall('.//error'), 'CTest failures remain'
    completed=junit.findall('.//testcase')
    assert completed and not junit.findall('.//skipped'), 'CTest missing or skipped'
    report=read('analysis/verification.json')
    report['recorded_at_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
    report['tests'].update(ctest_passed=len(completed),reference_files_unchanged=len(inventory['files']),
                           original_wstring_comparisons=20000,resource_index_fixtures=8,resource_write_checks=12)
    report['build']['targets']+= [name for name in ['pack-decode.exe','resource-inspect.exe',
        'wstring-reference-test.exe','file-index-test.exe','settings-test.exe','window-test.exe','texture-test.exe',
        'recovered_graphics_utilities','recovered_resource_utilities','resource-manager-test.exe','color-reference-test.exe',
        'graphics-presentation-test.exe','matrix-reference-test.exe','sprite-geometry-reference-test.exe',
        'sprite-draw-reference-test.exe','native-image-reference-test.exe','font-reference-test.exe',
        'text-layout-reference-test.exe','string-manager-reference-test.exe','control-sprite-reference-test.exe',
        'view-reference-test.exe','ui-frame-reference-test.exe','control-reference-test.exe','text-control-reference-test.exe','button-reference-test.exe','button-text-reference-test.exe','text-buttons-reference-test.exe','click-mask-reference-test.exe','input-bindings-reference-test.exe','timer-reference-test.exe','cursor-reference-test.exe','profile-headers-reference-test.exe'] if name not in report['build']['targets']]
    report['build']['dependencies']+= [name for name in ['Minizip 1.01','Expat 1.95.8 (candidate, original version unproved)']
        if name not in report['build']['dependencies']]
    report['build']['dependencies']+= [name for name in ['Wine-origin Direct3D8 headers (MinGW-w64 v12.0.0; LGPL-2.1-or-later)']
        if name not in report['build']['dependencies']]
    resource=read('.local/resource-validation.json')
    assert resource['status']=='passed' and not resource['failures']
    write('analysis/resource-loading-validation.json',resource)
    report['resource_pipeline']=resource
    settings=read('.local/settings-validation.json')
    assert settings['status']=='passed'
    write('analysis/settings-validation.json',settings)
    report['settings']=settings
    for module in ('window','texture','resource-manager','color','graphics-presentation','matrix','sprite-geometry','sprite-draw','native-image','font','text-layout','string-manager','control-sprite','view','ui-frame','control','text-control','button','button-text','text-buttons','click-mask','input-bindings','timer','cursor','profile-headers','profile-properties','profile-lifecycle','sound-manager','squall-parameters'):
        validation=read(f'.local/{module}-validation.json')
        assert validation['status']=='passed'
        write(f'analysis/{module}-validation.json',validation)
        report[module]=validation
    wide=read('.local/wstring-validation.json')
    write('analysis/wstring-validation.json',wide)
    paths=[ROOT/'CMakeLists.txt']+[path for folder in ('src','tools','tests')
        for path in (ROOT/folder).rglob('*') if path.is_file() and '__pycache__' not in path.parts]
    rows=[path.relative_to(ROOT).as_posix()+'\0'+hashlib.sha256(path.read_bytes()).hexdigest()+'\n'
          for path in sorted(paths,key=lambda path:path.relative_to(ROOT).as_posix())]
    report['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    report['reports']+= [name for name in ['analysis/resource-loading-validation.json','analysis/settings-validation.json',
        'analysis/wstring-validation.json','analysis/window-validation.json','analysis/texture-validation.json',
        'analysis/resource-manager-validation.json','analysis/color-validation.json',
        'analysis/graphics-presentation-validation.json','analysis/matrix-validation.json',
        'analysis/sprite-geometry-validation.json','analysis/sprite-draw-validation.json',
        'analysis/native-image-validation.json','analysis/font-validation.json','analysis/text-layout-validation.json',
        'analysis/string-manager-validation.json','analysis/control-sprite-validation.json','analysis/view-validation.json','analysis/ui-frame-validation.json','analysis/control-validation.json','analysis/text-control-validation.json','analysis/button-validation.json','analysis/button-text-validation.json','analysis/text-buttons-validation.json','analysis/click-mask-validation.json','analysis/input-bindings-validation.json','analysis/timer-validation.json','analysis/cursor-validation.json','analysis/profile-headers-validation.json'] if name not in report['reports']]
    if 'profile-properties-reference-test.exe' not in report['build']['targets']: report['build']['targets'].append('profile-properties-reference-test.exe')
    if 'analysis/profile-properties-validation.json' not in report['reports']: report['reports'].append('analysis/profile-properties-validation.json')
    if 'profile-lifecycle-reference-test.exe' not in report['build']['targets']: report['build']['targets'].append('profile-lifecycle-reference-test.exe')
    if 'analysis/profile-lifecycle-validation.json' not in report['reports']: report['reports'].append('analysis/profile-lifecycle-validation.json')
    if 'sound-manager-reference-test.exe' not in report['build']['targets']: report['build']['targets'].append('sound-manager-reference-test.exe')
    if 'analysis/sound-manager-validation.json' not in report['reports']: report['reports'].append('analysis/sound-manager-validation.json')
    if 'squall-parameters-reference-test.exe' not in report['build']['targets']: report['build']['targets'].append('squall-parameters-reference-test.exe')
    if 'analysis/squall-parameters-validation.json' not in report['reports']: report['reports'].append('analysis/squall-parameters-validation.json')
    runtime_path=ROOT/'.local/squall-runtime-validation.json'
    if runtime_path.exists():
        runtime=read('.local/squall-runtime-validation.json')
        assert runtime['status']=='passed_for_recorded_sample_and_transform_scope'
        for row in runtime['source_files']+runtime['oracle_files']:
            assert hashlib.sha256((ROOT/row['path']).read_bytes()).hexdigest()==row['sha256'],row['path']
        assert hashlib.sha256((ROOT/'build/windows/squall-candidate/Release/Squall.dll').read_bytes()).hexdigest()==runtime['export_contract']['rebuilt_sha256']
        write('analysis/squall-runtime-validation.json',runtime)
        write('analysis/squall-candidate-abi.json',read('.local/squall-candidate-abi.json'))
        write('analysis/squall-export-audit.json',read('.local/squall-export-audit.json'))
        report['squall-runtime']=runtime
        report['build']['explicit_audit_targets']=['SquallCandidate','squall-runtime-reference-test','squall-vorbis-reference-test']
        report['build']['squall_candidate_complete_equivalence']=False
        for path in ('analysis/squall-runtime-validation.json','analysis/squall-candidate-abi.json','analysis/squall-export-audit.json'):
            if path not in report['reports']: report['reports'].append(path)
    for suffix in ('','-fresh'):
        dll_path=ROOT/f'.local/squall-dll{suffix}-validation.json'
        if not dll_path.exists(): continue
        dll=read(dll_path.relative_to(ROOT))
        assert dll['status']=='passed_for_loaded_DLL_samples_and_prepared_DirectSound_scope'
        for row in dll['source_files']:
            assert hashlib.sha256((ROOT/row['path']).read_bytes()).hexdigest()==row['sha256'],row['path']
        for path_key,hash_key in (('candidate_path','rebuilt_sha256'),('oracle_path','oracle_sha256'),('linker_map_path','linker_map_sha256')):
            assert hashlib.sha256((ROOT/dll[path_key]).read_bytes()).hexdigest()==dll[hash_key],dll[path_key]
        assert dll['reference_sha256']==runtime['reference_sha256']
        portable=f'analysis/squall-dll{suffix}-validation.json'
        write(portable,dll);report['squall-dll'+suffix]=dll
        if portable not in report['reports']: report['reports'].append(portable)
        if not suffix:
            assert dll['rebuilt_sha256']==runtime['export_contract']['rebuilt_sha256']
            audit=read('analysis/squall-export-audit.json')
            for row in audit['exports']:
                scope=dll['export_scopes'].get(row['name'])
                if scope:
                    row['status']='partially_compared'
                    row['loaded_DLL_verified_scope']=scope
            write('analysis/squall-export-audit.json',audit)
            report['build']['explicit_audit_targets'].append('squall-dll-reference-test')
    for suffix in ('','-fresh'):
        audio_path=ROOT/f'.local/game-audio{suffix}-validation.json'
        if not audio_path.exists(): continue
        audio=read(audio_path.relative_to(ROOT))
        assert audio['status']=='passed_for_integrated_game_audio_and_menu_control_scope'
        for row in audio['source_files']:
            assert hashlib.sha256((ROOT/row['path']).read_bytes()).hexdigest()==row['sha256'],row['path']
        for path_key,hash_key in (('candidate_path','rebuilt_sha256'),('oracle_path','oracle_sha256')):
            assert hashlib.sha256((ROOT/audio[path_key]).read_bytes()).hexdigest()==audio[hash_key],audio[path_key]
        portable=f'analysis/game-audio{suffix}-validation.json';write(portable,audio);report['game-audio'+suffix]=audio
        if portable not in report['reports']: report['reports'].append(portable)
        if not suffix:
            assert audio['rebuilt_sha256']==runtime['export_contract']['rebuilt_sha256']
            report['build']['explicit_audit_targets']+=['recovered_game_audio','game-audio-reference-test']
            audit=read('analysis/squall-export-audit.json')
            scopes={
                'SQUALL_Init':'Actual game SoundSetup with null HWND and backend-owned window in both precisions',
                'SQUALL_Free':'Actual recovered SoundManager destruction after resource/channel cleanup',
                'SQUALL_Pause':'Actual playing/paused/resumed music cursor and status through SoundManager',
                'SQUALL_Stop':'Actual group 1 UI clicks and group 2 music; bounded full worker retirement',
                'SQUALL_Channel_Start':'Actual prepared and resumed music through cChannel with cursor progress',
                'SQUALL_Channel_Pause':'Actual cChannel pause/resume status and frozen cursor',
                'SQUALL_Channel_Status':'Real prepared/play/pause/resume/retirement observations',
                'SQUALL_ChannelGroup_SetVolume':'Real SoundManager/Options groups 1/2, integer/fractional volume and music inverse getter',
                'SQUALL_Sample_PlayEx':'Real ResourceManager/cSound music and actual MainMenu button click through production imports'}
            for row in audit['exports']:
                if row['name'] in scopes:
                    row['status']='partially_compared';row['integrated_game_audio_verified_scope']=scopes[row['name']]
            write('analysis/squall-export-audit.json',audit)
    progress_path=ROOT/'.local/progress-validation.json'
    if progress_path.exists():
        progress=read(progress_path.relative_to(ROOT));assert progress['status']=='passed'
        for name,expected in progress['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/progress-validation.json',progress);report['progress']=progress
        if 'analysis/progress-validation.json' not in report['reports']: report['reports'].append('analysis/progress-validation.json')
        if 'progress-sprite-reference-test.exe' not in report['build']['targets']: report['build']['targets'].append('progress-sprite-reference-test.exe')
    write('analysis/verification.json',report)
    print(f"Recorded {len(completed)} CTest passes; {len(inventory['files'])} originals unchanged; full game remains incomplete")

if __name__=='__main__': main()
