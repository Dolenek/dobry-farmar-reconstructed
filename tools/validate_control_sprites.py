#!/usr/bin/env python3
"""Compare control sprite geometry, animation UVs and colors with the original."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/control-sprite-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/control-sprite-validation.json')
    parser.add_argument('--device-precision',action='store_true')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision,output in run_precision_cases(executable,[original],args.device_precision):
        context=json.loads(output); assert context['sprite_comparisons']==100000 and context['animation_scenarios']==20000
        context['precision']=precision; contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in ('sprite_comparisons','animation_scenarios')}
    report['precision_contexts']=contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_functions=['004ef3b0','004ed4f0','004455e0','004f0b70','004ed510','00413a10','00580f98'],
        original_entry_point_executed=False,reference_files_modified=False,
        pending=['Original full control object/virtual ABI','Application-specific ended callback behavior',
        'Control XML loading, input and full UI integration','Nonfinite/degenerate input behavior','Other x87 precision/rounding modes'])
    args.output.parent.mkdir(parents=True,exist_ok=True); args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Control sprites: {report['sprite_comparisons']} exact original comparisons; {report['animation_scenarios']} animation scenarios")
if __name__=='__main__': main()
