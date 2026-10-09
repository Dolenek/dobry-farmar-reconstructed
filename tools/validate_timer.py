#!/usr/bin/env python3
"""Compare reconstructed timer state, constructors and WinMM boundaries with original routines."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/timer-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/timer-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    oracle=args.oracle.resolve(strict=True);contexts=[]
    for precision,output in run_precision_cases(oracle,[original]):
        context=json.loads(output)
        assert context['constructors']==100 and context['destructors']==100 and context['frames']==60000 and context['state_comparisons']==60100
        assert context['sleep_calls']>10000
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key!='precision'}
    report.update(status='passed',precision_contexts=contexts,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=expected,oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['00536ae0','00536430','00536720','00536550','00536690','005366e0','00536480','005364a0','00536500'],
        coverage=['Constructor defaults and Time/MinFps/MaxFps settings side effects including absent MaxFps writing MinFps',
            '60000 frames per context: uint32 wrap, signed boundary, minimum FPS clamp, zero/negative/fractional FPS and changing multiplier',
            'Shared smoothing accumulators persist across timer reinitialization; average computed before current sample; FPS uses unspilled quotient',
            'Exact float state/getters and clock/Sleep/WinMM call order; ignored period API errors and singleton cleanup'],
        test_adapters=['Clock, Sleep and timeBeginPeriod/timeEndPeriod are controlled WinAPI boundaries',
            'Native settings virtual float get/set use recovered Settings over independent empty diagnostic XML trees',
            'String byte assignment uses recovered String; private mapped reference operands are relocated'],
        pending=['Full original 64-byte object/internal callback/vtable and exception ABI','Original old CRT float settings extraction/formatting',
            'Native duplicate-singleton exception branch','Physical WinMM timing and end-to-end original game frame timing'])
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Timer: {report['frames']} native frames, {report['state_comparisons']} state comparisons, {report['constructors']} constructors")
if __name__=='__main__':main()
