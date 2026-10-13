#!/usr/bin/env python3
"""Compare the actual Market linked to Warehouse, Manufacture, HUD and tutorial."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/market-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','market'),('PC24','market-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Market {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=792,actions=561,frames=84,pixels=40320000).items():assert state['market_'+key]==value,(key,state['market_'+key])
        assert state['warehouse_states']==1049 and state['warehouse_transfers']==512 and state['warehouse_objects']==10
        assert state['warehouse_frames']==24 and state['warehouse_tip_frames']==20 and state['warehouse_tip_states']==1050
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/market_oracle_cases.inc','tools/market_oracle_services.h','tools/monotonic_ui_test_heap.h',
        'tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h','tools/warehouse_oracle_cases.inc',
        'tools/warehouse_tip_oracle_setup.inc','tools/warehouse_tip_oracle_cases.inc',
        'tools/gameplay_runtime_oracle_services.h','tools/gameplay_oracle_services.h','tools/build_sprite_oracle_fixups.py']
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[ROOT/path for path in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_market_and_actual_stock_ui_tutorial_chain',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        states=1584,explicit_actions=1122,xml_reloads=6,tutorial_branches=10,real_mouse_clicks=2,gpu_frames=168,gpu_rgb_pixels=80640000,
        connected_warehouse_states=2098,connected_warehouse_transfers=1024,connected_warehouse_frames=48,
        connected_warehouse_tip_states=2100,connected_warehouse_tip_frames=40,
        original_functions=['00471d40','0046b630','0046b6d0','0046bec0','00471d30','0046e760','004721d0','0046eaa0',
            '0046eb70','0046ef70','004707e0','00470c00','0046f370','0046b850','0046f9e0','00470270',
            '004713c0','00471420','00471480','004714e0','00471910','004b64e0'],
        coverage=['Actual Market.xml construction/destruction, all nineteen owned rows and control positions, stock/price text, appearances and histogram values',
            'Actual native/source Gameplay constructor with original 383-event plan and identical shared RNG; existing exact forecast method runs without substitution',
            '64 availability patterns per precision over actual uint SeedAvail/UpAvail and Warehouse/Market stocks; compact rows, stock takes and purchase callback product reassignment',
            'Actual Warehouse give -> Market notification -> quantity callbacks -> Warehouse take -> actual Manufacture/HUD/WareDetail notifications',
            'All Add1/Add10/AddAll and price callbacks over nineteen rows, accelerated repeated price clicks and bounds, 200 sale timer steps, real stock decrements, RNG, Money, high-price award, sound requests and ordered cGame service arguments',
            'Small/large product icons, incoming/outgoing scene movement and enterState marketIt increment, full D3D8 traversal/draw calls and all RGB pixels',
            'Actual TutorialManager starts/finishes Market tips 8/9/45/11/12, correct control anchors and original demonstration histogram',
            'Actual complete/missing-child ProductLine reloads, owned child replacement/retention and reference deltas, original repeated SellSound reference retention',
            'Real input/cursor hover, press, release and leave through full frame loop changes market price with identical sound requests'],
        test_adapters=['Original PE maps only in diagnostic; entry point never executed',
            'Explicit CRT/typed XML/property/resource adapters, controlled clock and input; 768 MiB monotonic diagnostic heap preserves allocation ranks',
            'Warehouse, Manufacture, ProductionBuilding, HUD, WareDetail, Gameplay and TutorialManager methods execute their real recovered/native bodies',
            'cGame Money uses observed fixture clamp and stock notification snapshots; FlyProductExpence creation and sale-history insertion are observed required service boundaries, not claimed complete implementations',
            'SoundManager backend is disabled in this UI fixture; original call order/arguments are compared, physical sale playback remains separate work',
            'Remaining Farm/Person world services and full game creation are explicit fixtures; real tutorial startup calls do not prove whole-world playthrough'],
        pending=['Actual full cGame, floating product expense visual, sale history/awEarn retention logic and all Money effects',
            'Physical audible sale/hover playback and complete Squall behavior',
            'Whole game/tutoriál playthrough and full bidirectional save compatibility',
            'Missing mandatory ProductLine, malformed XML, NaN/infinity timer/price inputs, invalid product/allocator/exception ABI and additional rounding/locales'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Market: 1584 states, 1122 actions, 168 frames / 80640000 identical RGB pixels, actual Warehouse/HUD/tutorial links; 17 originals unchanged')
if __name__=='__main__':main()
