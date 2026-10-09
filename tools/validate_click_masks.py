#!/usr/bin/env python3
"""Compare original GPU JNG click-mask extraction and packed alpha bits."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/click-mask-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/click-mask-validation.json')
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
        assert context['masks']==425 and context['columns']==150029 and context['alpha_bits']==81014421
        context['precision']=precision
        contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in ('masks','columns','alpha_bits')}
    report['precision_contexts']=contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['0051c040','0051a330','00519450','00519360','00516e10','0051a010','004f5200'],
        test_adapters=['Allocation/free use diagnostic CRT','File factory returns recovered iFile with original slot order',
            'JNG GPU loader binds to recovered getJngTexture (already independently pixel compared with original DLL)',
            'Original mask vector/bit arrays, loops, alpha threshold, resize and cleanup execute unmodified logic'],
        pending=['Non-JNG D3DX format conversion in getMask','Full old vector/exception ABI','Main menu input integration','Failure/malformed source cases'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Click masks: {report['masks']} inputs, {report['columns']} columns, {report['alpha_bits']} GPU alpha bits")
if __name__=='__main__': main()
