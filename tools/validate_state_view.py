#!/usr/bin/env python3
"""Compare the native moving-scene base, actual XML hierarchy and D3D8 frames."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/state-view-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=11121,moves=1254,steps=7296,frames=54,pixels=25920000,reloads=6)
    for precision,mode in [('PC53','state-view'),('PC24','state-view-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'StateView {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['state_view_'+key]==value,(key,state['state_view_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/state_view_oracle_cases.inc',ROOT/'tools/main_menu_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_state_view_motion_hierarchy_and_rendering_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},
        original_functions=['004c48e0','0042c7d0','004c4a80','004c4eb0','004c5270','004c5630','004c56f0'],
        coverage=['Native constructor/defaults and owned Picture/Control geometry, hierarchy and destruction; unspecified native remaining time uses zero-initialized diagnostic allocation',
            'Six real XML loads/reloads per precision across Farm, Manufacture and WareDetail, exact move speed and decoration reparenting',
            '1200 fixed-seed motion cases per precision, states 0/1/2/3/4, arbitrary finite coordinates/speeds/time, negative elapsed and parent enable/show changes',
            'Zero distance and zero/negative speed, native NaN comparison path, state zero preserves old motion fields, base enter-state virtual is native genuine RET',
            'Exact x87 delta/sqrt/duration/reciprocal spill boundaries and update/completion in PC53 and original device PC24',
            '108 actual D3D8 frames, ordered submissions/API and 51840000 exactly matching RGB pixels'],
        test_adapters=['Original mapped for diagnosis without entry point; implementation never calls original code',
            'Native StateView/Picture/Control/View/XML and movement functions execute their original instructions',
            'Existing explicit XML/old CRT/resource adapters and real reconstructed timer/cursor with deterministic clock/input services',
            'Farm, Manufacture and WareDetail definitions are loaded into the actual common StateView base; derived world controllers are not substituted or claimed verified'],
        pending=['Derived Farm/Warehouse/Manufacture enter-state hooks, GameMenu switches, tutorial and full gameplay scene integration',
            'Extreme finite/subnormal overflow and arbitrary NaN payloads, allocation/exception and invalid XML paths',
            'Physical input/audio, full game EXE, bidirectional saves, complete Squall and uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('StateView: 22242 states, 2508 motions, 108 GPU frames/51840000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
