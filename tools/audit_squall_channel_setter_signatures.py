#!/usr/bin/env python3
"""Prove eight original integer channel setters without relying on API names."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish

# Parameter roles follow actual signed comparisons, arithmetic, table indices
# and Boolean/bit-field consumption. Every listed instruction is PE verified.
METHODS = [
    ('Pause', 0x100053a0, 'esi', 'Pause', '32-bit Boolean integer consumed as zero/nonzero by actual private pause method', [
        (0x100053ec, 'mov', 'ecx,dword ptr [esp+0Ch]'), (0x100053f3, 'push', 'ecx'),
        (0x10005402, 'call', '10002260'), (0x1000226e, 'mov', 'ecx,dword ptr [esp+8]'),
        (0x10002272, 'test', 'ecx,ecx'), (0x10002277, 'je', '100022B1')]),
    ('SetVolume', 0x10005570, 'edi', 'Volume', 'Signed32 integer volume;bounds0..100 then scaled four-byte table index', [
        (0x10005594, 'mov', 'esi,dword ptr [esp+10h]'), (0x10005598, 'test', 'esi,esi'),
        (0x1000559a, 'jl', '100055A1'), (0x1000559c, 'cmp', 'esi,64h'),
        (0x1000559f, 'jle', '100055B8'), (0x100055e2, 'mov', 'ecx,dword ptr [esi*4+10074824h]')]),
    ('SetFrequency', 0x100056f0, 'edi', 'Frequency', 'Signed32 integer frequency;bounds100..100000', [
        (0x10005714, 'mov', 'esi,dword ptr [esp+10h]'), (0x10005718, 'cmp', 'esi,64h'),
        (0x1000571b, 'jl', '10005725'), (0x1000571d, 'cmp', 'esi,186A0h'),
        (0x10005723, 'jle', '1000573C'), (0x1000576f, 'push', 'esi'), (0x10005779, 'call', '10001FE0')]),
    ('SetPlayPosition', 0x10005840, 'esi', 'Position', '32-bit integer sample position;forwarded unchanged then added into numeric position cache', [
        (0x1000588c, 'mov', 'ecx,dword ptr [esp+0Ch]'), (0x10005893, 'push', 'ecx'),
        (0x100058a2, 'call', '100020D0'), (0x1000212d, 'mov', 'eax,dword ptr [esp+8]'),
        (0x10002131, 'add', 'edx,eax'), (0x10002137, 'mov', 'dword ptr [esi+30h],edx')]),
    ('SetPlayPositionMs', 0x10005970, 'edi', 'Position', 'Signed32 integer milliseconds;actual signed IMUL and64-bit integer division by1000', [
        (0x100059c7, 'push', '0'), (0x100059c9, 'push', '3E8h'),
        (0x100059db, 'imul', 'dword ptr [esp+18h]'), (0x100059df, 'push', 'edx'),
        (0x100059e0, 'push', 'eax'), (0x100059e1, 'call', '1003D280'),
        (0x100059e6, 'cmp', 'dword ptr [esi+34h],eax'), (0x100059e9, 'jge', '10005A02')]),
    ('SetPriority', 0x10006170, 'edi', 'Priority', 'Signed32 integer priority;negative rejected and complete DWORD stored in priority cache', [
        (0x10006195, 'mov', 'esi,dword ptr [esp+10h]'), (0x10006199, 'test', 'esi,esi'),
        (0x1000619b, 'jge', '100061B4'), (0x100061f2, 'mov', 'dword ptr [edx+ecx*4+44h],esi')]),
    ('SetLoop', 0x10006280, 'edi', 'Loop', '32-bit integer loop flag;only low bit retained and inserted into channel bit fields', [
        (0x100062ed, 'mov', 'eax,dword ptr [esp+14h]'), (0x100062f7, 'and', 'eax,1'),
        (0x100062fe, 'mov', 'edx,eax'), (0x10006300, 'shl', 'edx,8'),
        (0x10006306, 'or', 'edx,ebx'), (0x10006308, 'mov', 'dword ptr [esi+ecx+48h],edx'),
        (0x1000631e, 'shl', 'eax,9'), (0x10006321, 'or', 'ecx,eax')]),
    ('SetPan', 0x100063d0, 'esi', 'Pan', 'Signed32 integer pan;bounds0..100 then actual private numeric pan conversion', [
        (0x100063f8, 'mov', 'edi,dword ptr [esp+10h]'), (0x100063fc, 'test', 'edi,edi'),
        (0x100063fe, 'jl', '10006405'), (0x10006400, 'cmp', 'edi,64h'),
        (0x10006403, 'jle', '1000641C'), (0x1000647c, 'push', 'edi'), (0x1000647d, 'call', '10002040')])]


def main():
    native = ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    pe = PE(native)
    ins = instructions(path, pe)
    audit = Audit(pe, ins)
    evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    for row in [(0x10002480, 'mov', 'edx,dword ptr [esp+4]'), (0x1000248b, 'mov', 'eax,edx'),
                (0x1000248d, 'and', 'eax,0FFh'), (0x1000249f, 'sar', 'edx,8'),
                (0x100024af, 'cmp', 'ecx,edx'), (0x100024b9, 'or', 'eax,0FFFFFFFFh'),
                (0x100024bd, 'ret', '4')]:
        witness(*row)
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf-8'))
    rows = []
    for suffix, start, status, param, role, witnesses in METHODS:
        name = 'SQUALL_Channel_' + suffix
        assert 'DLL_APIint' + name + '(intChannelID,int' + param + ');' in header
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == [1, 2]
        for row in witnesses:
            witness(*row)
        handle_refs = [r for r in stack['stack_references'] if r['entry_esp_offset'] == 4]
        assert len(handle_refs) == 1 and handle_refs[0]['bytes'] == 4 and handle_refs[0]['op'] == 'mov'
        load = int(handle_refs[0]['address'], 16)
        assert ins[load]['args'].startswith('eax,dword ptr [esp+')
        current = load + len(ins[load]['raw'])
        assert ins[current]['op'] == 'mov' and ins[current]['args'] == 'ecx,10075034h'
        current += len(ins[current]['raw'])
        assert ins[current]['op'] == 'push' and ins[current]['args'] == 'eax'
        current += len(ins[current]['raw'])
        witness(current, 'call', '10002480')
        seen, todo, returns, constants = set(), [(start, False, False)], set(), set()
        while todo:
            address, initialized, eax_status = todo.pop()
            state = address, initialized, eax_status
            if state in seen:
                continue
            seen.add(state)
            row = ins[address]
            witness(address, row['op'], row['args'])
            op, args = row['op'], row['args']
            parts = args.split(',')
            if op == 'call':
                eax_status = False
            elif op == 'mov' and parts[0] == status:
                assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1])
                constants.add(parts[1])
                initialized = True
            elif op == 'or' and parts == [status, '0FFFFFFFFh']:
                constants.add('0FFFFFFFFh')
                initialized = True
            elif op == 'pop' and args == status:
                initialized = False
            elif op == 'mov' and parts[0] == 'eax':
                eax_status = parts[1] == status and initialized
            elif parts[0] == status and op not in ['test', 'cmp', 'push']:
                raise AssertionError(('Unenumerated status write', name, hex(address), row))
            if op == 'ret':
                assert not args and row['raw'] == b'\xc3' and eax_status
                returns.add(f'{address:08x}')
            else:
                todo += [(n, initialized, eax_status) for n in audit.successors(address)]
        assert returns == set(stack['balanced_return_addresses'])
        rows.append(dict(name=name, original_address=f'{start:08x}', argument_slots=2, calling_convention='cdecl',
            argument_role=dict(arguments_in_public_order=['Unchanged32-bit integer ChannelID;actual bit/generation resolver', role],
                               integer_bytes=4, source_header_matches=True),
            return_role='Complete initialized32-bit integer status from enumerated nonvolatile register values into EAX on every C3 return',
            all_native_return_sites=sorted(returns), native_status_immediates=sorted(constants), definite_assignment_states=len(seen),
            native_signature_fully_verified=True, whole_behavior_fully_verified=False))
    assert sum(len(row['all_native_return_sites']) for row in rows) == 23
    count = publish('squall-channel-setter-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        source_header_matches=True, new_runtime_audio_test_claimed=False,
        native_set_play_position_ms_inverted_private_status_preserved=True,
        dataflow_assumption='Standard Win32 x86 nonvolatile ESI/EDI preservation across native calls',
        behavior_evidence_separate_from_abi_semantics=True))
    print(f'Proved8 native integer channel setters and23 EAX/C3 return sites;{count}/97 ABI semantics verified.')


if __name__ == '__main__':
    main()
