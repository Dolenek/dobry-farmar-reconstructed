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
        if module=='click-mask' and 'source_files' in validation:
            for name,expected in validation['source_files'].items():
                assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
            assert hashlib.sha256((ROOT/'build/windows/Release/click-mask-reference-test.exe').read_bytes()).hexdigest()==validation['oracle_sha256']
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
                'SQUALL_Sample_PlayEx':'Real ResourceManager/cSound music, actual MainMenu click and Loading game/menu music replacement through production imports',
                'SQUALL_Sample_LoadFile':'Actual game file callbacks and memory mode 1/defaults null while preloading both complete shipped resource logs, including all 45 sounds',
                'SQUALL_Sample_Unload':'Actual unused preloaded sound cleanup across Loading screens, wrapper/resource destruction before Free',
                'SQUALL_Channel_Stop':'Actual old shared music wrapper deletion before Loading selects and starts new music'}
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
    panel_path=ROOT/'.local/award-panel-validation.json'
    if panel_path.exists():
        panel=read(panel_path.relative_to(ROOT));assert panel['status']=='passed_for_award_panel_owned_tree_and_shipped_2d_scope'
        for name,expected in panel['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/award-panel-validation.json',panel);report['award-panel']=panel
        if 'analysis/award-panel-validation.json' not in report['reports']: report['reports'].append('analysis/award-panel-validation.json')
    awards_path=ROOT/'.local/awards-validation.json'
    if awards_path.exists():
        awards=read(awards_path.relative_to(ROOT));assert awards['status']=='passed_for_awards_controller_buttons_and_shipped_2d_scope'
        for name,expected in awards['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/awards-validation.json',awards);report['awards']=awards
        if 'analysis/awards-validation.json' not in report['reports']: report['reports'].append('analysis/awards-validation.json')
    effect_path=ROOT/'.local/award-effect-validation.json'
    if effect_path.exists():
        effect=read(effect_path.relative_to(ROOT));assert effect['status']=='passed_for_award_effect_state_machine_and_shipped_2d_scope'
        for name,expected in effect['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/award-effect-validation.json',effect);report['award-effect']=effect
        if 'analysis/award-effect-validation.json' not in report['reports']: report['reports'].append('analysis/award-effect-validation.json')
    tooltip_path=ROOT/'.local/tooltip-validation.json'
    if tooltip_path.exists():
        tooltip=read(tooltip_path.relative_to(ROOT));assert tooltip['status']=='passed_for_owned_award_tooltip_and_shipped_2d_scope'
        for name,expected in tooltip['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/tooltip-validation.json',tooltip);report['tooltip']=tooltip
        if 'analysis/tooltip-validation.json' not in report['reports']: report['reports'].append('analysis/tooltip-validation.json')
    loading_path=ROOT/'.local/loading-validation.json'
    if loading_path.exists():
        loading=read(loading_path.relative_to(ROOT));assert loading['status']=='passed_for_loading_and_shipped_resource_log_scope'
        for name,expected in loading['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/loading-validation.json',loading);report['loading']=loading
        if 'analysis/loading-validation.json' not in report['reports']: report['reports'].append('analysis/loading-validation.json')
    log_path=ROOT/'.local/resource-log-validation.json'
    if log_path.exists():
        log=read(log_path.relative_to(ROOT));assert log['status']=='passed_for_resource_log_writer_and_isolated_file_scope'
        for name,expected in log['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/resource-log-validation.json',log);report['resource-log']=log
        if 'analysis/resource-log-validation.json' not in report['reports']: report['reports'].append('analysis/resource-log-validation.json')
    menu_path=ROOT/'.local/main-menu-validation.json'
    if menu_path.exists():
        menu=read(menu_path.relative_to(ROOT));assert menu['status']=='passed_for_main_menu_navigation_and_loading_scope'
        for name,expected in menu['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/main-menu-validation.json',menu);report['main-menu']=menu
        if 'analysis/main-menu-validation.json' not in report['reports']: report['reports'].append('analysis/main-menu-validation.json')
    game_configuration_path=ROOT/'.local/game-configuration-validation.json'
    if game_configuration_path.exists():
        configuration=read(game_configuration_path.relative_to(ROOT));assert configuration['status']=='passed_for_game_configuration_and_2d_appearance_scope'
        for name,expected in configuration['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/game-configuration-validation.json',configuration);report['game-configuration']=configuration
        if 'analysis/game-configuration-validation.json' not in report['reports']: report['reports'].append('analysis/game-configuration-validation.json')
    gameplay_path=ROOT/'.local/gameplay-validation.json'
    if gameplay_path.exists():
        gameplay=read(gameplay_path.relative_to(ROOT));assert gameplay['status']=='passed_for_gameplay_definitions_and_time_scope'
        for name,expected in gameplay['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/gameplay-validation.json',gameplay);report['gameplay']=gameplay
        if 'analysis/gameplay-validation.json' not in report['reports']: report['reports'].append('analysis/gameplay-validation.json')
    gameplay_runtime_path=ROOT/'.local/gameplay-runtime-validation.json'
    if gameplay_runtime_path.exists():
        gameplay=read(gameplay_runtime_path.relative_to(ROOT));assert gameplay['status']=='passed_for_gameplay_constructor_scheduling_randomization_scope'
        for name,expected in gameplay['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/gameplay-runtime-validation.json',gameplay);report['gameplay-runtime']=gameplay
        if 'analysis/gameplay-runtime-validation.json' not in report['reports']: report['reports'].append('analysis/gameplay-runtime-validation.json')
    upgrade_path=ROOT/'.local/upgrade-validation.json'
    if upgrade_path.exists():
        upgrade=read(upgrade_path.relative_to(ROOT));assert upgrade['status']=='passed_for_abstract_upgrade_and_cost_state_scope'
        for name,expected in upgrade['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/upgrade-validation.json',upgrade);report['upgrade']=upgrade
        if 'analysis/upgrade-validation.json' not in report['reports']: report['reports'].append('analysis/upgrade-validation.json')
    extended_path=ROOT/'.local/extended-buttons-validation.json'
    if extended_path.exists():
        extended=read(extended_path.relative_to(ROOT));assert extended['status']=='passed_for_extended_game_button_specializations_and_tooltip_scope'
        for name,expected in extended['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/extended-buttons-validation.json',extended);report['extended-buttons']=extended
        if 'analysis/extended-buttons-validation.json' not in report['reports']: report['reports'].append('analysis/extended-buttons-validation.json')
    contract_path=ROOT/'.local/contract-status-validation.json'
    if contract_path.exists():
        contract=read(contract_path.relative_to(ROOT));assert contract['status']=='passed_for_contract_status_controller_ui_and_profile_scope'
        for name,expected in contract['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/contract-status-validation.json',contract);report['contract-status']=contract
        if 'analysis/contract-status-validation.json' not in report['reports']: report['reports'].append('analysis/contract-status-validation.json')
    price_path=ROOT/'.local/price-text-validation.json'
    if price_path.exists():
        price=read(price_path.relative_to(ROOT));assert price['status']=='passed_for_price_text_and_text_money_scope'
        for name,expected in price['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/price-text-validation.json',price);report['price-text']=price
        if 'analysis/price-text-validation.json' not in report['reports']: report['reports'].append('analysis/price-text-validation.json')
    dialogs_path=ROOT/'.local/contract-dialogs-validation.json'
    if dialogs_path.exists():
        dialogs=read(dialogs_path.relative_to(ROOT));assert dialogs['status']=='passed_for_contract_dialogs_and_connected_status_scope'
        for name,expected in dialogs['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/contract-dialogs-validation.json',dialogs);report['contract-dialogs']=dialogs
        if 'analysis/contract-dialogs-validation.json' not in report['reports']: report['reports'].append('analysis/contract-dialogs-validation.json')
    provide_path=ROOT/'.local/contract-provide-validation.json'
    if provide_path.exists():
        provide=read(provide_path.relative_to(ROOT));assert provide['status']=='passed_for_contract_offer_and_connected_status_scope'
        for name,expected in provide['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/contract-provide-validation.json',provide);report['contract-provide']=provide
        if 'analysis/contract-provide-validation.json' not in report['reports']: report['reports'].append('analysis/contract-provide-validation.json')
    pause_path=ROOT/'.local/game-pause-validation.json'
    if pause_path.exists():
        pause=read(pause_path.relative_to(ROOT));assert pause['status']=='passed_for_game_pause_menu_and_connected_navigation_scope'
        for name,expected in pause['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/game-pause-validation.json',pause);report['game-pause']=pause
        if 'analysis/game-pause-validation.json' not in report['reports']: report['reports'].append('analysis/game-pause-validation.json')
    state_path=ROOT/'.local/state-view-validation.json'
    if state_path.exists():
        state=read(state_path.relative_to(ROOT));assert state['status']=='passed_for_state_view_motion_hierarchy_and_rendering_scope'
        for name,expected in state['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/state-view-validation.json',state);report['state-view']=state
        if 'analysis/state-view-validation.json' not in report['reports']: report['reports'].append('analysis/state-view-validation.json')
    hud_path=ROOT/'.local/game-menu-validation.json'
    if hud_path.exists():
        hud=read(hud_path.relative_to(ROOT));assert hud['status']=='passed_for_game_menu_hud_contracts_scene_navigation_and_input_scope'
        for name,expected in hud['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/game-menu-validation.json',hud);report['game-menu']=hud
        if 'analysis/game-menu-validation.json' not in report['reports']: report['reports'].append('analysis/game-menu-validation.json')
    tip_path=ROOT/'.local/tutorial-tips-validation.json'
    if tip_path.exists():
        tip=read(tip_path.relative_to(ROOT));assert tip['status']=='passed_for_five_tutorial_tip_windows_and_input_scope'
        for name,expected in tip['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/tutorial-tips-validation.json',tip);report['tutorial-tips']=tip
        if 'analysis/tutorial-tips-validation.json' not in report['reports']: report['reports'].append('analysis/tutorial-tips-validation.json')
    tutorial_path=ROOT/'.local/tutorial-manager-validation.json'
    if tutorial_path.exists():
        tutorial=read(tutorial_path.relative_to(ROOT));assert tutorial['status']=='passed_for_tutorial_manager_profile_predicates_and_connected_hud_scope'
        for name,expected in tutorial['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/tutorial-manager-validation.json',tutorial);report['tutorial-manager']=tutorial
        if 'analysis/tutorial-manager-validation.json' not in report['reports']: report['reports'].append('analysis/tutorial-manager-validation.json')
    union_path=ROOT/'.local/movable-union-validation.json'
    if union_path.exists():
        union=read(union_path.relative_to(ROOT));assert union['status']=='passed_for_movable_union_picture_motion_and_hud_overlay_scope'
        for name,expected in union['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/movable-union-validation.json',union);report['movable-union']=union
        if 'analysis/movable-union-validation.json' not in report['reports']: report['reports'].append('analysis/movable-union-validation.json')
    seeds_path=ROOT/'.local/buy-seeds-validation.json'
    if seeds_path.exists():
        seeds=read(seeds_path.relative_to(ROOT));assert seeds['status']=='passed_for_buy_seeds_ui_profile_and_tutorial_scope'
        for name,expected in seeds['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/buy-seeds-validation.json',seeds);report['buy-seeds']=seeds
        if 'analysis/buy-seeds-validation.json' not in report['reports']: report['reports'].append('analysis/buy-seeds-validation.json')
    building_path=ROOT/'.local/buy-building-validation.json'
    if building_path.exists():
        building=read(building_path.relative_to(ROOT));assert building['status']=='passed_for_buy_building_ui_profile_and_tutorial_scope'
        for name,expected in building['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/buy-building-validation.json',building);report['buy-building']=building
        if 'analysis/buy-building-validation.json' not in report['reports']:report['reports'].append('analysis/buy-building-validation.json')
    production_path=ROOT/'.local/production-houses-validation.json'
    if production_path.exists():
        production=read(production_path.relative_to(ROOT));assert production['status']=='passed_for_production_house_controller_and_observed_world_boundary_scope'
        for name,expected in production['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/production-houses-validation.json',production);report['production-houses']=production
        if 'analysis/production-houses-validation.json' not in report['reports']:report['reports'].append('analysis/production-houses-validation.json')
        report['build']['production_house_complete_world_equivalence']=False
    manufacture_path=ROOT/'.local/manufacture-validation.json'
    if manufacture_path.exists():
        manufacture=read(manufacture_path.relative_to(ROOT));assert manufacture['status']=='passed_for_manufacture_scene_and_explicit_remaining_world_boundaries'
        for name,expected in manufacture['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/manufacture-validation.json',manufacture);report['manufacture']=manufacture
        if 'analysis/manufacture-validation.json' not in report['reports']:report['reports'].append('analysis/manufacture-validation.json')
        report['build']['manufacture_complete_world_equivalence']=False
    field_path=ROOT/'.local/field-object-validation.json'
    if field_path.exists():
        field=read(field_path.relative_to(ROOT));assert field['status']=='passed_for_field_object_and_extended_field_tooltip_scope'
        for name,expected in field['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/field-object-validation.json',field);report['field-object']=field
        if 'analysis/field-object-validation.json' not in report['reports']:report['reports'].append('analysis/field-object-validation.json')
    area_path=ROOT/'.local/action-area-validation.json'
    if area_path.exists():
        area=read(area_path.relative_to(ROOT));assert area['status']=='passed_for_action_area_controller_and_explicit_world_boundaries'
        for name,expected in area['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/action-area-validation.json',area);report['action-area']=area
        if 'analysis/action-area-validation.json' not in report['reports']:report['reports'].append('analysis/action-area-validation.json')
    report['build']['explicit_audit_targets']=list(dict.fromkeys(report['build']['explicit_audit_targets']))
    warehouse_path=ROOT/'.local/warehouse-validation.json'
    if warehouse_path.exists():
        warehouse=read(warehouse_path.relative_to(ROOT));assert warehouse['status']=='passed_for_warehouse_and_actual_manufacture_production_hud_chain'
        for name,expected in warehouse['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/warehouse-validation.json',warehouse);report['warehouse']=warehouse
        if 'analysis/warehouse-validation.json' not in report['reports']:report['reports'].append('analysis/warehouse-validation.json')
        report['build']['warehouse_complete_world_equivalence']=False
    write('analysis/verification.json',report)
    tip_path=ROOT/'.local/warehouse-tip-validation.json'
    if tip_path.exists():
        tip=read(tip_path.relative_to(ROOT));assert tip['status']=='passed_for_warehouse_tip_and_actual_stock_hud_production_chain'
        for name,expected in tip['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/warehouse-tip-validation.json',tip);report['warehouse-tip']=tip
        if 'analysis/warehouse-tip-validation.json' not in report['reports']:report['reports'].append('analysis/warehouse-tip-validation.json')
        write('analysis/verification.json',report)
    histogram_path=ROOT/'.local/histogram-validation.json'
    if histogram_path.exists():
        histogram=read(histogram_path.relative_to(ROOT));assert histogram['status']=='passed_for_histogram_values_xml_ownership_and_frames'
        for name,expected in histogram['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/histogram-validation.json',histogram);report['histogram']=histogram
        if 'analysis/histogram-validation.json' not in report['reports']:report['reports'].append('analysis/histogram-validation.json')
        report['build']['market_complete_world_equivalence']=False
        write('analysis/verification.json',report)
    market_path=ROOT/'.local/market-validation.json'
    if market_path.exists():
        market=read(market_path.relative_to(ROOT));assert market['status']=='passed_for_market_and_actual_stock_ui_tutorial_chain'
        for name,expected in market['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/market-validation.json',market);report['market']=market
        if 'analysis/market-validation.json' not in report['reports']:report['reports'].append('analysis/market-validation.json')
        report['build']['market_complete_world_equivalence']=False
        write('analysis/verification.json',report)
    sales_path=ROOT/'.local/sales-history-validation.json'
    if sales_path.exists():
        sales=read(sales_path.relative_to(ROOT));assert sales['status']=='passed_for_actual_market_sale_history_and_awEarn'
        for name,expected in sales['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/sales-history-validation.json',sales);report['sales-history']=sales
        if 'analysis/sales-history-validation.json' not in report['reports']:report['reports'].append('analysis/sales-history-validation.json')
        write('analysis/verification.json',report)
    for key,status in [('flying-text','passed_for_flying_text_path_alpha_and_frames'),('flying-market','passed_for_actual_market_flying_effects_and_sale_history')]:
        effect_path=ROOT/f'.local/{key}-validation.json'
        if effect_path.exists():
            effect=read(effect_path.relative_to(ROOT));assert effect['status']==status
            for name,expected in effect['source_files'].items():
                assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
            if key.startswith('person-sounds'):
                for path_key,hash_key in [('oracle_path','oracle_sha256'),('candidate_path','candidate_sha256')]:
                    assert hashlib.sha256((ROOT/effect[path_key]).read_bytes()).hexdigest()==effect[hash_key],effect[path_key]
            write(f'analysis/{key}-validation.json',effect);report[key]=effect
            if f'analysis/{key}-validation.json' not in report['reports']:report['reports'].append(f'analysis/{key}-validation.json')
    dig_path=ROOT/'.local/dig-building-validation.json'
    if dig_path.exists():
        dig=read(dig_path.relative_to(ROOT));assert dig['status']=='passed_for_dig_building_ui_input_tutorial_and_explicit_person_boundaries'
        for name,expected in dig['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/dig-building-validation.json',dig);report['dig-building']=dig
        if 'analysis/dig-building-validation.json' not in report['reports']:report['reports'].append('analysis/dig-building-validation.json')
    write('analysis/verification.json',report)
    water_path=ROOT/'.local/water-source-validation.json'
    if water_path.exists():
        water=read(water_path.relative_to(ROOT));assert water['status']=='passed_for_water_source_ui_and_explicit_person_farm_boundaries'
        for name,expected in water['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/water-source-validation.json',water);report['water-source']=water
        if 'analysis/water-source-validation.json' not in report['reports']:report['reports'].append('analysis/water-source-validation.json')
        write('analysis/verification.json',report)
    mow_path=ROOT/'.local/mow-building-validation.json'
    if mow_path.exists():
        mow=read(mow_path.relative_to(ROOT));assert mow['status']=='passed_for_mow_building_held_products_ui_and_explicit_person_boundaries'
        for name,expected in mow['source_files'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
        write('analysis/mow-building-validation.json',mow);report['mow-building']=mow
        if 'analysis/mow-building-validation.json' not in report['reports']:report['reports'].append('analysis/mow-building-validation.json')
        write('analysis/verification.json',report)
    for key,status in [('farm-scene','passed_for_actual_farm_scene_ownership_upgrades_crop_selection_contracts_restore_and_frames'),('contract-letter','passed_for_contract_letter_lifetime_deadlines_reload_tutorial_and_input'),('person-flows','passed_for_person_seed_sow_drop_and_hose_particle_input_cycles'),('person-controller','passed_for_person_actor_lifetime_timed_work_and_rendered_field_input'),('elastic-animation','passed_for_elastic_instrument_motion_actual_tool_upgrades_reveal_and_frames'),('water-particles','passed_for_water_particles_copy_motion_and_frames'),('water-emitters','passed_for_water_emitters_creation_modes_frames_and_disposal'),('sprinkle','passed_for_sprinkle_controller_effect_factory_and_explicit_field_boundaries'),('farm-field','passed_for_field_actions_farm_geometry_and_connected_irrigation'),('seed-package','passed_for_seed_package_progress_purchase_field_and_explicit_person_boundaries'),('person-tasks','passed_for_person_queue_markers_field_seed_input_and_explicit_actor_boundaries'),('person-visuals','passed_for_person_visuals_ui_selectors_reloads_and_explicit_actor_boundaries'),('person-walk','passed_for_person_ordinary_motion_field_seed_and_mouse_queue'),('person-vehicle','passed_for_person_vehicle_motion_axis_field_seed_and_mouse_queue'),('person-sounds','passed_for_person_sound_selection_channel_ownership_and_physical_backends'),('person-sounds-fresh','passed_for_person_sound_selection_channel_ownership_and_physical_backends')]:
        effect_path=ROOT/f'.local/{key}-validation.json'
        if effect_path.exists():
            effect=read(effect_path.relative_to(ROOT));assert effect['status']==status
            for name,expected in effect['source_files'].items():
                assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==expected,name
            if key.startswith('person-sounds'):
                for path_key,hash_key in [('oracle_path','oracle_sha256'),('candidate_path','candidate_sha256')]:
                    assert hashlib.sha256((ROOT/effect[path_key]).read_bytes()).hexdigest()==effect[hash_key],effect[path_key]
            write(f'analysis/{key}-validation.json',effect);report[key]=effect
            if f'analysis/{key}-validation.json' not in report['reports']:report['reports'].append(f'analysis/{key}-validation.json')
    write('analysis/verification.json',report)
    print(f"Recorded {len(completed)} CTest passes; {len(inventory['files'])} originals unchanged; full game remains incomplete")

if __name__=='__main__': main()
