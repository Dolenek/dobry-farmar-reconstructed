#!/usr/bin/env python3
"""Prove native sample/millisecond fragment integer inputs and output pointers."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from squall_native_status import prove_status
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish

METHODS = [('SetFragment', 0x10005c30, 'ebx', False), ('GetFragment', 0x10005cf0, 'ebp', True),
           ('SetFragmentMs', 0x10005dd0, None, False), ('GetFragmentMs', 0x10005ef0, None, True)]


def main():
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe)
    audit = Audit(pe, ins)
    evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    for row in [
        (0x10005c55, 'mov', 'edi,dword ptr [esp+14h]'), (0x10005c59, 'test', 'edi,edi'), (0x10005c5b, 'jl', '10005C69'),
        (0x10005c5d, 'mov', 'esi,dword ptr [esp+18h]'), (0x10005c61, 'test', 'esi,esi'), (0x10005c63, 'jl', '10005C69'),
        (0x10005c65, 'cmp', 'edi,esi'), (0x10005cb5, 'push', 'esi'), (0x10005cb6, 'push', 'edi'),
        (0x10005cc0, 'call', '10002300'),
        (0x10002313, 'mov', 'esi,dword ptr [esp+8]'), (0x1000231d, 'mov', 'edx,dword ptr [esp+0Ch]'),
        (0x10002317, 'cmp', 'esi,eax'), (0x10002321, 'cmp', 'edx,eax'), (0x10002333, 'cmp', 'esi,edx'),
        (0x10002337, 'mov', 'dword ptr [ecx+28h],edx'), (0x1000233a, 'mov', 'dword ptr [ecx+2Ch],esi'),
        (0x1000233f, 'mov', 'dword ptr [ecx+28h],esi'), (0x10002342, 'mov', 'dword ptr [ecx+2Ch],edx'),
        (0x10005d9a, 'mov', 'ecx,dword ptr [edx+ecx*4+28h]'), (0x10005d9e, 'mov', 'dword ptr [esi],ecx'),
        (0x10005db2, 'mov', 'ecx,dword ptr [eax+edx*4+2Ch]'), (0x10005db6, 'mov', 'dword ptr [ebx],ecx'),
        (0x10005e04, 'mov', 'ebp,dword ptr [esp+10h]'), (0x10005e08, 'test', 'ebp,ebp'), (0x10005e0a, 'jl', '10005E18'),
        (0x10005e0c, 'mov', 'eax,dword ptr [esp+14h]'), (0x10005e10, 'test', 'eax,eax'), (0x10005e12, 'jl', '10005E18'),
        (0x10005e14, 'cmp', 'ebp,eax'), (0x10005e88, 'call', '1003D330'), (0x10005e8f, 'push', '3E8h'),
        (0x10005e96, 'call', '1003D280'), (0x10005ea6, 'call', '1003D330'), (0x10005ead, 'push', '3E8h'),
        (0x10005eb4, 'call', '1003D280'), (0x10005eb9, 'push', 'eax'), (0x10005eba, 'push', 'ebp'),
        (0x10005ebd, 'call', '10002300'),
        (0x10005fa6, 'call', '1003D330'), (0x10005fbf, 'call', '1003D280'), (0x10005fc4, 'mov', 'dword ptr [ebx],eax'),
        (0x10005fea, 'call', '1003D330'), (0x10005ffb, 'call', '1003D280'), (0x10006000, 'mov', 'dword ptr [ebp],eax')]:
        witness(*row)
    helper_report = ROOT / 'analysis/squall-scalar-handle-signatures.json'
    helper = __import__('json').loads(helper_report.read_text(encoding='utf8'))
    assert helper['original_sha256'] == digest(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    assert set(helper['details']['integer_helper_graphs']) == {'1003d330', '1003d280'}
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for suffix, start, status, output in METHODS:
        name = 'SQUALL_Channel_' + suffix
        declaration = '(intChannelID,int*Start,int*End);' if output else '(intChannelID,intStart,intEnd);'
        assert 'DLL_APIint' + name + declaration in header
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == [1, 2, 3]
        handle = [r for r in stack['stack_references'] if r['entry_esp_offset'] == 4]
        assert len(handle) == 1 and handle[0]['op'] == 'mov'
        address = int(handle[0]['address'], 16)
        assert ins[address]['args'].startswith('eax,dword ptr [esp+')
        address += len(ins[address]['raw'])
        witness(address, 'mov', 'ecx,10075034h');address += len(ins[address]['raw'])
        witness(address, 'push', 'eax');address += len(ins[address]['raw'])
        witness(address, 'call', '10002480')
        if output:
            optional = ([(0x10005d3a, 'esi', 'jne', 0x10005d5b), (0x10005d3e, 'ebx', 'jne', 0x10005d5b),
                         (0x10005d87, 'esi', 'je', 0x10005da0), (0x10005da0, 'ebx', 'je', 0x10005db8)] if suffix == 'GetFragment' else
                        [(0x10005f4b, 'ebx', 'jne', 0x10005f60), (0x10005f4f, 'ebp', 'jne', 0x10005f60),
                         (0x10005f82, 'ebx', 'je', 0x10005fc6), (0x10005fc6, 'ebp', 'je', 0x10006003)])
            for at, reg, op, target in optional:
                witness(at, 'test', reg + ',' + reg)
                witness(at + len(ins[at]['raw']), op, f'{target:08X}')
            checks = []
            todo, seen = [start], set()
            while todo:
                at = todo.pop()
                if at in seen:continue
                seen.add(at)
                row = ins[at]
                if row['op'] == 'push' and row['args'] == '4':
                    call = at + len(row['raw'])
                    assert ins[call]['op'] == 'push'
                    call += len(ins[call]['raw'])
                    assert audit.call_evidence[f'{call:08x}']['target'] == 'IsBadWritePtr'
                    checks.append(call)
                todo += audit.successors(at)
            assert len(checks) == 2
        result, public_evidence = prove_status(audit, start, status)
        evidence.update(public_evidence)
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=3,
            argument_role=dict(arguments_in_public_order=['Unchanged32-bit integer channel handle',
                'Two independently optional writable4-byte integer outputs;at least one valid output required' if output else
                'Two signed32 integer endpoints;negative or equal endpoints rejected before private length clamp/order'],
                units='milliseconds via original64-bit integer multiplication/division' if suffix.endswith('Ms') else 'integer sample positions',
                source_header_matches=True),
            return_role='Complete initialized32-bit status in EAX on every balanced C3 return',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 10
    count = publish('squall-fragment-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        integer_helper_evidence='analysis/squall-scalar-handle-signatures.json', integer_helper_evidence_sha256=digest(helper_report),
        status_proof_sha256=digest(ROOT / 'tools/squall_native_status.py'),
        public_ms_getter_argument_stack_word_reuse_preserved=True,
        new_runtime_audio_test_claimed=False, dataflow_assumption='Standard Win32 x86 nonvolatile status and caller-local stack preservation'))
    print(f'Proved4 native integer fragment signatures and10 EAX/C3 returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
