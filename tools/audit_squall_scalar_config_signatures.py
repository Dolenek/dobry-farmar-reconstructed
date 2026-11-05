#!/usr/bin/env python3
"""Prove four signed scalar configuration argument roles and integer returns."""
import re
from pe import PE
from audit_squall_native_arguments import instructions,Audit
from record_live_production_verification import ROOT,digest
from squall_signature_evidence import publish

METHODS=[('SetDevice',0x10003700,[(0x10003724,'mov','edi,dword ptr [esp+0Ch]'),(0x10003728,'test','edi,edi'),(0x1000372a,'jl','10003735'),(0x10003731,'cmp','edi,eax'),(0x10003733,'jl','1000374C')],
 'Signed32 device index;negative rejected and index must be below actual device count'),
 ('SetSpeakerMode',0x100038b0,[(0x100038d3,'mov','ecx,dword ptr [esp+8]'),(0x100038d7,'test','ecx,ecx'),(0x100038d9,'jl','100038E0'),(0x100038db,'cmp','ecx,6'),(0x100038de,'jle','100038F6'),(0x100038f8,'dec','ecx'),(0x100038f9,'cmp','ecx,4'),(0x100038fe,'jmp','dword ptr [ecx*4+10003954h]')],
 'Signed32 speaker-mode integer;bounds0..6 then actual five-target integer switch plus default'),
 ('Set3DAlgorithm',0x10003a70,[(0x10003a93,'mov','eax,dword ptr [esp+8]'),(0x10003a97,'test','eax,eax'),(0x10003a99,'jl','10003AA0'),(0x10003a9b,'cmp','eax,3'),(0x10003a9e,'jle','10003AB6'),(0x10003ab6,'mov','ecx,dword ptr [eax*4+10075148h]')],
 'Signed32 algorithm index;bounds0..3 then actual scaled integer support-table index'),
 ('SetBufferSize',0x10003b30,[(0x10003b4c,'mov','eax,dword ptr [esp+0Ch]'),(0x10003b53,'test','eax,eax'),(0x10003b55,'jle','10003B5E'),(0x10003b57,'cmp','eax,1388h'),(0x10003b5c,'jle','10003B74'),(0x10003b78,'mov','dword ptr ds:[10075138h],eax')],
 'Signed32 buffer milliseconds;reject <=0 and >5000;store the complete DWORD value')]

def main():
    pe=PE(ROOT.parent/'farmář/Dobrý farmář/Squall.dll');path=ROOT/'analysis/squall-exports-native-instructions.txt';ins=instructions(path,pe);audit=Audit(pe,ins)
    header=(ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf-8');rows=[]
    for suffix,start,witnesses,role in METHODS:
        name='SQUALL_'+suffix;assert re.search(r'DLL_API int '+name+r'\(int \w+\);',header)
        stack=audit.function(start);assert stack['explicit_argument_slots']==[1] and stack['referenced_argument_slots']==1
        evidence={}
        for address,op,args in witnesses:
            r=ins[address];assert r['file_verified'] and r['op']==op and r['args']==args,(name,hex(address),r)
            evidence[address]=dict(address=f'{address:08x}',instruction_bytes=r['raw'].hex(),op=op,operands=args)
        seen=set();todo=[(start,False,False)];returns=set();status_values=set()
        while todo:
            address,initialized,eax_status=todo.pop();state=(address,initialized,eax_status)
            if state in seen:continue
            seen.add(state);r=ins[address];assert r['file_verified'];op,args=r['op'],r['args'];parts=[p.strip() for p in args.split(',')]
            if op=='call':eax_status=False
            elif op=='mov' and parts[0]=='esi':
                value=parts[1];assert value[0].isdigit(),(name,hex(address),args)
                status_values.add(value);initialized=True;evidence[address]=dict(address=f'{address:08x}',instruction_bytes=r['raw'].hex(),op=op,operands=args)
            elif op=='or' and parts==['esi','0FFFFFFFFh']:
                initialized=True;status_values.add('0FFFFFFFFh');evidence[address]=dict(address=f'{address:08x}',instruction_bytes=r['raw'].hex(),op=op,operands=args)
            elif op=='pop' and args=='esi':initialized=False
            elif op=='mov' and parts[0]=='eax':
                eax_status=parts[1]=='esi' and initialized
                if parts[1]=='esi':evidence[address]=dict(address=f'{address:08x}',instruction_bytes=r['raw'].hex(),op=op,operands=args)
            elif parts[0]=='esi' and op not in ['test','cmp','push']:raise AssertionError(('Unenumerated status write',name,hex(address),r))
            if op=='ret':
                assert not args and r['raw']==b'\xc3' and eax_status,(name,hex(address),state)
                returns.add(f'{address:08x}');evidence[address]=dict(address=f'{address:08x}',instruction_bytes='c3',op='ret',operands='')
            else:todo += [(n,initialized,eax_status) for n in audit.successors(address)]
        assert returns==set(stack['balanced_return_addresses'])
        rows.append(dict(name=name,original_address=f'{start:08x}',calling_convention='cdecl',argument_slots=1,
            argument_role=dict(argument=role,pointer=False,bytes=4,independent_type_evidence='Actual DWORD argument load,signed conditional bounds and integer index/store operations'),
            return_role='Complete initialized32-bit integer status in EAX from enumerated ESI values on all C3 returns',
            all_native_return_sites=sorted(returns),native_status_immediates=sorted(status_values),definite_assignment_states=len(seen),
            instruction_evidence=[evidence[a] for a in sorted(evidence)],native_signature_fully_verified=True,whole_behavior_fully_verified=False))
    assert sum(len(r['all_native_return_sites']) for r in rows)==9
    count=publish('squall-scalar-config-signatures.json',rows,__file__,dict(disassembly_sha256=digest(path),native_returns=9,
        behavior_reports=['analysis/squall-config-validation.json','analysis/squall-config-fresh-validation.json'],
        behavior_report_hashes=[digest(ROOT/'analysis'/p) for p in ['squall-config-validation.json','squall-config-fresh-validation.json']],
        native_algorithm_support_table_mutation_preserved=True,new_runtime_audio_test_claimed=False,
        dataflow_assumption='Standard Win32 x86 nonvolatile ESI preservation across native direct/COM/Win32 calls'))
    print(f'Proved4 signed scalar native signatures and9 return sites;{count}/97 ABI semantic signatures now verified.')

if __name__=='__main__':main()
