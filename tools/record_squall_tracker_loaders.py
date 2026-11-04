#!/usr/bin/env python3
"""Audit actual loader/mixer repairs and preserve every preceding difference."""
import datetime
import json
import wave

from record_live_production_verification import ROOT,digest,read,write


def checked(name,current=True):
    report=read(ROOT/'.local'/name);work=ROOT/report['work_directory']
    for path,value in report.get('source_files',{}).items():
        assert digest(work/'snapshots'/path)==value,path
        if current and path.startswith('src/'):assert digest(ROOT/path)==value,path
    for path,value in report.get('binaries',{}).items():assert digest(work/path)==value,path
    for row in report.get('runs',[])+report.get('cases',[]):
        for path,value in row.get('artifacts',{}).items():assert digest(ROOT/row['directory']/path)==value,path
    return report


def main():
    reports={};builds=[];hashes=[]
    pointer_fields={0:'vtable',4:'borrowed SourceData',0x519c:'instrument1 sample',0x8d4c:'XM instrument header1',
                    0x9a08:'pattern0',0xef04:'pattern PCM',0xef08:'freed input allocation'}
    excluded={i for o in pointer_fields for i in range(o,o+4)}
    for suffix in ['', '-fresh']:
        names={kind:f'squall-tracker-loaders-{kind}{suffix}-validation.json' for kind in ['formats','agc','tail','runtime']}
        r={kind:checked(name) for kind,name in names.items()};expected=r['runtime']['binaries']['source.dll'];hashes.append(expected)
        assert all(value['binaries']['source.dll']==expected for value in r.values())
        assert r['formats']['different_pairs']==0 and r['formats']['exact_allocated_bytes']==43008 and len(r['formats']['cases'])==24
        assert all(row['exit_code']==0 for row in r['formats']['cases'])
        agc=r['agc'];assert agc['case_pairs']==9088 and agc['different_case_pairs']==agc['different_destination_bytes']==0
        assert agc['kernels']==1728 and agc['reads']==960 and agc['recovery_steps']==6400
        assert agc['destination_bytes']==[10390080,10390080] and agc['guards_intact']
        tail=r['tail'];assert tail['case_pairs']==288 and tail['different_case_pairs']==tail['different_destination_bytes']==0
        assert tail['channel_scalar_bytes']==72576 and tail['destination_bytes']==1179648
        runtime=r['runtime'];assert len(runtime['cases'])==64
        assert runtime['owned_reader_mod_payload_in_real_wav_data']
        fixtures=ROOT/runtime['work_directory']/'fixtures'
        for name,value in runtime['fixture_hashes'].items():assert digest(fixtures/name)==value,name
        for payload,container in runtime['wrapped_reader_files'].items():
            with wave.open(str(fixtures/container),'rb') as stream:
                assert stream.getnchannels()==1 and stream.getsampwidth()==1
                assert stream.readframes(stream.getnframes())==(fixtures/payload).read_bytes()
        pcm_bytes=object_bytes=0;lengths={}
        for a,b in zip(runtime['cases'][::2],runtime['cases'][1::2]):
            assert a['exit_code']==b['exit_code']==0
            x=json.loads(a['stdout']);y=json.loads(b['stdout']);lengths[a['fixture']]=x['samples']
            for key in ['constructor_backend','nonzero_bytes_past_native_factory_allocation']:x.pop(key);y.pop(key)
            assert x==y and x['wide_guards_intact']
            x=(ROOT/a['directory']/'decoded-pcm.bin').read_bytes();y=(ROOT/b['directory']/'decoded-pcm.bin').read_bytes()
            assert x==y;pcm_bytes+=len(x)
            x=(ROOT/a['directory']/'constructed-object.bin').read_bytes();y=(ROOT/b['directory']/'constructed-object.bin').read_bytes()
            assert len(x)==len(y)==65536 and not any(x[0xef0c:]) and not any(y[0xef0c:])
            assert all(x[i]==y[i] for i in range(0xef0c) if i not in excluded)
            object_bytes+=0xef0c-len(excluded)
        assert pcm_bytes==525312 and object_bytes==1957376 and len(lengths)==8
        builds.append(dict(build='fresh' if suffix else 'main',candidate_sha256=expected,
                           actual_loader_order_pairs=12,exact_allocated_musical_bytes=43008,
                           actual_agc_kernel_cases=1728,actual_recovery_steps=6400,actual_nonzero_mixer_reads=960,
                           exact_agc_and_guarded_mixer_bytes=10390080,
                           actual_channel_stop_mix_cases=288,exact_channel_stop_mix_bytes=1179648,
                           exact_channel_stop_scalar_bytes=72576,
                           safe_actual_constructor_pairs=32,exact_constructor_guarded_bytes=pcm_bytes,
                           exact_initialized_nonaddress_object_bytes=object_bytes,actual_lengths=lengths))
        for kind,name in names.items():reports[name]=r[kind]
    initial=checked('squall-tracker-formats-initial-validation.json',False)
    assert initial['different_pairs']==12 and len(initial['cases'])==24
    before=checked('squall-tracker-agc-before-validation.json',False)
    assert before['different_case_pairs']==482 and before['different_destination_bytes']==1429394
    partial=checked('squall-tracker-agc-validation.json',False)
    assert partial['different_case_pairs']==474 and partial['different_destination_bytes']==1419532
    tail_before=checked('squall-tracker-tail-before-validation.json',False)
    assert tail_before['different_destination_bytes']==269252
    for name,value in [('squall-tracker-formats-initial-validation.json',initial),('squall-tracker-agc-before-validation.json',before),
                       ('squall-tracker-agc-validation.json',partial),('squall-tracker-tail-before-validation.json',tail_before)]:reports[name]=value
    audit=read(ROOT/'.local/squall-tracker-loader-offsets-validation.json');assert len(audit['loaders'])==21
    for row in audit['loaders']:
        path=ROOT/audit['work_directory']/f"{row['name']}-{row['address']}.txt";assert digest(path)==row['instruction_sha256']
        source=ROOT/'src/squall/runtime/SoundFile/track'/row['path'];assert digest(source)==row['source_sha256']
        assert not row['field_instructions'] or row['legacy_view_already_present']
    # Keep the earlier MOD-only audit immutable rather than replacing its source hashes.
    reports['squall-tracker-loaders-offsets-validation.json']=audit
    prototype=ROOT/'.local/squall-tracker-runtime/20261104T190857765075Z';attempt=read(prototype/'report.json')
    assert len(attempt['cases'])==32 and attempt['binaries']['source.dll']==hashes[0]
    for name,report in reports.items():write(ROOT/'analysis'/name,report)
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_scoped_old_loader_fields_and_nonzero_mixer_reconstruction_both_builds',
                recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),candidate_sha256=hashes,builds=builds,
                production_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.cpp','.h']},
                reports={name:digest(ROOT/'analysis'/name) for name in reports},
                owned_object_address_fields_excluded={hex(k):v for k,v in pointer_fields.items()},
                raw_pcm_normalized=False,original_files_unchanged=17,original_program_binaries_used_in_production=False,
                diagnostic_module_data_written=True,diagnostic_code_or_IAT_written=False,
                preceding_differences=dict(loader_pairs=12,missing_agc_mixer_pairs=482,missing_agc_mixer_bytes=1429394,
                                           agc_only_repair_remaining_pairs=474,agc_only_repair_remaining_bytes=1419532,
                                           incorrect_channel_stop_bytes=269252),
                failed_extended_driver_attempt=dict(work_directory=prototype.relative_to(ROOT).as_posix(),
                    report_sha256=digest(prototype/'report.json'),completed_processes=32,
                    issue='FileExistsError WinError183: fixture.stem omitted extensions, colliding XM/IT/S3M directories. Full successful retry uses fixture.name; production binary unchanged.',
                    artifact_hashes={p.relative_to(prototype).as_posix():digest(p) for p in prototype.rglob('*') if p.is_file()}),
                patches=['Nineteen further loader translation units use explicit native legacy fields; MOD already restored; UMX dispatch contains no shifted fields',
                         'Restore original Read AGC branch1002a6e6..1002a6f7 and actual limiter/recovery100186d0/10018710',
                         '10018510: add0xfff to negative values before clipping; clipping sets both VU extremes',
                         '100185a0: explicit wrapping ADD then arithmetic SAR for stereo-to-mono',
                         '100185d0/10018670: store/add decayed offset levels rather than decay increments; explicit Win32 NEG/ADD/SUB wrapping'],
                pending=['Broader remaining tracker format/body cases and actual active sample voices/effects',
                         'Unmodified native tracker factory/title invalid frees and uninitialized Ogg fallback remain explicitly unsafe',
                         'Current whole-game regression/audio/saves and later natural gameplay/JNG/uninstaller goals'],
                whole_squall_complete=False,whole_game_complete=False)
    write(ROOT/'analysis/squall-tracker-loaders-scoped-reconstruction.json',result)
    print('Recorded both builds:12 loader/order pairs,9088 AGC/mixer cases,288 channel stops and32 safe actual constructor pairs each.')


if __name__=='__main__':main()
