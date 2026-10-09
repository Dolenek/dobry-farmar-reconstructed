#!/usr/bin/env python3
"""Compare every UTF-16 code unit against isolated original bitmap font lookups."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/font-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/font-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář'; inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    hashes={name:next(row['sha256'] for row in inventory['files'] if row['path']==name)
            for name in ('Dobrý farmář.exe','Data/Data.pack','Data/cz.pack')}
    def verify():
        for name,expected in hashes.items(): assert hashlib.sha256((original/name).read_bytes()).hexdigest()==expected,name
    verify(); executable=args.oracle.resolve(strict=True)
    result=subprocess.run([str(executable),str(original/'Dobrý farmář.exe'),str(original/'Data')],capture_output=True,text=True,timeout=60)
    if result.returncode: raise RuntimeError(f'Font oracle failed ({result.returncode}): {result.stderr}')
    report=json.loads(result.stdout); verify(); assert report['lookup_comparisons']==393216 and report['available_glyphs']==2217
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=hashes,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_functions=['0050b3d0','0050b450'],
        data_constructor_address='0050bd70',original_constructor_executed=False,original_entry_point_executed=False,
        reference_files_modified=False,pending=['Original constructor/lifetime runtime comparison','Original 68-byte object/container ABI',
        'Malformed or truncated font data exception behavior','TextLine layout and UI integration'])
    args.output.parent.mkdir(parents=True,exist_ok=True); args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Bitmap fonts: {report['fonts']} files, {report['lookup_comparisons']} original lookup comparisons, {report['cache_checks']} cache checks")
if __name__=='__main__': main()
