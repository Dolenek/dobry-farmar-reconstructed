#!/usr/bin/env python3
"""Prove six original ABI argument/return roles from verified instruction bytes."""
import datetime,hashlib
from pathlib import Path
from pe import PE
from audit_squall_native_arguments import instructions,Audit
from record_live_production_verification import ROOT,digest,read,write

FACTORS=[('DistanceFactor',0x10004600,0x100046c0,0x1007510c,0x2c),
         ('RolloffFactor',0x10004750,0x10004810,0x10075110,0x3c),
         ('DopplerFactor',0x100048a0,0x10004960,0x10075114,0x30)]

def main():
    native=ROOT.parent/'farmář/Dobrý farmář/Squall.dll';pe=PE(native)
    disassembly=ROOT/'analysis/squall-exports-native-instructions.txt';ins=instructions(disassembly,pe);audit=Audit(pe,ins)
    imports={pe.image_base+s['iat_rva']:s.get('name') for dll in pe.imports() for s in dll['symbols']}
    assert imports[0x10048030]=='IsBadWritePtr'
    header=(ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf-8')
    def check(address,op,args=None):
        row=ins[address];assert row['file_verified'] and row['op']==op,(hex(address),row)
        if args is not None:assert row['args']==args,(hex(address),row,args)
        return dict(address=f'{address:08x}',instruction_bytes=row['raw'].hex(),op=op,operands=row['args'])
    rows=[]
    for factor,setter,getter,cache,method in FACTORS:
        for kind,base in [('Set',setter),('Get',getter)]:
            name='SQUALL_Listener_'+kind+factor;body=audit.function(base)
            assert body['explicit_argument_slots']==[1] and body['referenced_argument_slots']==1
            evidence=[check(base,'push','esi')]
            if kind=='Set':
                expected=f'DLL_API int {name}(float {factor});';assert expected in header
                evidence += [check(base+0x1c,'fld','dword ptr [esp+0Ch]'),check(base+0x30,'fld','dword ptr [esp+8]'),
                    check(base+0x81,'fcomp','dword ptr [esp+8]'),check(base+0x8c,'mov','eax,dword ptr [esp+8]'),
                    check(base+0x90,'mov',f'dword ptr ds:[{cache:08X}h],eax'),check(base+0x9c,'push','eax'),
                    check(base+0x9d,'mov','eax,dword ptr [esp+0Ch]'),check(base+0xa1,'push','eax'),check(base+0xa2,'push','ecx'),
                    check(base+0xa3,'call',f'dword ptr [edx+{method:X}h]')]
                returns=[0x56,0x7a,0xbf];status_offsets=[1,0x46,0x6a,0xaa];status_values=[1,0xfffffffc,0xffffffe8,0xfffffff8]
                for offset,value in zip(status_offsets,status_values):
                    evidence.append(check(base+offset,'mov',f'esi,0{value:X}h' if value>9 else f'esi,{value}'))
                evidence.append(check(base+0xf,'or','esi,0FFFFFFFFh'))
                for offset in returns:
                    evidence.extend([check(base+offset-3,'mov','eax,esi'),check(base+offset-1,'pop','esi'),check(base+offset,'ret','')])
                role=dict(argument='IEEE754 binary32 value in one4-byte stack slot',pointer=False,
                    independent_type_evidence='Native FLD m32/FComp m32 on the actual argument and cached bits;not inferred from the source header',
                    output_or_cache_bytes=4,cache_address=f'{cache:08x}',directsound_vtable_byte_offset=method)
            else:
                expected=f'DLL_API int {name}(float* {factor});';assert expected in header
                evidence += [check(base+0x24,'mov','edi,dword ptr [esp+0Ch]'),check(base+0x28,'push','4'),check(base+0x2a,'push','edi'),
                    check(base+0x2b,'call','dword ptr ds:[10048030h]'),check(base+0x6c,'mov',f'eax,dword ptr ds:[{cache:08X}h]'),
                    check(base+0x71,'mov','dword ptr [edi],eax')]
                returns=[0x4b,0x6b,0x84];status_offsets=[2,0x3a,0x5a];status_values=[1,0xfffffffc,0xffffffe8]
                for offset,value in zip(status_offsets,status_values):
                    evidence.append(check(base+offset,'mov',f'esi,0{value:X}h' if value>9 else f'esi,{value}'))
                evidence.append(check(base+0x10,'or','esi,0FFFFFFFFh'))
                for offset in returns:
                    evidence.extend([check(base+offset-4,'mov','eax,esi'),check(base+offset-2,'pop','edi'),check(base+offset-1,'pop','esi'),check(base+offset,'ret','')])
                role=dict(argument='Writable pointer to one IEEE754 binary32 value',pointer=True,
                    independent_type_evidence='Actual IsBadWritePtr(argument,4),one DWORD write,and same cache proven binary32 by paired setter FLD/FComp',
                    output_or_cache_bytes=4,cache_address=f'{cache:08x}')
            assert set(body['balanced_return_addresses'])=={f'{base+offset:08x}' for offset in returns}
            # Every in-body ESI assignment is enumerated;all return sites copy
            # the complete32-bit status into EAX before restoring caller ESI.
            end=base+(0xc0 if kind=='Set' else 0x85)
            writes=[a for a,r in ins.items() if base<=a<end and r['args'].startswith('esi,') and r['op'] in ['mov','or']]
            assert set(writes)=={base+x for x in status_offsets}|{base+(0xf if kind=='Set' else 0x10)}
            rows.append(dict(name=name,original_address=f'{base:08x}',calling_convention='cdecl',argument_slots=1,argument_role=role,
                return_role='signed32 status in EAX;success1 or enumerated negative errors,no floating return',
                return_status_bits=[f'{v:08x}' for v in status_values+[0xffffffff]],all_native_return_sites=body['balanced_return_addresses'],
                instruction_evidence=evidence,native_signature_fully_verified=True,whole_behavior_fully_verified=False))
    old=read(ROOT/'analysis/squall-listener-factors-validation.json');assert old['result_comparisons']==24962 and old['initialized_getter_checks']==6240
    fresh=read(ROOT/'analysis/squall-listener-factors-fresh-validation.json');assert fresh['result_comparisons']==24962
    abi=[read(ROOT/'analysis'/name) for name in ['squall-tracker-loaders-abi.json','squall-tracker-loaders-fresh-abi.json']]
    assert all(v['status']=='matches_export_contract' for v in abi)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json');assert [v['rebuilt_sha256'] for v in abi]==full['candidate_sha256']
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
    result=dict(status='passed_six_native_listener_factor_argument_and_return_abi_semantics_only',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        auditor_sha256=digest(__file__),original_sha256=digest(native),disassembly_sha256=digest(disassembly),exports=rows,
        current_source_header_sha256=digest(ROOT/'src/squall/runtime/Squall.h'),
        dependency_source_hashes={name:digest(ROOT/name) for name in ['tools/pe.py','tools/audit_squall_native_arguments.py','tools/record_live_production_verification.py']},
        current_candidate_sha256=full['candidate_sha256'],prior_behavioral_evidence=['analysis/squall-listener-factors-validation.json','analysis/squall-listener-factors-fresh-validation.json'],
        prior_behavioral_report_sha256=[digest(ROOT/'analysis'/n) for n in ['squall-listener-factors-validation.json','squall-listener-factors-fresh-validation.json']],
        new_runtime_audio_test_claimed=False,all97_native_signatures_verified=False,production_changed=False,original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,
        pending=['Remaining91 native signature semantic proofs','Broader runtime behavior and remaining original goal scopes'])
    write(ROOT/'analysis/squall-listener-factor-signatures.json',result)
    exports=read(ROOT/'analysis/squall-export-audit.json');names={r['name']:r for r in rows}
    for row in exports['exports']:
        if row['name'] in names:
            row['native_signature_fully_verified']=True
            row['native_signature_semantic_evidence']=dict(report='analysis/squall-listener-factor-signatures.json',argument=names[row['name']]['argument_role'],return_role=names[row['name']]['return_role'],whole_behavior_verified=False)
    exports['rebuilt_sha256']=abi[0]['rebuilt_sha256'];exports['current_contract_reports']=['analysis/squall-tracker-loaders-abi.json','analysis/squall-tracker-loaders-fresh-abi.json']
    exports['native_signature_semantics_verified_count']=sum(bool(row.get('native_signature_fully_verified')) for row in exports['exports'])
    assert exports['native_signature_semantics_verified_count']>=6
    write(ROOT/'analysis/squall-export-audit.json',exports)
    verification=read(ROOT/'analysis/verification.json');report_name='analysis/squall-listener-factor-signatures.json'
    if report_name not in verification['reports']:verification['reports'].append(report_name)
    verification['native_squall_signature_semantics']=dict(report=report_name,verified_export_count=exports['native_signature_semantics_verified_count'],remaining_export_count=97-exports['native_signature_semantics_verified_count'],whole_behavior_verified=False)
    verification['latest_scoped_regression']=dict(report=report_name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    hashes=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(hashes).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']=report_name
    write(ROOT/'analysis/verification.json',verification)
    print('Proved6 native ABI signatures and18 complete EAX/caller-cleanup returns;91 semantic signatures remain open.')

if __name__=='__main__':main()
