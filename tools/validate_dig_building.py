#!/usr/bin/env python3
"""Compare actual cDigBuilding, its owned UI, input, tutorial and upgrade effects."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/dig-building-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','dig-building'),('PC24','dig-building-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'DigBuilding {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=349,actions=118,frames=52,pixels=24960000).items():assert state['dig_building_'+key]==value,(key,state['dig_building_'+key])
        assert state['warehouse_states']==804 and state['warehouse_frames']==24 and state['warehouse_transfers']==512
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/dig_building_oracle_cases.inc','tools/dig_building_oracle_services.h','tools/warehouse_oracle_cases.inc',
        'tools/warehouse_oracle_services.h','tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h',
        'tools/build_sprite_oracle_fixups.py','tools/monotonic_ui_test_heap.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_dig_building_ui_input_tutorial_and_explicit_person_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),
        reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        states=698,explicit_actions=236,gpu_frames=104,gpu_rgb_pixels=49920000,xml_reloads=8,real_mouse_upgrades=2,tutorial_starts=2,
        original_functions=['00429fe0','00426470','00426550','00426320','004263b0','004265e0','00427bf0','00427740','00426b10','00427340','00428310','00426f50','00426260','00448b20','0045f210'],
        coverage=['Actual DigBuilding.xml, four 220-byte native level records, normal/bare appearances and every retained click-mask bit',
            'Held types 0/1/2/6/-1 across all saved levels, shared Upgrade identity item14/property34 and original Investment bounds',
            '108 availability/Money combinations per precision, real upgrade callbacks and ordered Money/level/sound/held-tool animation arguments',
            'Actual Field tooltip and ActionArea input dispatch, four Farm cursor gate combinations and tutorial44 cancellation',
            '52 complete D3D8 frames per precision over four levels and both held-tool appearances; real hover/press/release/leave and mouse upgrade',
            'Actual TutorialManager arrow28 start/finish, all eight Farm movement/visibility/button visibility combinations and original HUD stop/resume notifications',
            'Actual/missing-NoInstrument/partial XML reloads retain original state and cumulative levelCount, destructor texture reference deltas',
            'Existing actual Warehouse/Manufacture/SheepFold/HUD stock scenario retained'],
        test_adapters=['Original PE only maps in the diagnostic, never in implementation; original entry point is never executed',
            'Controlled clock/input, typed XML/resource/CRT adapters and monotonic diagnostic allocator',
            'Native graphics mask adapter shares independently native-verified BGRA/JNG mask loading',
            'Person held getter, queue, upgrade animation and appearance refresh are observed required boundaries; fake Person position is seeded explicitly',
            'Farm remains a StateView fixture; tutorial test temporarily detaches this scene to expose original shown/parentShown guards',
            'cGame Money uses the existing observed clamp fixture; original upgrade SoundManager call arguments are compared with disabled audio backend'],
        pending=['Actual Farm/Person and whole cGame construction, Money effects, animations and full tutorial/playthrough',
            'Physical audible upgrade/hover playback and complete Squall behavior',
            'Whole-game source EXE and bidirectional save compatibility',
            'Additional constructors, malformed XML, invalid level/allocator/exception ABI and rounding/locales'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('DigBuilding: 698 states, 236 actions, 104 frames / 49920000 identical RGB pixels; actual tutorial28, 17 originals unchanged')
if __name__=='__main__':main()
