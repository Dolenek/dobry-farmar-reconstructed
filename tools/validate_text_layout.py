#!/usr/bin/env python3
"""Compare TextLine geometry arithmetic against its isolated original method."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-layout-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/text-layout-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář'; inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    hashes={name:next(row['sha256'] for row in inventory['files'] if row['path']==name)
            for name in ('Dobrý farmář.exe','Data/Data.pack','Data/cz.pack')}
    def verify():
        for name,expected in hashes.items(): assert hashlib.sha256((original/name).read_bytes()).hexdigest()==expected,name
    verify(); executable=args.oracle.resolve(strict=True)
    result=subprocess.run([str(executable),str(original/'Dobrý farmář.exe'),str(original/'Data')],capture_output=True,text=True,timeout=60)
    if result.returncode: raise RuntimeError(f'Text layout oracle failed ({result.returncode}): {result.stderr}')
    report=json.loads(result.stdout); verify(); assert report['layout_scenarios']==12000 and report['letter_sprite_comparisons']==report['letter_comparisons']
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=hashes,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_functions=['004dc800','0050b450','0050c500','004ed4f0','00417f80','004455e0'],
        test_only_adapter_addresses=['004017c0','005803af','004d2260','004d37d0','004d3bb0'],
        original_entry_point_executed=False,reference_files_modified=False,
        pending=['Localization and rotateText transformation','Control allocation/ownership/activation',
        'Complete TextLine object ABI, XML loading and render/input integration','NaN/infinite or overflowing inputs'])
    args.output.parent.mkdir(parents=True,exist_ok=True); args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"TextLine: {report['layout_scenarios']} original layout scenarios, {report['letter_comparisons']} exact letter geometries and sprites")
if __name__=='__main__': main()
