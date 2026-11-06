#!/usr/bin/env python3
"""Original hardware Boolean cache, actual consumption and optional int outputs."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit, REGS, number
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish


def main():
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe)
    audit = Audit(pe, ins)
    evidence = {}
    assert audit.imports[0x10048030] == 'IsBadWritePtr'

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    for row in [
        (0x100037d3, 'mov', 'eax,dword ptr [esp+8]'), (0x100037d7, 'mov', 'ecx,dword ptr [esp+0Ch]'),
        (0x100037db, 'mov', 'dword ptr ds:[10075158h],eax'), (0x100037e5, 'mov', 'dword ptr ds:[1007515Ch],ecx'),
        (0x100028cb, 'mov', 'ebp,ecx'),
        (0x10002a7c, 'mov', 'edx,dword ptr [ebp+00000128h]'), (0x10002a88, 'push', 'edx'),
        (0x10002b5f, 'mov', 'edx,dword ptr [ebp+00000124h]'), (0x10002b6e, 'push', 'edx'),
        (0x10001523, 'mov', 'eax,dword ptr [esp+5Ch]'), (0x1000152d, 'test', 'eax,eax'),
        (0x10001543, 'jne', '1000154D'),
        (0x10001666, 'mov', 'eax,dword ptr [esp+64h]'), (0x10001670, 'test', 'eax,eax'),
        (0x1000168d, 'jne', '10001697'), (0x1000168f, 'mov', 'dword ptr [esp+28h],280B8h'),
        (0x1000382f, 'mov', 'esi,dword ptr [esp+18h]'), (0x10003833, 'mov', 'edi,dword ptr ds:[10048030h]'),
        (0x1000383c, 'push', '4'), (0x1000383e, 'push', 'esi'), (0x1000383f, 'call', 'edi'),
        (0x10003847, 'mov', 'ebx,dword ptr [esp+18h]'), (0x1000384b, 'push', '4'),
        (0x1000384d, 'push', 'ebx'), (0x1000384e, 'call', 'edi'),
        (0x1000387f, 'mov', 'eax,dword ptr ds:[10075158h]'), (0x10003884, 'mov', 'dword ptr [esi],eax'),
        (0x1000388a, 'mov', 'ecx,dword ptr ds:[1007515Ch]'), (0x10003890, 'mov', 'dword ptr [ebx],ecx')]:
        witness(*row)
    assert 0x10075034 + 0x124 == 0x10075158 and 0x10075034 + 0x128 == 0x1007515c
    chains = []
    for push, call, target, argument in [(0x10002a88, 0x10002abd, 0x100015e0, 7), (0x10002b6e, 0x10002b95, 0x10001490, 5)]:
        sp, address = 0, push
        while address < call:
            row = ins[address]
            witness(address, row['op'], row['args'])
            if row['op'] == 'push':
                sp -= 4
            elif row['op'] == 'call':
                sp += audit.call_pop(address, {r: None for r in REGS})
            else:
                assert row['op'] in ['mov', 'add']
                assert not row['args'].startswith('esp,')
            address += len(row['raw'])
        assert address == call and sp + 4 * (argument - 1) == -4
        witness(call, 'call', f'{target:08X}')
        # Match the final native stack argument load after actual private pushes.
        private_load = 0x10001666 if target == 0x100015e0 else 0x10001523
        sp_private, current = 0, target
        while current < private_load:
            row = ins[current]
            witness(current, row['op'], row['args'])
            if row['op'] == 'push':sp_private -= 4
            elif row['op'] == 'pop':sp_private += 4
            elif row['op'] == 'sub' and row['args'].startswith('esp,'):sp_private -= number(row['args'].split(',')[1])
            assert row['op'] not in ['call', 'jmp']
            current += len(row['raw'])
        displacement = re.search(r'\[esp\+([^\]]+)\]', ins[private_load]['args'])[1]
        assert sp_private + number(displacement) == argument * 4
        chains.append(dict(hardware_push=f'{push:08x}', private_call=f'{call:08x}', actual_private_argument=argument, private_boolean_load=f'{private_load:08x}'))
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    assert 'DLL_APIintSQUALL_SetHardwareAcceleration(intUseHW2D,intUseHW3D);' in header
    assert 'DLL_APIintSQUALL_GetHardwareAcceleration(int*UseHW2D,int*UseHW3D);' in header
    rows = []
    for name, start, status, role in [
        ('SQUALL_SetHardwareAcceleration', 0x100037b0, 'esi', 'Two32-bit Boolean integer words;stored without canonicalizing bits and consumed as zero/nonzero in actual2D/3D buffer creation'),
        ('SQUALL_GetHardwareAcceleration', 0x10003810, 'ebp', 'Two optional writable4-byte integer output pointers;each IsBadWritePtr-checked separately and receives exact Boolean cache word')]:
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == [1, 2]
        todo, seen, returns = [(start, False, False)], set(), set()
        while todo:
            address, initialized, eax_status = todo.pop()
            state = address, initialized, eax_status
            if state in seen:continue
            seen.add(state)
            row = ins[address]
            witness(address, row['op'], row['args'])
            op, args = row['op'], row['args']
            parts = args.split(',')
            if op == 'call':eax_status = False
            elif op == 'mov' and parts[0] == status:
                assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]);initialized = True
            elif op == 'or' and parts == [status, '0FFFFFFFFh']:initialized = True
            elif op == 'pop' and args == status:initialized = False
            elif op == 'mov' and parts[0] == 'eax':eax_status = parts[1] == status and initialized
            elif parts[0] == status and op not in ['test', 'cmp', 'push']:
                raise AssertionError(('Unknown status write', hex(address), row))
            if op == 'ret':
                assert eax_status and not args and row['raw'] == b'\xc3'
                returns.add(f'{address:08x}')
            else:todo += [(n, initialized, eax_status) for n in audit.successors(address)]
        assert returns == set(stack['balanced_return_addresses'])
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=2,
            argument_role=dict(arguments=role, physical_word_bytes=4, source_header_matches=True),
            return_role='Complete initialized32-bit status in EAX on every balanced C3 return',
            all_native_return_sites=sorted(returns), native_signature_fully_verified=True, whole_behavior_fully_verified=False))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 3
    count = publish('squall-hardware-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)], actual_private_argument_chains=chains,
        setter_preserves_noncanonical_boolean_words=True, getter_allows_either_output_independently=True,
        new_runtime_audio_test_claimed=False, dataflow_assumption='Standard Win32 x86 nonvolatile ESI/EBP preservation'))
    print(f'Proved2 native hardware signatures and3 EAX/C3 returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
