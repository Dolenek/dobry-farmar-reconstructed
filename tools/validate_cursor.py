#!/usr/bin/env python3
"""Compare native cursor/window transformations and exercise the physical Win32 API."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/cursor-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/cursor-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    oracle=args.oracle.resolve(strict=True);contexts=[]
    for precision,output in run_precision_cases(oracle,[original]):
        context=json.loads(output)
        assert context['frames']==20000 and context['bounds']==20000 and context['transforms']==40000
        assert context['aliased']==20000 and context['failure_frames']>4000 and context['physical_windows']==1
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key!='precision'}
    report.update(status='passed',precision_contexts=contexts,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=expected,oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['00530290','00512330','0050d260','00531a40'],
        coverage=['Bit-exact screen-to-window/client/game cursor coordinates under both x87 contexts; full signed LONG range',
            'API failure retains position/output; shared POINT/WINDOWINFO, API input bytes and call order agree',
            'Fullscreen offset/scale operation order, zero/negative/fractional scales and aliased input/output',
            'Default Win32 backend in two owned hidden windows; no cursor repositioning or display mode changes'],
        test_adapters=['Native cursor frame, graphics window-bounds and inverse transforms execute original routines; WinAPI responses are controlled',
            'Graphics and logic objects are original-layout fixtures; reference copied to a distinct private mapping every run'],
        pending=['Full cLogic singleton/frame/shortcuts and object ABI','Native whole-game physical input and fullscreen timing','Physical menu navigation'])
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Cursor: {report['frames']} native frames, {report['bounds']} bounds, {report['transforms']} transforms, {report['physical_windows']} physical windows")
if __name__=='__main__':main()
