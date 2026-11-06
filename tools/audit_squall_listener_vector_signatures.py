#!/usr/bin/env python3
"""Prove six listener float-vector pointer interfaces and all integer returns."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit, number
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish

METHODS = [
    ('SetPosition', 0x10004470, ['Position'], 'edi', ['esi'], [0x100750dc], False),
    ('GetPosition', 0x10004560, ['Position'], 'esi', ['edi'], [0x100750dc], True),
    ('SetVelocity', 0x100042d0, ['Velocity'], 'edi', ['esi'], [0x100750e8], False),
    ('GetVelocity', 0x100043d0, ['Velocity'], 'esi', ['edi'], [0x100750e8], True),
    ('SetParameters', 0x10003f80, ['Position', 'Front', 'Top', 'Velocity'], None,
     ['edi', 'ebp', 'edx', 'esi'], [0x100750dc, 0x100750f4, 0x10075100, 0x100750e8], False),
    ('GetParameters', 0x100041a0, ['Position', 'Front', 'Top', 'Velocity'], None,
     ['ebp', 'eax', 'ebx', 'edi'], [0x100750dc, 0x100750f4, 0x10075100, 0x100750e8], True)]


def main():
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe)
    audit = Audit(pe, ins)
    assert audit.imports[0x10048024] == 'IsBadReadPtr' and audit.imports[0x10048030] == 'IsBadWritePtr'
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    evidence = {}
    rows = []
    for suffix, start, params, status, pointer_regs, caches, output in METHODS:
        name = 'SQUALL_Listener_' + suffix
        assert 'DLL_APIint' + name + '(' + ','.join('float*' + p for p in params) + ');' in header
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == list(range(1, len(params) + 1))
        todo, reachable = [start], set()
        while todo:
            address = todo.pop()
            if address in reachable:continue
            reachable.add(address)
            row = ins[address]
            assert row['file_verified']
            evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=row['op'], operands=row['args'])
            todo += audit.successors(address)
        validations = [a for a in reachable if ins[a]['op'] == 'push' and ins[a]['args'] == '0Ch']
        assert len(validations) == len(params)
        for address in validations:
            following = address + len(ins[address]['raw'])
            assert ins[following]['op'] == 'push'
            following += len(ins[following]['raw'])
            assert ins[following]['op'] == 'call'
            assert audit.call_evidence[f'{following:08x}']['target'] == ('IsBadWritePtr' if output else 'IsBadReadPtr')
        vector_witnesses = []
        optional_tests = ({'SetParameters': [(0x10004018, 'edi', 0x10004069), (0x10004069, 'esi', 0x100040ba),
                                             (0x100040ba, 'ebp', 0x1000410d), (0x10004111, 'edx', 0x1000415f)],
                           'GetParameters': [(0x10004232, 'ebp', 0x10004250), (0x10004254, 'eax', 0x10004272),
                                             (0x10004272, 'ebx', 0x1000428f), (0x1000428f, 'edi', 0x100042ac)]}.get(suffix, []))
        for address, register, target in optional_tests:
            assert ins[address]['op'] == 'test' and ins[address]['args'] == register + ',' + register
            branch = ins[address + len(ins[address]['raw'])]
            assert branch['op'] == 'je' and branch['args'] == f'{target:08X}'
        for reg, base in zip(pointer_regs, caches):
            for offset in [0, 4, 8]:
                operand = 'dword ptr [' + reg + ('+' + str(offset) if offset else '') + ']'
                cache = f'dword ptr ds:[{base + offset:08X}h]'
                if output:
                    matches = [a for a in reachable if ins[a]['op'] == 'mov' and ins[a]['args'].split(',')[1:] == [cache]]
                    assert len(matches) == 1
                    address = matches[0]
                    register = ins[address]['args'].split(',')[0]
                    store = ins[address + len(ins[address]['raw'])]
                    assert store['op'] == 'mov' and store['args'] == operand + ',' + register
                    vector_witnesses.append(dict(cache_word=f'{base+offset:08x}', actual_copy_load=f'{address:08x}', pointer_operand=operand))
                else:
                    matches = [a for a in reachable if ins[a]['op'] == 'fld' and ins[a]['args'] == operand]
                    assert len(matches) == 1
                    address = matches[0]
                    comparison = ins[address + len(ins[address]['raw'])]
                    assert comparison['op'] == 'fcomp' and comparison['args'] == cache
                    vector_witnesses.append(dict(cache_float=f'{base+offset:08x}', actual_float_load=f'{address:08x}', pointer_operand=operand))
        # Trace only the initialized integer status, including stack-local
        # status in the four-pointer methods. Argument zeroing is a different
        # stack slot, and must never establish the return value.
        seen, todo, returns = set(), [(start, 0, False, False)], set()
        while todo:
            address, sp, defined, eax_status = todo.pop()
            state = address, sp, defined, eax_status
            if state in seen:continue
            seen.add(state)
            row = ins[address]
            op, args = row['op'], row['args']
            parts = args.split(',')
            if op == 'call':
                sp += audit.call_evidence[f'{address:08x}']['callee_pop_bytes']
                eax_status = False
            elif op == 'push':sp -= 4
            elif op == 'pop':
                sp += 4
                if status is not None and args == status:defined = False
                if args == 'eax':eax_status = False
            elif op == 'add' and parts[0] == 'esp':sp += number(parts[1])
            elif status is not None and op == 'mov' and parts[0] == status:
                assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]);defined = True
            elif status is not None and op == 'or' and parts == [status, '0FFFFFFFFh']:defined = True
            elif op == 'mov':
                if status is None:
                    match = re.fullmatch(r'dword ptr \[esp(?:\+([0-9A-F]+h|[0-9]+))?\]', parts[0])
                    if match and sp + (number(match[1]) if match[1] else 0) == -4:
                        assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]);defined = True
                if parts[0] == 'eax':
                    match = re.fullmatch(r'dword ptr \[esp(?:\+([0-9A-F]+h|[0-9]+))?\]', parts[1])
                    eax_status = defined and (parts[1] == status if status is not None else
                        bool(match and sp + (number(match[1]) if match[1] else 0) == -4))
            elif status is not None and parts[0] == status and op not in ['test', 'cmp', 'push']:
                raise AssertionError(('Unenumerated status write', hex(address), row))
            elif (parts[0] in ['eax', 'ax', 'al', 'ah'] or op == 'fnstsw') and op not in ['test', 'cmp', 'push']:
                eax_status = False
            if op == 'ret':
                assert sp == 0 and eax_status and not args and row['raw'] == b'\xc3', (hex(address), state)
                returns.add(f'{address:08x}')
            else:todo += [(n, sp, defined, eax_status) for n in audit.successors(address)]
        assert returns == set(stack['balanced_return_addresses'])
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=len(params),
            argument_role=dict(arguments_in_public_order=params, pointer_direction='output' if output else 'input',
                               pointer_payload='3 consecutive binary32 float words per vector', bytes_per_vector=12,
                               pointers_independently_optional=len(params) == 4,
                               original_float_load_and_cache_evidence=vector_witnesses, source_header_matches=True),
            return_role='Initialized32-bit integer status in EAX on every balanced C3 return,never a floating-point return',
            all_native_return_sites=sorted(returns), definite_assignment_states=len(seen),
            native_signature_fully_verified=True, whole_behavior_fully_verified=False))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 14
    count = publish('squall-listener-vector-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        getter_type_evidence='Same12-byte caches are read by native FLD m32/FCOMP m32 in paired setters',
        whole_float_bit_space_behavior_claimed=False, new_runtime_audio_test_claimed=False,
        dataflow_assumption='Standard Win32 x86 nonvolatile status and caller-local stack preservation'))
    print(f'Proved6 native listener float-vector signatures and14 EAX/C3 returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
