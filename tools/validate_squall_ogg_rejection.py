#!/usr/bin/env python3
"""Observe originally untouched Ogg failure fields with real owned buffer pointers."""
import datetime
import json
from pathlib import Path
import shutil
import subprocess
import struct
import wave

from validate_squall_tracker_stages import ROOT,digest,symbol


def main():
    work=ROOT/'.local/squall-ogg-rejection'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    binaries={'native.dll':ROOT.parent/'farmář/Dobrý farmář/Squall.dll',
              'source.dll':ROOT/'build/windows/squall-candidate/Release/Squall.dll',
              'source.map':ROOT/'build/windows/squall-candidate/Release/Squall.map',
              'probe.exe':ROOT/'build/squall-tracker-probe-vs/Release/squall-ogg-failure-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    sources=['tools/validate_squall_ogg_rejection.py','tools/squall_ogg_failure_probe.cpp',
             'src/squall/runtime/SoundFile/Ogg/OggDecoder.cpp','src/squall/runtime/SoundFile/Ogg/OggDecoder.h']
    for name in sources:
        target=work/'snapshots'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,target)
    fixture=work/'owned-pcm.wav'
    with wave.open(str(fixture),'wb') as out:
        out.setparams((1,2,44100,0,'NONE','not compressed'))
        out.writeframes(struct.pack('<32h',*[i*499-7000 for i in range(32)]))
    file=symbol(work/'source.map','?api_SampleGetFile@@');seek=symbol(work/'source.map','?seek@CReader@@')
    ctor=symbol(work/'source.map','??0CDecompressOgg@@')
    report=dict(status='actual_rejected_constructor_and_destructor_field_observations',work_directory=work.relative_to(ROOT).as_posix(),
                binaries={name:digest(work/name) for name in binaries},sources={name:digest(work/'snapshots'/name) for name in sources},
                fixture_sha256=digest(fixture),runs=[])
    states=[]
    for backend in ['native','source']:
        folder=work/backend;folder.mkdir()
        run=subprocess.run([str(work/'probe.exe'),str(work/'source.dll'),str(work/'native.dll'),str(fixture),file,seek,backend,'138f0' if backend=='native' else ctor],cwd=folder,capture_output=True,timeout=30)
        (folder/'stdout.json').write_bytes(run.stdout);(folder/'stderr.log').write_bytes(run.stderr)
        report['runs'].append(dict(backend=backend,exit_code=run.returncode,stdout=run.stdout.decode('utf-8','replace'),stderr=run.stderr.decode('utf-8','replace'),directory=folder.relative_to(ROOT).as_posix(),artifacts={p.name:digest(p) for p in folder.iterdir()}))
        (work/'report.json').write_bytes((json.dumps(report,indent=2)+'\n').encode('utf-8'))
        assert run.returncode==0,report['runs'][-1]
        states.append(json.loads(run.stdout))
    assert states[0]==states[1] and states[0]['case_count']==4
    for name,value in report['sources'].items():assert digest(ROOT/name)==value,name
    report.update(exact_case_pairs=4,production_changed=False,uninitialized_temp_pointer_confirmed=True,
                  unsafe_uninitialized_values_not_normalized=True)
    encoded=(json.dumps(report,indent=2)+'\n').encode('utf-8')
    (work/'report.json').write_bytes(encoded);(ROOT/'.local/squall-ogg-rejection-validation.json').write_bytes(encoded)
    print('Four exact original/source Ogg rejected-input cases; actual destructor frees prior owned buffer; no production change.')


if __name__=='__main__':main()
