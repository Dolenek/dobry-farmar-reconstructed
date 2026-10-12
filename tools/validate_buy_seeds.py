#!/usr/bin/env python3
"""Compare BuySeeds UI, profiles and actual HUD/tutorial input progression."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/buy-seeds-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[];counts=dict(states=848,starts=132,frames=124,choices=88,letters=19638)
    for precision,mode in [('PC53','buy-seeds'),('PC24','buy-seeds-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'BuySeeds {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['buy_seeds_'+key]==value,(key,state['buy_seeds_'+key])
        assert state['hud_frames']==124 and state['hud_pixels']==59520000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',
        ROOT/'tools/text_buttons_oracle_fixups.inc',ROOT/'tools/game_menu_oracle_cases.inc',ROOT/'tools/game_menu_oracle_services.h',
        ROOT/'tools/tutorial_manager_oracle_services.h',ROOT/'tools/buy_seeds_oracle_cases.inc',ROOT/'tools/buy_seeds_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_buy_seeds_ui_profile_and_tutorial_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},pixels=119040000,
        original_functions=['00420cb0','004201f0','004200f0','00420f50','0041d520','0041d580','0041d5e0','0041d610','0041e050','0041e4c0','0041d9a0','0041d9f0','0041da60','00417910','00403740'],
        coverage=['Actual shipped BuySeeds.xml, eight definition records, seven animation buttons, TextMoney/coins and multiline labels, cancel button and owned MovableUnion',
            'All 128 seven-seed availability combinations from actual uints SeedAvail, actual configuration prices, signed money gates, source/native profile and ordered money queries',
            'Open/close/cancel/choice ordering with actual HUD enable/absolute and panel motion, remembered first slot when no seed is available, ownership destruction',
            'Actual first/last seed and cancel mouse input, actual tutorial 14 disabled-anchor then 15 repeated-action, real profile progression and double completion',
            '248 D3D8 frames, ordered sprites and 119040000 matching RGB pixels, 176 actual Farm-boundary choice observations',
            '160 direct cursor-state probes and original hit-test spill order, skipped first cell and original out-of-bounds sound-slot read'],
        test_adapters=['Original mapped instructions only in diagnosis, original entry point never executed and source never calls original program code',
            'Explicit XML/old CRT/resource/profile adapters; real source/native TutorialManager and GameMenu are connected',
            'Farm 0042c960 is an observed diagnostic boundary, not a source replacement: actual seed mover, placement, money/soil effects remain pending',
            'Silent UI sound boundary uses real recovered resources; actual full-game audible playback remains pending',
            'Both cursor probes share the same actual cSound object. This cannot establish cross-process heap-independent behavior of its original out-of-bounds read',
            'Earlier HUD navigation tutorial flags are preprocessed only in the controlled 14/15 scenario; full tutorial play sequence remains pending'],
        known_original_undefined_memory=dict(function='0041e050',instruction='0041e140',
            description='Loop skips cell 0 and reads cell 7 beyond seven cells; the +530 sound pointer is treated as a Button and +238 is read beyond the verified 40-byte cSound',
            production_implementation='Literal original read retained, no invented hover result or substituted eighth button',
            cross_process_or_full_game_equivalence_proven=False),
        pending=['Actual Farm 0042c960, seed mover/placement and effects, full game source lifecycle and full tutorial sequence',
            'Cross-process/physical runtime behavior of original undefined sound-slot memory access, missing BuySound/null resource branch and constructor errors',
            'XML reload retention with changed or absent sound, eighth ItemPlace overlap, malformed ItemProto and extreme costs/money',
            'Full game EXE and bidirectional saves, complete Squall and isolated uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('BuySeeds: 1696 states, 176 choices, 248 GPU frames/119040000 pixels; 17 originals unchanged')
if __name__=='__main__':main()
