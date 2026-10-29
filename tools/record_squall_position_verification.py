#!/usr/bin/env python3
"""Record the pointer-position repair only after complete build/audio/game regressions."""
import datetime,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from pe import PE
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(path.read_text(encoding='utf-8'))
def write(path,value):path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def validate_manifest(report):
    manifest=report['source_files']
    rows=manifest.items() if isinstance(manifest,dict) else [(r['path'],r['sha256']) for r in manifest]
    for name,value in rows:assert digest(ROOT/name)==value,name
    if 'candidate_path' in report:assert digest(ROOT/report['candidate_path'])==report.get('rebuilt_sha256',report.get('candidate_sha256'))
    if 'oracle_path' in report:assert digest(ROOT/report['oracle_path'])==report['oracle_sha256']
def main():
    inventory=read(ROOT/'analysis/inventory.json')
    for row in inventory['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    junit=ROOT/'.local/squall-position-ctest.xml';suite=ET.parse(junit).getroot();cases=suite.findall('.//testcase')
    assert len(cases)==102 and not suite.findall('.//skipped')
    failed=[c for c in cases if c.find('failure') is not None or c.find('error') is not None]
    retry=None
    if failed:
        assert len(failed)==1 and failed[0].get('name')=='recovered-game-source-world'
        retry_path=ROOT/'.local/squall-position-world-retry.xml';retry_suite=ET.parse(retry_path).getroot();retry_cases=retry_suite.findall('.//testcase')
        assert len(retry_cases)==1 and retry_cases[0].get('name')=='recovered-game-source-world'
        assert not retry_suite.findall('.//failure') and not retry_suite.findall('.//error') and not retry_suite.findall('.//skipped')
        retry_log=ROOT/'.local/squall-position-world-retry.log';assert retry_log.read_text(encoding='utf-8').count('Passed')==3
        world=read(ROOT/'.local/game-source-world-validation.json');validate_manifest(world)
        assert world['status']=='passed_for_source_game_factory_input_tutorial_and_binary_roundtrip_only'
        assert digest(ROOT/world['test_path'])==world['test_sha256']
        retry=dict(name=retry_cases[0].get('name'),repetitions=3,junit_sha256=digest(retry_path),log_sha256=digest(retry_log),
            source_world_report_sha256=digest(ROOT/'.local/game-source-world-validation.json'),single_fresh_all_passed_run=False)
        write(ROOT/'analysis/squall-position-source-world-validation.json',world)
    reports={}
    prefixes=['squall-channel-ranges','squall-dll','squall-groups','game-audio','person-sounds']
    for prefix in prefixes:
        for suffix in ['', '-fresh']:
            name=f'{prefix}{suffix}-validation.json';report=read(ROOT/'.local'/name)
            assert report['status'].startswith('passed_for_'),name;validate_manifest(report)
            if prefix=='squall-channel-ranges':
                assert report['result_comparisons']==2542 and len(report['bounded_nonreturn_experiments'])==8
                assert all(not e['returned_before_deadline'] and e['deadline_seconds']==4 for e in report['bounded_nonreturn_experiments'])
            write(ROOT/'analysis'/name,report);reports[name]=report
    main_report=reports['squall-channel-ranges-validation.json'];fresh_report=reports['squall-channel-ranges-fresh-validation.json']
    main_hash,fresh_hash=main_report['rebuilt_sha256'],fresh_report['rebuilt_sha256']
    for name,report in reports.items():assert report.get('rebuilt_sha256',report.get('candidate_sha256'))==(fresh_hash if '-fresh-' in name else main_hash),name
    # Two known linker timestamps may change. Compare every remaining byte.
    normalized=[];ranges=[]
    for report in [main_report,fresh_report]:
        pe=PE(ROOT/report['candidate_path']);mask=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28 and pe.u32(mask[0][0])==pe.u32(mask[1][0])
        payload=bytearray(pe.data)
        for first,last in mask:payload[first:last]=b'\0'*(last-first)
        normalized.append(payload);ranges.append(mask)
    assert ranges[0]==ranges[1] and normalized[0]==normalized[1]
    gui=read(ROOT/'.local/squall-position-game-interop-validation.json');assert gui['status']=='passed_for_natural_exhausted_seed_package_bidirectional_saves_only';validate_manifest(gui)
    assert gui['implementation_files']['Squall.dll']==main_hash and len(gui['sessions'])==4
    for name,value in gui['implementation_files'].items():
        filename='Dobrý farmář.exe' if name=='FarmarSourceInterop.exe' else name
        assert digest(ROOT/'build/windows/Release'/filename)==value
    for row in gui['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==2650 and not row['normalizations']
    write(ROOT/'analysis/squall-position-game-interop-validation.json',gui)
    audit=read(ROOT/'analysis/squall-export-audit.json');audit['rebuilt_sha256']=main_hash
    operations=['GetFragment','GetFragmentMs','SetFragment','SetFragmentMs','GetPlayPosition','GetPlayPositionMs','SetPlayPosition','SetPlayPositionMs','GetLoop','SetLoop','GetPriority','SetPriority']
    names={'SQUALL_Channel_'+operation for operation in operations}
    for row in audit['exports']:
        if row['name'] in names:
            row['status']='partially_compared';row['loaded_DLL_channel_ranges_verified_scope']='PC53/PC24 guarded ranges, selected paused seeks, loop/priority boundaries and invalid IDs; detailed independent/equal-address scopes in channel-range reports'
            if row['name'] in ['SQUALL_Channel_GetPlayPosition','SQUALL_Channel_GetPlayPositionMs']:
                row['native_pointer_seed_scope']='10002180 ECX-pushed initial cursor;240 equal-address cases per precision, source pointer artifact explicit; raw independent addresses are not normalized as identical values'
            assert not row['native_signature_fully_verified']
    audit['channel_range_reports']=['analysis/squall-channel-ranges-validation.json','analysis/squall-channel-ranges-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');patches=provenance.setdefault('instruction_backed_patches',[])
    entry=dict(path='SoundEngine/Channels.cpp',method='SSample::GetPlayPosition',original_address='10002180',
        instruction_evidence='push ecx reserves the cursor local initialized to this; PLAY and PAUSE overwrite it, other states retain pointer bits',
        source_difference='Public1.48 declares an uninitialized DWORD; reconstruction now explicitly seeds the channel address',
        current_sha256=digest(ROOT/'src/squall/runtime/SoundEngine/Channels.cpp'))
    patches[:]=[p for p in patches if not (p.get('path')==entry['path'] and p.get('method')==entry['method'])];patches.append(entry);write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    result=dict(status='passed_for_pointer_position_fix_and_recorded_regression_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(Path(__file__)),main_dll_sha256=main_hash,fresh_dll_sha256=fresh_hash,
        fresh_build_comparison=dict(bytes=len(normalized[0]),only_normalized_byte_ranges=ranges[0],normalized_sha256=hashlib.sha256(normalized[0]).hexdigest(),files_modified=False),
        reports={name:digest(ROOT/'analysis'/name) for name in reports},game_interop_report_sha256=digest(ROOT/'analysis/squall-position-game-interop-validation.json'),
        ctest=dict(current_checks_covered=102,full_run_passed=102-len(failed),full_run_failed=len(failed),
            full_run_failed_names=[c.get('name') for c in failed],focused_retry=retry,
            junit_sha256=digest(junit),seconds=sum(float(c.get('time','0')) for c in cases)),
        build_logs={n:digest(ROOT/'.local'/n) for n in ['squall-position-full-build.log','squall-position-audio-build.log','squall-channel-ranges-fresh-build.log']},
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Other3D/listener/callback/decoder/device export behavior and all remaining full-game scopes'])
    write(ROOT/'analysis/squall-position-regression.json',result)
    verification=read(ROOT/'analysis/verification.json');verification['tests']['ctest_passed']=102-len(failed)
    verification['tests']['ctest_current_checks_covered_across_full_and_retry']=102
    verification['tests']['ctest_focused_retry_repetitions']=3 if retry else 0
    verification['latest_scoped_regression']=dict(report='analysis/squall-position-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    for name in list(result['reports'])+['squall-position-regression.json','squall-position-game-interop-validation.json']:
        relative='analysis/'+name
        if relative not in verification['reports']:verification['reports'].append(relative)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded pointer-position fix: two builds,102 regression checks with explicit full/retry results, current audio and four complete game saves; overall reconstruction remains incomplete')
if __name__=='__main__':main()
