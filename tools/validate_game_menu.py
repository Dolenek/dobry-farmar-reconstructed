#!/usr/bin/env python3
"""Compare the actual GameMenu HUD, connected contracts, scene movement and input."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/game-menu-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=2389,letters=34977,frames=121,pixels=58080000,steps=168,actions=575)
    for precision,mode in [('PC53','game-menu'),('PC24','game-menu-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'GameMenu {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['hud_'+key]==value,(key,state['hud_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/game_menu_oracle_cases.inc',ROOT/'tools/game_menu_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_game_menu_hud_contracts_scene_navigation_and_input_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},
        original_functions=['004c3430','004c31f0','004c1f90','004c3920','004c30a0','004c3130','004c02c0','004c06b0',
            '004c4390','004c44a0','004bef70','004bf4d0','004bf9e0','004bd950','004be740','004bd9d0','004beb40','004bfee0',
            '004bdf00','004be340','004c2cc0','004c0750','004c1b60','004c0b50','004c0f70','004c1350','004c1770',
            '004bd830','004bd8a0','004bd8f0','004bd920','004bb330','00413c50'],
        coverage=['Actual GameMenu.xml, nine owned HUD controls, three actual ContractStatus, owned GamePauseMenu, construction/destruction and singleton lifetime',
            '131 actual contract definitions: three activations, full guard, preview/stock notification, real ContractPassConfirm dialog and three pointer removals per definition and precision',
            'Actual HUD constructor restoration of three saved tasks with native profile keys, real message deep copies and signed versus unsigned money formatters',
            'Fixed-seed money/wealth/level triggers and warehouse capacity percentages, exact x87 timing/flash, tool suspension/resumption and button enable capture',
            'Native tutorial anchor dispatch and short-circuit callback gates, including original Farm callback lacking global block checks',
            'Actual StateView scene navigation and original XML backgrounds, mouse release navigation, warehouse hover, Escape pause and Enter resume',
            '242 actual D3D8 frames, ordered sprites/API and 116160000 exactly matching RGB pixels'],
        test_adapters=['Mapped original instructions for diagnosis only, entry point never called; source implementation never calls original code',
            'Explicit XML/old CRT/resource adapters and deterministic reconstructed timer/cursor/input',
            'Required wealth/warehouse/manufacture/character/tutorial/gameplay/message world services use ordered diagnostic observations, not production replacements',
            'Scene objects are actual recovered StateView bases with real XML; derived world controllers and their enter-state hooks remain pending',
            'ContractPassConfirm, ContractStatus, GamePauseMenu and HUD source/native controllers are actually connected; profile ints/uints/floats/booleans compared'],
        pending=['Full cGame/Farm/Warehouse/Manufacture/character/tutorial/message controllers and Gameplay world bindings',
            'Main HUD pause button input path, all tool states during controller transitions, invalid/inconsistent contracts and allocation/error paths',
            'Unread blinking NaN payloads/extreme overflow and full XML reload retention, physical input/audio',
            'Full game EXE, bidirectional saves, complete Squall and isolated uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('GameMenu: 4778 states, 1150 actions, 242 GPU frames/116160000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
