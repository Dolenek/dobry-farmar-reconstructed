#!/usr/bin/env python3
"""Compare the original contract controller, real UI and profile state."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/contract-status-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=15632,objects=4,assignments=2572,steps=6240,clicks=6240,reloads=4,letters=126339,formats=8192,frames=32,pixels=15360000,input_clicks=4)
    for precision,mode in [('PC53','contract-status'),('PC24','contract-status-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Contract status {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['contract_'+key]==value,(key,state['contract_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/contract_status_oracle_cases.inc',ROOT/'tools/contract_status_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_contract_status_controller_ui_and_profile_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},
        original_functions=['004bb830','004b92c0','004ba730','004bd2f0','004b98e0','004bcc20','004ba0b0','004bbee0','004bc460','004bc840','004b9350','004b93e0','00467fa0','004b1080','004036e0'],
        coverage=['Owned substrate, extended animation goal button, product pictures, TextLine labels, CheckBox ticks and ProgressBar; real ContractStatus.xml',
            'All 131 shipped contracts in four controller ids including unsigned high-bit id; new and restored assignments, profile stores and missing defaults',
            '512 additional states per controller/precision: unsigned counts, stock guards, reqType 0/1/2, timer accumulator boundaries, hour/minute borrowing, inactive and expiry states',
            'Both expiry branches, profile clearing, Bonus-on-loss and swapped note/sound selection; unsigned awQTask wrap, ordered world requests including original same-frame continuation',
            'Tutorial show anchor and click guards; three world transition predicates; readiness writes passCancel even when stocks insufficient',
            '16384 native time formatting comparisons, with unsigned hours/minutes/seconds and omitHours',
            '64 complete Direct3D8 frames with matching ordered draw API/submissions and 30720000 RGB pixels; 8 normal input releases open dialogs, tutorial-blocked releases do not',
            'Constructor XML release, cumulative tooltip/ProgressBar XML children, original retained sound pointers and per-destructor release'],
        test_adapters=['Original EXE mapped only for diagnosis without entry point; production never loads original program code',
            'Native XML and old CRT stream/typed/string services remain explicit adapters; time padding, controller algorithms and UI methods execute native instructions',
            'Native profile getters use independent real ProfileProperties; borrowed event and product records are supplied from separately verified definition/configuration data',
            'World/dialog/money/stock mutation and tutorial methods are diagnostic trace bindings; production requires ContractStatusWorld and supplies no default implementation',
            'Actual resources and disabled SoundManager compare sound requests/lifetime; audible playback not verified here',
            'Native controller constructor leaves accumulator/fullDuration/event/initialStocks unspecified; zeroed fixture does not establish allocator defaults'],
        pending=['Real ContractStatusWorld binding to cGame, warehouse, three dialogs, GameMenu and tutorial; complete game/main EXE',
            'Original invalid product dereference/exception paths and old CRT locale execution',
            'Physical device input, audible playback, full scene integration and bidirectional saves'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Contract status: 31264 states, 5144 assignments, 64 GPU frames/30720000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
