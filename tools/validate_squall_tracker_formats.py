#!/usr/bin/env python3
"""Retain actual XM/IT/S3M Create/order differences and allocated musical data."""
import argparse
import datetime
import json
from pathlib import Path
import shutil
import subprocess
import struct

from squall_tracker_format_fixtures import generate
from validate_squall_tracker_stages import ROOT,digest,symbol,FIELDS


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-tracker-formats-validation.json')
    args=parser.parse_args()
    work=ROOT/'.local/squall-tracker-formats'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    binaries={'native.dll':ROOT.parent/'farmář/Dobrý farmář/Squall.dll','source.dll':args.candidate.resolve(),
              'source.map':args.candidate.resolve().with_suffix('.map'),
              'probe.exe':ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-stage-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    sources=['tools/validate_squall_tracker_formats.py','tools/squall_tracker_format_fixtures.py',
             'tools/squall_tracker_stage_probe.cpp','tools/validate_squall_tracker_stages.py']
    sources += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.cpp','.h']]
    for name in sources:
        dest=work/'snapshots'/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,dest)
    fixtures=work/'fixtures';fixtures.mkdir();manifest=generate(fixtures)
    native=['2c100','26910','2a590','2c5f0']
    own=[symbol(work/'source.map',name) for name in ['?Create@CDecompressTrack@@','?CreateOrderInfo@CDecompressTrack@@','?Read@CDecompressTrack@@','?Destroy@CDecompressTrack@@']]
    report=dict(status='actual_formats_observations_not_assumed_equivalent',work_directory=work.relative_to(ROOT).as_posix(),
                source_files={name:digest(work/'snapshots'/name) for name in sources},
                binaries={name:digest(work/name) for name in binaries},fixtures=manifest,
                fixture_hashes={p.name:digest(p) for p in fixtures.iterdir()},cases=[])
    for item in manifest:
        for precision in [53,24]:
            for stage in ['create','orders']:
                for backend,rvas in [('native',native),('source',own)]:
                    folder=work/f"{item['format']}-{precision}-{stage}-{backend}";folder.mkdir()
                    run=subprocess.run([str(work/'probe.exe'),str(work/f'{backend}.dll'),str(fixtures/item['name']),*rvas,stage,str(precision)],cwd=folder,capture_output=True,timeout=30)
                    (folder/'stdout.json').write_bytes(run.stdout);(folder/'stderr.log').write_bytes(run.stderr)
                    states={}
                    for path in folder.glob('after-*.bin'):
                        data=path.read_bytes();assert len(data)==65536
                        states[path.stem]={name:struct.unpack_from('<I',data,offset)[0] for name,offset in FIELDS.items()}
                    row=dict(format=item['format'],precision=precision,stage=stage,backend=backend,exit_code=run.returncode,
                             directory=folder.relative_to(ROOT).as_posix(),stdout=run.stdout.decode('utf-8','replace'),stderr=run.stderr.decode('utf-8','replace'),
                             states=states,artifacts={p.name:digest(p) for p in folder.iterdir()})
                    report['cases'].append(row)
                    (work/'report.json').write_bytes((json.dumps(report,indent=2)+'\n').encode('utf-8'))
    differences=[];exact_bytes=0
    for a,b in zip(report['cases'][::2],report['cases'][1::2]):
        diff=[]
        if a['exit_code']!=b['exit_code']:diff.append('exit')
        if a['states']!=b['states']:diff.append('scalar states')
        for name in ['pattern0.bin','instrument1-pcm.bin']:
            x=ROOT/a['directory']/name;y=ROOT/b['directory']/name
            if x.exists()!=y.exists() or (x.exists() and x.read_bytes()!=y.read_bytes()):diff.append(name)
            elif x.exists():exact_bytes+=x.stat().st_size
        if diff:differences.append(dict(format=a['format'],precision=a['precision'],stage=a['stage'],differences=diff))
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    report.update(different_pairs=len(differences),differences=differences,exact_allocated_bytes=exact_bytes,
                  normal_factory_safe_claimed=False)
    encoded=(json.dumps(report,indent=2)+'\n').encode('utf-8');(work/'report.json').write_bytes(encoded);args.output.write_bytes(encoded)
    print(json.dumps(dict(work_directory=report['work_directory'],processes=len(report['cases']),different_pairs=len(differences),exact_allocated_bytes=exact_bytes)))


if __name__=='__main__':main()
