#!/usr/bin/env python3
"""Compare real WareDetail tooltip integrated with Warehouse, production and HUD."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/warehouse-tip-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    reference=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(reference/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','warehouse-tip'),('PC24','warehouse-tip-device')]:
        result=subprocess.run([str(oracle),str(reference/'Dobrý farmář.exe'),str(reference/'Data'),mode],capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'WarehouseTip {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['warehouse_tip_states']==1050 and state['warehouse_tip_frames']==20 and state['warehouse_tip_pixels']==9600000
        assert state['warehouse_tip_reloads']==6 and state['warehouse_states']==1049 and state['warehouse_objects']==10
        assert state['warehouse_transfers']==512 and state['warehouse_frames']==24 and state['warehouse_pixels']==11520000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,**state));unchanged()
    names=('CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc','tools/build_sprite_oracle_fixups.py',
        'tools/warehouse_tip_oracle_cases.inc','tools/warehouse_tip_oracle_setup.inc','tools/warehouse_oracle_cases.inc','tools/warehouse_oracle_services.h',
        'tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h','tools/production_house_oracle_cases.inc','tools/production_house_oracle_services.h',
        'tools/production_placement_oracle_cases.inc','tools/action_area_oracle_services.h','tools/tutorial_manager_oracle_services.h')
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/name for name in names]+[Path(__file__).resolve()])
    report=dict(status='passed_for_warehouse_tip_and_actual_stock_hud_production_chain',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(reference/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,oracle_sha256=digest(oracle),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        states=2100,frames=40,pixels=19200000,connected_warehouse_states=2098,connected_warehouse_frames=48,connected_warehouse_pixels=23040000,
        original_functions=['004658b0','00465a10','004645b0','00464410','00464b40','00465050','00465460','004654b0','00465a70','004bd8f0','004bd920'],
        coverage=[
            'Actual UI/gameUI/WareDetail.xml, native 884-byte cWareDetail, 19 display rows and actual Picture/TextLine/MovableUnion ownership',
            'Real Warehouse/profile goods compact into nonzero rows with actual small/large product appearances and unsigned x quantity labels',
            'All 512 existing Warehouse transfers per precision retain actual Manufacture/production/HUD flow and now call real WareDetail refresh instructions/source rather than observer arithmetic',
            'Actual HUD WarehouseButton hover opens the real tooltip; leave arms VisibleTime while opened remains true',
            'Incoming/outgoing motion, billion-second open timer, repeated timeout close and zero/negative/infinity/NaN stored timer words',
            '64 deterministic sparse/zero/all/random uint stock patterns per precision with both icon classes and actual loading/HUD',
            'Missing/zero/three/oversized/full XML reloads preserve old owned children, clamp rows to 19 and replace row pointers in the original way',
            '40 complete GPU frames / 19200000 identical pixels include open panel motion, populated compacted rows and timed hide; connected Warehouse still supplies its own 48 frames'],
        test_adapters=[
            'Original PE methods run only in independent diagnostic mapping; production sources never load/call original instructions',
            'Farm/Person, market, cGame SetMoney and physical audio boundaries retain the explicitly recorded Warehouse audit scope',
            'Actual WarehouseTip, HUD and stock controllers now replace the prior WareDetail/open/hide observer boundaries in this mode',
            'Original +36c is uninitialized by ctor; both diagnostic timer words are explicitly seeded before absolute dispatch, without claiming ordinary allocator parity',
            'Texture/mask services use previously independently compared source loaders; current controller comparison requires their separate native validation reports',
            'XML reload can retain old visible children. Pixel traversal includes those old children as well as current row pointers'],
        pending=[
            'Full cGame initial allocator/hidden tooltip startup and exceptional/missing initial ProductInfo or malformed XML behavior',
            'Actual Farm/Person task execution, complete market/cGame world initialization and tutorial playthrough',
            'Main source game EXE, all world systems and bidirectional saves, complete Squall and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('WareDetail: 2100 states, actual Warehouse/HUD/production, 40 frames/19200000 pixels; 17 originals unchanged')
if __name__=='__main__':main()
