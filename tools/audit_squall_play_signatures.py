#!/usr/bin/env python3
"""Prove eight play interfaces by propagating each actual public argument word."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit, REGS, number
from squall_integer_word_flow import prove_integer_word
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish

METHODS = [('Sample_Play',0x10007520,4), ('Sample_PlayEx',0x100075a0,8),
    ('Sample_Play3D',0x10007600,6), ('Sample_Play3DEx',0x100076c0,11),
    ('SampleGroup_Play',0x10007870,4), ('SampleGroup_PlayEx',0x10007910,8),
    ('SampleGroup_Play3D',0x10007990,6), ('SampleGroup_Play3DEx',0x10007a60,11)]


def trace_arguments(audit, start, count):
    """Exact stack offsets and register copies; preserve unknown default paths."""
    todo = [(start,0,('?',)*len(REGS),tuple((4*n,f'arg{n}') for n in range(1,count+1)))]
    seen, calls = set(), set()
    while todo:
        address, sp, tags, words = todo.pop();state = address, sp, tags, words
        if state in seen:continue
        seen.add(state);assert len(seen)<12000
        row=audit.ins[address];assert row['file_verified'];op,args=row['op'],row['args'];parts=args.split(',')
        regs, memory=dict(zip(REGS,tags)),dict(words)

        def offset(text):
            m=re.fullmatch(r'(?:dword ptr )?\[esp(?:\+([0-9A-F]+h|[0-9]+))?\]',text)
            return sp+(number(m[1]) if m[1] else 0) if m else None

        def value(text):
            if text in regs:return regs[text]
            k=offset(text)
            if k is not None:return memory.get(k,'?')
            if re.fullmatch(r'[0-9A-F]+h|[0-9]+',text):return 'ctx' if number(text)==0x10075034 else str(number(text))
            if text=='dword ptr ds:[10048024h]':return 'IsBadReadPtr'
            return '?'

        def release(size):
            nonlocal sp
            for k in range(sp,sp+size,4):memory.pop(k,None)
            sp+=size

        if op=='push':tag=value(args);sp-=4;memory[sp]=tag
        elif op=='pop':regs[args]=memory.get(sp,'?');release(4)
        elif op in ['add','sub'] and parts[0]=='esp':
            if op=='add':release(number(parts[1]))
            else:sp-=number(parts[1])
        elif op=='mov':
            tag=value(parts[1])
            if parts[0] in regs:regs[parts[0]]=tag
            else:
                k=offset(parts[0]);assert k is not None,(hex(address),row);memory[k]=tag
        elif op=='lea':
            k=offset(parts[1]);assert k is not None;regs[parts[0]]='local:'+str(k)
        elif op=='xor' and parts[0]==parts[1]:regs[parts[0]]='0'
        elif op=='or' and parts[1]=='0FFFFFFFFh':regs[parts[0]]=str(0xffffffff)
        elif op=='call':
            target=int(args,16) if re.fullmatch('[0-9A-F]{8}',args) else None
            result='?'
            if target is None:
                assert regs[args]=='IsBadReadPtr' and memory[sp] in ['arg5','arg6'] and memory[sp+4]=='12'
            if target==0x10008410:
                out=memory[sp+4];assert out.startswith('local:')
                base=int(out[6:])
                for n in range(7):memory[base+4*n]=f'default{n}'
            elif target==0x10008330:
                assert memory[sp]=='arg1' and memory[sp+4]=='0';result='sample_from_group'
            elif target in [0x10004da0,0x10005010]:
                size=9 if target==0x10004da0 else 12
                calls.add((f'{address:08x}',tuple(memory.get(sp+4*n,'?') for n in range(size))))
                result='integer_result'
            cleanup=audit.call_pop(address,{r:regs[r] if regs[r]=='IsBadReadPtr' else None for r in REGS})
            for r in ['eax','ecx','edx']:regs[r]='?'
            regs['eax']=result;release(cleanup)
        elif op=='ret':
            assert sp==0 and regs['eax']=='integer_result',(hex(address),state)
            continue
        else:assert op in ['cmp','test'] or op.startswith('j'),(hex(address),row)
        todo += [(n,sp,tuple(regs[r] for r in REGS),tuple(sorted(memory.items()))) for n in audit.successors(address)]
    return [dict(call=a,backend_arguments=list(b)) for a,b in sorted(calls)]


def main():
    native=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    pe=PE(native);path=ROOT/'.local/squall-complete-native-disassembly.txt'
    ins=instructions(path,pe);audit=Audit(pe,ins);evidence={}

    def graph(start):
        todo,seen=[start],set()
        while todo:
            address=todo.pop()
            if address in seen:continue
            seen.add(address);r=ins[address];assert r['file_verified']
            evidence[address]=dict(address=f'{address:08x}',bytes=r['raw'].hex(),op=r['op'],operands=r['args'])
            todo+=audit.successors(address)
        return sorted(seen)

    def witness(address,op,args):
        r=ins[address];assert r['file_verified'] and r['op']==op and r['args']==args,(hex(address),r)
        graph(address) if op=='ret' else None
        evidence[address]=dict(address=f'{address:08x}',bytes=r['raw'].hex(),op=op,operands=args)

    for s in [0x10002530,0x10008330,0x10001a70]:graph(s)
    for row in [(0x1000256e,'xor','edx,edx'),(0x10002570,'mov','dh,byte ptr [ecx+eax]'),
        (0x10002573,'or','edx,edi'),(0x10002577,'mov','eax,edx'),
        (0x10004e28,'mov','dword ptr [ebx+ecx+3Ch],eax'),
        (0x100050ff,'mov','dword ptr [ebx+ecx+3Ch],eax'),
        (0x10004ffe,'mov','eax,dword ptr [ecx+eax*4+3Ch]'),
        (0x100052fa,'mov','ebx,dword ptr [ecx+eax*4+3Ch]'),
        (0x10005021,'fsub','dword ptr [edi+8]'),(0x10005039,'fsub','dword ptr [edi+4]'),
        (0x10005053,'fsub','dword ptr [edi]'),(0x10005068,'fcomp','dword ptr [esp+3Ch]'),
        (0x10001a8d,'fcomp','dword ptr [edx]'),(0x10001a99,'fcomp','dword ptr [edx+4]'),
        (0x10001aa6,'fcomp','dword ptr [edx+8]'),(0x10001ace,'fcomp','dword ptr [edx]'),
        (0x10001ada,'fcomp','dword ptr [edx+4]'),(0x10001ae7,'fcomp','dword ptr [edx+8]'),
        (0x10004e4c,'mov','dword ptr [ebx+edx+40h],eax'),
        (0x10004e5e,'and','edx,1'),(0x10004ec2,'mov','dword ptr [ebx+ecx+44h],edx'),
        (0x10004f39,'mov','eax,dword ptr [edx*4+10074824h]'),
        (0x10004f65,'call','10001FE0'),(0x10004f7d,'call','10002040'),
        (0x10004f8a,'test','eax,eax'),
        (0x10005206,'mov','edx,dword ptr [esp+44h]'),
        (0x1000520a,'mov','eax,dword ptr [esp+40h]'),
        (0x10005213,'push','edx'),(0x10005218,'push','eax'),
        (0x1000522d,'call','10001A70'),
        (0x10001b87,'fld','dword ptr [esp+24h]'),
        (0x10001bb5,'fld','dword ptr [esp+28h]'),
        (0x1000835e,'cmp','dword ptr [ecx+8],ebp')]:witness(*row)
    backends=[]
    for start in [0x10004da0,0x10005010]:
        graph(start)
        proof,extra=prove_integer_word(audit,start,numeric_calls=[0x100024c0,0x1002e140,0x10002530],numeric_memory=[0x10004ffe,0x100052fa])
        evidence.update(extra);backends.append(dict(address=f'{start:08x}',stack_proof=audit.function(start),**proof))
    header=re.sub(r'\s+','',(ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows=[]
    for suffix,start,count in METHODS:
        name='SQUALL_'+suffix;three='3D' in suffix;extended=suffix.endswith('Ex');group='Group' in suffix
        types=['int']*4+(['float*']*2 if three else [])
        if extended:types+=['int']*3+(['float']*2 if three else ['int'])
        assert len(types)==count
        assert re.search('DLL_APIint'+name+r'\('+','.join(re.escape(t)+r'\w+' for t in types)+r'\);',header)
        graph(start);stack=audit.function(start);assert stack['explicit_argument_slots']==list(range(1,count+1))
        calls=trace_arguments(audit,start,count)
        for call in calls:
            actual=call['backend_arguments'];assert actual[0]=='ctx'
            assert actual[1] in (['sample_from_group','0'] if group else ['arg1'])
            assert actual[2:5]==['arg2','arg3','arg4']
            index=5
            if three:
                assert actual[5] in ['arg5','0'] and actual[6] in ['arg6','0'];index=7
            if extended:
                assert actual[index:]==[f'arg{n}' for n in range(7 if three else 5,count+1)]
            else:
                defaults=[1,3,2,5,6] if three else [1,3,2,4]
                assert all(v in [f'default{n}','?'] for v,n in zip(actual[index:],defaults))
        result,extra=prove_integer_word(audit,start,numeric_calls=[0x10004da0,0x10005010]);evidence.update(extra)
        assert result['all_native_return_sites']==stack['balanced_return_addresses'] and len(result['all_native_return_sites'])==1
        rows.append(dict(name=name,original_address=f'{start:08x}',calling_convention='cdecl',argument_slots=count,
            argument_role=dict(public_types_in_order=types,actual_backend_word_propagation=calls,source_header_matches=True),
            return_role='Integer32 encoded channel ID or negative error in EAX on balanced C3 return',
            native_signature_fully_verified=True,whole_behavior_fully_verified=False,**result))
    count=publish('squall-play-signatures.json',rows,__file__,dict(
        instruction_evidence=[evidence[a] for a in sorted(evidence)],disassembly_sha256=digest(path),
        private_integer_return_proofs=backends,integer_word_proof_sha256=digest(ROOT/'tools/squall_integer_word_flow.py'),
        vector_readable_extent_bytes=12,optional_velocity_null_supported=True,
        all_3d_invalid_position_paths_safe_claimed=False,default_local_initialized_on_all_failure_paths_claimed=False,
        new_runtime_playback_test_claimed=False))
    print(f'Proved8 play signatures,8 public and6 backend returns;{count}/97 ABI semantics verified.')


if __name__=='__main__':main()
