#!/usr/bin/env python3
"""Compare the FieldObject tooltip base against its own original specialization."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/field-object-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    reference=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:
            assert digest(reference/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        result=subprocess.run([str(oracle),str(reference/'Dobrý farmář.exe'),str(reference/'Data'),'extended-field-buttons'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:
            raise RuntimeError(f'{precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        expected=dict(states=19431,objects=9,reloads=27,letters=673140,frames=36,pixels=17280000,steps=19332,sound_residues=0)
        for key,value in expected.items():
            assert state['extended_'+key]==value,(precision,key,state['extended_'+key],value)
        assert state['field_object_states']==1536
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,**state));unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/name for name in [
        'CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/build_sprite_oracle_fixups.py','tools/extended_button_oracle_cases.inc','tools/extended_button_oracle_run.inc',
    ]]+[Path(__file__).resolve()])
    report=dict(status='passed_for_field_object_and_extended_field_tooltip_scope',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(reference/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts,states=38862,field_geometry_states=3072,objects=18,reloads=54,
        letters=1346280,steps=38664,frames=72,pixels=34560000,
        original_functions=['0041be20','004182b0','0041c0e0','0041b010','0041b520','00419e70',
            '0041ab10','0041a250','0041b910','00419680','004192a0','00419a90','00417fe0','00413a50'],
        coverage=[
            'Own original 868-byte FieldObject tooltip specialization, both fixture sections and seven shipped Farm ActionAreas XML roots',
            'Exact HeightOffset, three reloads per root, all four tip appearances, retained layouts, wait/fade/NaN/direction behavior and owned text/picture lifecycle',
            'Six offsets including negative/fractional/large values, 64 positions each and four repeated transform/hit passes; root geometry drift and exact sprites compared',
            'Actual native/source frame traversal, input hover, ordered sprites, D3D calls and 72 GPU frames/34,560,000 pixels',
            'Shared rectangle hit test now follows original PC24 arithmetic at exact edges; complete regression required for affected controllers',
        ],
        test_adapters=[
            'Production does not load original code; only the diagnostic maps inventoried methods and records operand relocations',
            'The fixture uses a declared tutorial state, resource/XML/old CRT adapters and captured sound requests',
            'This is the actual FieldObject/tooltip base; it does not stand in for complete ActionArea, Warehouse or Farm behavior',
        ],
        pending=[
            'Actual ActionArea click/parking offsets, overlay, Farm registration and person-task lifecycle',
            'Complete Warehouse and notifications, actual Farm/character/cGame, whole tutorial and game EXE',
            'Malformed XML, exceptions, nonstandard hardware and complete world/audio/save/uninstaller equivalence',
        ])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('FieldObject: 38,862 states + 3,072 geometry states, 72 frames/34,560,000 pixels; 17 originals unchanged')


if __name__=='__main__':
    main()
