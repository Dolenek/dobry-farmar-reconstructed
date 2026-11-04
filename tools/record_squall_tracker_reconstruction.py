#!/usr/bin/env python3
"""Verify actual MOD reconstruction evidence and keep native unsafe paths explicit."""
import datetime
import json
import wave
from pathlib import Path

from record_live_production_verification import ROOT,digest,read,write


def checked(name):
    report=read(ROOT/'.local'/name)
    work=ROOT/report['work_directory']
    for key in ['source_files','sources']:
        for path,value in report.get(key,{}).items():
            assert digest(work/'snapshots'/path)==value,path
            if path.startswith('src/'):assert digest(ROOT/path)==value,path
    for key in ['binaries','binary_hashes']:
        for path,value in report.get(key,{}).items():assert digest(work/path)==value,path
    for row in report.get('cases',[])+report.get('runs',[]):
        if 'directory' in row:
            for path,value in row.get('artifacts',{}).items():assert digest(ROOT/row['directory']/path)==value,path
    return report


def main():
    reports={}
    hashes=[]
    summaries=[]
    # Only address-valued fields are excluded from the owned object comparison.
    # Whole raw objects remain retained. PCM/guards are never normalized.
    pointer_fields={0:'vtable',4:'borrowed SourceData',0x519c:'instrument1 sample',
                    0x9a08:'pattern0 commands',0xef04:'pattern PCM allocation',0xef08:'freed input allocation'}
    pointer_bytes={i for offset in pointer_fields for i in range(offset,offset+4)}
    for suffix in ['', '-fresh']:
        names=dict(wrappers=f'squall-tracker-native-layout-wrappers{suffix}-validation.json',
                   allocations=f'squall-tracker-native-layout-allocations{suffix}-validation.json',
                   owned=f'squall-tracker-wrapped-runtime{suffix}-validation.json')
        loaded={kind:checked(name) for kind,name in names.items()}
        wrappers,allocations,owned=[loaded[k] for k in ['wrappers','allocations','owned']]
        expected=owned['binaries']['source.dll'];hashes.append(expected)
        assert owned['owned_reader_mod_payload_in_real_wav_data']
        for name,value in owned['fixture_hashes'].items():
            path=ROOT/owned['work_directory']/'fixtures'/name
            assert digest(path)==value
            if path.suffix=='.wav':
                with wave.open(str(path),'rb') as stream:
                    assert stream.getnchannels()==1 and stream.getsampwidth()==1
                    assert stream.readframes(stream.getnframes())==path.with_suffix('.mod').read_bytes()
        assert wrappers['binaries']['source.dll']==expected and allocations['binaries']['source.dll']==expected
        assert wrappers['exact_case_pairs']==432 and wrappers['exact_guarded_destination_bytes']==15082200
        assert len(allocations['cases'])==48 and len(owned['cases'])==40
        allocation_bytes=owned_bytes=object_bytes=0;cleanup_failure_pairs=0
        for a,b in zip(allocations['cases'][::2],allocations['cases'][1::2]):
            assert a['backend']=='native' and b['backend']=='source'
            assert a['states']==b['states'] and a['exit_code']==b['exit_code']
            cleanup_failure_pairs+=a['exit_code']==0xc0000374
            assert a['exit_code'] in [0,0xc0000374]
            for name in ['pattern0.bin','instrument1-pcm.bin']:
                x=(ROOT/a['directory']/name).read_bytes();y=(ROOT/b['directory']/name).read_bytes()
                assert x==y;allocation_bytes+=len(x)
        assert cleanup_failure_pairs==12 and allocation_bytes==86016
        sample_lengths={}
        for a,b in zip(owned['cases'][::2],owned['cases'][1::2]):
            assert a['kind']==b['kind']=='owned-constructor' and a['exit_code']==b['exit_code']==0
            x=json.loads(a['stdout']);y=json.loads(b['stdout']);sample_lengths[a['fixture']]=x['samples']
            for key in ['constructor_backend','nonzero_bytes_past_native_factory_allocation']:
                x.pop(key);y.pop(key)
            assert x==y and x['wide_guards_intact']
            x=(ROOT/a['directory']/'decoded-pcm.bin').read_bytes();y=(ROOT/b['directory']/'decoded-pcm.bin').read_bytes()
            assert x==y;owned_bytes+=len(x)
            x=(ROOT/a['directory']/'constructed-object.bin').read_bytes();y=(ROOT/b['directory']/'constructed-object.bin').read_bytes()
            assert len(x)==len(y)==65536 and not any(x[0xef0c:]) and not any(y[0xef0c:])
            assert all(x[i]==y[i] for i in range(0xef0c) if i not in pointer_bytes)
            object_bytes+=0xef0c-len(pointer_bytes)
        assert owned_bytes==328320 and object_bytes==1223440
        assert sorted(sample_lengths.values())==[0,0,0,7056000,28224000]
        summaries.append(dict(build='fresh' if suffix else 'main',candidate_sha256=expected,
                              wrapper_case_pairs=432,exact_wrapper_guarded_bytes=15082200,
                              loader_order_pairs=24,shared_unsafe_titled_cleanup_pairs=12,
                              exact_allocated_pattern_and_instrument_bytes=allocation_bytes,
                              safe_owned_constructor_and_read_pairs=20,exact_owned_guarded_bytes=owned_bytes,
                              exact_initialized_object_nonaddress_bytes=object_bytes,actual_sample_lengths=sample_lengths))
        for kind,name in names.items():reports[name]=loaded[kind]
    initial=checked('squall-tracker-runtime-validation.json');assert len(initial['cases'])==168
    factory=[r for r in initial['cases'] if r['kind']=='factory']
    native=[r for r in factory if r['backend']=='native'];source=[r for r in factory if r['backend']=='source']
    assert len(native)==len(source)==72 and all(r['exit_code']==0xc0000374 for r in native)
    assert sum(r['exit_code']==0 for r in source)==33
    preloads=sum((ROOT/r['directory']/'operations.log').read_text().splitlines()==['load'] for r in source)
    assert preloads==6
    trace=checked('squall-tracker-crash-validation.json');assert len(trace['cases'])==21
    faults=[r for r in trace['cases'] if r['backend']=='source' and r['exit_code']]
    assert len(faults)==1 and faults[0]['exit_code']==0xc0000374
    first=next(e for e in faults[0]['events'] if e['event']=='fault')
    assert first['exception_code']==0xc0000374
    assert any(f['address']-faults[0]['state']['module_base']==0x12faa for f in first['frames'])
    ogg=checked('squall-ogg-rejection-validation.json');assert ogg['exact_case_pairs']==4 and ogg['uninitialized_temp_pointer_confirmed']
    assert all(r['exit_code']==0 for r in ogg['runs'])
    for name,r in [('squall-tracker-runtime-validation.json',initial),('squall-tracker-crash-validation.json',trace),('squall-ogg-rejection-validation.json',ogg)]:reports[name]=r
    audit=read(ROOT/'.local/squall-tracker-loader-offsets-validation.json')
    assert len(audit['loaders'])==21
    for row in audit['loaders']:
        path=ROOT/audit['work_directory']/f"{row['name']}-{row['address']}.txt"
        assert digest(path)==row['instruction_sha256']
    reports['squall-tracker-loader-offsets-validation.json']=audit
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    for name,report in reports.items():write(ROOT/'analysis'/name,report)
    prototype=ROOT/'.local/squall-ogg-rejection/20261104T183413805056Z'
    result=dict(status='passed_defined_mod_loader_mixer_views_wrappers_and_owned_constructors_unsafe_original_factory_open',
                recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),candidate_sha256=hashes,
                builds=summaries,owned_object_address_fields_excluded={hex(k):v for k,v in pointer_fields.items()},
                raw_pcm_normalized=False,original_files_unchanged=17,
                original_program_binaries_used_in_production=False,
                normal_factory_observations=dict(native72_heap_failures=72,source_successes=33,
                                                 source_titled_cleanup_failures=33,source_ogg_preload_failures=preloads,
                                                 claimed_equivalent=False),
                ogg_failure_scope=dict(actual_rejected_constructor_pairs=4,read_only_debugged_source_processes=20,
                                       source_heap_faults=1,original_constructor='100138f0',
                                       source_fault_rva='12faa',production_ogg_changed=False,
                                       diagnosis='Actual rejected constructor leaves tempBuffer+12/tempSize+16 untouched; destructor frees prior pointer. Owned valid allocations plus HeapWalk prove the effect without guessing zero for uninitialized values.'),
                failed_probe_prototype=dict(path=prototype.relative_to(ROOT).as_posix(),
                                            artifacts={p.relative_to(prototype).as_posix():digest(p) for p in prototype.rglob('*') if p.is_file()},
                                            issue='The first Ogg diagnostic queried GlobalSize after freeing a GPTR. It crashed. The retained successor uses HeapWalk before/after and never queries the freed pointer.'),
                reports={name:digest(ROOT/'analysis'/name) for name in reports},
                production_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.cpp','.h']},
                patches=[dict(path='TrackDecoder.h',detail='Explicit new/legacy union field views; no CurOrder; FileBuffer+ef08; checked Win32 offsets'),
                         dict(path='TrackDecoder.cpp',detail='Actual frame-return wrappers1002bfe0/1002c000 and original SetCurrentOrder1002ca90, including entry0 accumulation and MaxSamplesInPattern stereo predecode'),
                         dict(path='load_mod.cpp',detail='Actual old loader fields at Order+b7a8/channels+d058/title+d0e4'),
                         dict(path='sndmix.cpp',detail='Old Read/ProcessRow/ReadNote fields, retaining new snd_fx fields as in original'),
                         dict(path='fastmix.cpp',detail='Old CreateStereoMix fields; common channel/sample arrays unchanged')],
                pending=['Remaining tracker loaders and formats still require original offset/body reconciliation',
                         'Unmodified native factory underallocates0xeef8 versus constructor extent0xef0c; no unsafe undersized allocation added to production',
                         'Original named-MOD invalid delete and Ogg uninitialized fallback remain unsafe and are not claimed successful playback',
                         'Actual general tracker playback,more mixer voices/effects and error/ownership branches',
                         'Full current CTest/game-save regression and remaining original whole-game/JNG/uninstaller goals'],
                whole_game_complete=False,whole_squall_complete=False)
    write(ROOT/'analysis/squall-tracker-scoped-reconstruction.json',result)
    print('Recorded both builds:432 wrapper pairs,24 loader/order pairs,20 safe actual constructors and explicit native unsafe limits each.')


if __name__=='__main__':main()
