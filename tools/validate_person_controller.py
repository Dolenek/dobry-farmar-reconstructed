#!/usr/bin/env python3
"""Compare full Person actor and connected timed work against original instructions."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/person-controller-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']: assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'person-controller'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=120)
        if run.returncode: raise RuntimeError(f'Person controller {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        assert state['person_controller_states']==231 and state['person_controller_actions']==142
        assert state['person_controller_frames']==64 and state['person_controller_pixels']==30720000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_person_controller.py','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/build_sprite_oracle_fixups.py','tools/monotonic_ui_test_heap.h','tools/elastic_animation_oracle_cases.inc','tools/person_controller_oracle_cases.inc','tools/farm_field_oracle_cases.inc',
        'tools/mow_building_oracle_cases.inc','tools/game_menu_oracle_cases.inc','tools/warehouse_oracle_cases.inc']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    total=lambda key:sum(c['result']['person_controller_'+key] for c in contexts)
    report=dict(status='passed_for_person_actor_lifetime_timed_work_and_rendered_field_input',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,
        original_program_code_used_in_implementation=False,oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts,states=total('states'),actions=total('actions'),gpu_frames=total('frames'),gpu_rgb_pixels=total('pixels'),
        original_functions=['0044f340','0044b3a0','0044b600','00449ad0','00451760','0045a2c0','00451c30','004590f0','0045c5a0',
            '00452bc0','00453380','0045a730','0045bbe0','00453b10','00453f20','00454320',
            '00454750','004547a0','00454ba0','00454fd0','00455020','00455420','00455850','00455c20','00456020',
            '004570b0','004574b0','0045bfa0','004578b0','00457c80','00458080','004584d0','004588b0','00458cb0',
            '00433040','0042f630','0044ec00','0044ec80','0044ed10','0044ed50','0044ed80','0044f0a0','0044f120'],
        coverage=[
            'Actual full Person constructor, FieldObject and ActionProcessor bases, singleton, owned UI/appearances/sounds and destructor interleaving',
            'Five valid carries0/1/2/6/11 across four Dig/Mow levels; actual profile position, idle UI, ordinary and hose idle steps, cancellation without a current marker and teardown',
            'Actual seven timed workers: take spade/scythe/bucket, Dig, Harvest, warehouse deposit and watering; original action factories and ActionProcessor begin/step/finish/replacement execute',
            'Actual Field culture/state/fertility/water, source random state, profile PCC/products and signed awGrowType, actual Warehouse acceptance and tool appearance refresh',
            'Exact animation/time progression and unordered-aware unspilled work completion arithmetic in PC53/PC24; Source work sounds call the real recovered manager',
            'Real Field mouse press/release creates actual PersonTasks; full actor chooses and executes vehicle travel, arrival dispatch, timed Dig and new idle without a manual motion or work scheduler',
            'Actual inherited ButtonPrototype.enabledStep executes in both normal and absolute actor processing as in original00449ad0',
            '128 complete D3D8 frames compare commands and every RGB pixel including automatic marker completion, deferred cleanup and actual Field change',
            'Texture teardown deltas and GameAction ownership counts match throughout actual actor/field construction, replacement and destruction'],
        test_adapters=[
            'Farm is the existing actual StateView with actual FarmAreas, tool buildings, Warehouse, Field and HoseEmitter objects; complete Farm scene class and cGame remain pending',
            'Remaining Farm indication and game Money/HUD refresh services use their explicitly recorded diagnostic boundaries; actor, queue, motion, arrival and the exercised work actions execute their real bodies',
            'Sound requests and resources use the recovered SoundManager with the existing disabled UI diagnostic backend; physical channel behavior is separately covered by PersonSounds and game-audio reports',
            'Native numeric configuration is adapted from the already compared source XML data, including actual product icons; no gameplay mechanics are invented',
            'Controlled clock drives actual GameTimer/UI elapsed behavior in the rendered scene; direct CPU cases supply explicit elapsed values',
            'This mode keeps Field services but leaves the independent full original-farm-field matrix to its own required CTest, avoiding redundant monotonic diagnostic heap allocation',
            'Native CRT normal math and SSE2 floor settings are initialized as in earlier connected UI tests; exceptional CRT ABI remains pending',
            'Mapped original code executes only as reference and is never called by reconstructed production classes'],
        pending=['Seed/sow/drop/exhaustion and full hose work/emitter cycles are covered separately by person-flows; other arrival branches remain pending',
            'Actual current-marker cancellation, repeated tutorial predicates and long queued award branches',
            'The newly reconstructed but not yet compared source arrival/worker branches are not counted as verified by this report',
            'Full Farm/cGame EXE and bidirectional complete saves','Complete97-export Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(f"Person controller: {total('states')} states / {total('actions')} actions, {total('frames')} frames / {total('pixels')} identical RGB pixels; actual actor, timed workers and automatic mouse-to-Field Dig; 17 originals unchanged")
if __name__=='__main__': main()
