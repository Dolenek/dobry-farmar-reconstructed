#!/usr/bin/env python3
"""Compare actual instrument transfer and connected tool upgrades against original instructions."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/elastic-animation-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']: assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'elastic-animation'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=120)
        if run.returncode: raise RuntimeError(f'ElasticAnimation {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        assert state['elastic_states']==762 and state['elastic_actions']==677
        assert state['elastic_frames']==84 and state['elastic_pixels']==40320000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_elastic_animation.py','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/build_sprite_oracle_fixups.py','tools/monotonic_ui_test_heap.h','tools/elastic_animation_oracle_cases.inc',
        'tools/mow_building_oracle_cases.inc','tools/game_menu_oracle_cases.inc','tools/warehouse_oracle_cases.inc']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    total=lambda key:sum(c['result']['elastic_'+key] for c in contexts)
    report=dict(status='passed_for_elastic_instrument_motion_actual_tool_upgrades_reveal_and_frames',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,
        original_program_code_used_in_implementation=False,oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts,states=total('states'),actions=total('actions'),gpu_frames=total('frames'),gpu_rgb_pixels=total('pixels'),
        original_functions=['0042c2d0','0042b1a0','0042b280','0042b7a0','0042bce0','00426b10','00448b60','00469bd0','00426260','004471a0','00468420'],
        coverage=[
            'Actual constructor, singleton, ten XML appearances, rotation/speed/acceleration, optional movement sound resource and full destructor ownership',
            'All ten instrument kinds across all quadrants, vertical/horizontal/near/equal/signed-zero paths, restart in flight and exact-duration expiration',
            'Exact normalization with unspilled X coefficient, duration arithmetic, remaining time, Euler position/velocity and rotation in PC53 and PC24',
            'Zero/negative/slow/fast speeds and zero acceleration with original unordered duration branch; no final destination snap',
            'Actual DigBuilding, MowBuilding and WaterSource instances refresh their real appearances at completion; actual hidden View is revealed for kind9',
            'Actual three tool upgrade controllers deduct money, update profiles, launch real flight and complete it through the registered UI frame',
            'Real Mow upgrade mouse press/release launches real source/native flight and finishes with actual building appearance',
            '168 complete D3D8 frames compare commands and every RGB pixel; native inherited Picture animation and absolute flight both execute',
            'Three XML reloads including partial attributes preserve original retained sound references; current sound and all texture teardown deltas match'],
        test_adapters=[
            'Full tool buildings and ElasticAnimation are actual recovered classes; Person position, held appearance and cancellation remain explicit observed boundaries',
            'A real View supplies the kind9 reveal boundary; the future full Person seed-drop action is not claimed by this reveal test',
            'Sound requests and resource references are compared through the recovered SoundManager with the existing disabled diagnostic backend; this is not an audible/refill test',
            'Controlled clock drives actual GameTimer/UI elapsed behavior in rendered scenes; direct arithmetic cases supply explicit elapsed values',
            'Native CRT normal math and SSE2 floor settings are initialized as in earlier connected UI tests; exceptional CRT ABI remains pending',
            'Native code is mapped only as reference; production class contains no calls to original instructions or original DLL forwarding'],
        pending=['Full Person constructor, arrival and timed work actions including seed drop and carrying transitions',
            'Full Farm/cGame EXE and bidirectional complete save files','Complete97-export Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(f"ElasticAnimation: {total('states')} states / {total('actions')} actions, {total('frames')} frames / {total('pixels')} identical RGB pixels; actual tool upgrades and mouse; 17 originals unchanged")
if __name__=='__main__': main()
