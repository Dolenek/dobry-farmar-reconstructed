#!/usr/bin/env python3
"""Compare complete owned tracker objects and actual additional loader allocations."""
import argparse
import datetime
import json
from pathlib import Path
import shutil
import subprocess
import struct

from squall_tracker_more_fixtures import generate
from validate_squall_tracker_stages import ROOT,digest,symbol,FIELDS


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-tracker-loader-data-validation.json')
    args=parser.parse_args()
    work=ROOT/'.local/squall-tracker-loader-data'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    binaries={'native.dll':ROOT.parent/'farmář/Dobrý farmář/Squall.dll','source.dll':args.candidate.resolve(),
              'source.map':args.candidate.resolve().with_suffix('.map'),
              'probe.exe':ROOT/'build/squall-tracker-loader-data-probe-vs/Release/squall-tracker-loader-data-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    sources=['tools/validate_squall_tracker_loader_data.py','tools/squall_tracker_more_fixtures.py',
             'tools/squall_tracker_loader_data_probe.cpp','tools/squall_tracker_loader_data_probe/CMakeLists.txt','tools/validate_squall_tracker_stages.py','tools/squall_tracker_format_fixtures.py']
    sources += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.cpp','.h']]
    for name in sources:
        dest=work/'snapshots'/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,dest)
    fields=dict(FIELDS);fields.update({('legacy_'+name):offset-4 for name,offset in FIELDS.items() if name not in ['samples','max_pattern_samples']})
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
                        states[path.stem]={name:struct.unpack_from('<I',data,offset)[0] for name,offset in fields.items()}
                    row=dict(format=item['format'],precision=precision,stage=stage,backend=backend,exit_code=run.returncode,
                             directory=folder.relative_to(ROOT).as_posix(),stdout=run.stdout.decode('utf-8','replace'),stderr=run.stderr.decode('utf-8','replace'),
                             states=states,artifacts={p.name:digest(p) for p in folder.iterdir()})
                    if run.returncode==0:row['actual_call_state']=json.loads(run.stdout)
                    report['cases'].append(row)
                    (work/'report.json').write_bytes((json.dumps(report,indent=2)+'\n').encode('utf-8'))
    excluded={0,4,0xd0dc,0xd0e0,0xd0e4,0xef04,0xef08}
    excluded|={8+i*256+d for i in range(80) for d in [0,96,140,144]}
    excluded|={0x5148+i*64+20 for i in range(240)}
    excluded|={0x8d48+i*4 for i in range(240)}|{0x9a08+i*4 for i in range(240)}
    excluded|={base+i*144+d for base in [0xcbc8,0xcbcc] for i in range(8) for d in [0,4,12]}
    excluded=sorted(excluded);assert len(excluded)==1087
    spans=[];start=0
    for offset in excluded:
        if offset>start:spans.append((start,offset))
        start=offset+4
    spans.append((start,61196));compared_per_object=sum(end-start for start,end in spans);assert compared_per_object==56848
    differences=[];exact_bytes=0;object_pairs=0;exact_object_bytes=0;address_states=0

    for a,b in zip(report['cases'][::2],report['cases'][1::2]):
        diff=[]
        assert a['exit_code']==b['exit_code']==0,(a['format'],a['stage'],a['exit_code'],b['exit_code'])
        if a['states']!=b['states']:diff.append('scalar states')
        native=dict(a['actual_call_state']);own=dict(b['actual_call_state']);native_base=native.pop('object_base');source_base=own.pop('object_base')
        assert native['guards_intact'] and own['guards_intact'] and native['create_result']==own['create_result']==1
        assert native['object_size']==own['object_size']==61196 and native['instrument_header_size']==own['instrument_header_size']==632
        expected=0x20000 if a['precision']==24 else 0x10000
        for values in [native,own]:
            for name,value in values.items():
                if name.endswith('control_before'):assert value&0x30000==expected,(a['format'],a['stage'],name,value)
        if native!=own:diff.append('actual call/control state')
        for stage in a['states']:
            x=(ROOT/a['directory']/(stage+'.bin')).read_bytes();y=(ROOT/b['directory']/(stage+'.bin')).read_bytes();object_pairs+=1
            for begin,end in spans:
                if x[begin:end]!=y[begin:end]:diff.append('initialized storage '+stage+' '+hex(begin))
            exact_object_bytes+=compared_per_object
            for offset in excluded:
                left=struct.unpack_from('<I',x,offset)[0];right=struct.unpack_from('<I',y,offset)[0]
                def classify(value,base):
                    if not value:return ('null',0)
                    if base<=value<base+61196:return ('object_relative',value-base)
                    return ('external_nonnull',0)
                if classify(left,native_base)!=classify(right,source_base):diff.append('pointer class '+stage+' '+hex(offset))
                address_states+=1

        for name in ['pattern0.bin','pattern1.bin','instrument1-pcm.bin','header1.bin','legacy-comments.bin']:
            x=ROOT/a['directory']/name;y=ROOT/b['directory']/name
            if x.exists()!=y.exists() or (x.exists() and x.read_bytes()!=y.read_bytes()):diff.append(name)
            elif x.exists():exact_bytes+=x.stat().st_size
        if diff:differences.append(dict(format=a['format'],precision=a['precision'],stage=a['stage'],differences=diff))
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    report.update(status='passed_additional_loader_data_scope' if not differences else 'different_additional_loader_data_scope',object_pairs=object_pairs,initialized_storage_bytes=exact_object_bytes,address_slot_states=address_states,excluded_address_dword_offsets=excluded,initialized_bytes_compared_per_object=compared_per_object,requested_precision_measured_immediately_before_actual_calls=True,nullable_and_object_relative_pointer_classes_compared=True,opaque_external_pointer_contents_only_verified_for_explicitly_dumped_allocations=True,different_pairs=len(differences),differences=differences,exact_allocated_bytes=exact_bytes,
                  normal_factory_safe_claimed=False)
    encoded=(json.dumps(report,indent=2)+'\n').encode('utf-8');(work/'report.json').write_bytes(encoded);args.output.write_bytes(encoded)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],processes=len(report['cases']),different_pairs=len(differences),exact_allocated_bytes=exact_bytes,initialized_storage_bytes=exact_object_bytes)))


if __name__=='__main__':main()
