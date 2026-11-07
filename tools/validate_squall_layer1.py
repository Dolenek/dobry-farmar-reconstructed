#!/usr/bin/env python3
"""Compare independently decoded owned Layer-I files through real DLL PCM methods."""
import datetime
import json
import shutil
import subprocess
from record_live_production_verification import ROOT,digest,read,write
from validate_squall_tracker_stages import symbol
from squall_mpeg_layer1_fixtures import generate


def main():
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    work=ROOT/'.local/squall-layer1'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    candidates={'source':ROOT/'build/windows/squall-candidate/Release/Squall.dll',
        'fresh':ROOT/'build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll'}
    probe=ROOT/'build/squall-mpeg-formats-probe-vs/Release/squall-mpeg-decode-reference-test.exe'
    encoder=ROOT/'.local/audio-fixture-encoder/ffmpeg.exe'
    provenance=read(ROOT/'.local/audio-fixture-encoder/provenance.json');assert digest(encoder)==provenance['executable_sha256']
    paths={'native.dll':original/'Squall.dll','source.dll':candidates['source'],'fresh.dll':candidates['fresh'],
        'source.map':candidates['source'].with_suffix('.map'),'fresh.map':candidates['fresh'].with_suffix('.map'),'probe.exe':probe}
    for name,path in paths.items():shutil.copyfile(path,work/name)
    sources=[p for p in (ROOT/'src/squall').rglob('*') if p.suffix in ['.cpp','.h']]+[ROOT/n for n in [
        'tools/validate_squall_layer1.py','tools/squall_mpeg_layer1_fixtures.py','tools/squall_mpeg_decode_reference_test.cpp','tools/squall_mpeg_formats_probe/CMakeLists.txt','tools/squall_exports.inc']]
    for path in sources:
        out=work/'snapshots'/path.relative_to(ROOT);out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,out)
    items=generate(work)
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        binaries={name:digest(path) for name,path in paths.items()},source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sources},
        independent_decoder_provenance=provenance,fixtures=items,cases=[],physical_audio_device_opened=False,whole_squall_complete=False)
    try:
        for item in items:
            path=work/item['name'];out=path.with_suffix('.independent.s16le')
            run=subprocess.run([str(encoder),'-hide_banner','-loglevel','error','-nostdin','-y','-f','mp3','-i',str(path),'-f','s16le',str(out)],capture_output=True,timeout=30)
            (path.with_suffix('.independent.stderr.log')).write_bytes(run.stderr)
            assert run.returncode==0,(item['name'],run.stderr.decode(errors='replace'))
            assert out.stat().st_size==item['decoded_frames']*item['channels']*2,(item['name'],out.stat().st_size)
            raw=out.read_bytes();assert bool(any(raw))!=item['silent'],item['name']
            item.update(input_sha256=digest(path),independent_PCM_sha256=digest(out),independent_PCM_bytes=len(raw))
        for backend in ['source','fresh']:
            table=work/(backend+'.map');rvas=[symbol(table,n) for n in ['?api_SampleGetFile@@','?GetMonoData@CSoundFile@@','?GetStereoData@CSoundFile@@']]
            rvas.append(f'{int(symbol(table,"?api@@3Usquall_api_s@@A"),16)+12:x}')
            for precision in [53,24]:
                for index,item in enumerate(items):
                    folder=work/f'{backend}-pc{precision}-{index}';folder.mkdir();shutil.copyfile(work/item['name'],folder/item['name'])
                    command=[str(work/'probe.exe'),str(work/'native.dll'),str(work/(backend+'.dll')),*rvas,item['name']]
                    if precision==24:command.append('pc24')
                    p=subprocess.run(command,cwd=folder,capture_output=True,timeout=60)
                    (folder/'stdout.json').write_bytes(p.stdout);(folder/'stderr.log').write_bytes(p.stderr)
                    row=dict(backend=backend,precision=precision,fixture=item['name'],directory=folder.relative_to(ROOT).as_posix(),exit_code=p.returncode,
                        artifacts={q.name:digest(q) for q in folder.iterdir()});report['cases'].append(row);write(work/'report.json',report)
                    assert p.returncode==0,(backend,precision,index,p.returncode,p.stderr.decode(errors='replace'))
                    value=json.loads(p.stdout);row['result']=value
                    assert value['layer']==1 and value['frequency']==item['rate'] and value['guarded_pcm_reads']==40
                    assert value['physical_buffers']==0 and not value['actual_directsound']
                    assert bool(value['nonzero_pcm_bytes'])!=item['silent'],(backend,precision,index,'unexpected silent decoded PCM')
                    print(json.dumps(dict(backend=backend,precision=precision,index=index,nonzero_PCM_bytes=value['nonzero_pcm_bytes'],passed=True)),flush=True)
        for name,sha in report['source_files'].items():assert digest(ROOT/name)==sha,name
        for name,path in paths.items():assert digest(path)==report['binaries'][name]
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/row['path'])==row['sha256']
        report.update(status='passed_owned_MPEG_LayerI_defined_frame_prefix_PCM_and_native_metadata_both_builds_only',
            independent_valid_inputs=8,isolated_pair_processes=32,guarded_PCM_reads=1280,original_files_unchanged=17,
            original_program_code_used_in_implementation=False,
            defined_PCM_prefix_frames_per_packet=384,uninitialized_native_packet_gaps_not_compared=True,
            pending=['Native HeaderInfo1152/576 versus actual384 samples per LayerI frame;unwritten gaps and original public length retained,not claimed valid general playback','Physical buffers,end-wrap,CRC,padding,free-format and malformed variants'])
    except Exception as error:
        import traceback
        report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    write(work/'report.json',report);write(ROOT/'.local/squall-layer1-validation.json',report)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],completed_cases=len(report['cases']),error=report.get('error'))))
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
