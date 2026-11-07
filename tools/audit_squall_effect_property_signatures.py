#!/usr/bin/env python3
"""Prove six opaque property interfaces, exact unions and original wire tables."""
import re
import struct
import uuid
from pe import PE
from audit_squall_native_arguments import instructions, Audit, number
from squall_native_status import prove_status
from record_live_production_verification import ROOT, digest, read
from squall_signature_evidence import publish

METHODS = [
    ('Listener_EAX_SetProperties', 0x10004a40, 'ebp', ['int', 'squall_eax_listener_t*'], 112, 0x10004a9b, False, 0x10004acb, 28),
    ('Listener_EAX_GetProperties', 0x10004b30, 'ebx', ['int', 'squall_eax_listener_t*'], 112, 0x10004b8b, True, 0x10004bf7, 28),
    ('Channel_EAX_SetProperties', 0x10006cc0, 'ebp', ['int', 'int', 'squall_eax_channel_t*'], 72, 0x10006ce5, False, 0x10006d92, 18),
    ('Channel_EAX_GetProperties', 0x10006dd0, 'ebp', ['int', 'int', 'squall_eax_channel_t*'], 72, 0x10006e0b, True, 0x10006ed8, 18),
    ('Channel_ZOOMFX_SetProperties', 0x10006ef0, 'ebx', ['int', 'squall_zoomfx_channel_t*'], 52, 0x10006f14, False, 0x10006faf, 13),
    ('Channel_ZOOMFX_GetProperties', 0x10006ff0, 'ebx', ['int', 'squall_zoomfx_channel_t*'], 52, 0x1000701d, True, 0x100070e1, 13)]


