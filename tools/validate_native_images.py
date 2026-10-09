#!/usr/bin/env python3
"""Verify native DDS/TGA/BMP source storage against isolated original readers."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/native-image-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/native-image-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář'; inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    hashes={name:next(row['sha256'] for row in inventory['files'] if row['path']==name)
            for name in ('Dobrý farmář.exe','Data/Data.pack','Data/cz.pack')}
    def verify():
        for name,expected in hashes.items(): assert hashlib.sha256((original/name).read_bytes()).hexdigest()==expected,name
    verify(); executable=args.oracle.resolve(strict=True)
    result=subprocess.run([str(executable),str(original/'Dobrý farmář.exe'),str(original/'Data')],capture_output=True,text=True,timeout=60)
    if result.returncode: raise RuntimeError(f'Native image oracle failed ({result.returncode}): {result.stderr}')
    report=json.loads(result.stdout); verify(); assert report['images_compared']==10
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=hashes,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_functions=['0059847a','0059781c','00596f2d'],
        original_entry_point_executed=False,reference_files_modified=False,test_only_allocator_hook='0057edb1',
        pending=['D3DX texture requirements/format fallback/surface filtering','Actual non-JNG texture GPU upload',
                 'Indexed TGA and BMP','Compressed BMP','Other image formats','Invalid/malformed and overflow inputs',
                 'DDS mip/cube runtime comparisons beyond shipped files'])
    args.output.parent.mkdir(parents=True,exist_ok=True); args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Native image storage: {report['images_compared']} original images, {report['source_bytes_compared']} identical bytes")
if __name__=='__main__': main()
