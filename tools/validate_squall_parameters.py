#!/usr/bin/env python3
"""Verify the recovered Squall parameter normalizer against original instructions."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/squall-parameters-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-parameters-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Squall.dll'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    oracle=args.oracle.resolve(strict=True);contexts=[]
    for precision,output in run_precision_cases(oracle,[original]):
        context=json.loads(output)
        assert context['checks']==100577 and context['invalid_inputs']==72 and context['overlap_cases']==25,context
        assert context['relocations']>0 and context['device_enumerations']>10000,context
        context['precision']=precision;contexts.append(context)
    report=dict(status='passed',reference_sha256=expected,
        oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        checks=sum(row['checks'] for row in contexts),precision_contexts=contexts,
        original_functions=['100030a0'],original_entry_point_executed=False,reference_files_modified=False,
        coverage=['All thirteen output words compared byte-for-byte: 100000 seeded inputs per precision plus individual integer/float boundaries',
            'Signed integer limits, all hardware/listener/algorithm flags, zero/negative values, NaNs including signaling payloads, infinities, subnormal and maximum floats',
            'Real hidden Win32 window, null/invalid HWND, null/unmapped/unreadable/page-straddling input pointers',
            'Device enumeration short-circuit and return boundaries; forward word copy for identical and overlapping buffers'],
        test_adapters=['Native IsBadReadPtr and IsWindow imports use the same real Win32 APIs as source',
            'Native api_GetNumDevice is redirected to an explicitly scripted test-only device count; no DirectSound initialization or enumeration is claimed',
            'Original PE is privately relocated using its relocation directory; DLL entry point is never executed'],
        pending=['Whole Squall initialization, actual device enumeration and all exported interfaces',
            'Floating-point exception/status flags and invalid output pointers',
            'DirectSound backend, decoder versions and sample/playback behavior'])
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Squall parameters: {report['checks']} exact native comparisons across both precisions")

if __name__=='__main__':main()
