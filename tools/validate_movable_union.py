#!/usr/bin/env python3
"""Compare the actual common moving Picture base of both purchase panels."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/movable-union-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[];counts=dict(states=5802,moves=518,steps=4096,frames=72,reloads=2)
    for precision,mode in [('PC53','movable-union'),('PC24','movable-union-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'MovableUnion {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['movable_union_'+key]==value,(key,state['movable_union_'+key])
        assert state['hud_frames']==72 and state['hud_pixels']==34560000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',
        ROOT/'tools/text_buttons_oracle_fixups.inc',ROOT/'tools/game_menu_oracle_cases.inc',ROOT/'tools/game_menu_oracle_services.h',
        ROOT/'tools/movable_union_oracle_cases.inc',Path(__file__).resolve()])
    report=dict(status='passed_for_movable_union_picture_motion_and_hud_overlay_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},pixels=69120000,
        original_functions=['00413780','00413fa0','00414070','00414430','004147f0','004148e0','00414d60'],
        coverage=['Actual cMovableUnion common Picture base used by both BuySeeds and BuyBuilding, owned origin and Picture background list',
            'Both actual shipped XML definitions and repeated reloads of their empty Background sections, MoveSpeed/HiddenPlace/original position',
            '1036 movements and 8192 steps with exact x87 duration/reciprocal spill/velocity/position, outgoing/incoming/state-zero and degenerate zero-speed/distance paths',
            'Actual GameMenu uses these bases as both overlay world objects while source/native scene callbacks execute; 144 GPU frames, ordered sprites and 69120000 identical RGB pixels',
            'Native ownership destruction and texture references, exact origin rise/unrise state'],
        test_adapters=['Mapped original instructions only in diagnostic executable; source never calls original program code',
            'Explicit XML/old CRT/resource adapters and reconstructed frame/timer/cursor',
            'Speed/remaining unspecified native constructor fields explicitly seeded in diagnostic allocation',
            'Degenerate probes can produce NaN child geometry; explicit diagnostic restoration to recorded initial positions precedes visible valid-world frames',
            'The actual shared bases replace HUD overlay fixtures, but the full derived BuySeeds/BuyBuilding purchase controllers are still pending'],
        pending=['Actual BuySeeds/BuyBuilding controllers, their definition records and purchases connected to Farm/warehouse/character',
            'Populated Background append/ownership fixture, full parent-disabled/callback/invalid XML/exception and overflow NaN payload matrices',
            'Full game EXE and bidirectional saves, complete Squall and isolated uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('MovableUnion: 11604 states, 1036 moves, 144 GPU frames/69120000 pixels; 17 originals unchanged')
if __name__=='__main__':main()
