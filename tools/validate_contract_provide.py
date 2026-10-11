#!/usr/bin/env python3
"""Compare ContractProvide, original activation and three real ContractStatus controllers."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/contract-provide-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=1369,objects=1,starts=654,clicks=652,reloads=6,letters=215399,frames=48,pixels=23040000,input_clicks=2)
    for precision,mode in [('PC53','contract-provide'),('PC24','contract-provide-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Contract offer {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['contract_offer_'+key]==value,(key,state['contract_offer_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/contract_provide_oracle_cases.inc',ROOT/'tools/contract_provide_oracle_services.h',ROOT/'tools/contract_status_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_contract_offer_and_connected_status_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},
        original_functions=['004ad4b0','004ada90','004abef0','004abea0','004ac570','004accc0','004ad0d0','004c02c0','004bd2f0'],
        coverage=['Real ContractProvide constructor, owned controls, XML release, singleton and destruction resources',
            'All 131 shipped contracts with counts 0/1/2/3 and 128 signed/unsigned/time/reqType edge events in each precision',
            'Original accept consumes letter, re-reads event after consumption, queries money, deducts penalty with wrap, and executes actual native GameMenu activation into three real ContractStatus controllers',
            'Count equals 3 guard, negative/4 count edges with a free slot, money getter order and exact disabled/parent-disabled tutorial query/anchor conditions',
            '12 XML reloads: original and partial prototypes, absent/one/three Place sections and cumulative positions',
            '96 complete Direct3D8 frames, ordered draw submissions/API and 46080000 identical RGB pixels; 4 actual input releases wait or accept and activate a contract'],
        test_adapters=['Original mapped only for diagnosis without entry point; production never calls original program code',
            'Native XML/string/old CRT/resource interfaces use explicit diagnostic adapters; original offer constructors, starts, callbacks, GameMenu activation, status algorithms and UI frames execute original instructions',
            'Actual independent ProfileProperties, real resource textures/fonts and independently verified GameConfiguration products',
            'Source slot selection is an explicit diagnostic reproduction of native 004c02c0; full source GameMenu constructor/controller still pending',
            'GameMenu fixture supplies recovered View and original three status slots/count; Farm letter consumption and event replacement, money query/refresh and tutorial are explicit trace service boundaries',
            'Disabled real SoundManager compares requests and resource lifetime; audible output not validated'],
        pending=['Actual Farm letter actor and consuming effects; full source GameMenu, tutorial, warehouse and cGame binding',
            'Full physical input, audible playback, scene integration, main EXE and bidirectional saves',
            'Invalid product, inconsistent occupied-slot/count invariant, native exception/allocation/locale paths; unspecified constructor letter pointer outside zeroed diagnostic heap'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Contract offer: 2738 states, 1308 starts, 96 GPU frames/46080000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
