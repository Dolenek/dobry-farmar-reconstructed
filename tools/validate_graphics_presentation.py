#!/usr/bin/env python3
"""Verify recovered presentation parameters and sprite buffers on a real HAL device."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--test',type=Path,default=ROOT/'build/windows/Release/graphics-presentation-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/graphics-presentation-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    references={}
    for name in ('Data/Data.pack','Data/cz.pack'):
        expected=next(row['sha256'] for row in inventory['files'] if row['path']==name)
        assert hashlib.sha256((original/name).read_bytes()).hexdigest()==expected
        references[name]=expected
    executable=args.test.resolve(strict=True)
    result=subprocess.run([str(executable),str(original/'Data')],capture_output=True,text=True,check=True,timeout=60)
    report=json.loads(result.stdout)
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  executable_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),reference_sha256=references,
                  original_graphics_runtime_compared=False,native_hal_windowed_device=True,fullscreen_device_created=False,
                  sprite_buffer_capacities=[3000,17,3000],
                  pending=['Full cGraphics constructor/setup','Full settings lighting configuration',
                           'Fullscreen device and monitor mode transition','Device loss/reset','Actual rendered game screens',
                           'Original graphics runtime oracle','Primary cGraphics ABI and exception wrappers'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Graphics presentation: {report['checks']} native checks, original windowed parameters and sprite buffers")
if __name__=='__main__': main()
