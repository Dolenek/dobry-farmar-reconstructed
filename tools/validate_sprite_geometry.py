#!/usr/bin/env python3
"""Hash-check reference and compare original sprite vertex/index output."""
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
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/sprite-geometry-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/sprite-geometry-validation.json')
    parser.add_argument('--device-precision',action='store_true')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    fixups=json.loads((ROOT/'analysis/sprite-oracle-fixups.json').read_text())
    assert fixups['reference_sha256']==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision,output in run_precision_cases(executable,[original],args.device_precision):
        comparisons=int(output); assert comparisons==20000
        contexts.append(dict(precision=precision,comparisons=comparisons))
    count=sum(context['comparisons'] for context in contexts)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report=dict(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                comparisons=count,trials=10000*len(contexts),precision_contexts=contexts,subdivision_range=[1,8],fullscreen_and_windowed=True,
                vertex_bytes_compared=True,index_words_compared=True,nonzero_base_vertex_tested=True,
                original_sha256=expected,oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),
                original_function='00517f40',original_entry_point_executed=False,reference_file_modified=False,
                test_only_operand_fixups=len(fixups['fixups']),
                pending=['Batch transition/list runtime comparison','Full game screens','Sprite shader ownership',
                         'Zero/overflow subdivisions and buffer capacities','Non-finite inputs and other FPU modes'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Sprite geometry: {count} original vertex/index comparisons passed")
if __name__=='__main__': main()
