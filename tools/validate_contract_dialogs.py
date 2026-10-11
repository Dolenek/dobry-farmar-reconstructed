#!/usr/bin/env python3
"""Compare original contract dialogs and their real connected controller."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/contract-dialogs-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=1681,objects=3,starts=785,clicks=781,reloads=18,letters=330495,frames=96,pixels=46080000,input_clicks=4)
    for precision,mode in [('PC53','contract-dialogs'),('PC24','contract-dialogs-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Contract dialogs {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['contract_dialog_'+key]==value,(key,state['contract_dialog_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/contract_dialog_oracle_cases.inc',ROOT/'tools/contract_status_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_contract_dialogs_and_connected_status_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},
        original_functions=['004aadb0','004a8b30','004ab380','004aa690','004aa890','004aa040','004aa660','004a8d50','004a94e0','004a9c60','004b73f0','004b7980','004b65b0','004b6b90','004b7010','004b8ce0','004b9290','004b7a00','004b7fe0','004b8510','004b8900'],
        coverage=['Real ContractAim/ContractFail/ContractPassConfirm constructors, singleton ownership and XML release; common ContractDialogBase',
            'Owned MultilineText, TextLine, TextMoney with coins, TextAnimationButton and two/ four product requirement rows; navigation appearance resources',
            'All 131 shipped contract events plus 128 synthetic numeric/reqType edge events per precision; signed bonus, unsigned stocks/count clamp and missing quantity',
            '36 XML reloads including shipped definitions, partial prototypes, absent/one/three Place sections and cumulative positions',
            'Real connected ContractStatus::goalClicked opens requirements or confirmation; native and source readiness/expiry open real confirmation/failure dialogs without start adapters',
            'Confirmation acceptance invokes real ContractStatus::fail with ordered profile, stock, money, sound and notification requests',
            '192 complete Direct3D8 frames, ordered draw API/submissions and 92160000 exactly matching RGB pixels; 8 actual input releases close requirements/result/confirmation dialogs, including acceptance'],
        test_adapters=['Original EXE mapped only for diagnosis without entry point; production never calls original program code',
            'Native XML/string/old CRT/resource service boundaries use explicit diagnostic adapters; original constructors, dialog starts, callbacks, status algorithms and UI frame methods execute native instructions',
            'Profiles are independent actual ProfileProperties; game configuration appearances use independently verified source definitions',
            'Context game is a real recovered View in this component fixture; full GameMenu and cGame constructor/world scene remain pending',
            'World stock changes/money refresh/removal/notification and tutorial are explicit diagnostic trace bindings; disabled real SoundManager compares requests and resources, not audible output'],
        pending=['Actual Farm letter objects; GameMenu, warehouse, tutorial and full cGame binding',
            'Full physical mouse/device input, audible playback, scene integration, complete main EXE and bidirectional saves',
            'Invalid product, native allocation/exception/locale paths; constructor unspecified borrowed pointer differs from zeroed diagnostic heap'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Contract dialogs: 3362 states, 1570 starts, 192 GPU frames/92160000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
