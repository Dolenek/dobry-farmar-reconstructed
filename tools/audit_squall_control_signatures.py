#!/usr/bin/env python3
"""Native Boolean/group/preset argument roles and complete numeric status returns."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit, number, REGS
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish

METHODS = [
    ('Pause', 0x10003610, ['Pause'], '32-bit Boolean integer;unchanged channel Pause argument consumed as zero/nonzero', [
        (0x1000364c, 'mov', 'ebp,dword ptr [esp+10h]'), (0x10003663, 'push', 'ebp'),
        (0x10003664, 'call', '10002260'), (0x1000226e, 'mov', 'ecx,dword ptr [esp+8]'),
        (0x10002272, 'test', 'ecx,ecx'), (0x10002277, 'je', '100022B1')]),
    ('Listener_EAX_SetPreset', 0x10004c30, ['Preset'], 'Signed32 preset;increment then signed bounds0..26,public ordinary range-1..25', [
        (0x10004c62, 'mov', 'eax,dword ptr [esp+8]'), (0x10004c66, 'inc', 'eax'),
        (0x10004c6b, 'test', 'eax,eax'), (0x10004c6d, 'jl', '10004C74'),
        (0x10004c6f, 'cmp', 'eax,1Ah'), (0x10004c72, 'jle', '10004C8A'), (0x10004c8a, 'push', 'eax')]),
    ('ChannelGroup_Pause', 0x10007100, ['ChannelGroupID', 'Pause'], 'Integer group equality selector and32-bit Boolean pause forwarded unchanged', [
        (0x1000713c, 'mov', 'ebx,dword ptr [esp+10h]'), (0x10007141, 'mov', 'ebp,dword ptr [esp+18h]'),
        (0x10007150, 'mov', 'eax,dword ptr [esi+eax+40h]'), (0x10007154, 'cmp', 'eax,ebx'),
        (0x1000715e, 'push', 'ebp'), (0x1000715f, 'call', '10002260')]),
    ('ChannelGroup_Stop', 0x10007190, ['ChannelGroupID'], '32-bit integer group equality selector', [
        (0x100071cc, 'mov', 'ebp,dword ptr [esp+10h]'),
        (0x100071db, 'mov', 'eax,dword ptr [esi+eax+40h]'), (0x100071df, 'cmp', 'eax,ebp'),
        (0x100071e9, 'call', '10001F40')]),
    ('ChannelGroup_SetVolume', 0x10007220, ['ChannelGroupID', 'Volume'], 'Integer group equality selector and integer volume table index;original has no per-channel0..100 guard here', [
        (0x1000725c, 'mov', 'ebx,dword ptr [esp+10h]'), (0x10007261, 'mov', 'ebp,dword ptr [esp+18h]'),
        (0x10007270, 'mov', 'eax,dword ptr [esi+eax+40h]'), (0x10007274, 'cmp', 'eax,ebx'),
        (0x1000727e, 'mov', 'edx,dword ptr [ebp*4+10074824h]'), (0x10007285, 'push', 'edx')]),
    ('ChannelGroup_SetFrequency', 0x100072c0, ['ChannelGroupID', 'Frequency'], 'Integer group equality selector and signed32 frequency100..100000', [
        (0x100072f1, 'mov', 'ebx,dword ptr [esp+14h]'), (0x100072f5, 'cmp', 'ebx,64h'),
        (0x100072f8, 'jl', '10007302'), (0x100072fa, 'cmp', 'ebx,186A0h'),
        (0x10007300, 'jle', '1000730C'), (0x10007318, 'mov', 'ebp,dword ptr [esp+14h]'),
        (0x10007327, 'mov', 'eax,dword ptr [esi+eax+40h]'), (0x1000732b, 'cmp', 'eax,ebp'),
        (0x10007335, 'push', 'ebx')])]


def main():
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe)
    audit = Audit(pe, ins)
    evidence = {}

    def witness(address, op, args):
        r = ins[address]
        assert r['file_verified'] and r['op'] == op and r['args'] == args, (hex(address), r)
        evidence[address] = dict(address=f'{address:08x}', bytes=r['raw'].hex(), op=op, operands=args)

    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for suffix, start, params, role, witnesses in METHODS:
        name = 'SQUALL_' + suffix
        assert 'DLL_APIint' + name + '(' + ','.join('int' + p for p in params) + ');' in header
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == list(range(1, len(params) + 1))
        for row in witnesses:
            witness(*row)
        # All groups/global Pause keep status in the single DWORD local at
        # entryESP-4. Preset keeps it in ESI. Never infer it from stale EAX.
        preset = suffix == 'Listener_EAX_SetPreset'
        todo, seen, returns = [(start, 0, False, False)], set(), set()
        while todo:
            address, sp, defined, eax_status = todo.pop()
            state = address, sp, defined, eax_status
            if state in seen:
                continue
            seen.add(state)
            r = ins[address]
            witness(address, r['op'], r['args'])
            op, args = r['op'], r['args']
            parts = args.split(',')
            if op == 'call':
                sp += audit.call_pop(address, {reg: None for reg in REGS})
                eax_status = False
            elif op == 'push':
                sp -= 4
            elif op == 'pop':
                sp += 4
                if preset and args == 'esi':
                    defined = False
                if args == 'eax':
                    eax_status = False
            elif op == 'add' and parts[0] == 'esp':
                sp += number(parts[1])
            elif preset and op == 'mov' and parts[0] == 'esi':
                assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1])
                defined = True
            elif preset and op == 'or' and parts == ['esi', '0FFFFFFFFh']:
                defined = True
            elif op == 'mov':
                if not preset:
                    match = re.fullmatch(r'dword ptr \[esp(?:\+([0-9A-F]+h|[0-9]+))?\]', parts[0])
                    if match:
                        assert sp + (number(match[1]) if match[1] else 0) == -4
                        assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1])
                        defined = True
                if parts[0] == 'eax':
                    match = re.fullmatch(r'dword ptr \[esp(?:\+([0-9A-F]+h|[0-9]+))?\]', parts[1])
                    eax_status = defined and (parts[1] == 'esi' if preset else
                        bool(match and sp + (number(match[1]) if match[1] else 0) == -4))
            elif preset and parts[0] == 'esi' and op not in ['test', 'cmp', 'push']:
                raise AssertionError(('Unknown native status write', hex(address), r))
            elif parts[0] in ['eax', 'al', 'ah'] and op not in ['test', 'cmp', 'push']:
                eax_status = False
            if op == 'ret':
                assert sp == 0 and eax_status and not args and r['raw'] == b'\xc3', (hex(address), state)
                returns.add(f'{address:08x}')
            else:
                todo += [(n, sp, defined, eax_status) for n in audit.successors(address)]
        assert returns == set(stack['balanced_return_addresses'])
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=len(params),
            argument_role=dict(arguments=role, integer_word_bytes=4, source_header_matches=True),
            return_role='Complete initialized32-bit integer status into EAX on all balanced C3 returns',
            all_native_return_sites=sorted(returns), numeric_status_states=len(seen),
            native_signature_fully_verified=True, whole_behavior_fully_verified=False))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 8
    count = publish('squall-control-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        native_group_volume_missing_bounds_guard_preserved=True, new_runtime_audio_test_claimed=False,
        dataflow_assumption='Standard Win32 x86 nonvolatile ESI and caller-local stack preservation',
        behavior_evidence_separate_from_abi_semantics=True))
    print(f'Proved6 native scalar control signatures and8 EAX/C3 returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':
    main()
