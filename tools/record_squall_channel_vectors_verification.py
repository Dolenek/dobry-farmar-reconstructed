#!/usr/bin/env python3
"""Record instruction-backed channel C3/raw-copy repair with whole-game regression evidence."""
import datetime,hashlib,json,platform,re,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from pe import PE
from record_squall_listener_verification import checked
from record_live_production_verification import checked as checked_game
ROOT=Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text(encoding='utf-8'))
def write(p,v):Path(p).write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def main():
    reports={};hashes=[]
    for prefix in ['squall-channel-vectors','squall-channel-vectors-dll','squall-channel-vectors-game-audio','squall-channel-vectors-person-sounds']:
        for suffix in ['','-fresh']:
            name=f'{prefix}{suffix}-validation.json';report=read(ROOT/'.local'/name);checked(report)
            if prefix=='squall-channel-vectors':
                assert report['status']=='passed_for_actual_positioned_channels_c3_raw_copies_and_selected_bounds_only'
                assert len(report['precision_contexts'])==len(report['method_fixture_contexts'])==2
                assert all(row['api_comparisons']==13275 and row['snapshots']==2636 for row in report['precision_contexts'])
                assert all(row['setter_fixtures']==2048 and row['getter_fixtures']==16384 for row in report['method_fixture_contexts'])
                assert digest(ROOT/report['linker_map_path'])==report['linker_map_sha256']
                hashes.append(report['rebuilt_sha256'])
            if 'work_directory' in report and 'logs' in report:
                for log,value in report['logs'].items():assert digest(ROOT/report['work_directory']/log)==value
            reports[name]=report
    assert len(hashes)==2
    for name,report in reports.items():assert report.get('rebuilt_sha256',report.get('candidate_sha256'))==hashes[int('-fresh-' in name)],name
    for label,expected in [('before','c3'),('c3-only','signaling-copy')]:
        name=f'squall-channel-vectors-{label}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='confirmed_preserved_own_dll_difference_in_both_precisions' and report['baseline_difference']==expected
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256']
        assert all(row['expected_difference_observed'] for row in report['precision_contexts'])
        for log,value in report['logs'].items():assert digest(ROOT/report['work_directory']/log)==value
        reports[name]=report
    abi={}
    normalized=[];masks=[]
    for index,suffix in enumerate(['','-fresh']):
        name=f'squall-channel-vectors{suffix}-abi.json';value=read(ROOT/'.local'/name)
        assert value['status']=='matches_export_contract' and value['rebuilt_sha256']==hashes[index] and len(value['exports'])==97
        abi[name]=value
        pe=PE(ROOT/reports[f'squall-channel-vectors{suffix}-validation.json']['candidate_path'])
        mask=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28 and pe.u32(mask[0][0])==pe.u32(mask[1][0]);payload=bytearray(pe.data)
        for first,last in mask:payload[first:last]=b'\0'*(last-first)
        normalized.append(payload);masks.append(mask)
    assert masks[0]==masks[1] and normalized[0]==normalized[1]
    game=read(ROOT/'.local/squall-channel-vectors-game-interop-validation.json');game_work=checked_game(game)
    assert game['status']=='passed_for_seven_simultaneous_buildings_bidirectional_saves_only' and game['implementation_files']['Squall.dll']==hashes[0]
    assert len(game['sessions'])==len(game['comparison']['transfer_comparisons'])==4
    assert game['fixture_report_sha256']==digest(ROOT/'.local/seven-factories-state-validation.json')
    assert digest(ROOT/'.local/seven-factories-state-validation.json')==digest(ROOT/'analysis/seven-factories-state-validation.json')
    for row in game['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4089 and not row['normalizations']
    traces={}
    for path in game_work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    junit_candidates=[ROOT/'build/windows/.local-squall-channel-vectors-ctest.xml',ROOT/'.local-squall-channel-vectors-ctest.xml']
    junit=next(p for p in junit_candidates if p.is_file());suite=ET.parse(junit).getroot();cases=suite.findall('.//testcase')
    assert len(cases)==102 and not any(suite.findall('.//'+n) for n in ['failure','error','skipped'])
    log=ROOT/'.local/squall-channel-vectors-ctest.log';text=log.read_text(encoding='utf-8-sig');assert re.search(r'100% tests passed(?:, 0 tests failed)? out of 102',text)
    elapsed=float(re.search(r'Total Test time \(real\) =\s*([\d.]+) sec',text)[1])
    focused=ROOT/'build/windows/Testing/Temporary/LastTest.log';focused_bytes=focused.read_bytes();focused_encoding='utf-8-sig'
    try:focused_text=focused_bytes.decode(focused_encoding)
    except UnicodeDecodeError:focused_encoding='cp1250';focused_text=focused_bytes.decode(focused_encoding)
    assert 'Testing: reconstruction-tools' in focused_text and focused_text.count('Test Passed.')==1
    (ROOT/'.local/squall-channel-vectors-tools-focused.log').write_bytes(focused_bytes)
    instructions=ROOT/'.local/squall-channel-vectors-method-instructions.txt';instruction_text=instructions.read_text(encoding='utf-8')
    for address in ['10001A70','10001A91','10001B13','10001B93','10001BC1','10001C10','10006570','10006AA0']:assert address+':' in instruction_text,address
    (ROOT/'analysis/squall-channel-vectors-native-instructions.txt').write_text(instruction_text,encoding='utf-8')
    for name,value in {**reports,**abi,'squall-channel-vectors-game-interop-validation.json':game}.items():write(ROOT/'analysis'/name,value)
    pcm_initial=read(ROOT/'.local/squall-channel-vectors-dll-initial-PC53.stdout.json')
    assert pcm_initial['devices']==2 and pcm_initial['api_comparisons']==297 and pcm_initial['dll_pcm_bytes']==61263876
    initial=dict(status='initial_backend_creation_and_device_count_checker_findings_preserved',
        algorithm3=dict(source_result=-11,native_result=-11,stderr_sha256=digest(ROOT/'.local/squall-channel-vectors-detail.stderr.log'),exact_backend_hresult_observed=False,recorded_as_successful_3d_channel=False),
        pcm_checker=dict(previous_devices=3,previous_expected_calls=299,current_devices=2,current_actual_calls=297,original_and_source_oracle_comparison_passed=True,
            raw_oracle_result=pcm_initial,stdout_sha256=digest(ROOT/'.local/squall-channel-vectors-dll-initial-PC53.stdout.json'),resolution='Retain293 fixed calls and two guarded name/capability comparisons for each actual endpoint; production code unchanged by this checker fix'),whole_game_complete=False)
    write(ROOT/'analysis/squall-channel-vectors-initial-probes.json',initial)
    audit=read(ROOT/'analysis/squall-export-audit.json');audit['rebuilt_sha256']=hashes[0]
    methods=['Set3DPosition','Get3DPosition','SetVelocity','GetVelocity','SetMinMaxDistance','GetMinMaxDistance','SetConeParameters','GetConeParameters']
    for row in audit['exports']:
        if row['name'] in ['SQUALL_Channel_'+m for m in methods]:
            row['status']='partially_compared';row['loaded_DLL_3d_channel_verified_scope']='PC53/PC24 real prepared mono channels with algorithm1, immediate/deferred listeners;108 vector patterns,19x19 distances,512 cone tuples and guarded pointers per context; C3 and raw sNaN copy repair; separate same-address byte fixtures record COM boundary'
    audit['channel_vector_reports']=['analysis/squall-channel-vectors-validation.json','analysis/squall-channel-vectors-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');patches=provenance.setdefault('instruction_backed_patches',[]);method='SSample::Set3DParameters/Get3DParameters C3 and raw copies'
    patches[:]=[p for p in patches if p.get('method')!=method]
    patches.append(dict(path='SoundEngine/Channels.cpp',method=method,original_addresses=['10001a70','10001c10'],instruction_evidence='Cached-first FLD/FCOMP then TEST AH,40h; unordered/equal skip updates. Vector/scalar getter MOVs retain signaling-NaN bits and original forward word order.',current_sha256=digest(ROOT/'src/squall/runtime/SoundEngine/Channels.cpp')))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    source={p.relative_to(ROOT).as_posix():digest(p) for folder in ['src/game','src/squall','src/jngload','src/uninstall'] for p in sorted((ROOT/folder).rglob('*')) if p.is_file()}
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_channel_3d_c3_raw_copy_fix_current_dlls_complete_ctest_and_whole_game_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        base_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),production_source_files=source,dll_sha256=hashes,reports={n:digest(ROOT/'analysis'/n) for n in list(reports)+list(abi)+['squall-channel-vectors-game-interop-validation.json','squall-channel-vectors-native-instructions.txt','squall-channel-vectors-initial-probes.json']},
        actual_api_comparisons_per_build=26550,actual_guarded_snapshots_per_build=5272,method_setter_fixtures_per_build=4096,method_getter_fixtures_per_build=32768,
        diagnostic_COM_boundary_is_not_actual_audio=True,selected_live_channels_use_actual_DirectSound=True,audio_spatial_waveform_compared=False,
        tests_passed=102,tests_failed=0,ctest_elapsed_seconds=elapsed,sum_test_seconds=sum(float(c.get('time','0')) for c in cases),junit_path=junit.relative_to(ROOT).as_posix(),junit_sha256=digest(junit),ctest_log_sha256=digest(log),tools_focused_after_device_count_checker_fix=dict(tests_passed=1,log_encoding=focused_encoding,original_log_bytes_preserved=True,log_sha256=digest(ROOT/'.local/squall-channel-vectors-tools-focused.log')),
        build_logs={n:digest(ROOT/'.local'/n) for n in ['squall-channel-vectors-full-build.log','squall-channel-vectors-dependent-build.log','squall-channel-vectors-fresh-build.log','squall-channel-vectors-build.log']},
        fresh_build_comparison=dict(bytes=len(normalized[0]),only_normalized_byte_ranges=masks[0],normalized_sha256=hashlib.sha256(normalized[0]).hexdigest(),files_modified=False),
        implementation_files=game['implementation_files'],source_restore_debugger_traces=traces,whole_game_bidirectional_transfers=4,complete_identical_save_bytes=4089,
        original_program_code_used_in_implementation=False,original_files_unchanged=17,validation_system=platform.platform(),whole_game_complete=False,
        pending=['All remaining97-export signatures and behaviors, running3D audio, unsupported hardware/EAX/ZoomFX and general codecs','Other crops, natural awards and engine boundaries','General JNG and remaining isolated uninstaller scopes'])
    write(ROOT/'analysis/squall-channel-vectors-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(result['reports'])+['squall-channel-vectors-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-channel-vectors-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests'].update(ctest_passed=102,last_full_ctest_report='analysis/squall-channel-vectors-regression.json')
    if 'analysis/squall-channel-vectors-game-interop-validation.json' not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/squall-channel-vectors-game-interop-validation.json')
    write(ROOT/'analysis/verification.json',verification);print('Recorded actual3D C3/raw-copy repair, two DLL builds,102/102 current CTest and four4089-byte whole-game transfers; whole goal continues')
if __name__=='__main__':main()
