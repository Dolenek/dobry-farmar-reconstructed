#!/usr/bin/env python3
"""Compare actual ActionArea lifecycle, offsets, goals, clicks and rendering."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/action-area-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    reference=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:
            assert digest(reference/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    expected=dict(states=8921,objects=43,goals=7392,clicks=1104,frames=42,pixels=20160000,partial_reloads=5)
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        result=subprocess.run([str(oracle),str(reference/'Dobrý farmář.exe'),str(reference/'Data'),'action-area'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:
            raise RuntimeError(f'{precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in expected.items():
            assert state['action_area_'+key]==value,(precision,key,state['action_area_'+key],value)
        assert state['production_placement_states']==24576 and state['production_house_objects']==0
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,**state));unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/name for name in [
        'CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/build_sprite_oracle_fixups.py','tools/action_area_oracle_cases.inc','tools/action_area_oracle_services.h',
        'tools/production_house_oracle_cases.inc','tools/production_placement_oracle_cases.inc','tools/game_menu_oracle_cases.inc',
        'tools/game_menu_oracle_services.h','tools/tutorial_manager_oracle_services.h',
    ]]+[Path(__file__).resolve()])
    report=dict(status='passed_for_action_area_controller_and_explicit_world_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(reference/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts,states=17842,objects=86,goals=14784,clicks=2208,frames=84,pixels=40320000,
        original_functions=['0041c010','004187a0','0041cd10','00418800','00418c60','00418110','00417e10','00417f60','00431530'],
        stencil_format='D24S8',coverage=[
            'Seven shipped Farm ActionAreas XML roots across six area kinds; actual ctor, native Farm vector append and ownership teardown',
            'Three full reloads per object retain appended click offsets and reset all 48 parking offsets; separate missing/oversized-row/column/comma fixtures',
            'Twelve goal indexes across four equal levels plus independent random level combinations; exact target floats and ordered upgrade queries',
            'Four button-state frame/overlay/render-flag branches, fractional position changes, native clipping-mask rendering and actual mouse click/release traversal',
            'Normal and exhausted offset queues keep the final target, with different counter/callback ordering and explicit uint wrap test',
            'Actual tutorial-44 cancellation request and ordinary Button clicked/sound behavior; source/native task target and callback-time index observations',
            'Three explicitly seeded native +368 words confirm stored extent accessor; reused/uninitialized original storage remains unproved',
            '84 complete GPU frames with identical sprites, D3D calls and 40,320,000 pixels in both precisions',
        ],corrections=[
            'Tutorial 44 sets requestCancel (004cdf10 writes true), rather than consuming the request',
            'Area kind 4 uses WaterSource and kind 16 DigBuilding level; index 2 independently uses DigBuilding, index 6 MowBuilding',
            'ActionArea MI table has two function pointers followed by zero; the diagnostic preserves its exact table length',
        ],test_adapters=[
            'Production never loads original methods; only diagnostic instructions are mapped and their absolute operands recorded',
            'Source Farm registration provider records pointers; the original Farm vector append executes directly in an explicitly allocated diagnostic base',
            'Upgrade levels and Person queue/marker entry are observed primitive boundaries, not shipping world providers; real Person effects remain required',
            'Real source/native TutorialManager, owned tooltip/UI and profile/resource interfaces execute with declared old CRT adapters',
        ],pending=[
            'Actual Farm and Person task/marker providers, full derived Warehouse/WaterSource/DigBuilding/MowBuilding/FarmField/SeedPackage/Sprinkle controllers',
            'Meaning and initialization of native +368 across the complete real world, allocation/invalid-index/empty-list exception behavior',
            'Full cGame, actual tutorial/world/audio and bidirectional saves, source game EXE, complete Squall and isolated source uninstaller',
        ])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('ActionArea: 17,842 states, 14,784 goals, 84 frames/40,320,000 pixels; 17 originals unchanged')


if __name__=='__main__':
    main()
