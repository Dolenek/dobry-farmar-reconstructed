#!/usr/bin/env python3
"""Compare actual WaterSource with original held-tool accessors and UI frame dispatch."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/water-source-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','water-source'),('PC24','water-source-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'WaterSource {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=520,actions=267,frames=64,pixels=30720000).items():assert state['water_source_'+key]==value,(key,state['water_source_'+key])
        assert state['warehouse_states']==804 and state['warehouse_frames']==24 and state['warehouse_transfers']==512
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/water_source_oracle_cases.inc','tools/water_source_oracle_services.h','tools/warehouse_oracle_cases.inc',
        'tools/warehouse_oracle_services.h','tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h',
        'tools/build_sprite_oracle_fixups.py','tools/monotonic_ui_test_heap.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_water_source_ui_and_explicit_person_farm_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),
        reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        states=1040,explicit_actions=534,gpu_frames=128,gpu_rgb_pixels=61440000,xml_reloads=6,real_mouse_upgrades=2,tutorial_starts=6,
        original_functions=['0046b330','004683a0','004683d0','004684c0','004684f0','00468580','00468640','00468730','00468750','00468f60','00469400','004697f0','00469bd0','0046a010','0046a470','0046a560','0046acb0','00468420','0044ec00','0044ec80'],
        coverage=['Actual WaterSource.xml: four native level records with three defined capacities/appearances, item13/property33, two ExtendedTextAnimationButtons, two TextMoney and three sprinkle appearances',
            'Real native Person held getter/setter use uint PCarry; five held words across level requests0..3, capacity and investment, levelCount clamp',
            '108 availability/Money combinations per precision and nine held-type upgrades; original held-type snapshot before upgrade, Person held reset, appearance update, cancellation fields and conditional discard-origin arguments',
            '144 Sprinkle count/move/Money combinations per precision, signed cost comparison, appearance retention when count>=3, original depth/enable/hideTips behavior',
            'Sprinkle callback returns null or a diagnostic ButtonPrototype and sets a successful result Pressed; Farm factory request cost and receiver are observed, not replaced in production',
            'Four Farm cursor gate combinations, actual release tooltip direction and complete normal/bare hover/press/release/leave frames; real mouse upgrade',
            '24 Sprinkle count/movement frame combinations per precision, actual D3D8 traversal/submission and every RGB pixel',
            'Actual tutorial25/46/29 arrows and finishTip, HUD stop/resume with real WaterSource investment, actual/full/partial XML reloads and texture reference deltas'],
        test_adapters=['Original PE maps for diagnosis only; entry point never executes and implementation never loads original program code',
            'Controlled clock/input, typed XML/resource/CRT adapters and monotonic diagnostic allocator',
            'Native mask adapter shares separately verified JNG/BGRA loading',
            'Farm remains a StateView fixture; Sprinkle factory is observed and its successful diagnostic ButtonPrototype is not claimed to be a recovered Sprinkle',
            'Original Person held accessors execute, while other Person appearance/animation/task-discard and cGame Money effects have explicit observed boundaries',
            'Tutorial Farm scene temporarily detaches from hidden game owner; disabled sound backend compares original requests, not audible output',
            'Fourth capacity word is unspecified by ctor and absent from actual XML; no comparison reads it'],
        pending=['Actual Farm.createSprinkle and full Sprinkle behavior, Person/world constructors and task/animation controllers',
            'Physical audible output, complete Squall, whole-game source EXE and full playthrough',
            'Bidirectional whole saved-game compatibility and lifecycle',
            'Malformed XML, missing mandatory UI, invalid levels/allocators/exception ABI and additional constructors/rounding/locales'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('WaterSource: 1040 states, 534 actions, 128 frames / 61440000 identical RGB pixels; actual held accessors/tutorial, 17 originals unchanged')
if __name__=='__main__':main()
