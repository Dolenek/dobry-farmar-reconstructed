#!/usr/bin/env python3
"""Record the ordered listener-factor repair after current binary regressions."""
import datetime,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from pe import PE
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text(encoding='utf-8'))
def write(path,value):Path(path).write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def checked(report):
    assert report['status'].startswith('passed_for_'),report['status']
    manifest=report['source_files'];rows=manifest.items() if isinstance(manifest,dict) else [(r['path'],r['sha256']) for r in manifest]
    for name,value in rows:assert digest(ROOT/name)==value,name
    assert digest(ROOT/report['candidate_path'])==report.get('rebuilt_sha256',report.get('candidate_sha256'))
    assert digest(ROOT/report['oracle_path'])==report['oracle_sha256']
def main():
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    reports={}
    for prefix in ['squall-listener-factors','squall-dll','squall-groups','squall-channel-ranges','game-audio','person-sounds']:
        for suffix in ['', '-fresh']:
            name=f'{prefix}{suffix}-validation.json';report=read(ROOT/'.local'/name);checked(report);reports[name]=report
    hashes=[reports[f'squall-listener-factors{suffix}-validation.json']['rebuilt_sha256'] for suffix in ['', '-fresh']]
    for index,suffix in enumerate(['','-fresh']):
        abi=read(ROOT/'.local'/f'squall-listener{suffix}-abi.json')
        assert abi['status']=='matches_export_contract' and abi['rebuilt_sha256']==hashes[index] and len(abi['exports'])==97 and not abi['failures']
        write(ROOT/'analysis'/f'squall-listener{suffix}-abi.json',abi)
    for name,report in reports.items():
        assert report.get('rebuilt_sha256',report.get('candidate_sha256'))==hashes[int('-fresh-' in name)],name
        if name.startswith('squall-listener-factors'):
            assert report['result_comparisons']==24962 and report['before_nan_mismatches']==6144 and report['initialized_getter_checks']==6240
    normalized=[];masks=[]
    for suffix in ['', '-fresh']:
        pe=PE(ROOT/reports[f'squall-listener-factors{suffix}-validation.json']['candidate_path'])
        mask=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28 and pe.u32(mask[0][0])==pe.u32(mask[1][0])
        payload=bytearray(pe.data)
        for first,last in mask:payload[first:last]=b'\0'*(last-first)
        normalized.append(payload);masks.append(mask)
    assert masks[0]==masks[1] and normalized[0]==normalized[1]
    junit=ROOT/'.local/squall-listener-ctest.xml';suite=ET.parse(junit).getroot();cases=suite.findall('.//testcase')
    assert len(cases)==102 and not any(suite.findall('.//'+n) for n in ['failure','error','skipped'])
    for name,report in reports.items():write(ROOT/'analysis'/name,report)
    audit=read(ROOT/'analysis/squall-export-audit.json');audit['rebuilt_sha256']=hashes[0]
    for row in audit['exports']:
        if row['name'] in {'SQUALL_Listener_'+action+factor+'Factor' for action in ['Set','Get'] for factor in ['Distance','Rolloff','Doppler']}:
            row['status']='partially_compared';row['ordered_factor_verified_scope']='PC53/PC24 before Init, actual DirectSound listener, after Free;1040 input bit patterns per setter/lifecycle, cached factors and output guards; NaNs rejected with -4'
            assert not row['native_signature_fully_verified']
    audit['listener_factor_reports']=['analysis/squall-listener-factors-validation.json','analysis/squall-listener-factors-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');patches=provenance.setdefault('instruction_backed_patches',[])
    method='SQUALL_Listener_SetDistanceFactor/SetRolloffFactor/SetDopplerFactor'
    patches[:]=[p for p in patches if p.get('method')!=method]
    patches.append(dict(path='SoundEngine/Squall.cpp',method=method,original_addresses=['10004600','10004750','100048a0'],
        instruction_evidence='Each original setter accepts only ordered0<factor<=10; unordered compares take invalid-parameter return-4 before listener access',
        source_difference='Public1.48 disjunction factor<=0||factor>10 accepts NaNs; use negation of the ordered acceptance predicate',
        current_sha256=digest(ROOT/'src/squall/runtime/SoundEngine/Squall.cpp')))
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    result=dict(status='passed_for_ordered_listener_factor_fix_and_current_regression_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),dll_sha256=hashes,full_ctest_passed=102,full_ctest_failed=0,junit_sha256=digest(junit),
        ctest_seconds=sum(float(c.get('time','0')) for c in cases),reports={name:digest(ROOT/'analysis'/name) for name in reports},
        abi_reports={f'squall-listener{suffix}-abi.json':digest(ROOT/'analysis'/f'squall-listener{suffix}-abi.json') for suffix in ['', '-fresh']},
        fresh_build_comparison=dict(bytes=len(normalized[0]),only_normalized_byte_ranges=masks[0],normalized_sha256=hashlib.sha256(normalized[0]).hexdigest(),files_modified=False),
        original_files_unchanged=17,whole_game_complete=False)
    write(ROOT/'analysis/squall-listener-regression.json',result)
    verification=read(ROOT/'analysis/verification.json');verification['tests']['ctest_passed']=102
    verification['tests']['ctest_current_checks_covered_across_full_and_retry']=102
    verification['tests']['ctest_focused_retry_repetitions']=0
    verification['full_game_build_available_scope']='Completion flag for all required behavior, every Squall export and all game systems; not the availability of the already buildable main source EXE'
    verification['historical_embedded_sections_note']='Older embedded stage objects retain their own recorded_at_utc and hashes; latest_scoped_regression and its report manifests identify the current verified binaries'
    verification['latest_scoped_regression']=dict(report='analysis/squall-listener-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    for name in list(reports)+list(result['abi_reports'])+['squall-listener-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded ordered listener factors, two current DLL builds and102 clean regression checks; full reconstruction remains incomplete')
if __name__=='__main__':main()
