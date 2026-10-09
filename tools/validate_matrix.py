#!/usr/bin/env python3
"""Hash-check original PE and compare its pure projection/matrix routines."""
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
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/matrix-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/matrix-validation.json')
    parser.add_argument('--device-precision',action='store_true')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision,output in run_precision_cases(executable,[original],args.device_precision):
        comparisons=int(output); assert comparisons==70002
        contexts.append(dict(precision=precision,comparisons=comparisons))
    count=sum(context['comparisons'] for context in contexts)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report=dict(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                comparisons=count,precision_contexts=contexts,original_sha256=expected,oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),
                original_pure_routines_compared=['00591fe8','005920cb','0059221a'],
                bitwise_float_comparison=True,original_entry_point_executed=False,reference_file_modified=False,
                test_only_relocated_projection_constant_operands=2,
                pending=['Non-finite/singular inputs','Other x87 precision/rounding modes','Game rendering'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Matrices: {count} original bitwise comparisons passed")
if __name__=='__main__': main()
