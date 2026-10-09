#!/usr/bin/env python3
"""Compare ButtonPrototype/AnimationButton/Button input, masks and states."""
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
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/button-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/button-validation.json')
    parser.add_argument('--device-precision',action='store_true',help='Test x87 precision control 24 as set by original Direct3D')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision,output in run_precision_cases(executable,[original,original.parent/'Data'],args.device_precision):
        context=json.loads(output)
        assert context['operations']==20000 and context['state_comparisons']==240012 and context['buttons_destroyed']==12
        context['precision']=precision; contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key!='precision'}
    report['precision_contexts']=contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['004f53b0','004f5480','004f2770','004f2cc0','004f28c0','004f30e0','004f34c0',
            '004f2620','004f2690','004f26b0','004f3a00','004f3e00','004f41f0','004f4620','004f1c30','004f1ce0','00507ec0','00507fd0','00508120'],
        test_adapters=['Allocation/free use diagnostic CRT','Cursor returns controlled position','Input registration routes to recovered binding sets',
            'Sound manager records calls; controlled sound resources have real recovered cResource references',
            'Click-mask columns are constructed fixture data; file-mask loading is not verified by this test'],
        pending=['Original XML extraction and resource loading comparison','State textures with nonnull resources',
            'Full old object/exception ABI','Native DirectInput device acquisition and dispatch','Actual menu/frame integration','Nonfinite masks/geometry'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Buttons: {report['state_comparisons']} states, {report['callback_events']} events, {report['mask_hit_tests']} mask hits")
if __name__=='__main__': main()
