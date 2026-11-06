#!/usr/bin/env python3
"""Prove original sample ID encoding, narrow filename and mixed28-byte defaults."""
import re
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from squall_integer_word_flow import prove_integer_word
from record_live_production_verification import ROOT, digest, read
from squall_signature_evidence import publish


def main():
    native = ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    pe = PE(native);path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe);audit = Audit(pe, ins);evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    layout_path = ROOT / 'analysis/squall-device-info-signatures.json'
    metadata = read(layout_path)['details']['compiler_layout']
    assert digest(ROOT / metadata['path']) == metadata['sha256']
    layout = metadata['value']['types']['sample_default']
    assert layout['size'] == 28
    assert layout['fields'] == {name: dict(offset=i*4, bytes=4, kind='signed' if i<5 else 'float', elements=1)
        for i, name in enumerate(['SampleGroupID', 'Priority', 'Frequency', 'Volume', 'Pan', 'MinDist', 'MaxDist'])}
    for row in [
        (0x10008000, 'mov', 'edx,dword ptr [esp+4]'), (0x1000800c, 'and', 'eax,0FFFFh'),
        (0x1000801e, 'sar', 'edx,10h'), (0x10008024, 'cmp', 'byte ptr [ecx],dl'),
        (0x10008219, 'xor', 'eax,eax'), (0x1000821b, 'mov', 'al,byte ptr [ecx]'),
        (0x10008221, 'shl', 'eax,10h'), (0x10008224, 'or', 'eax,esi'),
        (0x100080dc, 'cmp', 'eax,0FFFFh'),
        (0x1000737d, 'push', '1'), (0x1000737f, 'push', 'esi'), (0x10007380, 'call', 'ebx'),
        (0x10007388, 'mov', 'edi,dword ptr [esp+18h]'), (0x1000738c, 'push', '1Ch'),
        (0x1000738e, 'push', 'edi'), (0x1000738f, 'call', 'ebx'),
        (0x1000739b, 'mov', 'eax,dword ptr [esp+14h]'), (0x1000739f, 'push', 'edi'),
        (0x100073a0, 'push', 'eax'), (0x100073a1, 'push', 'esi'), (0x100073a2, 'call', '10008040'),
        (0x100073aa, 'test', 'eax,eax'), (0x100073ac, 'jg', '100073B3'),
        (0x10008090, 'mov', 'dl,byte ptr [ecx]'), (0x10008094, 'cmp', 'dl,byte ptr [esi]'),
        (0x1000816e, 'mov', 'edx,dword ptr [esp+28h]'), (0x10008172, 'mov', 'eax,dword ptr [esp+24h]'),
        (0x10008176, 'push', 'edx'), (0x10008177, 'push', 'eax'), (0x10008182, 'call', '1002DEA0'),
        (0x100074a6, 'push', '1Ch'), (0x100074a8, 'push', 'esi'), (0x100074ae, 'call', 'dword ptr ds:[10048030h]'),
        (0x100074c8, 'push', 'esi'), (0x100074c9, 'push', 'eax'), (0x100074ca, 'call', '100083C0'),
        (0x100074e6, 'push', '1Ch'), (0x100074e8, 'push', 'esi'), (0x100074ee, 'call', 'dword ptr ds:[10048030h]'),
        (0x10007504, 'push', 'esi'), (0x10007505, 'push', 'eax'), (0x10007506, 'call', '10008410'),
        (0x10007f21, 'mov', 'esi,dword ptr [esp+8]'), (0x10007f29, 'mov', 'edx,dword ptr [esp+0Ch]'),
        (0x10007f2e, 'mov', 'ecx,7'), (0x10007f35, 'rep', 'movs    dword ptr es:[edi],dword ptr [esi]'),
        (0x10007f37, 'mov', 'eax,dword ptr [edx+8]'), (0x10007f4d, 'cmp', 'eax,ecx'),
        (0x10007f64, 'cmp', 'dword ptr [edx+0Ch],ecx'), (0x10007f6c, 'cmp', 'dword ptr [edx+10h],ecx'),
        (0x10007f78, 'fld', 'dword ptr [edx+14h]'), (0x10007f7b, 'fcomp', 'dword ptr ds:[10048170h]'),
        (0x10007f8f, 'fld', 'dword ptr [edx+18h]'), (0x10007f92, 'fcomp', 'dword ptr ds:[10048170h]'),
        (0x100083c9, 'mov', 'eax,dword ptr [esp+0Ch]'), (0x100083ce, 'call', '10008000'),
        (0x100083e7, 'mov', 'edx,dword ptr [esp+10h]'), (0x100083f0, 'push', 'edx'), (0x100083f1, 'call', '10007F20'),
        (0x10008418, 'mov', 'eax,dword ptr [esp+8]'), (0x1000841d, 'call', '10008000'),
        (0x1000842a, 'mov', 'edi,dword ptr [esp+10h]'), (0x10008431, 'mov', 'ecx,7'),
        (0x10008436, 'rep', 'movs    dword ptr es:[edi],dword ptr [esi]')]:
        witness(*row)
    # Verify every normal backend return as an integer, including encoded IDs.
    backend, extra = prove_integer_word(audit, 0x10008040);evidence.update(extra)
    assert len(backend['all_native_return_sites']) == 3
    assert all(ins[int(a, 16)]['raw'] == b'\xc3' for a in backend['all_native_return_sites'])
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for name, start, params, role in [
        ('SQUALL_Sample_LoadFile', 0x10007370, ['char*', 'int', 'squall_sample_default_t*'],
         'NUL-terminated byte filename pointer(first byte checked),32-bit memory-mode word forwarded unchanged,optional readable28-byte mixed default structure'),
        ('SQUALL_Sample_SetDefault', 0x100074a0, ['int', 'squall_sample_default_t*'],
         'Encoded32-bit integer sample ID and28-byte mixed input pointer;original requires writability even though normalizer copies into library record'),
        ('SQUALL_Sample_GetDefault', 0x100074e0, ['int', 'squall_sample_default_t*'],
         'Encoded32-bit integer sample ID and writable28-byte mixed default output pointer copied as7 DWORD')]:
        assert re.search('DLL_APIint' + name + r'\(' + ','.join(re.escape(t) + r'\w+' for t in params) + r'\);', header)
        stack = audit.function(start);assert stack['explicit_argument_slots'] == list(range(1, len(params) + 1))
        result, extra = prove_integer_word(audit, start, numeric_calls=[0x10008040]);evidence.update(extra)
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=len(params),
            argument_role=dict(arguments_in_public_order=role, source_header_matches=True),
            return_role='Complete32-bit integer sample ID/status in EAX on every balanced C3 return',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 5
    count = publish('squall-sample-load-default-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        compiled_default_layout=layout, compiled_layout_report='analysis/squall-device-info-signatures.json', compiled_layout_report_sha256=digest(layout_path),
        backend_integer_return_proof=backend, integer_word_proof_sha256=digest(ROOT / 'tools/squall_integer_word_flow.py'),
        native_sample_id_low_index_bits=16, native_sample_id_generation_byte_shift=16,
        full_filename_memory_extent_validated_by_native_claimed=False, setter_writability_check_preserved=True,
        new_runtime_audio_or_loader_test_claimed=False, dataflow_assumption='Standard Win32 x86 nonvolatile preservation;actual sample slot count is an integer'))
    print(f'Proved3 native sample load/default signatures,5 public returns and3 encoded-ID backend returns;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
