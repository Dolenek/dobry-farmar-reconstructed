#!/usr/bin/env python3
"""Prove listener/channel worker argument roles, actual callback ABI and all returns."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from record_live_production_verification import ROOT,digest
from squall_signature_evidence import publish


def main():
    pe=PE(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path=ROOT/'analysis/squall-exports-native-instructions.txt'
    ins=instructions(path,pe);audit=Audit(pe,ins);evidence={}
    assert audit.imports[0x10048034]=='IsBadCodePtr' and audit.imports[0x10048010]=='CreateThread'

    def witness(address,op,args):
        row=ins[address]
        assert row['file_verified'] and row['op']==op and row['args']==args,(hex(address),row)
        evidence[address]=dict(address=f'{address:08x}',bytes=row['raw'].hex(),op=op,operands=args)

    witnesses=[
        # Listener public callback pointer, opaque parameter and unsigned period.
        (0x10004d14,'mov','edi,dword ptr [esp+0Ch]'),(0x10004d40,'push','edi'),
        (0x10004d41,'call','dword ptr ds:[10048034h]'),
        (0x10004d4b,'mov','eax,dword ptr [esp+14h]'),(0x10004d4f,'test','eax,eax'),
        (0x10004d51,'ja','10004D6A'),(0x10004d6a,'mov','ecx,dword ptr [esp+10h]'),
        (0x10004d6e,'mov','dword ptr ds:[10075164h],edi'),
        (0x10004d74,'mov','dword ptr ds:[10075168h],ecx'),
        (0x10004d7a,'mov','dword ptr ds:[1007516Ch],eax'),
        # Init passes the actual same context to CreateThread through thiscall.
        (0x100034a2,'mov','ecx,10075034h'),(0x100034a7,'call','100026B0'),
        (0x100026b7,'mov','esi,ecx'),(0x100026d8,'push','esi'),
        (0x100026d9,'push','10002580h'),(0x100026e2,'call','dword ptr ds:[10048010h]'),
        (0x10002583,'mov','esi,dword ptr [esp+10h]'),
        (0x1000264f,'mov','eax,dword ptr [esi+00000130h]'),
        (0x10002667,'mov','ecx,dword ptr [esi+00000138h]'),
        (0x1000266f,'cmp','eax,ecx'),(0x10002671,'jbe','1000268B'),
        (0x10002675,'mov','ecx,dword ptr [esi+00000134h]'),
        (0x10002681,'push','ecx'),(0x10002682,'call','dword ptr [esi+00000130h]'),
        (0x10002688,'add','esp,4'),
        # Channel handle resolver, code pointer, userdata, raw unsigned setter.
        (0x10005b39,'mov','eax,dword ptr [esp+10h]'),(0x10005b42,'push','eax'),
        (0x10005b43,'call','10002480'),(0x10005b66,'mov','edi,dword ptr [esp+14h]'),
        (0x10005bb4,'push','edi'),(0x10005bb5,'call','dword ptr ds:[10048034h]'),
        (0x10005bbf,'mov','ecx,dword ptr [esp+1Ch]'),(0x10005bc3,'test','ecx,ecx'),
        (0x10005bc5,'jbe','10005C11'),(0x10005bd0,'mov','esi,dword ptr [esp+18h]'),
        (0x10005bca,'mov','edx,dword ptr ds:[10075134h]'),
        (0x10005bd9,'lea','eax,[eax+eax*4]'),(0x10005bdc,'lea','eax,[eax+eax*4]'),
        (0x10005bdf,'shl','eax,2'),
        (0x10005be2,'mov','dword ptr [eax+edx+00000124h],edi'),
        (0x10005bef,'mov','dword ptr [eax+edx+00000128h],esi'),
        (0x10005bfc,'mov','dword ptr [eax+edx+0000011Ch],ecx'),
        # Actual channel call consumes two parameters and a32-bit return.
        (0x10001eb3,'mov','edx,dword ptr [esi+0000011Ch]'),
        (0x10001eb9,'cmp','ecx,edx'),(0x10001ebb,'jle','10001EE8'),
        (0x10001ec3,'mov','eax,dword ptr [esi+00000124h]'),
        (0x10001ecd,'mov','edx,dword ptr [esi+00000128h]'),
        (0x10001ed3,'mov','ecx,dword ptr [esi+3Ch]'),
        (0x10001ed6,'push','edx'),(0x10001ed7,'push','ecx'),
        (0x10001ed8,'call','eax'),(0x10001eda,'add','esp,8'),
        (0x10001edd,'test','eax,eax'),(0x10001edf,'jne','10001EE8'),
        (0x10001ee3,'call','10001850')]
    for row in witnesses:witness(*row)
    assert 0x10075034+0x130==0x10075164 and 0x10075034+0x134==0x10075168 and 0x10075034+0x138==0x1007516c
    header=re.sub(r'\s+','',(ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf-8'))
    for declaration in [
        'typedefvoid(__cdecl*squall_callback_listener_t)(void*UserData);',
        'typedefint(__cdecl*squall_callback_channel_t)(intChannelID,void*UserData);',
        'DLL_APIintSQUALL_Listener_SetWorker(squall_callback_listener_tWorker,void*Param,unsignedintUpdateTime);',
        'DLL_APIintSQUALL_Channel_SetWorker(intChannelID,squall_callback_channel_tWorker,void*Param,unsignedintUpdateTime);']:
        assert declaration in header,declaration
    roles={
        'SQUALL_Listener_SetWorker':['32-bit nullable code pointer;actual cdecl callback(void*),register result ignored',
            'opaque32-bit userdata pointer stored and forwarded unchanged','unsigned32-bit period;zero rejected for nonnull callback'],
        'SQUALL_Channel_SetWorker':['32-bit integer channel handle consumed by actual resolver10002480',
            '32-bit nullable code pointer;actual cdecl callback(int channel,void*) returns32-bit zero/nonzero in EAX',
            'opaque32-bit userdata pointer stored and forwarded unchanged','unsigned32-bit setter period;zero rejected for nonnull callback']}
    rows=[]
    for name,start,status in [('SQUALL_Listener_SetWorker',0x10004cf0,'esi'),('SQUALL_Channel_SetWorker',0x10005b10,'ebp')]:
        stack=audit.function(start)
        assert stack['explicit_argument_slots']==list(range(1,len(roles[name])+1))
        seen=set();todo=[(start,False,False)];returns=set();values=set()
        while todo:
            address,initialized,eax_status=todo.pop();state=(address,initialized,eax_status)
            if state in seen:continue
            seen.add(state);row=ins[address];assert row['file_verified']
            op,args=row['op'],row['args'];parts=args.split(',')
            if op=='call':eax_status=False
            elif op=='mov' and parts[0]==status:
                assert parts[1][0].isdigit();initialized=True;values.add(parts[1]);witness(address,op,args)
            elif op=='or' and parts==[status,'0FFFFFFFFh']:
                initialized=True;values.add('0FFFFFFFFh');witness(address,op,args)
            elif op=='pop' and args==status:initialized=False
            elif op=='mov' and parts[0]=='eax':
                eax_status=parts[1]==status and initialized
                if eax_status:witness(address,op,args)
            elif parts[0]==status and op not in ['test','cmp','push']:
                raise AssertionError(('Unenumerated native status write',name,hex(address),row))
            if op=='ret':
                assert not args and row['raw']==b'\xc3' and eax_status
                returns.add(f'{address:08x}');witness(address,op,args)
            else:todo.extend((n,initialized,eax_status) for n in audit.successors(address))
        assert returns==set(stack['balanced_return_addresses'])
        rows.append(dict(name=name,original_address=f'{start:08x}',calling_convention='cdecl',argument_slots=len(roles[name]),
            argument_role=dict(arguments_in_public_order=roles[name],independent_evidence='Actual setter argument loads,code validation,storage and thread callback pushes/cleanup/result tests'),
            return_role='Complete initialized32-bit integer status in EAX on every C3 return',
            native_status_immediates=sorted(values),all_native_return_sites=sorted(returns),definite_assignment_states=len(seen),
            native_signature_fully_verified=True,whole_behavior_fully_verified=False))
    assert sum(len(r['all_native_return_sites']) for r in rows)==7
    count=publish('squall-worker-signatures.json',rows,__file__,dict(
        instruction_file_sha256=digest(path),instruction_evidence=[evidence[a] for a in sorted(evidence)],
        native_channel_scheduling_uses_signed_comparison=True,native_listener_scheduling_uses_unsigned_comparison=True,
        setter_millisecond_arguments_preserve_all32_bits=True,listener_callback_void_means_register_result_ignored=True,
        source_header_matches=True,new_runtime_audio_test_claimed=False,
        dataflow_assumption='Standard Win32 x86 nonvolatile ESI/EBP preservation across native calls',
        behavior_reports=['analysis/squall-channel-worker-validation.json','analysis/squall-channel-worker-fresh-validation.json',
                          'analysis/squall-listener-worker-validation.json','analysis/squall-listener-worker-fresh-validation.json']))
    print(f'Proved2 native worker signatures,actual1/2-argument callbacks and7 return sites;{count}/97 ABI semantics verified.')


if __name__=='__main__':main()
