#!/usr/bin/env python3
"""Prove three cleanup calling contracts without inventing an EAX guarantee."""
import re
import struct
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from record_live_production_verification import ROOT, digest, read
from squall_signature_evidence import publish


def main():
    base=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    native=base/'Squall.dll';pe=PE(native);path=ROOT/'.local/squall-complete-native-disassembly.txt'
    ins=instructions(path,pe);audit=Audit(pe,ins);evidence={}

    def graph(start):
        todo,seen=[start],set()
        while todo:
            address=todo.pop()
            if address in seen:continue
            seen.add(address);row=ins[address];assert row['file_verified']
            evidence[address]=dict(address=f'{address:08x}',bytes=row['raw'].hex(),op=row['op'],operands=row['args'])
            todo+=audit.successors(address)
        return sorted(seen)

    def witness(address,op,args):
        r=ins[address];assert r['file_verified'] and r['op']==op and r['args']==args,(hex(address),r)
        evidence[address]=dict(address=f'{address:08x}',bytes=r['raw'].hex(),op=op,operands=args)

    for row in [(0x100073c0,'jmp','10008290'),(0x100073d0,'mov','eax,dword ptr [esp+4]'),
        (0x100073d4,'push','eax'),(0x100073d5,'call','10008240'),(0x100073da,'pop','ecx'),
        (0x10008246,'mov','eax,dword ptr [esp+8]'),(0x1000824a,'push','eax'),
        (0x1000824b,'call','10008000'),(0x10003604,'call','100084F0'),
        (0x100082f6,'call','10007FF0')]:witness(*row)
    for start in [0x10008240,0x10008000,0x100084f0,0x10007ff0]:graph(start)
    # Native game's import thunks and actual CALL sites independently show
    # these callers discard EAX. This cannot uniquely recover a C declaration.
    game_name=next(f['path'] for f in read(ROOT/'analysis/inventory.json')['files']
        if f['path'].lower().endswith('.exe') and 'SQUALL_Free' in
        [s.get('name') for d in PE(base/f['path']).imports() for s in d['symbols']])
    game=PE(base/game_name)
    imported={s.get('name'):s for d in game.imports() for s in d['symbols']}
    callers=[]
    for name,site,prefix in [
        ('SQUALL_Free',0x00535441,bytes.fromhex('8b470c8b00')),
        ('SQUALL_Sample_Unload',0x0053470c,bytes.fromhex('83c4048b4c240864890d000000008b0c24'))]:
        off=game.offset(site-game.image_base,5+len(prefix));raw=game.data[off:off+5+len(prefix)]
        assert raw[0]==0xe8 and raw[5:]==prefix
        target=site+5+struct.unpack_from('<i',raw,1)[0]
        thunk=game.offset(target-game.image_base,6)
        expected=b'\xff\x25'+struct.pack('<I',game.image_base+imported[name]['iat_rva'])
        assert game.data[thunk:thunk+6]==expected
        callers.append(dict(name=name,call_address=f'{site:08x}',call_and_following_bytes=raw.hex(),
            thunk_address=f'{target:08x}',thunk_bytes=expected.hex(),
            eax_observation='Free caller immediately overwrites EAX;Unload caller performs only stack/ECX operations before its next CALL'))
    header=re.sub(r'\s+','',(ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows=[]
    for name,start,count in [('SQUALL_Free',0x10003520,0),
        ('SQUALL_Sample_UnloadAll',0x100073c0,0),('SQUALL_Sample_Unload',0x100073d0,1)]:
        assert 'DLL_APIvoid'+name+('();' if count==0 else '(intSampleID);') in header or count==0 and 'DLL_APIvoid'+name+'(void);' in header
        graph(start);stack=audit.function(start);assert stack['explicit_argument_slots']==list(range(1,count+1))
        assert len(stack['balanced_return_addresses'])==1
        assert ins[int(stack['balanced_return_addresses'][0],16)]['raw']==b'\xc3'
        rows.append(dict(name=name,original_address=f'{start:08x}',calling_convention='cdecl',argument_slots=count,
            argument_role=dict(arguments_in_public_order=['Integer32 encoded SampleID passed unchanged to actual generation/index resolver'] if count else [],source_header_matches=True),
            return_role='No public return value required;caller-owned argument cleanup and balanced C3;EAX is deliberately unspecified',
            all_native_return_sites=stack['balanced_return_addresses'],stack_proof=stack,
            original_source_void_keyword_uniquely_recoverable=False,
            native_signature_fully_verified=True,whole_behavior_fully_verified=False))
    count=publish('squall-void-signatures.json',rows,__file__,dict(
        disassembly_sha256=digest(path),instruction_evidence=[evidence[a] for a in sorted(evidence)],
        original_game_sha256=digest(base/game_name),actual_original_game_callers=callers,
        original_game_unload_all_import_present='SQUALL_Sample_UnloadAll' in imported,
        return_contract='Physical void-compatible ABI only;the erased original C return declaration is not uniquely inferable',
        arbitrary_eax_value_used_as_status=False,new_runtime_cleanup_test_claimed=False))
    assert count==97
    print('Proved3 void-compatible cleanup contracts and all3 balanced returns;97/97 physical ABI signatures,whole behavior still partial.')


if __name__=='__main__':main()
