#!/usr/bin/env python3
"""Compare TextLine/Letter ownership and state with original UI methods."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-control-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/text-control-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision in ('preserved','original-device-0x20'):
        command=[str(executable),str(original),str(original.parent/'Data')]
        if precision=='original-device-0x20': command.append('device-precision')
        result=subprocess.run(command,capture_output=True,text=True,timeout=90)
        if result.returncode: raise RuntimeError(f'Text control oracle failed ({result.returncode}, {precision}): {result.stderr}')
        context=json.loads(result.stdout)
        assert context['scenarios']==9000 and context['lines_destroyed']==300 and context['direction_comparisons']==2400
        context['precision']=precision
        contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in ('scenarios','letter_comparisons','direction_comparisons','lines_destroyed')}
    report['precision_contexts']=contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['004daca0','004da840','004dd030','004dc800','004dc1c0','004da400','004dbdc0',
            '004dae40','0050c490','0050c500','004f0110','004f1080'],
        test_adapters=['Allocation/free use diagnostic CRT','String assign/append/replace use recovered ABI strings',
            'Settings bool and localization bind to recovered services','Texture factory returns recovered native GPU texture',
            'Font lookup executes original 0050b450 over fixture original-layout nodes from shipped font data',
            'Original XML loading is bypassed: matching fields are initialized from shipped UserText attributes'],
        pending=['Original XML typed extraction comparison','Full TextLine 696-byte ABI','Further setters/disabled transitions',
            'Input/menu/main frame integration','Nonfinite geometry and malformed resource behavior'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Text controls: {report['scenarios']} scenarios, {report['letter_comparisons']} owned letters/sprites, {report['direction_comparisons']} RTL comparisons")
if __name__=='__main__': main()
