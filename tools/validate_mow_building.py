#!/usr/bin/env python3
"""Compare actual MowBuilding and its held-product, upgrade and tutorial behavior."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/mow-building-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','mow-building'),('PC24','mow-building-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'MowBuilding {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=874,actions=118,frames=60,pixels=28800000).items():assert state['mow_building_'+key]==value,(key,state['mow_building_'+key])
        assert state['warehouse_states']==804 and state['warehouse_frames']==24 and state['warehouse_transfers']==512
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/mow_building_oracle_cases.inc','tools/mow_building_oracle_services.h','tools/warehouse_oracle_cases.inc',
        'tools/warehouse_oracle_services.h','tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h',
        'tools/build_sprite_oracle_fixups.py','tools/monotonic_ui_test_heap.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_mow_building_held_products_ui_and_explicit_person_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),
        reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        states=1748,explicit_actions=236,gpu_frames=120,gpu_rgb_pixels=57600000,xml_reloads=6,real_mouse_upgrades=2,tutorial_starts=4,
        original_functions=['00449560','004471a0','00447260','004472f0','004473b0','00447490','004474b0','00447b40','00447f30','00448330','004483e0','00448b60','00449050','0044ec00','0044ed10','004036e0'],
        coverage=['Actual MowBuilding.xml: four native224-byte levels, ProductionNum/MowTime and four appearance/mask records, item11/property31, owned upgrade/price/sign/substrate controls',
            'Real native Person held getter and eight-element uint PCC sum; 512 held/level/PCC patterns per precision include sum wrapping to zero and each original product icon',
            'Bare appearance for held6; normal appearance and first nonzero PCC product icon when level<3 and held sum nonzero',
            'Conditional Person presence, actual held-indicator hide and observed display arguments (true,count,ProductionNum)',
            '108 availability/Money combinations per precision, original upgrade/animation arguments kind=level+4, original sound and appearance refresh order',
            'Four Farm cursor gate combinations, full normal/bare hover/press/release/leave through actual input and cursor, real mouse upgrade',
            'Every RGB pixel in60 complete D3D8 frames per precision including all eight held-product icons',
            'Actual tutorial27/47 arrow/finishTip and HUD stop/resume with actual Mow investment; full/partial/full XML reloads retain cumulative levelCount and resource references'],
        test_adapters=['Original PE maps for diagnosis only, entry point never executes and production libraries never load original code',
            'Controlled clock/input and typed XML/resource/CRT adapters; monotonic diagnostic heap',
            'Native mask adapter shares separately verified JNG/BGRA loading',
            'Farm is still a StateView fixture; Person held getter and held sum execute original bodies, display/animation/appearance refresh are explicit observed boundaries',
            'Person mock owns a real native/source View for indicator hide; actual full Person display is not claimed reconstructed',
            'Tutorial scene temporarily detaches from hidden game owner; cGame Money uses observed clamp fixture and audio backend is disabled'],
        pending=['Actual Farm and full Person constructor/display/task/animation behavior, cGame Money effects and whole source game EXE',
            'Full game/tutorial playthrough, bidirectional saves and complete audio equivalence',
            'Additional constructors, malformed XML, invalid levels/allocator/exception ABI and additional rounding/locales'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('MowBuilding: 1748 states, 236 actions, 120 frames / 57600000 identical RGB pixels; actual PCC getters/tutorial, 17 originals unchanged')
if __name__=='__main__':main()
