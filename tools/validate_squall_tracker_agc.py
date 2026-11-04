#!/usr/bin/env python3
"""Compare actual limiter, recovery and nonzero mixer buffers, not emulated audio."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-tracker-agc-validation.json')
    parser.add_argument('--mode',choices=['all','read'],default='all')
    parser.add_argument('--allow-differences',action='store_true')
    parser.add_argument('--source-snapshot-root',type=Path,default=ROOT)
    args=parser.parse_args()
    work=ROOT/'.local/squall-tracker-agc'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    binaries={'native.dll':ROOT.parent/'farmář/Dobrý farmář/Squall.dll','source.dll':args.candidate.resolve(),
              'source.map':args.candidate.resolve().with_suffix('.map'),
              'probe.exe':ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-agc-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    sources=['tools/validate_squall_tracker_agc.py','tools/squall_tracker_agc_probe.cpp','tools/squall_tracker_probe/CMakeLists.txt']
    paths={name:ROOT/name for name in sources}
    production=args.source_snapshot_root.resolve()/'src/squall'
    for path in sorted(production.rglob('*')):
        if path.suffix in ['.cpp','.h']:paths['src/squall/'+path.relative_to(production).as_posix()]=path
    for name,path in paths.items():
        target=work/'snapshots'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    native=['2a590','18710','759ac','5e1a8','5e1a0','5e19c','819cc','819c8','819d4','819d8']
    own=[]
    for name in ['?Read@CDecompressTrack@@','?ProcessAGC@CDecompressTrack@@','?MixSoundBuffer@@',
                 '?gnAGC@CDecompressTrack@@','?gdwMixingFreq@CDecompressTrack@@','?gnChannels@CDecompressTrack@@',
                 '?gnVUMeter@CDecompressTrack@@','?gdwSoundSetup@CDecompressTrack@@','?gnDryROfsVol@@','?gnDryLOfsVol@@']:
        try:own.append(symbol(work/'source.map',name))
        except AssertionError:
            assert name=='?ProcessAGC@CDecompressTrack@@' and args.mode=='read'
            own.append('0')
    report=dict(status='running_actual_agc',work_directory=work.relative_to(ROOT).as_posix(),mode=args.mode,
                binaries={name:digest(work/name) for name in binaries},source_files={name:digest(work/'snapshots'/name) for name in paths},
                source_snapshot_root=str(args.source_snapshot_root),native_rvas=native,source_rvas=own,runs=[])
    states=[];data=[]
    for backend,rvas in [('native',native),('source',own)]:
        folder=work/backend;folder.mkdir();config=folder/'config.txt';config.write_text('\n'.join(rvas)+'\n',encoding='ascii')
        run=subprocess.run([str(work/'probe.exe'),str(work/f'{backend}.dll'),str(config),args.mode],cwd=folder,capture_output=True,timeout=45)
        (folder/'stdout.json').write_bytes(run.stdout);(folder/'stderr.log').write_bytes(run.stderr)
        report['runs'].append(dict(backend=backend,exit_code=run.returncode,directory=folder.relative_to(ROOT).as_posix(),
                                  stderr=run.stderr.decode('utf-8','replace'),artifacts={p.name:digest(p) for p in folder.iterdir()}))
        (work/'report.json').write_bytes((json.dumps(report,indent=2)+'\n').encode('utf-8'))
        assert run.returncode==0,report['runs'][-1]
        states.append(json.loads(run.stdout));data.append((folder/'destinations.bin').read_bytes())
    assert len(states[0]['cases'])==len(states[1]['cases'])
    different=[i for i,(a,b) in enumerate(zip(states[0]['cases'],states[1]['cases'])) if a!=b]
    different_bytes=sum(a!=b for a,b in zip(data[0],data[1]))+abs(len(data[0])-len(data[1]))
    for name,path in paths.items():assert digest(path)==report['source_files'][name],name
    report.update(status='different_actual_agc_observations' if different or different_bytes else 'passed_scoped_actual_agc_and_mixer',
                  case_pairs=len(states[0]['cases']),different_case_pairs=len(different),different_indices=different,
                  destination_bytes=[len(x) for x in data],different_destination_bytes=different_bytes,
                  kernels=states[0]['kernels'],reads=states[0]['reads'],recovery_steps=states[0]['recovery_steps'],
                  actual_buffers_equal=data[0]==data[1],guards_intact=all(s['guards_intact'] for s in states),
                  loaded_module_data_written=True,code_or_IAT_written=False,production_forwarding=False)
    encoded=(json.dumps(report,indent=2)+'\n').encode('utf-8');(work/'report.json').write_bytes(encoded);args.output.write_bytes(encoded)
    print(json.dumps({key:report[key] for key in ['status','work_directory','case_pairs','different_case_pairs','different_destination_bytes']}))
    assert args.allow_differences or (not different and not different_bytes), 'Actual AGC mismatch retained in report'


if __name__=='__main__':main()
