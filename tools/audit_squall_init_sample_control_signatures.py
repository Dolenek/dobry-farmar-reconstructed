#!/usr/bin/env python3
"""Prove original52-byte mixed initialization and integer sample selectors."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from squall_native_status import prove_status
from record_live_production_verification import ROOT, digest, read
from squall_signature_evidence import publish


def main():
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll')
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe);audit = Audit(pe, ins);evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    layout_report = ROOT / 'analysis/squall-device-info-signatures.json'
    metadata = read(layout_report)['details']['compiler_layout']
    assert digest(ROOT / metadata['path']) == metadata['sha256']
    assert all(digest(ROOT / n) == sha for n, sha in metadata['source_files'].items())
    layout = metadata['value']['types']['parameters']
    names = ['Window', 'Device', 'SampleRate', 'BitPerSample', 'Channels', 'UseHW2D', 'UseHW3D', 'UseAlg', 'BufferSize', 'ListenerMode', 'DistanceFactor', 'RolloffFactor', 'DopplerFactor']
    assert layout['size'] == 52
    assert layout['fields'] == {name: dict(offset=n * 4, bytes=4, kind='pointer' if n == 0 else 'float' if n >= 10 else 'signed', elements=1) for n, name in enumerate(names)}
    assert audit.imports[0x10048024] == 'IsBadReadPtr' and audit.imports[0x10048148] == 'IsWindow'
    for row in [
        (0x100032f1, 'mov', 'ecx,dword ptr [esp+4Ch]'), (0x100032f5, 'lea', 'eax,[esp+14h]'),
        (0x100032f9, 'push', 'eax'), (0x100032fa, 'push', 'ecx'), (0x100032fb, 'call', '100030A0'),
        (0x100030a2, 'mov', 'esi,dword ptr [esp+0Ch]'), (0x100030a7, 'push', '34h'),
        (0x100030a9, 'push', 'esi'), (0x100030aa, 'call', 'dword ptr ds:[10048024h]'),
        (0x100030b2, 'mov', 'ecx,0Dh'), (0x100030b7, 'je', '100030C5'),
        (0x100030c1, 'rep', 'stos    dword ptr es:[edi]'), (0x100030cb, 'rep', 'movs    dword ptr es:[edi],dword ptr [esi]'),
        (0x100030cd, 'mov', 'eax,dword ptr [ebx+8]'), (0x100030d4, 'jg', '100030DF'),
        (0x100030ed, 'mov', 'eax,dword ptr [ebx+0Ch]'), (0x100030f5, 'cmp', 'eax,8'), (0x100030fa, 'cmp', 'eax,ecx'),
        (0x10003101, 'mov', 'eax,dword ptr [ebx+10h]'), (0x10003106, 'jg', '1000310D'),
        (0x1000311b, 'mov', 'eax,dword ptr [ebx+20h]'), (0x10003120, 'jg', '1000312B'),
        (0x10003139, 'mov', 'eax,dword ptr [ebx+1Ch]'), (0x1000313e, 'jl', '10003145'),
        (0x10003148, 'mov', 'eax,dword ptr [ebx+24h]'), (0x10003150, 'cmp', 'eax,edi'), (0x10003154, 'cmp', 'eax,esi'),
        (0x1000315b, 'fld', 'dword ptr [ebx+28h]'), (0x1000315e, 'fcomp', 'dword ptr ds:[10048170h]'),
        (0x1000318d, 'fld', 'dword ptr [ebx+2Ch]'), (0x10003190, 'fcomp', 'dword ptr ds:[10048170h]'),
        (0x100031b5, 'fld', 'dword ptr [ebx+30h]'), (0x100031b8, 'fcomp', 'dword ptr ds:[10048170h]'),
        (0x100031dd, 'mov', 'eax,dword ptr [ebx+14h]'), (0x100031e0, 'cmp', 'eax,edi'), (0x100031e4, 'cmp', 'eax,esi'),
        (0x100031eb, 'mov', 'eax,dword ptr [ebx+18h]'), (0x100031ee, 'cmp', 'eax,edi'), (0x100031f2, 'cmp', 'eax,esi'),
        (0x100031f9, 'mov', 'eax,dword ptr [ebx]'), (0x100031fb, 'push', 'eax'), (0x100031fc, 'call', 'dword ptr ds:[10048148h]'),
        (0x10003208, 'cmp', 'dword ptr [ebx+4],edi'), (0x1000320b, 'jle', '10003217'),
        (0x10003212, 'cmp', 'dword ptr [ebx+4],eax'), (0x10003215, 'jl', '1000321A'),
        (0x1000778c, 'mov', 'ebp,dword ptr [esp+14h]'), (0x100077a3, 'mov', 'edx,dword ptr [esp+14h]'),
        (0x100077a7, 'mov', 'eax,dword ptr [ecx+4Ch]'), (0x100077aa, 'cmp', 'eax,edx'),
        (0x100077ae, 'push', 'ebp'), (0x100077af, 'call', '10002260'),
        (0x1000226e, 'mov', 'ecx,dword ptr [esp+8]'), (0x10002272, 'test', 'ecx,ecx'),
        (0x1000781c, 'mov', 'ebp,dword ptr [esp+10h]'),
        (0x10007833, 'cmp', 'dword ptr [ecx+4Ch],ebp'), (0x10007838, 'call', '10001F40')]:
        witness(*row)
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for name, start, status, params, role in [
        ('SQUALL_Init', 0x10003280, 'ebp', ['squall_parameters_t*'],
         'Input pointer to52-byte mixed structure:32-bit Window handle,nine signed integer/Boolean settings,three binary32 factors;invalid/null input uses original defaults in private local copy'),
        ('SQUALL_Sample_Pause', 0x10007750, None, ['int', 'int'],
         '32-bit integer sample equality selector against channel cache+4c,then32-bit zero/nonzero Pause passed unchanged'),
        ('SQUALL_Sample_Stop', 0x100077e0, None, ['int'],
         '32-bit integer sample equality selector against channel cache+4c;matching active channels invoke actual stop')]:
        assert re.search('DLL_APIint' + name + r'\(' + ','.join(re.escape(t) + r'\w+' for t in params) + r'\);', header)
        stack = audit.function(start);assert stack['explicit_argument_slots'] == list(range(1, len(params) + 1))
        result, extra = prove_status(audit, start, status);evidence.update(extra)
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=len(params),
            argument_role=dict(arguments_in_public_order=role, source_header_matches=True),
            return_role='Complete initialized32-bit status in EAX on every balanced C3 return',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 3
    count = publish('squall-init-sample-control-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        current_layout_report='analysis/squall-device-info-signatures.json', current_layout_report_sha256=digest(layout_report),
        source_parameters_layout=layout, status_proof_sha256=digest(ROOT / 'tools/squall_native_status.py'),
        original_invalid_init_pointer_defaulting_preserved=True, original_callers_parameter_structure_written=False,
        new_runtime_audio_or_initialization_test_claimed=False, dataflow_assumption='Standard Win32 x86 nonvolatile status and caller-local preservation'))
    print(f'Proved52-byte Init and2 sample-control signatures with3 EAX/C3 returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
