#!/usr/bin/env python3
"""Actual stopped-channel mixer buffers and ownership-state comparisons."""
import argparse
import datetime
import json
from pathlib import Path
import shutil
import subprocess

from validate_squall_tracker_stages import ROOT,digest,symbol


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-tracker-tail-validation.json')
    parser.add_argument('--source-snapshot-root',type=Path,default=ROOT)
    parser.add_argument('--allow-differences',action='store_true')
    args=parser.parse_args()
    work=ROOT/'.local/squall-tracker-tail'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    binaries={'native.dll':ROOT.parent/'farmář/Dobrý farmář/Squall.dll','source.dll':args.candidate.resolve(),
              'source.map':args.candidate.resolve().with_suffix('.map'),
              'probe.exe':ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-tail-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    paths={name:ROOT/name for name in ['tools/validate_squall_tracker_tail.py','tools/squall_tracker_tail_probe.cpp','tools/squall_tracker_probe/CMakeLists.txt']}
    production=args.source_snapshot_root.resolve()/'src/squall'
    for path in sorted(production.rglob('*')):
        if path.suffix in ['.cpp','.h']:paths['src/squall/'+path.relative_to(production).as_posix()]=path
    for name,path in paths.items():
        target=work/'snapshots'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    native=['17fe0','759ac','819d4','819d8']
    own=[symbol(work/'source.map',name) for name in ['?CreateStereoMix@CDecompressTrack@@','?MixSoundBuffer@@','?gnDryROfsVol@@','?gnDryLOfsVol@@']]
    report=dict(status='running_actual_stopped_channel_mix',work_directory=work.relative_to(ROOT).as_posix(),
                binaries={name:digest(work/name) for name in binaries},source_files={name:digest(work/'snapshots'/name) for name in paths},
                source_snapshot_root=str(args.source_snapshot_root),native_rvas=native,source_rvas=own,runs=[])
    states=[];buffers=[];channels=[]
    for backend,rvas in [('native',native),('source',own)]:
        folder=work/backend;folder.mkdir();config=folder/'config.txt';config.write_text('\n'.join(rvas)+'\n',encoding='ascii')
        run=subprocess.run([str(work/'probe.exe'),str(work/f'{backend}.dll'),str(config)],cwd=folder,capture_output=True,timeout=30)
        (folder/'stdout.json').write_bytes(run.stdout);(folder/'stderr.log').write_bytes(run.stderr)
        report['runs'].append(dict(backend=backend,exit_code=run.returncode,directory=folder.relative_to(ROOT).as_posix(),stderr=run.stderr.decode('utf-8','replace'),artifacts={p.name:digest(p) for p in folder.iterdir()}))
        (work/'report.json').write_bytes((json.dumps(report,indent=2)+'\n').encode('utf-8'));assert run.returncode==0,report['runs'][-1]
        states.append(json.loads(run.stdout));buffers.append((folder/'destinations.bin').read_bytes());channels.append((folder/'channel-scalars.bin').read_bytes())
    assert all(s['case_count']==288 and s['guards_intact'] for s in states)
    diff_rows=sum(a!=b for a,b in zip(states[0]['cases'],states[1]['cases']))
    diff_bytes=sum(a!=b for a,b in zip(buffers[0],buffers[1]))
    assert channels[0]==channels[1]
    report.update(status='passed_actual_stopped_channel_mix' if not diff_rows and not diff_bytes else 'different_actual_stopped_channel_mix',
                  case_pairs=288,different_case_pairs=diff_rows,different_destination_bytes=diff_bytes,
                  channel_scalar_bytes=len(channels[0]),destination_bytes=len(buffers[0]),
                  address_field_excluded='Channel pCurrentSample is tested as actual cleared/retained ownership and excluded from raw scalar comparison',
                  code_or_IAT_written=False,production_forwarding=False)
    for name,path in paths.items():assert digest(path)==report['source_files'][name],name
    encoded=(json.dumps(report,indent=2)+'\n').encode('utf-8');(work/'report.json').write_bytes(encoded);args.output.write_bytes(encoded)
    print(json.dumps({k:report[k] for k in ['status','work_directory','case_pairs','different_case_pairs','different_destination_bytes']}))
    assert args.allow_differences or (not diff_rows and not diff_bytes),'Actual stopped mixer difference retained'


if __name__=='__main__':main()
