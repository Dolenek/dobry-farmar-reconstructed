#!/usr/bin/env python3
"""Record compiled Win32 layout of the pinned public Squall reference."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe',type=Path,default=ROOT/'build/windows/Release/squall-reference-layout.exe')
    args=parser.parse_args()
    subprocess.run(['python',str(ROOT/'tools/inspect_squall.py')],check=True)
    probe=args.probe.resolve(strict=True)
    layout=json.loads(subprocess.check_output([str(probe)],text=True))
    expected=dict(sample=308,context=1344,parameters=52,defaults=28,channel_end_worker=300,
                  channel_user_data=296,channel_worker=292,channel_parameters=84)
    assert layout==expected,layout
    lock=json.loads((ROOT/'analysis/squall-source.lock.json').read_text(encoding='utf-8'))
    report=dict(status='passed',architecture='Win32',source_commit=lock['commit'],layout=layout,
        probe_sha256=hashlib.sha256(probe.read_bytes()).hexdigest(),
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        native_channel_size=300,native_size_evidence=['100033d2..100033e5','100033fe..10003418'],
        source_runtime_compared=False,original_runtime_compared=False,
        limitation='sizeof/offsetof of the public reference only; does not prove remaining context or channel fields match the original')
    (ROOT/'analysis/squall-layout-validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    subprocess.run(['python',str(ROOT/'tools/inspect_squall.py')],check=True)
    print('Reference channel size 308 differs from native allocation size 300')

if __name__=='__main__':main()
