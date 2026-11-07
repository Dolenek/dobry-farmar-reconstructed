#!/usr/bin/env python3
"""Compare sequential real tracker decoding on owned constructor storage."""
import datetime
import json
import shutil
import subprocess
import wave
from record_live_production_verification import ROOT,digest,read,write
from validate_squall_tracker_stages import symbol
from squall_tracker_fixtures import generate


def main():
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    work=ROOT/'.local/squall-tracker-stream'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    paths={'native.dll':original/'Squall.dll','source.dll':ROOT/'build/windows/squall-candidate/Release/Squall.dll',
        'fresh.dll':ROOT/'build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll',
        'source.map':ROOT/'build/windows/squall-candidate/Release/Squall.map',
        'fresh.map':ROOT/'build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.map',
        'probe.exe':ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-stream-probe.exe'}
    for name,path in paths.items():shutil.copyfile(path,work/name)
    sources=[p for p in (ROOT/'src/squall').rglob('*') if p.suffix in ['.cpp','.h']]+[ROOT/n for n in [
        'tools/validate_squall_tracker_stream.py','tools/squall_tracker_stream_probe.cpp','tools/squall_tracker_probe/CMakeLists.txt','tools/squall_tracker_fixtures.py','tools/validate_squall_tracker_stages.py']]
    for path in sources:
        out=work/'snapshots'/path.relative_to(ROOT);out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,out)
    fixtures=work/'fixtures';fixtures.mkdir();generate(fixtures)
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sources},
        binaries={n:dict(path=p.relative_to(ROOT).as_posix() if p.is_relative_to(ROOT) else str(p),sha256=digest(p)) for n,p in paths.items()},
        cases=[],deviations=[],native_factory_used=False,original_code_or_IAT_written=False,
        constructor_storage_bytes=65536,carrier='Real source WAV reader containing blank-title owned MOD payload; native and source use the same reconciled reader ABI',
        whole_squall_complete=False)
    try:
        for variant in range(3):
            mod=fixtures/f'owned-protracker-{variant}.mod';carrier=fixtures/f'blank-{variant}.wav'
            payload=bytearray(bytes(20)+mod.read_bytes()[20:]);payload[950]=5
            with wave.open(str(carrier),'wb') as out:
                out.setparams((1,1,44100,0,'NONE','not compressed'));out.writeframes(payload)
            for pc in [53,24]:
                for slot in [1,2]:
                    outcomes={};data={};row=dict(variant=variant,orders=5,precision=pc,slot=slot,outcomes=outcomes)
                    for backend in ['native','source','fresh']:
                        source='source' if backend=='native' else backend;table=work/(source+'.map')
                        file=symbol(table,'?api_SampleGetFile@@');seek=symbol(table,'?seek@CReader@@')
                        ctor='2bdb0' if backend=='native' else symbol(table,'??0CDecompressTrack@@')
                        folder=work/f'{variant}-pc{pc}-slot{slot}-{backend}';folder.mkdir()
                        command=[str(work/'probe.exe'),str(work/'native.dll'),str(work/(source+'.dll')),str(carrier),file,seek,ctor,str(slot),'native' if backend=='native' else 'source']
                        if pc==24:command.append('pc24')
                        p=subprocess.run(command,cwd=folder,capture_output=True,timeout=90)
                        (folder/'stdout.json').write_bytes(p.stdout);(folder/'stderr.log').write_bytes(p.stderr)
                        value=dict(exit_code=p.returncode,directory=folder.relative_to(ROOT).as_posix(),artifacts={q.name:digest(q) for q in folder.iterdir()});outcomes[backend]=value
                        assert p.returncode==0,(variant,pc,slot,backend,p.returncode,p.stderr.decode(errors='replace'))
                        state=json.loads(p.stdout);value['state']=state
                        assert state['blocks']==128 and state['wide_guards_intact'] and state['precision_bits']==(0x10000 if pc==53 else 0x20000)
                        assert len(state['block_returns'])==128
                        data[backend]=(folder/'decoded-pcm.bin').read_bytes()
                    for backend in ['source','fresh']:
                        native=dict(outcomes['native']['state']);own=dict(outcomes[backend]['state'])
                        for key in ['constructor_backend','nonzero_bytes_past_native_factory_allocation']:native.pop(key);own.pop(key)
                        differing=sum(a!=b for a,b in zip(data['native'],data[backend]))
                        if native!=own or data['native']!=data[backend]:
                            report['deviations'].append(dict(variant=variant,precision=pc,slot=slot,backend=backend,
                                differing_pcm_bytes=differing,native_bytes=len(data['native']),candidate_bytes=len(data[backend]),
                                differing_state_fields={k:[v,own.get(k)] for k,v in native.items() if v!=own.get(k)}))
                    report['cases'].append(row);write(work/'report.json',report)
                    print(json.dumps(dict(variant=variant,precision=pc,slot=slot,completed_cases=len(report['cases']),deviations=len(report['deviations']))),flush=True)
        for name,sha in report['source_files'].items():assert digest(ROOT/name)==sha,name
        for name,path in paths.items():assert digest(path)==report['binaries'][name]['sha256']
        for item in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/item['path'])==item['sha256']
        report.update(status='passed_actual_sequential_tracker_PCM_on_owned_constructor_storage_only' if not report['deviations'] else 'observed_actual_sequential_tracker_differences',
            isolated_processes=36,blocks_per_process=128,frames_requested_per_process=524288,original_files_unchanged=17,
            pending=['Unmodified native undersized tracker factory and named-title invalid free remain unsafe','Other format variants and general malformed or concurrent playback'])
    except Exception as error:
        import traceback
        report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    write(work/'report.json',report);write(ROOT/'.local/squall-tracker-stream-validation.json',report)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],deviations=len(report['deviations']),error=report.get('error'))))
    return int(report['status']!='passed_actual_sequential_tracker_PCM_on_owned_constructor_storage_only')


if __name__=='__main__':raise SystemExit(main())
