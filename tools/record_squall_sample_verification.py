#!/usr/bin/env python3
"""Record the sample distance repair with current DLL and complete-game evidence."""
import datetime,hashlib,json
from pathlib import Path
from pe import PE
from record_squall_listener_verification import checked
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text(encoding='utf-8'))
def write(path,value):Path(path).write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def main():
    reports={}
    for prefix in ['squall-sample-defaults','squall-sample-dll','squall-sample-game-audio','squall-sample-person-sounds']:
        for suffix in ['', '-fresh']:
            name=f'{prefix}{suffix}-validation.json';report=read(ROOT/'.local'/name);checked(report);reports[name]=report
    hashes=[reports[f'squall-sample-defaults{suffix}-validation.json']['rebuilt_sha256'] for suffix in ['', '-fresh']]
    for name,report in reports.items():
        assert report.get('rebuilt_sha256',report.get('candidate_sha256'))==hashes[int('-fresh-' in name)],name
        if name.startswith('squall-sample-defaults'):assert report['result_comparisons']==8710 and report['before_nan_mismatches']==4096 and report['explicit_load_default_cases']==44
        write(ROOT/'analysis'/name,report)
    normalized=[];masks=[]
    for index,suffix in enumerate(['','-fresh']):
        abi=read(ROOT/'.local'/f'squall-sample{suffix}-abi.json');assert abi['status']=='matches_export_contract' and abi['rebuilt_sha256']==hashes[index] and len(abi['exports'])==97
        write(ROOT/'analysis'/f'squall-sample{suffix}-abi.json',abi)
        pe=PE(ROOT/reports[f'squall-sample-defaults{suffix}-validation.json']['candidate_path'])
        mask=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28 and pe.u32(mask[0][0])==pe.u32(mask[1][0]);payload=bytearray(pe.data)
        for first,last in mask:payload[first:last]=b'\0'*(last-first)
        normalized.append(payload);masks.append(mask)
    assert masks[0]==masks[1] and normalized[0]==normalized[1]
    gui=read(ROOT/'.local/cucumber-interop-validation.json');assert gui['status']=='passed_for_natural_eight_crop_bidirectional_saves_only'
    for name,value in gui['source_files'].items():assert digest(ROOT/name)==value,name
    assert gui['implementation_files']['Squall.dll']==hashes[0] and len(gui['sessions'])==4
    for name,value in gui['implementation_files'].items():assert digest(ROOT/'build/windows/Release'/('Dobrý farmář.exe' if name=='FarmarSourceInterop.exe' else name))==value
    for row in gui['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==3557 and not row['normalizations']
    write(ROOT/'analysis/cucumber-interop-validation.json',gui)
    audit=read(ROOT/'analysis/squall-export-audit.json');audit['rebuilt_sha256']=hashes[0]
    for row in audit['exports']:
        if row['name'] in ['SQUALL_Sample_SetDefault','SQUALL_Sample_GetDefault','SQUALL_Sample_LoadFile']:
            row['status']='partially_compared';row['sample_default_verified_scope']='PC53/PC24 actual loaded sample; all7 fields,2130 normalizations per precision, protected pointer cases and selected explicit LoadFile defaults; ordered positive distance required'
            assert not row['native_signature_fully_verified']
    audit['sample_default_reports']=['analysis/squall-sample-defaults-validation.json','analysis/squall-sample-defaults-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');patches=provenance.setdefault('instruction_backed_patches',[]);method='InitSoundDefault distance normalization'
    patches[:]=[p for p in patches if p.get('method')!=method]
    patches.append(dict(path='SoundEngine/SquallApi.cpp',method=method,original_addresses=['10007f20','10007f78','10007f8f'],
        instruction_evidence='FCOMP0, FNSTSW AX, TEST AH,41h; nonpositive/unordered MinDist becomes1 and MaxDist becomes0x4e6e6b28',
        source_difference='Public1.48 <=0 retains NaNs; negate ordered>0 acceptance for both fields',current_sha256=digest(ROOT/'src/squall/runtime/SoundEngine/SquallApi.cpp')))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    full=read(ROOT/'analysis/focused-view-upgrade-full-regression.json');assert full['tests_passed']==102 and full['tests_failed']==0
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_for_sample_distance_NaN_fix_current_audio_and_eight_crop_save_transfers',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),dll_sha256=hashes,reports={n:digest(ROOT/'analysis'/n) for n in reports},game_interop_report_sha256=digest(ROOT/'analysis/cucumber-interop-validation.json'),
        fresh_build_comparison=dict(bytes=len(normalized[0]),only_normalized_byte_ranges=masks[0],normalized_sha256=hashlib.sha256(normalized[0]).hexdigest(),files_modified=False),
        previous_full_ctest=dict(report='analysis/focused-view-upgrade-full-regression.json',implementation_commit=full['implementation_commit'],passed=102,repeated_for_current_sample_fix=False),
        original_files_unchanged=17,whole_game_complete=False)
    write(ROOT/'analysis/squall-sample-regression.json',result)
    verification=read(ROOT/'analysis/verification.json');verification['latest_scoped_regression']=dict(report='analysis/squall-sample-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests']['full_ctest_repeated_for_latest_sample_fix']=False
    for name in list(reports)+['squall-sample-regression.json','squall-sample-abi.json','squall-sample-fresh-abi.json','cucumber-interop-validation.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    if 'analysis/cucumber-interop-validation.json' not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/cucumber-interop-validation.json')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded sample distance normalization, two builds/current audio and four complete3557-byte transfers; reconstruction remains incomplete')
if __name__=='__main__':main()
