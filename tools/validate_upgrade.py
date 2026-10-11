#!/usr/bin/env python3
"""Compare original abstract cUpgrade properties, loading, cost and setter methods."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/upgrade-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','upgrades'),('PC24','upgrades-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Upgrade {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=65648,queries=32768,changes=32768,costs=122880,objects=16,reloads=96,level_loads=208,sound_calls=22408).items():
            assert state['upgrade_'+key]==value,(key,state['upgrade_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/upgrade_oracle_cases.inc',ROOT/'tools/upgrade_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_abstract_upgrade_and_cost_state_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        states=131296,queries=65536,changes=65536,costs=245760,objects=32,reloads=192,level_loads=416,sound_calls=44816,
        original_functions=['004169c0','00416970','00416a30','00417050','00417430','004174b0','0040ba00','00410ce0','00404170'],
        coverage=['Constructor/destructor, original two pure virtual slots, independent real ProfileProperties maps, missing-key insertion',
            'Up0..Up3 named loading bounded by child count; missing/sparse/empty definitions, repeated loads accumulate levelCount, callback changes count before increment',
            'Random uint32 costs and availability with signed comparisons; all 15 items and 4 valid cost levels; wrapped next level and signed upper clamp',
            'Setters refresh even on unchanged level; negative levels and levelCount extremes, changed level only requests shared upgrade sound',
            'Actual game.xml configuration and actual ResourceManager/SoundManager disabled playback path; loop false, start true'],
        test_adapters=['Original EXE mapped only for diagnosis without entry point; production never loads original code',
            'Derived loadLevel/refresh slots replaced ONLY in diagnostic by trace callbacks; source base remains abstract with required real derived implementation',
            'Native cGame configuration/profile/sound storage fixture; profile lookup redirects to independent real source maps',
            'Old CRT stream services use explicit string adapters; unsigned decimal key format is independently established by 00406570 facet +18, 00408550 and lu bytes at 005ee678',
            'Native sound virtual call redirects to actual source disabled SoundManager; no channel is created in this test'],
        pending=['Real derived building/farm appearance, UI, upgrade purchase and gameplay dispatch, complete cGame and main executable',
            'Non-null upgrade audio channel replacement/deletion and teardown order in real cGame',
            'Original C++ exception ABI for invalid item/level guards; guards inspected, valid cost calls executed',
            'Original locale/old CRT stream execution and game save interoperability'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Upgrade: 131296 states, 65536 setters, 245760 costs, 44816 sound calls; 17 originals unchanged')
if __name__=='__main__':main()
