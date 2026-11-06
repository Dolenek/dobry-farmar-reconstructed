#!/usr/bin/env python3
"""Native mixed float-vector/integer cone signatures, including unsafe output."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from squall_native_status import prove_status
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish


def main():
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe);audit = Audit(pe, ins);evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    for row in [
        (0x10006ac8, 'mov', 'ebx,dword ptr [esp+20h]'), (0x10006acf, 'push', '0Ch'),
        (0x10006ad1, 'push', 'ebx'), (0x10006ad2, 'call', 'dword ptr ds:[10048030h]'),
        (0x10006aea, 'mov', 'ebp,dword ptr [esp+28h]'), (0x10006aee, 'test', 'ebp,ebp'),
        (0x10006af0, 'jl', '10006B1B'), (0x10006af2, 'cmp', 'ebp,64h'), (0x10006af5, 'jg', '10006B1B'),
        (0x10006af7, 'mov', 'edi,dword ptr [esp+20h]'), (0x10006afb, 'test', 'edi,edi'),
        (0x10006afd, 'jl', '10006B1B'), (0x10006aff, 'cmp', 'edi,168h'), (0x10006b05, 'jg', '10006B1B'),
        (0x10006b07, 'mov', 'esi,dword ptr [esp+24h]'), (0x10006b0b, 'test', 'esi,esi'),
        (0x10006b0d, 'jl', '10006B1B'), (0x10006b0f, 'cmp', 'esi,168h'), (0x10006b15, 'jg', '10006B1B'),
        (0x10006b48, 'mov', 'edx,dword ptr [ebp*4+10074824h]'),
        (0x10006b60, 'push', 'edx'), (0x10006b61, 'push', 'esi'), (0x10006b62, 'push', 'edi'),
        (0x10006b66, 'push', 'ebx'), (0x10006b6e, 'call', '10001A70'),
        (0x10001b04, 'mov', 'edx,dword ptr [esp+14h]'),
        (0x10001b0c, 'fld', 'dword ptr [ecx+78h]'), (0x10001b0f, 'fcomp', 'dword ptr [edx]'),
        (0x10001b18, 'fld', 'dword ptr [ecx+7Ch]'), (0x10001b1b, 'fcomp', 'dword ptr [edx+4]'),
        (0x10001b25, 'fld', 'dword ptr [ecx+00000080h]'), (0x10001b2b, 'fcomp', 'dword ptr [edx+8]'),
        (0x10001b4b, 'mov', 'eax,dword ptr [esp+18h]'), (0x10001b53, 'cmp', 'dword ptr [ecx+70h],eax'),
        (0x10001b5d, 'mov', 'eax,dword ptr [esp+1Ch]'), (0x10001b65, 'cmp', 'dword ptr [ecx+74h],eax'),
        (0x10001b6f, 'mov', 'eax,dword ptr [esp+20h]'), (0x10001b77, 'cmp', 'dword ptr [ecx+00000084h],eax'),
        (0x10001c4e, 'mov', 'eax,dword ptr [esp+0Ch]'),
        (0x10001c56, 'mov', 'edx,dword ptr [ecx+78h]'), (0x10001c59, 'mov', 'dword ptr [eax],edx'),
        (0x10001c5b, 'mov', 'edx,dword ptr [ecx+7Ch]'), (0x10001c5e, 'mov', 'dword ptr [eax+4],edx'),
        (0x10001c61, 'mov', 'edx,dword ptr [ecx+00000080h]'), (0x10001c67, 'mov', 'dword ptr [eax+8],edx'),
        (0x10001c72, 'mov', 'edx,dword ptr [ecx+70h]'), (0x10001c75, 'mov', 'dword ptr [eax],edx'),
        (0x10001c7f, 'mov', 'edx,dword ptr [ecx+74h]'), (0x10001c82, 'mov', 'dword ptr [eax],edx'),
        (0x10001c6a, 'mov', 'eax,dword ptr [esp+10h]'), (0x10001c6e, 'test', 'eax,eax'), (0x10001c70, 'je', '10001C77'),
        (0x10001c77, 'mov', 'eax,dword ptr [esp+14h]'), (0x10001c7b, 'test', 'eax,eax'), (0x10001c7d, 'je', '10001C84'),
        (0x10006bd5, 'push', '0Ch'), (0x10006bd7, 'push', 'ebp'), (0x10006bd8, 'call', 'esi'),
        (0x10006be4, 'push', '4'), (0x10006be6, 'push', 'ebx'), (0x10006be7, 'call', 'esi'),
        (0x10006bf3, 'push', '4'), (0x10006bf5, 'push', 'edi'), (0x10006bf6, 'call', 'esi'),
        (0x10006c02, 'push', '4'), (0x10006c04, 'push', 'eax'), (0x10006c05, 'call', 'esi'),
        (0x10006c0b, 'xor', 'esi,esi'), (0x10006c23, 'mov', 'dword ptr [esp+28h],0'),
        (0x10006c4f, 'lea', 'edx,[esp+2Ch]'), (0x10006c58, 'push', 'edx'),
        (0x10006c59, 'push', 'edi'), (0x10006c5a, 'push', 'ebx'), (0x10006c5e, 'push', 'ebp'),
        (0x10006c66, 'call', '10001C10'), (0x10006c7f, 'fild', 'dword ptr [esp+28h]'),
        (0x10006ca0, 'call', '1003D254'), (0x10006ca5, 'mov', 'dword ptr [esi],eax'),
        (0x1003d26d, 'fistp', 'qword ptr [ebp-0Ch]'), (0x1003d273, 'mov', 'eax,dword ptr [ebp-0Ch]')]:
        witness(*row)
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for output, start in [(False, 0x10006aa0), (True, 0x10006ba0)]:
        name = 'SQUALL_Channel_' + ('Get' if output else 'Set') + 'ConeParameters'
        params = ['intChannelID', 'float*Orientation'] + [('int*' if output else 'int') + p for p in ['InsideConeAngle', 'OutsideConeAngle', 'OutsideVolume']]
        assert 'DLL_APIint' + name + '(' + ','.join(params) + ');' in header
        stack = audit.function(start);assert stack['explicit_argument_slots'] == [1, 2, 3, 4, 5]
        handle = [r for r in stack['stack_references'] if r['entry_esp_offset'] == 4]
        assert len(handle) == 1 and handle[0]['op'] == 'mov'
        at = int(handle[0]['address'], 16)
        register = ins[at]['args'].split(',')[0]
        assert register in ['eax', 'ecx']
        at += len(ins[at]['raw']);forwarded = False
        while ins[at]['op'] != 'call':
            row = ins[at]
            if row['op'] == 'push':
                assert row['args'] == register and not forwarded;forwarded = True
            else:
                assert row['op'] == 'mov' and (row['args'].startswith('dword ptr [esp+') or forwarded and row['args'] == 'ecx,10075034h' or row['args'] == 'ecx,10075034h' and register == 'eax')
            at += len(row['raw'])
        assert forwarded and ins[at]['args'] == '10002480'
        result, extra = prove_status(audit, start);evidence.update(extra)
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        checks = [a for a in extra if ins[a]['op'] == 'call' and audit.call_evidence[f'{a:08x}']['target'] == 'IsBadWritePtr']
        assert len(checks) == (4 if output else 1)
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=5,
            argument_role=dict(arguments_in_public_order=['Unchanged32-bit integer ChannelID',
                '12-byte pointer to three binary32 orientation floats;original checks writability even for setter input',
                'Two4-byte integer output pointers,independently optional' if output else 'Two signed32 integer angles0..360',
                '4-byte integer volume output pointer;unconditional final write on successful private getter' if output else 'Signed32 volume0..100,converted by actual integer table index'], source_header_matches=True),
            return_role='Initialized32-bit status in EAX on each balanced C3 return;invalid final output may instead fault in the original',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 2
    count = publish('squall-cone-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        status_proof_sha256=digest(ROOT / 'tools/squall_native_status.py'),
        native_setter_uses_IsBadWritePtr_for_input_orientation=True,
        native_getter_unconditionally_writes_successful_outside_volume=True,
        native_null_final_output_fault_path_inferred_from_instructions=True,
        new_runtime_fault_or_audio_test_claimed=False, dataflow_assumption='Standard Win32 x86 caller-local preservation'))
    print(f'Proved2 mixed native cone signatures and2 EAX/C3 return sites;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
