#!/usr/bin/env python3
"""Record exact MPEG reconstruction, separately proved native unsafe ranges, and limits."""
import datetime,json
from pe import PE
from record_live_production_verification import ROOT,digest,read,write

def checked(name):
    report=read(ROOT/'.local'/name);work=ROOT/report['work_directory']
    for filename,value in report['source_files'].items():
        assert digest(ROOT/filename)==value,filename
        assert digest(work/'source-snapshots'/filename)==value,filename
    for filename,value in report.get('build_artifacts',report.get('build_artifact_snapshots',{})).items():assert digest(work/'build-artifacts'/filename)==value,filename
    assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256']
    for row in report['cases']:
        for filename,value in row['artifacts'].items():
            path=work/filename if row.get('kind') else ROOT/row['directory']/filename
            assert digest(path)==value,filename
    return report

def main():
    reports={};hashes=[]
    for suffix in ['', '-fresh']:
        name=f'squall-mpeg-safe-formats{suffix}-validation.json';report=checked(name)
        assert report['status']=='passed_for_eight_owned_mpeg_inputs_defined_ranges_and_seven_safe_physical_buffer_formats_only' and report['failed_cases']==0 and len(report['cases'])==16
        assert sum(row['result']['guarded_pcm_reads'] for row in report['cases'])==640
        assert sum(row['result']['pcm_bytes'] for row in report['cases'])==4421736
        assert sum(row['result']['physical_buffers'] for row in report['cases'])==56
        assert sum(row['result']['physical_pcm_bytes'] for row in report['cases'])==1690416
        unsafe=[row for row in report['cases'] if row['result']['physical_buffers_skipped_native_layer2_overrun']]
        assert len(unsafe)==2 and all(row['fixture']=='mpeg-2-mp2-22050-2.mp2' and row['result']['frames']==11520 and row['result']['defined_seek_frames']==5760 and row['result']['frame_table_entries']==10 and row['result']['samples_per_frame']==576 for row in unsafe)
        hashes.append(report['rebuilt_sha256']);reports[name]=report
        name=f'squall-mpeg-kernels{suffix}-validation.json';report=checked(name)
        assert report['status']=='passed_eight_exact_table_initializers_and_40960_guarded_kernel_cases_only' and report['rebuilt_sha256']==hashes[-1]
        assert report['kernel_cases']==40960 and report['table_object_bytes_compared']==381952 and len(report['cases'])==18
        for row in report['cases']:
            assert row['exit_code']==0
            if row['kind']=='tables':assert row['result']['different_words']==0
            else:assert row['result']['cases']==4096 and row['result']['failed_cases']==0
        reports[name]=report
    guard=ROOT/'.local/squall-mpeg-overrun-guard';g=read(guard/'stdout.json');assert g['guarded_overreads']==8 and g['matched_wide_backing_output_overrun_pairs']==4
    for row in g['cases']:assert row['source_exception']==row['native_exception']==0xc0000005 and row['table_index']==16 and row['entries']==10 and row['guard_byte_offset']==24 and row['stereo_returned_bytes']==2308 and row['stereo_changed_bytes_past_declared_end']==2298 and row['stereo_last_changed_byte']==4608
    reports['squall-mpeg-native-unsafe-ranges.json']=dict(status='matched_actual_native_and_source_table_overread_and_wide_backing_output_overrun',result=g,artifacts={path.relative_to(ROOT).as_posix():digest(path) for path in guard.iterdir() if path.is_file()},probe_sha256=digest(ROOT/'build/squall-mpeg-formats-probe-vs/Release/squall-mpeg-seek-probe.exe'),probe_source_sha256=digest(ROOT/'tools/squall_mpeg_seek_probe.cpp'),source_dll_sha256=hashes[0],production_modified_for_guard=False,diagnostic_pointer_temporarily_replaced=True,raw_undefined_heap_values_claimed_equal=False,unsafe_physical_buffers_claimed_verified=False)
    instruction_names=['squall-mpeg-alias-native.txt','squall-mpeg-pow-native.txt','squall-mpeg-pow-tail-native.txt','squall-mpeg-quant-native.txt','squall-mpeg-hwin-native.txt','squall-mpeg-window-native.txt','squall-mpeg-imdct18-native.txt','squall-mpeg-imdct6-native.txt','squall-mpeg-constructor-native.txt','squall-mpeg-seek-native.txt']
    instruction=ROOT/'analysis/squall-mpeg-native-instructions.txt';instruction.write_bytes(b''.join(('\n'+name+'\n').encode()+ (ROOT/'.local'/name).read_bytes() for name in instruction_names))
    comparison=[];ranges=[]
    for name in ['build/windows/squall-candidate/Release/Squall.dll','build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll']:
        pe=PE(ROOT/name);data=bytearray(pe.data);masked=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28
        for first,last in masked:data[first:last]=b'\0'*(last-first)
        comparison.append(data);ranges.append(masked)
    assert comparison[0]==comparison[1] and ranges[0]==ranges[1]
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path'])==row['sha256']
    for name,report in reports.items():write(ROOT/'analysis'/name,report)
    files={path.relative_to(ROOT).as_posix():digest(path) for path in (ROOT/'src/squall').rglob('*') if path.is_file()}
    result=dict(status='passed_scoped_mpeg_instruction_reconstruction_both_builds_full_game_regression_pending',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),candidate_sha256=hashes,production_source_files=files,reports={name:digest(ROOT/'analysis'/name) for name in reports},native_instructions_sha256=digest(instruction),kernel_cases_per_build=40960,guarded_mpeg_reads_per_build=640,exact_returned_pcm_bytes_per_build=4421736,actual_safe_physical_buffers_per_build=56,exact_physical_pcm_bytes_per_build=1690416,
        initial_observation_report='analysis/squall-mpeg-initial-observations.json',initial_pcm_failures=13,initial_seek_failures=1,
        known_native_unsafe_ranges='MPEG2 Layer2: constructor retains11520 samples after reducing table to10 frames of576; tail seek reads index16; 577-frame stereo request writes4608 bytes while returning2308. Both reproduced with owned guards/wide backing. Actual unsafe DirectSound playback is not claimed verified.',
        prototype_failures=['First IMDCT18 transcription omitted six-byte-column instructions; failed decoder run and snapshots preserved at .local/squall-mpeg-formats/20261102T211412777467Z','First revised MPEG2 range run still invoked unaligned direct writes and failed with0xc0000374 in both precision processes; preserved at .local/squall-mpeg-formats/20261103T204418133713Z'],
        fresh_build_comparison=dict(bytes=len(comparison[0]),only_ignored_timestamp_ranges=ranges[0],files_modified=False),original_files_unchanged=17,original_program_binaries_used_in_implementation=False,whole_game_complete=False,whole_squall_complete=False,
        pending=['Full current game/audio/102-test regression and current whole-game restores','Additional MPEG layer1,metadata/corruption and all remaining Squall/WMA/tracker behavior','Later game timeline,remaining JNG and uninstaller scope; full original objective remains active'])
    write(ROOT/'analysis/squall-mpeg-scoped-reconstruction.json',result)
    print('Recorded both exact MPEG reconstructions and separately proved original unsafe ranges; full regression pending.')
if __name__=='__main__':main()
