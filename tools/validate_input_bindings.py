#!/usr/bin/env python3
"""Compare original Input binding sets and complete buffered dispatch on COM fixtures."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/input-bindings-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/input-bindings-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    result=subprocess.run([str(executable),str(original)],capture_output=True,text=True,timeout=90)
    if result.returncode:raise RuntimeError(f'Input oracle failed ({result.returncode}): {result.stderr}')
    report=json.loads(result.stdout)
    assert report['operations']==10000 and report['set_comparisons']==5340000 and report['frames']==500 and report['format_objects']==267
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['00526190','005280f0','00526580','005284e0','00526970','005288e0','00524640','00524a30','00524e60',
            '00527310','005245e0','00525620','00525240','00525a00','00523e20','00524200'],
        test_adapters=['Allocation/free/memmove use diagnostic CRT','Own original-layout handler sets are initialized without original constructor',
            'DirectInput COM fixtures return controlled device data/state and HRESULTs; no physical user input is read',
            'Callbacks record order, mutate control registrations, and append characters during live dispatch'],
        sdk_formats={'keyboard_objects':256,'mouse_objects':11,'all_object_flags_offsets_guids':'exact original PE match'},
        pending=['Real-device initialization/foreground window integration','Original Input constructor/key-name map and destructor',
            'Old exception ABI for DIERR_INPUTLOST/other failures','Character handler erasing itself during traversal',
            'Main menu/cursor integration','Full old Input object ABI'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Input: {report['set_comparisons']} sets, {report['frames']} frames, {report['dispatch_events']} dispatch events, {report['cached_key_queries']} cached queries")
if __name__=='__main__':main()
