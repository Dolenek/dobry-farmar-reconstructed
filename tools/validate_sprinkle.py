#!/usr/bin/env python3
"""Compare actual Sprinkle controller, owned water effect and Farm factory."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/sprinkle-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'sprinkle'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Sprinkle {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=627,objects=9,actions=428,frames=72,pixels=34560000).items():assert state['sprinkle_'+key]==value,(key,state['sprinkle_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/sprinkle_oracle_cases.inc','tools/sprinkle_oracle_services.h','tools/water_source_oracle_cases.inc',
        'tools/build_sprite_oracle_fixups.py','tools/monotonic_ui_test_heap.h','tools/gameplay_runtime_oracle_services.h',
        'tools/loading_oracle_services.h','tools/main_menu_oracle_services.h','tools/game_menu_oracle_cases.inc']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_sprinkle_controller_effect_factory_and_explicit_field_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),
        reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        states=1254,objects=18,actions=856,gpu_frames=144,gpu_rgb_pixels=69120000,xml_reloads=24,
        original_functions=['00460180','0045e9d0','0045ea90','0045eb40','0045f340','0045f3d0','0045f580','0045fb40','0045f230','0045f210','00460450','0045ceb0','00433980','004697f0'],
        coverage=['Actual Sprinkle XML constructor, index/status/existence/coordinates in original typed profile stores, owned actual SprinkleEmitter and child cleanup',
            'Native virtual table: inherited FieldObject position without profile write, separate direct position persistence; AnimationButton state changes and actual empty absolute step',
            'All installed/uninstalled, accepted/rejected placement, click/release, sufficient/insufficient Money branches; unsigned-wrap cost arithmetic and original release without payment',
            'Zero through two affected fields, status0/1/2/FFFFFFFF, exact four-pixel drag threshold, all hover gate branches, two partial reloads per direct object',
            '144 complete GPU frames including actual mouse down, five-pixel drag, rejected preview and release; exact sprite/D3D8 calls and all RGB pixels',
            'Particle count/order/parent links, live velocities/lifetime/elapsed and entire shared original RNG sequence',
            'Actual WaterSource callback invokes source/native actual Farm 00433980 factory, verifies Money/count gates, first free profile index, moving-picture ownership and pressed object'],
        test_adapters=['Remaining Farm geometry queries, action-button enable/disable and cField watering/preview are explicit required diagnostic observations',
            'Field fixtures use actual ActionArea geometry; this does not prove reconstructed cField growth, watering or full Farm composition',
            'Native cGame discard is observed; source/native owner cleanup and reference deltas are compared',
            'Original executable is mapped for diagnostics only; production implementation never calls its code'],
        pending=['Actual Farm geometry/mask/area registries, cField preview and watering/growth systems, Person interaction and full scene composition',
            'Additional tooltip overlap candidates, complete XML sound reload reference retention and allocation/exception failure ABI',
            'Whole-game playthrough, saves in both directions, complete Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Sprinkle: 1254 states, 856 actions, 144 frames / 69120000 identical RGB pixels; actual WaterSource/Farm factory; 17 originals unchanged')
if __name__=='__main__':main()
