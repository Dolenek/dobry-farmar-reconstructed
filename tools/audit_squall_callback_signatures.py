#!/usr/bin/env python3
"""Prove public callback order, reader invocation ABI and both integer returns."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish


def main():
    native = ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    pe = PE(native)
    export_path = ROOT/'analysis/squall-exports-native-instructions.txt'
    extra_path = ROOT/'analysis/squall-callback-signatures-native-instructions.txt'
    ins = instructions(export_path,pe)
    ins.update(instructions(extra_path,pe))
    audit = Audit(pe,ins)
    assert audit.imports[0x10048034] == 'IsBadCodePtr'
    evidence = {}

    def witness(address,op,operands):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == operands,(hex(address),row)
        evidence[address] = dict(address=f'{address:08x}',bytes=row['raw'].hex(),op=op,operands=operands)

    # Public File(Open,Seek,Read,Close) deliberately reorders to the internal
    # storage tuple(Open,Read,Seek,Close). Verify every actual argument transfer.
    public_file = [(0x10003c11,'mov','ebx,dword ptr [esp+8]'),
        (0x10003c26,'mov','ebp,dword ptr [esp+20h]'),
        (0x10003c50,'mov','edx,dword ptr [esp+18h]'),
        (0x10003c54,'mov','eax,dword ptr [esp+1Ch]'),
        (0x10003c58,'push','ebp'),(0x10003c59,'push','edx'),
        (0x10003c5a,'push','eax'),(0x10003c5b,'push','ebx'),
        (0x10003c5c,'call','100085A0'),(0x10003c61,'add','esp,10h')]
    public_memory = [(0x10003bd1,'mov','ebx,dword ptr [esp+8]'),
        (0x10003be6,'mov','ebp,dword ptr [esp+18h]'),
        (0x10003bfa,'push','ebp'),(0x10003bfb,'push','ebx'),
        (0x10003bfc,'call','100085E0'),(0x10003c01,'add','esp,8')]
    stores = [(0x100085aa,'mov','eax,dword ptr [esp+4]'),
        (0x100085ae,'mov','ecx,dword ptr [esp+8]'),
        (0x100085b2,'mov','edx,dword ptr [esp+0Ch]'),
        (0x100085b6,'mov','dword ptr ds:[10075990h],eax'),
        (0x100085bb,'mov','eax,dword ptr [esp+10h]'),
        (0x100085bf,'mov','dword ptr ds:[10075994h],ecx'),
        (0x100085c5,'mov','dword ptr ds:[10075998h],edx'),
        (0x100085cb,'mov','dword ptr ds:[1007599Ch],eax'),
        (0x100085ea,'mov','eax,dword ptr [esp+4]'),
        (0x100085ee,'mov','ecx,dword ptr [esp+8]'),
        (0x100085f2,'mov','dword ptr ds:[100759A0h],eax'),
        (0x100085f7,'mov','dword ptr ds:[100759A4h],ecx')]
    bridge = [(0x1002def2,'mov','ecx,dword ptr ds:[100759A4h]'),
        (0x1002def8,'mov','edx,dword ptr ds:[100759A0h]'),
        (0x1002defe,'push','ecx'),(0x1002deff,'mov','ecx,dword ptr ds:[1007599Ch]'),
        (0x1002df05,'push','edx'),(0x1002df06,'mov','edx,dword ptr ds:[10075998h]'),
        (0x1002df0c,'push','ecx'),(0x1002df0d,'mov','ecx,dword ptr ds:[10075994h]'),
        (0x1002df13,'push','edx'),(0x1002df14,'mov','edx,dword ptr ds:[10075990h]'),
        (0x1002df1a,'push','ecx'),(0x1002df1f,'push','edx'),
        (0x1002df2d,'call','1002D470')]
    fields = [(0x1002d474,'mov','edx,dword ptr [esp+10h]'),
        (0x1002d484,'mov','dword ptr [ebp],edx'),
        (0x1002d487,'mov','edx,dword ptr [esp+20h]'),
        (0x1002d48f,'mov','dword ptr [ebp+4],edx'),
        (0x1002d492,'mov','edx,dword ptr [esp+24h]'),
        (0x1002d498,'mov','dword ptr [ebp+8],edx'),
        (0x1002d49b,'mov','edx,dword ptr [esp+28h]'),
        (0x1002d49f,'mov','dword ptr [ebp+0Ch],edx'),
        (0x1002d4a2,'mov','edx,dword ptr [esp+2Ch]'),
        (0x1002d4c4,'mov','dword ptr [ebp+10h],edx'),
        (0x1002d48b,'mov','esi,dword ptr [esp+30h]'),
        (0x1002d4c7,'mov','dword ptr [ebp+14h],esi')]
    calls = [(0x1002d4d2,'repne','scas  byte ptr es:[edi]'),
        (0x1002d4d7,'lea','esi,[ecx+1]'),(0x1002d4da,'push','esi'),
        (0x1002d4db,'call','edx'),(0x1002d4dd,'add','esp,4'),
        (0x1002d4e0,'mov','dword ptr [ebp+1Ch],eax'),
        (0x1002d52a,'push','ebx'),(0x1002d52b,'call','dword ptr [ebp]'),
        (0x1002d52e,'add','esp,4'),(0x1002d533,'mov','dword ptr [ebp+34h],eax'),
        (0x1002d53c,'push','2'),(0x1002d53e,'push','edi'),(0x1002d53f,'push','eax'),
        (0x1002d540,'call','dword ptr [ebp+8]'),(0x1002d543,'add','esp,0Ch'),
        (0x1002d548,'mov','dword ptr [ebp+24h],eax'),
        (0x1002d564,'push','eax'),(0x1002d565,'call','dword ptr [ebp+10h]'),
        (0x1002d568,'add','esp,4'),(0x1002d56d,'mov','dword ptr [ebp+18h],eax'),
        (0x1002d578,'push','ecx'),(0x1002d579,'push','eax'),(0x1002d57a,'push','edx'),
        (0x1002d57b,'call','dword ptr [ebp+4]'),(0x1002d581,'add','esp,0Ch'),
        (0x1002d584,'cmp','eax,ecx'),(0x1002d58b,'push','eax'),
        (0x1002d58c,'call','dword ptr [ebp+0Ch]'),(0x1002d593,'add','esp,4'),
        (0x1002d596,'mov','eax,1'),(0x1002d5e2,'push','eax'),
        (0x1002d5e3,'call','dword ptr [ebp+14h]'),(0x1002d5e6,'add','esp,4')]
    for row in public_file+public_memory+stores+bridge+fields+calls:
        witness(*row)
    header = re.sub(r'\s+','',(ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf-8'))
    for declaration in [
        'typedefunsignedint(__cdecl*squall_callback_open_t)(constchar*Name);',
        'typedefint(__cdecl*squall_callback_read_t)(unsignedintHandle,void*Buffer,intSize);',
        'typedefint(__cdecl*squall_callback_seek_t)(unsignedintHandle,intPosition,intSeekMode);',
        'typedefvoid(__cdecl*squall_callback_close_t)(unsignedintHandle);',
        'typedefvoid*(__cdecl*squall_callback_alloc_t)(unsignedintSize);',
        'typedefvoid(__cdecl*squall_callback_free_t)(void*Ptr);',
        'DLL_APIintSQUALL_SetFileCallbacks(squall_callback_open_tUserOpen,squall_callback_seek_tUserSeek,squall_callback_read_tUserRead,squall_callback_close_tUserClose);',
        'DLL_APIintSQUALL_SetMemoryCallbacks(squall_callback_alloc_tUserAlloc,squall_callback_free_tUserFree);']:
        assert declaration in header,declaration
    roles = dict(
        SQUALL_SetFileCallbacks=['cdecl Open(const char*)->32bit opaque handle in EAX',
            'cdecl Seek(32bit handle,32bit position,32bit mode)->32bit integer in EAX',
            'cdecl Read(32bit handle,void* output buffer,32bit size)->32bit integer count in EAX',
            'cdecl Close(32bit handle);return ignored by native caller'],
        SQUALL_SetMemoryCallbacks=['cdecl Allocate(32bit byte size)->void* in EAX',
            'cdecl Free(void* allocation);return ignored by native caller'])
    rows = []
    for name,start,status in [('SQUALL_SetFileCallbacks',0x10003c10,'edi'),('SQUALL_SetMemoryCallbacks',0x10003bd0,'esi')]:
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == list(range(1,len(roles[name])+1))
        seen = set();todo = [(start,False,False)];returns = set();values = set()
        while todo:
            address,initialized,eax_status = todo.pop()
            state = (address,initialized,eax_status)
            if state in seen:
                continue
            seen.add(state)
            row = ins[address]
            assert row['file_verified']
            op,args = row['op'],row['args'];parts = args.split(',')
            if op == 'call':
                eax_status = False
            elif op == 'mov' and parts[0] == status:
                assert parts[1] in ['1','0FFFFFFFCh']
                values.add(parts[1]);initialized = True;witness(address,op,args)
            elif op == 'pop' and args == status:
                initialized = False
            elif op == 'mov' and parts[0] == 'eax':
                eax_status = parts[1] == status and initialized
                if eax_status:
                    witness(address,op,args)
            elif parts[0] == status and op not in ['test','cmp','push']:
                raise AssertionError(('Unknown status write',name,hex(address),row))
            if op == 'ret':
                assert not args and row['raw'] == b'\xc3' and eax_status
                returns.add(f'{address:08x}');witness(address,op,args)
            else:
                todo.extend((target,initialized,eax_status) for target in audit.successors(address))
        assert returns == set(stack['balanced_return_addresses']) and values == {'1','0FFFFFFFCh'}
        rows.append(dict(name=name,original_address=f'{start:08x}',calling_convention='cdecl',
            argument_slots=len(roles[name]),argument_role=dict(callback_parameters_in_public_order=roles[name],
                independent_evidence='IsBadCodePtr validation,exact public-to-global-to-reader pointer mapping,actual callback pushes,caller cleanup and EAX usage'),
            return_role='Initialized32-bit integer1 or-4 in EAX on every C3 return',
            all_native_return_sites=sorted(returns),definite_assignment_states=len(seen),
            native_signature_fully_verified=True,whole_behavior_fully_verified=False))
    count = publish('squall-callback-signatures.json',rows,__file__,dict(
        evidence_files={p.relative_to(ROOT).as_posix():digest(p) for p in [export_path,extra_path]},
        instruction_evidence=[evidence[a] for a in sorted(evidence)],
        original_internal_file_tuple=['Open','Read','Seek','Close'],original_public_file_tuple=['Open','Seek','Read','Close'],
        native_callback_void_returns_mean_ignored_register_result=True,
        source_typedefs_and_public_parameter_order_match=True,
        dataflow_assumption='Standard Win32 x86 nonvolatile ESI/EDI preservation across calls',
        behavior_reports=['analysis/squall-file-callbacks-main-validation.json','analysis/squall-file-callbacks-fresh-validation.json'],
        new_runtime_audio_test_claimed=False))
    print(f'Proved2 public callback ABI signatures through real reader calls;{count}/97 native ABI semantics verified.')


if __name__ == '__main__':
    main()
