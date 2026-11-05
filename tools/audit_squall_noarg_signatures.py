#!/usr/bin/env python3
"""Prove no caller arguments and complete initialized integer EAX returns."""
from pathlib import Path
from pe import PE
from audit_squall_native_arguments import instructions,Audit
from record_live_production_verification import ROOT,digest
from squall_signature_evidence import publish

METHODS=[('Stop',0x10003690,'ebp'),('GetDevice',0x10003770,'esi'),('GetSpeakerMode',0x10003970,'esi'),
 ('Get3DAlgorithm',0x10003af0,'esi'),('GetBufferSize',0x10003b90,'esi'),('GetNumDevice',0x10003c70,'eax'),
 ('GetEAXVersion',0x10003e00,'esi'),('Listener_Update',0x100049f0,'esi')]

def main():
    pe=PE(ROOT.parent/'farmář/Dobrý farmář/Squall.dll');path=ROOT/'analysis/squall-exports-native-instructions.txt'
    ins=instructions(path,pe);audit=Audit(pe,ins);header=(ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf-8');rows=[]
    for suffix,start,status in METHODS:
        name='SQUALL_'+suffix;assert f'DLL_API int {name}(void);' in header
        stack=audit.function(start);assert stack['explicit_argument_slots']==[] and stack['referenced_argument_slots']==0
        # Definite assignment on every branch,including the speaker jump table
        # and GetNumDevice's tail jump. Calls invalidate volatile EAX;the
        # Win32 x86 ABI preserves the nonvolatile ESI/EBP status register.
        seen=set();todo=[(start,False,False)];returns=set();evidence={}
        while todo:
            address,status_defined,eax_integer=todo.pop();state=(address,status_defined,eax_integer)
            if state in seen:continue
            seen.add(state);row=ins[address];assert row['file_verified'];op,args=row['op'],row['args'];parts=[p.strip() for p in args.split(',')]
            if op=='call':eax_integer=False
            elif op=='pop' and args==status:status_defined=False
            elif op=='mov' and parts[0] in [status,'eax']:
                dest,source=parts
                if source==status and status!='eax':defined=status_defined
                elif source=='eax':defined=eax_integer
                elif source.startswith('dword ptr') or source[0].isdigit():defined=True
                else:raise AssertionError(('Unknown integer return producer',name,hex(address),args))
                if dest==status:status_defined=defined
                if dest=='eax':eax_integer=defined
                evidence[address]=dict(address=f'{address:08x}',instruction_bytes=row['raw'].hex(),op=op,operands=args)
            elif op=='or' and parts[0]==status and parts[1]=='0FFFFFFFFh':
                status_defined=True;evidence[address]=dict(address=f'{address:08x}',instruction_bytes=row['raw'].hex(),op=op,operands=args)
            elif op in ['xor','sub'] and parts==[status,status]:status_defined=True
            elif op in ['and','or','xor','add','sub','shl','shr','sar','inc','dec','neg','not'] and parts[0]==status:
                assert status_defined,(name,hex(address),args)
            if status=='eax':
                # GetNumDevice has no nonvolatile accumulator;the only final
                # producer is the complete DWORD load of its device count.
                if op=='call':status_defined=False
                if op=='mov' and parts[0]=='eax':status_defined=eax_integer
            if op=='ret':
                assert not args and row['raw']==b'\xc3' and eax_integer,(name,hex(address),state)
                returns.add(f'{address:08x}');evidence[address]=dict(address=f'{address:08x}',instruction_bytes='c3',op='ret',operands='')
            else:todo += [(n,status_defined,eax_integer) for n in audit.successors(address)]
        assert returns==set(stack['balanced_return_addresses'])
        rows.append(dict(name=name,original_address=f'{start:08x}',calling_convention='cdecl',argument_slots=0,argument_role=dict(argument='none',independent_type_evidence='All reachable original ESP/EBP paths have no caller stack argument references'),
            return_role='Complete initialized32-bit integer/status in EAX on every reachable C3 return;no floating return',
            all_native_return_sites=sorted(returns),definite_assignment_states=len(seen),instruction_evidence=[evidence[a] for a in sorted(evidence)],
            native_signature_fully_verified=True,whole_behavior_fully_verified=False))
    assert sum(len(row['all_native_return_sites']) for row in rows)==13
    count=publish('squall-noarg-signatures.json',rows,__file__,dict(disassembly_sha256=digest(path),native_returns=13,
        tail_jump_verified='GetNumDevice10003C70 to10007E40;actual DWORD count load10007E45 and C3 at10007E4A',
        switch_table_verified='GetSpeakerMode resolves all five10003A58 targets and error path',
        dataflow_assumption='Standard Win32 x86 nonvolatile ESI/EBP preservation across native direct/COM/Win32 calls',
        scope='Native machine-ABI-compatible declaration roles;behavioral coverage and arbitrary state remain separate'))
    print(f'Proved8 no-argument native integer signatures and13 return sites;{count}/97 ABI semantic signatures now verified.')

if __name__=='__main__':main()
