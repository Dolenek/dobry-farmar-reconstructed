#!/usr/bin/env python3
"""Determine whether legacy TODO effect branches are absent in the original too."""
import datetime
import json
import shutil
import struct
import subprocess
from record_live_production_verification import ROOT,digest,read,write
from validate_squall_tracker_stages import symbol
from squall_tracker_remaining_fixtures import mt2
from squall_tracker_format_fixtures import sample_bytes


def fixtures(folder):
    payload=bytearray(mt2(sample_bytes()))
    # Original unpacked MT2 commands: note,instrument,volume,pan,fxcmd,p1,p2.
    for i in range(256):payload[402+i*7:402+(i+1)*7]=bytes([25,1,0x70,0,i,(29*i+7)&255,12])
    (folder/'effects.mt2').write_bytes(payload)
    song=b'AMShdr\x1a\0'+struct.pack('<HBHHHBBBBH',0x202,1,1,1,125<<8,6,32,1,64,0)
    sample=sample_bytes()
    instrument=bytes(1)+bytes([1])+bytes(120+15+5) # empty name,one sample,map,3 empty envelopes,flags
    instrument+=bytes(1)+struct.pack('<IIIHBHbBB',len(sample),0,len(sample),8363,0,8363,0,96,8)
    comments=bytes(1+32)+struct.pack('<I',4)
    pattern=bytes([63,0,0])+b''.join(bytes([0x80,0x80|40,1,fx,(fx*3+5)&255]) for fx in range(64))
    (folder/'effects.ams2').write_bytes(song+instrument+comments+bytes(2)+struct.pack('<I',len(pattern))+pattern+sample+bytes(2048))
    return [('MT2','effects.mt2',4),('AMS2','effects.ams2',32)]


def main():
    work=ROOT/'.local/squall-tracker-effect-formats'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    paths={'native.dll':original/'Squall.dll','source.dll':ROOT/'build/windows/squall-candidate/Release/Squall.dll',
        'fresh.dll':ROOT/'build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll',
        'source.map':ROOT/'build/windows/squall-candidate/Release/Squall.map',
        'fresh.map':ROOT/'build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.map',
        'probe.exe':ROOT/'build/squall-tracker-loader-data-probe-vs/Release/squall-tracker-loader-data-probe.exe'}
    for name,path in paths.items():shutil.copyfile(path,work/name)
    inputs=work/'fixtures';inputs.mkdir();items=fixtures(inputs)
    sources=[ROOT/n for n in ['tools/validate_squall_tracker_effect_formats.py','tools/squall_tracker_remaining_fixtures.py','tools/squall_tracker_format_fixtures.py','tools/squall_tracker_loader_data_probe.cpp']]
    sources += [p for p in (ROOT/'src/squall').rglob('*') if p.suffix in ['.cpp','.h']]
    for path in sources:
        out=work/'snapshots'/path.relative_to(ROOT);out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,out)
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),
        binaries={n:digest(p) for n,p in paths.items()},source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sources},
        fixture_hashes={p.name:digest(p) for p in inputs.iterdir()},cases=[],production_changed=False,whole_squall_complete=False)
    try:
        for kind,name,channels in items:
            for precision in [53,24]:
                outcomes={};patterns=[]
                for backend in ['native','source','fresh']:
                    rvas=['2c100','26910','2a590','2c5f0'] if backend=='native' else [symbol(work/(backend+'.map'),n) for n in ['?Create@CDecompressTrack@@','?CreateOrderInfo@CDecompressTrack@@','?Read@CDecompressTrack@@','?Destroy@CDecompressTrack@@']]
                    folder=work/f'{kind}-{precision}-{backend}';folder.mkdir()
                    p=subprocess.run([str(work/'probe.exe'),str(work/(backend+'.dll')),str(inputs/name),*rvas,'create',str(precision)],cwd=folder,capture_output=True,timeout=30)
                    (folder/'stdout.json').write_bytes(p.stdout);(folder/'stderr.log').write_bytes(p.stderr)
                    outcome=dict(exit_code=p.returncode,directory=folder.relative_to(ROOT).as_posix(),artifacts={q.name:digest(q) for q in folder.iterdir()});outcomes[backend]=outcome
                    assert p.returncode==0,(kind,precision,backend,p.returncode,p.stderr.decode(errors='replace'))
                    state=json.loads(p.stdout);outcome['state']=state;assert state['create_result']==1 and state['guards_intact']
                    pattern=(folder/'pattern0.bin').read_bytes();assert len(pattern)==64*channels*6
                    if kind=='MT2':
                        assert any(pattern[:6]) and pattern[3]!=0
                        assert all(pattern[i*6+3]==pattern[i*6+5]==0 for i in range(1,256))
                    else:
                        assert any(pattern) and all(pattern[row*channels*6+3]==pattern[row*channels*6+5]==0 for row in range(16,64))
                    patterns.append(pattern)
                assert patterns[0]==patterns[1]==patterns[2],(kind,precision,'actual allocated pattern differs')
                report['cases'].append(dict(format=kind,precision=precision,channels=channels,pattern_bytes=len(patterns[0]),pattern_sha256=digest(ROOT/outcomes['native']['directory']/'pattern0.bin'),outcomes=outcomes))
                print(json.dumps(dict(format=kind,precision=precision,pattern_bytes=len(patterns[0]),passed=True)),flush=True)
        for name,sha in report['source_files'].items():assert digest(ROOT/name)==sha,name
        for item in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/item['path'])==item['sha256']
        report.update(status='passed_actual_MT2_256_fx_commands_and_AMS2_64_effects_in_both_builds_only',
            isolated_processes=12,native_ignored_MT2_nonzero_fxcmd_values=255,native_ignored_AMS2_fx_values=list(range(16,64)),
            original_files_unchanged=17,native_factory_used=False,code_or_IAT_written=False,
            interpretation='Public-source TODO branches preserve native absence of extended effects; do not invent effect mappings',
            pending=['Broader packed patterns,envelopes,compressed samples and malformed inputs'])
    except Exception as error:
        import traceback
        report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    write(work/'report.json',report);write(ROOT/'.local/squall-tracker-effect-formats-validation.json',report)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
