#!/usr/bin/env python3
"""Compare recovered 2D Control/Picture state, callbacks and depth traversal."""
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
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/control-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/control-validation.json')
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
        assert context['operations']==20000 and context['state_comparisons']==200010 and context['controls_destroyed']==10
        context['precision']=precision; contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key!='precision'}
    report['precision_contexts']=contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['004edc80','004f16f0','004f1780','004f1560','004ed670','004f1810','004f17a0','004f17c0',
            '004f1680','004f14e0','004f1510','004f0110','004f1080','004f0b70','00413a50','004eebb0','00506a10'],
        test_adapters=['Allocation/free use diagnostic CRT','Timer +8 returns controlled elapsed value',
            'Animation-ended virtual slot records an event','Picture callbacks record events with deterministic keep/remove results'],
        pending=['Modern object ABI differs from original 556/568-byte classes','Control/Picture XML resource loading comparison',
            'Mesh/Animation resources and 3D branch','Actual menu/input/main frame integration','Nonfinite geometry/animation cases'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"UI controls: {report['state_comparisons']} states, {report['callback_events']} callbacks, {report['sprite_pairs']} sprite pairs, {report['draw_traversals']} depth traversals")
if __name__=='__main__': main()