def main():
    native = ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    pe = PE(native);path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe);audit = Audit(pe, ins);evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    def data(image, address, count):
        offset = image.offset(address - image.image_base, count)
        return image.data[offset:offset + count]

    layout_report = ROOT / 'analysis/squall-device-info-signatures.json'
    metadata = read(layout_report)['details']['compiler_layout']
    assert digest(ROOT / metadata['path']) == metadata['sha256']
    assert all(digest(ROOT / n) == sha for n, sha in metadata['source_files'].items())
    layouts = metadata['value']['types']
    assert [layouts[n]['size'] for n in ['eax_listener', 'eax_channel', 'zoomfx_channel']] == [112, 72, 52]
    sizes = list(struct.unpack('<8I', data(pe, 0x1004b040, 32)))
    assert sizes == [0] + [layouts[f'eax_listener_eax{n}']['size'] for n in [1, 2, 3]] + [0] + [layouts[f'eax_channel_eax{n}']['size'] for n in [1, 2, 3]]
    specifications = [('EAX_StructSize', 0x1004b040, 32), ('EAX_Param', 0x1004b060, 32),
                      ('EAX_Guid', 0x1004b080, 128), ('ZOOMFX_Guid', 0x1004c468, 16)]
    from current_production_regression import current_regression
    _, full = current_regression()
    tables = []
    for index, folder in enumerate(['build/windows', 'build/squall-source-repro-vs20261010']):
        library = ROOT / folder / 'squall-candidate/Release/Squall.dll'
        map_file = library.with_suffix('.map')
        assert digest(library) == full['candidate_sha256'][index]
        built = PE(library);symbols = map_file.read_text(encoding='latin1');values = []
        for name, original_address, count in specifications:
            match = re.search(r'\?' + name + r'@@\S*\s+([0-9a-fA-F]{8})\b', symbols)
            assert match, name
            actual_address = int(match[1], 16)
            left, right = data(pe, original_address, count), data(built, actual_address, count)
            assert left == right, (folder, name)
            values.append(dict(name=name, original_address=f'{original_address:08x}', source_address=f'{actual_address:08x}',
                exact_data_bytes=count, sha256=__import__('hashlib').sha256(left).hexdigest()))
        tables.append(dict(library=library.relative_to(ROOT).as_posix(), sha256=digest(library), map_sha256=digest(map_file), exact_tables=values))
    for row in [
        (0x10001060, 'mov', 'edx,dword ptr [esp+8]'), (0x10001069, 'lea', 'eax,[eax+edx*4]'),
        (0x1000106f, 'mov', 'esi,dword ptr [eax*4+1004B040h]'), (0x10001076, 'push', 'esi'),
        (0x1000107c, 'mov', 'esi,dword ptr [eax*4+1004B060h]'), (0x10001083, 'shl', 'eax,4'),
        (0x1000108a, 'add', 'eax,1004B080h'), (0x10001092, 'call', 'dword ptr [edx+10h]'),
        (0x100010c4, 'mov', 'esi,dword ptr [eax*4+1004B040h]'), (0x100010cb, 'push', 'esi'),
        (0x100010d1, 'mov', 'esi,dword ptr [eax*4+1004B060h]'), (0x100010df, 'add', 'eax,1004B080h'),
        (0x100010e7, 'call', 'dword ptr [edx+0Ch]'),
        (0x1000137e, 'push', '34h'), (0x10001387, 'push', '1004C468h'), (0x1000138d, 'call', 'dword ptr [ecx+10h]'),
        (0x100013b3, 'push', '34h'), (0x100013bc, 'push', '1004C468h'), (0x100013c2, 'call', 'dword ptr [ecx+0Ch]'),
        (0x100013a6, 'je', '100013C5'), (0x100013c5, 'mov', 'eax,1'), (0x100013ca, 'ret', ''),
        # Preserve the original public range and support-polarity differences.
        (0x10004a8e, 'test', 'ebx,ebx'), (0x10004a90, 'jle', '10004AA8'), (0x10004a92, 'cmp', 'ebx,3'),
        (0x10004a95, 'jg', '10004AA8'), (0x10004ad7, 'je', '10004AF1'),
        (0x10004b7e, 'test', 'esi,esi'), (0x10004b80, 'jle', '10004B98'), (0x10004b82, 'cmp', 'esi,3'),
        (0x10004b85, 'jg', '10004B98'), (0x10004bb9, 'je', '10004BD3'),
        (0x10006d04, 'test', 'ebx,ebx'), (0x10006d06, 'jl', '10006D0D'), (0x10006d08, 'cmp', 'ebx,3'),
        (0x10006d0b, 'jle', '10006D25'), (0x10006d2e, 'jne', '10006D48'),
        (0x10006dfe, 'test', 'ebx,ebx'), (0x10006e00, 'jl', '10006E18'), (0x10006e02, 'cmp', 'ebx,3'),
        (0x10006e05, 'jg', '10006E18'), (0x10006e3a, 'jne', '10006E55')]:
        witness(*row)
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for suffix, start, status, params, count, check, output, copy, words in METHODS:
        name = 'SQUALL_' + suffix
        assert re.search('DLL_APIint' + name + r'\(' + ','.join(re.escape(t) + r'\w+' for t in params) + r'\);', header)
        stack = audit.function(start);assert stack['explicit_argument_slots'] == list(range(1, len(params) + 1))
        if suffix.startswith('Channel_'):
            handles = [r for r in stack['stack_references'] if r['entry_esp_offset'] == 4]
            assert len(handles) == 1 and handles[0]['op'] == 'mov'
            at = int(handles[0]['address'], 16)
            assert ins[at]['args'].startswith('eax,dword ptr [esp+')
            at += len(ins[at]['raw']);witness(at, 'mov', 'ecx,10075034h')
            at += len(ins[at]['raw']);witness(at, 'push', 'eax')
            at += len(ins[at]['raw']);witness(at, 'call', '10002480')
        witness(check, 'push', f'{count:X}h')
        at = check + len(ins[check]['raw'])
        assert ins[at]['op'] == 'push';at += len(ins[at]['raw'])
        assert audit.call_evidence[f'{at:08x}']['target'] == ('IsBadWritePtr' if output else 'IsBadReadPtr')
        witness(copy, 'rep', 'movs    dword ptr es:[edi],dword ptr [esi]')
        assert any(ins[a]['op'] == 'mov' and ins[a]['args'].startswith('ecx,') and
                   number(ins[a]['args'].split(',')[1]) == words for a in ins if copy - 18 <= a < copy)
        result, extra = prove_status(audit, start, status);evidence.update(extra)
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=len(params),
            argument_role=dict(arguments_in_public_order=params, aggregate_bytes=count, direction='output' if output else 'input',
                public_payload_is_full_union_even_for_earlier_version='EAX' in suffix, source_header_matches=True,
                independent_original_payload_size_and_copy_bytes=count, backend_wire_tables_exact_in_both_builds=True),
            return_role='Complete initialized32-bit status in EAX on every balanced C3 return',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 27
    count = publish('squall-effect-property-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        current_layout_report='analysis/squall-device-info-signatures.json', current_layout_report_sha256=digest(layout_report),
        exact_original_wire_sizes=sizes, exact_original_property_ids=list(struct.unpack('<8i', data(pe, 0x1004b060, 32))),
        original_eax_guids=[str(uuid.UUID(bytes_le=data(pe, 0x1004b080 + n * 16, 16))) for n in range(8)],
        original_zoomfx_guid=str(uuid.UUID(bytes_le=data(pe, 0x1004c468, 16))), current_build_data_comparisons=tables,
        public_data_treated_as_opaque_property_bytes=True, individual_member_interpretation_proved_from_public_caller=False,
        public_listener_version_range=[1, 3], public_channel_version_range=[0, 3],
        actual_backend_support_for_channel_version0_claimed=False, native_support_test_polarity_difference_preserved=True,
        native_zoomfx_private_getter_unconditionally_returns_one=True,
        status_proof_sha256=digest(ROOT / 'tools/squall_native_status.py'),
        new_runtime_audio_or_property_provider_test_claimed=False, dataflow_assumption='Standard Win32 x86 nonvolatile status preservation'))
    print(f'Proved6 property aggregate signatures,27 EAX/C3 returns and208 exact backend data bytes per build;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
