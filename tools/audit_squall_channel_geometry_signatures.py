#!/usr/bin/env python3
"""Prove public channel vectors/distances through actual private float use."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish

METHODS = [
    ('Set3DPosition', 0x10006570, 'edi', ['intChannelID', 'float*Position'], 0x10006626, 0x1000663c, 'esi', 1, 9),
    ('Get3DPosition', 0x10006660, 'esi', ['intChannelID', 'float*Position'], 0x10006710, 0x1000671f, 'edi', 1, 8),
    ('SetVelocity', 0x10006740, 'edi', ['intChannelID', 'float*Velocity'], 0x100067f6, 0x1000680c, 'esi', 2, 9),
    ('GetVelocity', 0x10006830, 'esi', ['intChannelID', 'float*Velocity'], 0x100068e0, 0x100068ef, 'edi', 2, 8),
    ('SetMinMaxDistance', 0x10006910, 'esi', ['intChannelID', 'floatMinDist', 'floatMaxDist'], None, None, None, None, None),
    ('GetMinMaxDistance', 0x100069b0, 'ebp', ['intChannelID', 'float*MinDist', 'float*MaxDist'], None, None, None, None, None)]


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

    # Actual private positions/velocity are binary32 vectors. These are the
    # cache words read by the paired private getter, not generic byte buffers.
    for row in [
        (0x10001a82, 'mov', 'edx,dword ptr [esp+0Ch]'), (0x10001ac3, 'mov', 'edx,dword ptr [esp+10h]'),
        (0x10001a8a, 'fld', 'dword ptr [ecx+58h]'), (0x10001a8d, 'fcomp', 'dword ptr [edx]'),
        (0x10001a96, 'fld', 'dword ptr [ecx+5Ch]'), (0x10001a99, 'fcomp', 'dword ptr [edx+4]'),
        (0x10001aa3, 'fld', 'dword ptr [ecx+60h]'), (0x10001aa6, 'fcomp', 'dword ptr [edx+8]'),
        (0x10001acb, 'fld', 'dword ptr [ecx+64h]'), (0x10001ace, 'fcomp', 'dword ptr [edx]'),
        (0x10001ad7, 'fld', 'dword ptr [ecx+68h]'), (0x10001ada, 'fcomp', 'dword ptr [edx+4]'),
        (0x10001ae4, 'fld', 'dword ptr [ecx+6Ch]'), (0x10001ae7, 'fcomp', 'dword ptr [edx+8]'),
        (0x10001c1c, 'mov', 'eax,dword ptr [esp+4]'), (0x10001c35, 'mov', 'eax,dword ptr [esp+8]'),
        (0x10001c24, 'mov', 'edx,dword ptr [ecx+58h]'), (0x10001c27, 'mov', 'dword ptr [eax],edx'),
        (0x10001c29, 'mov', 'edx,dword ptr [ecx+5Ch]'), (0x10001c2c, 'mov', 'dword ptr [eax+4],edx'),
        (0x10001c2f, 'mov', 'edx,dword ptr [ecx+60h]'), (0x10001c32, 'mov', 'dword ptr [eax+8],edx'),
        (0x10001c3d, 'mov', 'edx,dword ptr [ecx+64h]'), (0x10001c40, 'mov', 'dword ptr [eax],edx'),
        (0x10001c42, 'mov', 'edx,dword ptr [ecx+68h]'), (0x10001c45, 'mov', 'dword ptr [eax+4],edx'),
        (0x10001c48, 'mov', 'edx,dword ptr [ecx+6Ch]'), (0x10001c4b, 'mov', 'dword ptr [eax+8],edx'),
        (0x10006962, 'mov', 'edx,dword ptr [esp+10h]'), (0x10006969, 'push', 'ecx'),
        (0x1000696a, 'mov', 'ecx,dword ptr [esp+10h]'), (0x1000696e, 'push', 'edx'), (0x10006972, 'push', 'ecx'),
        (0x10001a70, 'push', 'ebx'), (0x10001a71, 'push', 'esi'),
        (0x10001b87, 'fld', 'dword ptr [esp+24h]'), (0x10001b9e, 'fcomp', 'dword ptr [esp+24h]'),
        (0x10001baf, 'mov', 'dword ptr [ecx+00000088h],eax'),
        (0x10001bb5, 'fld', 'dword ptr [esp+28h]'), (0x10001bcc, 'fcomp', 'dword ptr [esp+28h]'),
        (0x10001bdb, 'mov', 'dword ptr [ecx+0000008Ch],edx'),
        (0x100069dc, 'push', '4'), (0x100069de, 'push', 'esi'), (0x100069df, 'call', 'edi'),
        (0x100069eb, 'push', '4'), (0x100069ed, 'push', 'ebx'), (0x100069ee, 'call', 'edi'),
        (0x10006a4b, 'test', 'esi,esi'), (0x10006a4d, 'je', '10006A67'),
        (0x10006a5e, 'mov', 'ecx,dword ptr [edx+ecx*4+00000088h]'), (0x10006a65, 'mov', 'dword ptr [esi],ecx'),
        (0x10006a67, 'test', 'ebx,ebx'), (0x10006a69, 'je', '10006A82'),
        (0x10006a79, 'mov', 'ecx,dword ptr [eax+edx*4+0000008Ch]'), (0x10006a80, 'mov', 'dword ptr [ebx],ecx')]:
        witness(*row)
    assert audit.imports[0x10048030] == 'IsBadWritePtr'
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for suffix, start, status, params, push_start, private_call, pointer_reg, argument, count in METHODS:
        name = 'SQUALL_Channel_' + suffix
        assert 'DLL_APIint' + name + '(' + ','.join(params) + ');' in header
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == list(range(1, len(params) + 1))
        handle = [r for r in stack['stack_references'] if r['entry_esp_offset'] == 4]
        assert len(handle) == 1 and handle[0]['op'] == 'mov'
        current = int(handle[0]['address'], 16)
        assert ins[current]['args'].startswith('eax,dword ptr [esp+')
        current += len(ins[current]['raw'])
        witness(current, 'mov', 'ecx,10075034h');current += len(ins[current]['raw'])
        witness(current, 'push', 'eax');current += len(ins[current]['raw'])
        witness(current, 'call', '10002480')
        if push_start is not None:
            address, pushed = push_start, []
            while address < private_call:
                row = ins[address]
                witness(address, row['op'], row['args'])
                assert row['op'] in ['mov', 'push']
                if row['op'] == 'push':pushed.append(row['args'])
                address += len(row['raw'])
            assert address == private_call and len(pushed) == count
            assert pushed[-argument] == pointer_reg and pushed.count(pointer_reg) == 1
            witness(private_call, 'call', '10001A70' if count == 9 else '10001C10')
            pointer_checks = [r for r in stack['stack_references'] if r['entry_esp_offset'] == 8]
            assert len(pointer_checks) == 1 and pointer_checks[0]['operands'].startswith(pointer_reg + ',dword ptr [esp+')
            checks = [a for a in ins if start <= a < private_call and ins[a]['op'] == 'push' and ins[a]['args'] == '0Ch']
            assert len(checks) == 1
            at = checks[0] + len(ins[checks[0]]['raw'])
            witness(at, 'push', pointer_reg);at += len(ins[at]['raw'])
            assert audit.call_evidence[f'{at:08x}']['target'] == ('IsBadWritePtr' if suffix.startswith('Get') else 'IsBadReadPtr')
        elif suffix == 'SetMinMaxDistance':
            arguments = {int(r['address'], 16): r['entry_esp_offset'] for r in stack['stack_references']}
            assert arguments[0x1000696a] == 8 and arguments[0x10006962] == 12
            at, pushes = 0x10006969, []
            while at < 0x1000698a:
                row = ins[at]
                assert row['op'] in ['push', 'mov', 'lea']
                if row['op'] == 'push':pushes.append(row['args'])
                at += len(row['raw'])
            assert pushes == ['ecx', 'edx', 'ecx', '0', '0', '0', '0', '0', '0']
            witness(at, 'call', '10001A70')
            assert 0x24 - 8 == 7 * 4 and 0x28 - 8 == 8 * 4
        seen, todo, returns = set(), [(start, False, False)], set()
        while todo:
            address, defined, eax_status = todo.pop()
            state = address, defined, eax_status
            if state in seen:continue
            seen.add(state)
            row = ins[address];op,args = row['op'],row['args'];parts = args.split(',')
            witness(address, op, args)
            if op == 'call':eax_status = False
            elif op == 'mov' and parts[0] == status:
                assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]);defined = True
            elif op == 'or' and parts == [status, '0FFFFFFFFh']:defined = True
            elif op == 'pop' and args == status:defined = False
            elif op == 'mov' and parts[0] == 'eax':eax_status = defined and parts[1] == status
            elif parts[0] == status and op not in ['push', 'cmp', 'test']:
                raise AssertionError(('Unknown status write', hex(address), row))
            if op == 'ret':
                assert eax_status and not args and row['raw'] == b'\xc3';returns.add(f'{address:08x}')
            else:todo += [(n, defined, eax_status) for n in audit.successors(address)]
        assert returns == set(stack['balanced_return_addresses'])
        roles = ['Unchanged32-bit integer channel handle consumed by actual index/generation resolver']
        if 'Distance' in suffix:
            roles += (['Two binary32 float value arguments;actual private argument7/8 FLD m32 and float cache'] if suffix.startswith('Set') else
                      ['Two independently optional writable4-byte binary32 float output pointers;at least one valid output required'])
        else:roles += [('Writable' if suffix.startswith('Get') else 'Readable') + '12-byte pointer to three binary32 float coordinates']
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=len(params),
            argument_role=dict(arguments_in_public_order=roles, source_header_matches=True),
            return_role='Complete initialized32-bit integer status in EAX on every balanced C3 return',
            all_native_return_sites=sorted(returns), definite_assignment_states=len(seen),
            native_signature_fully_verified=True, whole_behavior_fully_verified=False))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 21
    count = publish('squall-channel-geometry-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        private_setter_argument_count=9, private_setter_callee_pop_bytes=audit.cleanup(0x10001a70),
        private_getter_argument_count=8, private_getter_callee_pop_bytes=audit.cleanup(0x10001c10),
        new_runtime_audio_test_claimed=False, dataflow_assumption='Standard Win32 x86 nonvolatile ESI/EDI/EBP preservation'))
    print(f'Proved6 native channel geometry signatures and21 EAX/C3 returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
