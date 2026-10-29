#!/usr/bin/env python3
"""Record the instruction-backed C3 repair and precisely scoped current regression."""
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
    for prefix in ['squall-listener-vectors','squall-vectors-factor','squall-vectors-game-audio','squall-vectors-person-sounds']:
        for suffix in ['', '-fresh']:
            name=f'{prefix}{suffix}-validation.json';report=read(ROOT/'.local'/name);checked(report);reports[name]=report
    hashes=[reports[f'squall-listener-vectors{suffix}-validation.json']['rebuilt_sha256'] for suffix in ['', '-fresh']]
    for name,report in reports.items():
        assert report.get('rebuilt_sha256',report.get('candidate_sha256'))==hashes[int('-fresh-' in name)],name
        if name.startswith('squall-listener-vectors'):assert report['result_comparisons']==4556 and report['snapshots']==1776 and report['getter_pointer_cases']==1008
        write(ROOT/'analysis'/name,report)
    normalized=[];masks=[]
    for index,suffix in enumerate(['','-fresh']):
        abi=read(ROOT/'.local'/f'squall-vectors{suffix}-abi.json');assert abi['status']=='matches_export_contract' and abi['rebuilt_sha256']==hashes[index] and len(abi['exports'])==97
        write(ROOT/'analysis'/f'squall-vectors{suffix}-abi.json',abi)
        pe=PE(ROOT/reports[f'squall-listener-vectors{suffix}-validation.json']['candidate_path'])
        mask=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28 and pe.u32(mask[0][0])==pe.u32(mask[1][0])
        payload=bytearray(pe.data)
        for first,last in mask:payload[first:last]=b'\0'*(last-first)
        normalized.append(payload);masks.append(mask)
    assert masks[0]==masks[1] and normalized[0]==normalized[1]
    gui=read(ROOT/'.local/tomato-contract-interop-validation.json');assert gui['status']=='passed_for_natural_tomato_contract_bidirectional_saves_only'
    for name,value in gui['source_files'].items():assert digest(ROOT/name)==value,name
    assert gui['implementation_files']['Squall.dll']==hashes[0] and len(gui['sessions'])==4
    for name,value in gui['implementation_files'].items():assert digest(ROOT/'build/windows/Release'/('Dobrý farmář.exe' if name=='FarmarSourceInterop.exe' else name))==value
    for row in gui['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==3278 and not row['normalizations']
    write(ROOT/'analysis/tomato-contract-interop-validation.json',gui)
    audit=read(ROOT/'analysis/squall-export-audit.json');audit['rebuilt_sha256']=hashes[0]
    names={'SQUALL_Listener_'+n for n in ['SetParameters','GetParameters','SetPosition','GetPosition','SetVelocity','GetVelocity','Update']}
    for row in audit['exports']:
        if row['name'] in names:
            row['status']='partially_compared';row['listener_vector_verified_scope']='PC53/PC24 actual immediate/deferred listener, pre-Init/post-Free,11 triples including NaNs,16 input-presence masks,21 guarded/aliased output arrangements; returns and exact cached words'
            assert not row['native_signature_fully_verified']
    audit['listener_vector_reports']=['analysis/squall-listener-vectors-validation.json','analysis/squall-listener-vectors-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');patches=provenance.setdefault('instruction_backed_patches',[]);method='Listener vector change detection'
    patches[:]=[p for p in patches if p.get('method')!=method]
    patches.append(dict(path='SoundEngine/Squall.cpp',method=method,original_addresses=['10003f80','100042d0','10004470'],
        instruction_evidence='FCOMP followed by FNSTSW AX and TEST AH,40h at all six vector comparisons; equal/unordered C3=1 skips a component change',
        source_difference='Public C++ != treats NaN as changed; new Win32 x87 helper reads input/cache memory words and tests only C3, preserving short-circuit component order'))
    for row in patches:
        if row.get('path')=='SoundEngine/Squall.cpp':row['current_sha256']=digest(ROOT/'src/squall/runtime/SoundEngine/Squall.cpp')
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_for_listener_C3_fix_current_audio_and_four_contract_save_transfers',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),support_recorder_sha256=digest(ROOT/'tools/record_squall_listener_verification.py'),dll_sha256=hashes,
        reports={n:digest(ROOT/'analysis'/n) for n in reports},game_interop_report_sha256=digest(ROOT/'analysis/tomato-contract-interop-validation.json'),
        fresh_build_comparison=dict(bytes=len(normalized[0]),only_normalized_byte_ranges=masks[0],normalized_sha256=hashlib.sha256(normalized[0]).hexdigest(),files_modified=False),
        previous_full_ctest=dict(report='analysis/squall-listener-regression.json',implementation_commit='2745060',passed=102,repeated_for_current_vector_fix=False),
        build_logs={n:digest(ROOT/'.local'/n) for n in ['squall-vectors-main-build.log','squall-vectors-main-audio-build.log','squall-vectors-fresh-audio-build.log']},
        original_files_unchanged=17,whole_game_complete=False)
    write(ROOT/'analysis/squall-vectors-regression.json',result)
    verification=read(ROOT/'analysis/verification.json');verification['latest_scoped_regression']=dict(report='analysis/squall-vectors-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests']['last_full_ctest_report']='analysis/squall-listener-regression.json'
    verification['tests']['last_full_ctest_implementation_commit']='2745060'
    verification['tests']['full_ctest_repeated_for_latest_vector_fix']=False
    for name in list(reports)+['squall-vectors-regression.json','squall-vectors-abi.json','squall-vectors-fresh-abi.json','tomato-contract-state-validation.json','tomato-contract-interop-validation.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded C3 listener vectors, two builds/current audio and four complete3278-byte transfers; no claim of a repeated full102-test run')
if __name__=='__main__':main()
