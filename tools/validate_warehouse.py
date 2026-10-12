#!/usr/bin/env python3
"""Compare the actual Warehouse/Manufacture/production/HUD controller chain."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/warehouse-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    reference=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(reference/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','warehouse'),('PC24','warehouse-device')]:
        result=subprocess.run([str(oracle),str(reference/'Dobrý farmář.exe'),str(reference/'Data'),mode],capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Warehouse {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['warehouse_states']==804 and state['warehouse_objects']==10 and state['warehouse_transfers']==512
        assert state['warehouse_frames']==24 and state['warehouse_pixels']==11520000
        assert state['production_house_objects']==0 and state['production_placement_states']==24576
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,**state));unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/name for name in (
        'CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/build_sprite_oracle_fixups.py','tools/warehouse_oracle_cases.inc','tools/warehouse_oracle_services.h',
        'tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h','tools/production_house_oracle_cases.inc',
        'tools/production_house_oracle_services.h','tools/production_placement_oracle_cases.inc','tools/action_area_oracle_services.h',
        'tools/tutorial_manager_oracle_services.h')]+[Path(__file__).resolve()])
    report=dict(status='passed_for_warehouse_and_actual_manufacture_production_hud_chain',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(reference/'Dobrý farmář.exe'),
        reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        states=1608,objects=20,transfers=1024,frames=48,pixels=23040000,
        original_functions=['00467fe0','00466020','00467220','00467910','00466530','00466110','00467fa0','00465e70','00467ec0','004682d0','00466a40','00465ee0','00466e30','00448b20','0045f210','004678b0','004953e0','004beb40','004bd9d0'],
        coverage=[
            'Actual shipped Warehouse.xml, four level appearances/masks and owned Loading/LoadingSubstrain/UpgradeButton/CostText controls',
            'Twenty source/native constructor/destructor pairs: saved levels 0..3, zero/nonzero stocks, exact occupied sum and uint free-space wrap',
            'Goods invalid-product gates, Investment indexes 1..level-1 and 1024 Give/Take calls with zero/oversized quantities and all notification flags',
            'Actual Manufacture constructor/factory owns a real SheepFold. Real Warehouse changes refresh its actual input label and the actual HUD',
            'Actual ProductionBuilding UI callbacks consume and return Warehouse inputs; absoluteStep completes production into that same actual Warehouse output/profile/HUD',
            '216 affordability/availability configurations in both precisions, including signed negative availability words; upgrade callbacks mutate actual Upgrade profile/UI and shared sound context',
            'Farm 64/32 gate short circuit and actual tutorial-44 cancel request, with declared Person queue observations',
            'Actual mouse hover/press/release/leave and full 48 frames across all four appearances, exact ordered sprites/D3D calls and 23040000 pixels',
            'Missing/partial/full XML reloads and direct level load with absent Hovered, original cumulative Upgrade levelCount and native ownership reference deltas'],
        test_adapters=[
            'Only diagnostic instructions execute the original PE; production uses editable source controllers',
            'GPU mask service remains recovered loadClickMask; the independent click-mask oracle separately compares all 432 JNG/native BGRA masks through actual original D3DX for seven native images',
            'Farm registration uses real native vector append over reserved StateView diagnostic storage; full Farm and Person marker/task effects remain pending',
            'Market and WareDetail refresh boundaries record receiver, order and current occupied/progress snapshots, rather than implementing false world providers',
            'Money reads real profile; cGame SetMoney boundary records/clamps the value while other money notifications/upgrade refreshes await cGame',
            'Actual recovered SoundManager with disabled physical backend records sound/channel flow; independent physical game-audio oracle covers its own declared scope',
            'Purchase panel is a required unexercised fixture path that fails if entered; no production fallback is supplied',
            'The existing separate 55-house and Manufacture tests remain required; this mode does not repeat those fixtures in its fixed 512 MiB heap'],
        pending=[
            'Full Farm/Person/world providers, whole tutorial playthrough including arrow-26 gameplay predicate and actual task execution',
            'Real market/WareDetail effects and complete cGame Money notifications/initial world construction',
            'Malformed/missing initial XML, invalid/uninitialized level/capacity/area extent and exceptional construction/old vector ABI',
            'Full source game EXE and gameplay systems, bidirectional saves, complete Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Warehouse: 1608 states, actual Manufacture/production/HUD, 48 frames/23040000 pixels; 17 originals unchanged')
if __name__=='__main__':main()
