#!/usr/bin/env python3
"""Compare original price text spacing and owned money icons."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/price-text-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=8288,objects=16,reloads=16,letters=52146,frames=64,pixels=30720000)
    for kind in ('price-text','text-money'):
        for precision,mode in [('PC53',kind),('PC24',kind+'-device')]:
            result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
            if result.returncode:raise RuntimeError(f'{kind} {precision}: {result.returncode} {result.stderr}')
            state=json.loads(result.stdout)
            for key,value in counts.items():assert state['price_'+key]==value,(key,state['price_'+key])
            assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
            state['precision']=precision;state['specialization']=kind;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/price_text_oracle_cases.inc',Path(__file__).resolve()])
    report=dict(status='passed_for_price_text_and_text_money_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        **{key:4*value for key,value in counts.items()},
        original_functions=['0048bd30','0048bd60','0048bd80','0048be00','0048c210','00415b70','00415bf0','00415d10','004161a0','00416580'],
        coverage=['14 shipped TextMoney XML definitions and two explicit degree/coin fixtures; each native specialization tested independently',
            'Reverse glyph-list traversal with forward raw UTF-16 digit classification; digit runs, punctuation, spaces, empty text, unsigned decimal and signed boundaries',
            '256 mutation sequences per object and precision: degree offset, alignment, multiplier, text, enable/disable, position and direct makeLetters',
            'XML degree offset ordering and repeated XML reload; real owned coin texture, position, frame and full sprite/lifetime comparison',
            '256 complete Direct3D8 frames, ordered draw submissions/API calls and 122880000 exactly matching RGB pixels'],
        test_adapters=['Reference EXE mapped only for diagnosis without entry point; production never loads original program code',
            'Native XML/string/typed/font/resource boundaries use explicit diagnostic adapters; original text/money algorithms, hierarchy and lifetimes execute native instructions',
            'Raw string and display text lengths coincide in exercised cases; physical input and audio are not exercised'],
        pending=['Integration into contract dialogs and actual Farm/building/warehouse/market controllers; complete main EXE',
            'Localized text with a longer resolved display than the original raw string: native algorithm can read beyond raw storage; behavior not established',
            'Native allocation/exception/locale paths, full scene runtime and bidirectional saves'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Price/money: 33152 states, 256 GPU frames/122880000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
