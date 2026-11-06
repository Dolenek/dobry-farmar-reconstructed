#!/usr/bin/env python3
"""Native byte string, capability words and six counter output contracts."""
import json
import re
import subprocess
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from squall_native_status import prove_status
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish


def main():
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe)
    audit = Audit(pe, ins)
    executable = ROOT / 'build/squall-public-layout-vs/Release/squall-public-layout.exe'
    layout = json.loads(subprocess.check_output([str(executable)], creationflags=0x08000000))
    assert layout['pointer_bytes'] == layout['int_bytes'] == layout['long_bytes'] == layout['float_bytes'] == 4
    for name, fields in [('device_caps', ['Flags', 'HardwareChannels', 'Hardware3DChannels']),
                         ('channels', ['Play', 'Pause', 'Prepare', 'Play3D', 'Pause3D', 'Prepare3D'])]:
        value = layout['types'][name]
        assert value['size'] == len(fields) * 4
        assert value['fields'] == {field: dict(offset=n * 4, bytes=4, kind='signed', elements=1) for n, field in enumerate(fields)}
    evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    for row in [
        (0x10003c93, 'mov', 'ebp,dword ptr [esp+18h]'), (0x10003c97, 'test', 'ebp,ebp'), (0x10003c99, 'jle', '10003CBC'),
        (0x10003c9b, 'mov', 'esi,dword ptr [esp+10h]'), (0x10003c9f, 'test', 'esi,esi'), (0x10003ca1, 'jl', '10003CBC'),
        (0x10003ca8, 'cmp', 'esi,eax'), (0x10003caa, 'jge', '10003CBC'),
        (0x10003cac, 'mov', 'ebx,dword ptr [esp+14h]'), (0x10003cb0, 'push', 'ebp'), (0x10003cb1, 'push', 'ebx'),
        (0x10003cb2, 'call', 'dword ptr ds:[10048030h]'),
        (0x10003ce5, 'repne', 'scas  byte ptr es:[edi]'), (0x10003cea, 'lea', 'eax,[ebp-1]'),
        (0x10003ced, 'cmp', 'ecx,eax'), (0x10003d14, 'rep', 'movs    byte ptr es:[edi],byte ptr [esi]'),
        (0x10003d18, 'mov', 'byte ptr [eax+ebx],0'),
        (0x10003d48, 'mov', 'edi,dword ptr [esp+10h]'), (0x10003d4c, 'test', 'edi,edi'), (0x10003d4e, 'jl', '10003D6A'),
        (0x10003d55, 'cmp', 'edi,eax'), (0x10003d57, 'jge', '10003D6A'),
        (0x10003d59, 'mov', 'esi,dword ptr [esp+14h]'), (0x10003d5d, 'push', '0Ch'),
        (0x10003d5f, 'push', 'esi'), (0x10003d60, 'call', 'dword ptr ds:[10048030h]'),
        (0x10003d78, 'mov', 'dword ptr [ecx],eax'), (0x10003d7a, 'mov', 'dword ptr [ecx+4],eax'),
        (0x10003d7d, 'mov', 'dword ptr [ecx+8],eax'), (0x10003d93, 'mov', 'dword ptr [esi+4],ecx'),
        (0x10003da4, 'mov', 'dword ptr [esi+8],ecx'), (0x10003d98, 'or', 'ecx,1'), (0x10003da9, 'or', 'ecx,2'),
        (0x10003db8, 'or', 'dword ptr [esi],4'), (0x10003dc5, 'or', 'dword ptr [esi],8'),
        (0x10003dd2, 'or', 'dword ptr [esi],10h'), (0x10003de1, 'or', 'ah,1'),
        (0x10003e69, 'mov', 'esi,dword ptr [esp+10h]'), (0x10003e6d, 'push', '18h'),
        (0x10003e6f, 'push', 'esi'), (0x10003e70, 'call', 'dword ptr ds:[10048030h]'),
        (0x10003e92, 'mov', 'ecx,6'), (0x10003e97, 'xor', 'eax,eax'),
        (0x10003e9b, 'rep', 'stos    dword ptr es:[edi]'),
        (0x10003ed3, 'inc', 'dword ptr [esi]'), (0x10003ee7, 'inc', 'dword ptr [esi+4]'),
        (0x10003efc, 'inc', 'dword ptr [esi+8]'), (0x10003f1e, 'inc', 'dword ptr [esi+0Ch]'),
        (0x10003f33, 'inc', 'dword ptr [esi+10h]'), (0x10003f48, 'inc', 'dword ptr [esi+14h]')]:
        witness(*row)
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    methods = [
        ('GetDeviceName', 0x10003c80, ['intNum', 'char*Buffer', 'intSize'],
         'Signed32 device index0..count-1,writable byte buffer with original NUL terminator,signed positive byte capacity'),
        ('GetDeviceCaps', 0x10003d30, ['intNum', 'squall_device_caps_t*Caps'],
         'Signed32 device index0..count-1,writable12-byte structure:integer flags+0,hardware count+4,3D hardware count+8'),
        ('GetChannelsInfo', 0x10003e40, ['squall_channels_t*Info'],
         'Writable24-byte structure of six consecutive32-bit integer counters,zeroed then independently incremented')]
    for suffix, start, params, role in methods:
        name = 'SQUALL_' + suffix
        assert 'DLL_APIint' + name + '(' + ','.join(params) + ');' in header
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == list(range(1, len(params) + 1))
        if suffix == 'GetChannelsInfo':
            result, extra = prove_status(audit, start, 'ebp');evidence.update(extra)
        else:
            # Device queries also return literal EAX errors directly. Keep
            # these distinct from pointer/strlen/intermediate register words.
            todo, seen, returns = [(start, False, False)], set(), set()
            while todo:
                address, ebx_numeric, eax_numeric = todo.pop()
                state = address, ebx_numeric, eax_numeric
                if state in seen:continue
                seen.add(state)
                row = ins[address];op,args = row['op'],row['args'];parts = args.split(',')
                witness(address, op, args)
                if op == 'call':eax_numeric = False
                elif op == 'mov' and parts[0] == 'ebx':ebx_numeric = bool(re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]))
                elif op == 'pop' and args == 'ebx':ebx_numeric = False
                elif op == 'mov' and parts[0] == 'eax':
                    eax_numeric = parts[1] == 'ebx' and ebx_numeric or bool(re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]))
                elif op == 'xor' and args == 'eax,eax' or op == 'or' and args == 'eax,0FFFFFFFFh':eax_numeric = True
                elif parts[0] in ['eax', 'ax', 'al', 'ah'] and op not in ['push', 'cmp', 'test']:eax_numeric = False
                if op == 'ret':
                    assert eax_numeric and not args and row['raw'] == b'\xc3';returns.add(f'{address:08x}')
                else:todo += [(n, ebx_numeric, eax_numeric) for n in audit.successors(address)]
            result = dict(all_native_return_sites=sorted(returns), definite_assignment_states=len(seen))
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=len(params),
            argument_role=dict(arguments_in_public_order=role, source_header_matches=True),
            return_role='Complete initialized32-bit integer status in EAX on every balanced C3 return',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 10
    count = publish('squall-device-info-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        compiler_layout=dict(path=executable.relative_to(ROOT).as_posix(), sha256=digest(executable), value=layout,
            source_files={n:digest(ROOT / n) for n in ['tools/squall_public_layout.cpp', 'tools/squall_layout/CMakeLists.txt']},
            original_layout_match_claimed_for_all13_types=False),
        status_proof_sha256=digest(ROOT / 'tools/squall_native_status.py'),
        native_string_encoding_claimed_utf8=False, native_string_bytes_not_wide_characters=True,
        new_runtime_audio_test_claimed=False, dataflow_assumption='Standard Win32 x86 nonvolatile and caller-local preservation'))
    print(f'Proved3 native device/channel information signatures and10 EAX/C3 returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
