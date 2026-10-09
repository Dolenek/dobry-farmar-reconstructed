#!/usr/bin/env python3
"""Compare localization lookup with the original, isolating writes to fixtures."""
import argparse
import datetime
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/string-manager-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/string-manager-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář'; inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    hashes={name:next(row['sha256'] for row in inventory['files'] if row['path']==name)
            for name in ('Dobrý farmář.exe','Data/Data.pack','Data/cz.pack')}
    def verify():
        for name,expected in hashes.items(): assert hashlib.sha256((original/name).read_bytes()).hexdigest()==expected,name
    verify(); executable=args.oracle.resolve(strict=True)
    with tempfile.TemporaryDirectory(dir=ROOT/'.local',prefix='localization-') as common:
        result=subprocess.run([str(executable),str(original/'Dobrý farmář.exe'),str(original/'Data'),common],capture_output=True,text=True,timeout=60)
    if result.returncode: raise RuntimeError(f'StringManager oracle failed ({result.returncode}): {result.stderr}')
    report=json.loads(result.stdout); verify(); assert report['translations']>100 and report['original_lookup_comparisons']>10000 and report['dump_fixture_checks']==4
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=hashes,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_functions=['0052ed20','0052d2a0','0052d1a0','00401050'],
        test_only_adapter_addresses=['004017c0'],original_dump_mode_executed=False,
        original_entry_point_executed=False,reference_files_modified=False,
        pending=['Original constructor/lifetime and DumpStrings runtime comparison','Original object/container and exception ABI',
        'XML section text getter runtime comparison','Full menu/input/game integration'])
    args.output.parent.mkdir(parents=True,exist_ok=True); args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Localization: {report['translations']} translations, {report['original_lookup_comparisons']} original comparisons; {report['dump_fixture_checks']} isolated dump checks")
if __name__=='__main__': main()
