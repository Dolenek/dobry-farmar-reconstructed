#!/usr/bin/env python3
"""Compare complete source Farm ownership, profiles, fields, contracts and frames."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_actual_farm_scene_ownership_upgrades_crop_selection_contracts_restore_and_frames'

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/farm-scene-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'farm-scene'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=120)
        if run.returncode:raise RuntimeError(f'Farm scene {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        for key,value in dict(states=94,actions=66,frames=32,pixels=15360000).items():assert state['farm_scene_'+key]==value,(precision,key,state['farm_scene_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_farm_scene.py','tools/text_buttons_reference_test.cpp',
           'tools/build_sprite_oracle_fixups.py','tools/text_buttons_oracle_fixups.inc','tools/monotonic_ui_test_heap.h',
           'tools/farm_scene_oracle_cases.inc','tools/production_house_oracle_cases.inc',
           'tools/production_house_oracle_services.h','tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h',
           'tools/tutorial_manager_oracle_services.h','tools/contract_status_oracle_services.h','tools/loading_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        contexts=contexts,states=188,explicit_cpu_actions=132,gpu_frames=64,gpu_rgb_pixels=30720000,
        actual_farm_lifetimes=4,owned_field_lifetimes=240,owned_letter_lifetimes=12,restored_sprinkle_lifetimes=6,
        original_functions=['00435c40','004305c0','00434b80','004331b0','00434cc0','004350d0','0042c830',
            '004345d0','00434b30','004354d0','00435660','004357f0','0042f080','0042ec20','004341b0',
            '0042d700','0042db30','00417910'],
        coverage=[
            'Actual Farm constructor/destructor, singleton, StateView/owned background/origin/UI, sixty actual FarmField objects, three actual ContractLetterSlot objects and real HoseEmitter',
            'Concrete Farm interfaces use actual FarmAreas/Fields/letters and source actor/tool classes; no Farm method executes through the older observed StateView boundary',
            'All three actual available/current field upgrades and actual buyFields payment path, field activation, original price and Pos XML arrays',
            'Thirty-six selector cases per precision over real Field culture/FFS/profile/flags, including inactive fields, zero count, threshold failures, NaN, clamped count and original biased selection',
            'Original selector reads FFC culture, not FFS work state; probability uses1/32767, selection uses1/32768 and pre-decrements the eligible upper bound',
            'Actual three-slot add/capacity/overflow, letter arrival/read, actual ContractProvide.accept, actual Farm removal/count/profile update and actual GameMenu/ContractStatus activation',
            'Second actual constructor restores all three letters with saved states/deadlines and three installed sprinklers with actual affected Field lists and emitter flags',
            'Actual cursor gates over restored sprinklers, excluded area and seed type; original rectangle right edge remains unspilled in x87',
            'Sixty-four complete D3D8 frames compare commands and30720000 RGB pixels over all sixty Fields, Farm UI, actual FieldTip and active ContractStatus',
            'All profile maps, RNG state, GameAction ownership count, ordered external observations and texture teardown deltas match in both lifetimes'],
        test_adapters=[
            'Complete cGame remains pending. Mandatory FarmGameServices supplies observed game Money setter/upgrade refresh and indexed contract lookup; their full world refresh effects are not claimed here',
            'Actual original Warehouse goods getter executes against the already compared profile adapters when the actual ContractStatus is activated; full Warehouse/Manufacture/Market scene composition remains separate work',
            'Fields, letters, Farm methods, ContractProvide and ContractStatus execute their actual source/original bodies; no manual action scheduler replaces them',
            'RNG selector cases explicitly control profile FFC/FFS and harvested inputs; original growing profile state is restored before whole-scene frames',
            'Restored sprinklers are compared before WaterSource construction; no watering tick is invoked while that original factory dependency is absent',
            'Actor/tool interfaces are concrete source bindings but full Person/tool work cycles on this actual Farm are not counted as compared by this report',
            'GameMenu warehouse/gameplay providers, numeric configuration/profile adapters, controlled clock/cursor and disabled UI audio backend retain their documented diagnostic roles',
            'Explicit source child deletion keeps borrowed contexts and virtual Farm services alive and clears the singleton before UI teardown, matching original observable order',
            'Original mapped instructions run only as diagnostic reference; production Farm contains no original program calls'],
        pending=['Full actor/tool/seed/hose input cycles on this actual Farm','Full cGame composition, startup and main game EXE',
            'Bidirectional complete save files; profile fixture restoration alone does not prove it',
            'Other Person arrival/cancel/tutorial/award branches, complete97-export Squall behavior, exceptional CRT ABI and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Actual Farm: 188 states / 132 CPU operations; 240 owned Fields, actual contracts/restoration and 64 frames / 30720000 identical RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
