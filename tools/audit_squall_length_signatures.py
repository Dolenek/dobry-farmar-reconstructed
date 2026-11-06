#!/usr/bin/env python3
"""Prove five native integer frame/rate/millisecond interfaces and decoder dispatch."""
import re
from pathlib import Path
from pe import PE
from audit_squall_native_arguments import instructions, Audit
from squall_integer_word_flow import prove_integer_word
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish


def main():
    native = ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    pe = PE(native);path = ROOT / '.local/squall-complete-native-disassembly.txt'
    ins = instructions(path, pe);audit = Audit(pe, ins);evidence = {}
    # The linear disassembler started an instruction inside a preceding jump
    # table. The factory's direct CALL establishes this actual entry boundary.
    raw = pe.data[pe.offset(0x2bdb0, 2):][:2]
    assert raw == b'\x6a\xff'
    ins[0x1002bdb0] = dict(raw=raw, op='push', args='0FFFFFFFFh', file_verified=True)

    def witness(address, op=None, args=None):
        row = ins[address];assert row['file_verified'], hex(address)
        if op is not None:assert row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=row['op'], operands=row['args'])

    def graph(start):
        todo, seen = [start], set()
        while todo:
            address = todo.pop()
            if address in seen:continue
            seen.add(address);witness(address);todo += audit.successors(address)
        return sorted(seen)

    factory = graph(0x1002d8c0)
    decoders = []
    for store, table, getter, ctor in [
        (0x1002d95c, 0x10048868, 0x1002de20, None),
        (0x1002d9e3, 0x10048848, 0x1002de20, None),
        (0x100144ed, 0x100484cc, 0x10013f40, 0x100144c0),
        (0x10013e5b, 0x1004846c, 0x10013f40, 0x10013e30),
        (0x10012998, 0x10048250, 0x10013470, 0x10012960),
        (0x10014f34, 0x100484f4, 0x100155f0, 0x10014f00),
        (0x1001390c, 0x100482a0, 0x10013ad0, 0x100138f0),
        (0x1002bddd, 0x10048810, 0x1002c080, 0x1002bdb0)]:
        if ctor:
            assert any(ins[a]['op'] == 'call' and ins[a]['args'] == f'{ctor:08X}' for a in factory)
            assert store in graph(ctor)
        else:assert store in factory
        row = ins[store]
        assert row['op'] == 'mov' and re.fullmatch(r'dword ptr \[\w+\],' + f'{table:08X}h', row['args'])
        offset = pe.offset(table - pe.image_base + 0x14, 4)
        assert pe.u32(offset) == getter
        nodes = graph(getter)
        assert all(not ins[a]['op'].startswith('f') for a in nodes)
        decoders.append(dict(vtable=f'{table:08x}', constructor=f'{ctor:08x}' if ctor else 'inline_factory',
            store=f'{store:08x}', slot=5, slot_bytes=pe.data[offset:offset+4].hex(), getter=f'{getter:08x}',
            integer_instructions=[f'{a:08x}' for a in nodes], normal_returns=[f'{a:08x}' for a in nodes if ins[a]['op']=='ret']))
    for start in [0x1002e140, 0x1002e000, 0x1002ee50, 0x1003d330, 0x1003d280]:graph(start)
    for row in [
        (0x1002e14c, 'mov', 'ecx,dword ptr [esi+0Ch]'),
        (0x1002e151, 'call', 'dword ptr [eax+14h]'),
        (0x1002e156, 'xor', 'eax,eax'),
        (0x1002e00c, 'lea', 'eax,[esi+10h]'),
        (0x1002de30, 'mov', 'cx,word ptr [eax+0Ch]'),
        (0x10013f40, 'mov', 'eax,dword ptr [ecx+14h]'),
        (0x10013f43, 'imul', 'eax,dword ptr [ecx+0Ch]'),
        (0x10013470, 'mov', 'eax,dword ptr [ecx+0000A750h]'),
        (0x1002c080, 'mov', 'eax,dword ptr [ecx+0000B7A8h]'),
        (0x10013ad0, 'mov', 'eax,dword ptr [ecx+8]'),
        (0x10013ad3, 'push', '0FFFFFFFFh'),
        (0x10013ad6, 'call', '1002EE50'),
        (0x1002eea4, 'mov', 'eax,dword ptr [edx+ecx+8]'),
        (0x1002eea8, 'mov', 'edx,dword ptr [edx+ecx+0Ch]'),
        (0x10015601, 'call', 'dword ptr [ecx+24h]'),
        (0x1001560c, 'mov', 'eax,dword ptr [esi+20h]'),
        (0x10015614, 'call', '1003D330'),
        (0x1001561b, 'push', '989680h'),
        (0x10015622, 'call', '1003D280'),
        (0x10007449, 'mov', 'esi,dword ptr [eax+4]'),
        (0x1000748e, 'mov', 'eax,dword ptr [eax+4]'),
        (0x10006139, 'mov', 'esi,dword ptr [eax+4]')]:witness(*row)
    sdk = Path('C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/shared/mmreg.h')
    fmt = re.search(r'typedef struct tWAVEFORMATEX\s*\{(.*?)\} WAVEFORMATEX;', sdk.read_text(encoding='utf-8-sig'), re.S)[1]
    fmt = re.sub(r'/\*.*?\*/', '', fmt, flags=re.S)
    assert re.findall(r'(WORD|DWORD)\s+(\w+)\s*;', fmt) == [
        ('WORD','wFormatTag'), ('WORD','nChannels'), ('DWORD','nSamplesPerSec'),
        ('DWORD','nAvgBytesPerSec'), ('WORD','nBlockAlign'), ('WORD','wBitsPerSample'), ('WORD','cbSize')]
    header = re.sub(r'\s+', '', (ROOT/'src/squall/runtime/Squall.h').read_text(encoding='utf8'))
    rows = []
    for name, start, role in [
        ('Channel_GetLength', 0x10006020, 'encoded integer channel handle;returns decoded frame count'),
        ('Channel_GetLengthMs', 0x100060a0, 'encoded integer channel handle;returns frames*1000/rate using integer64 helpers'),
        ('Sample_GetFileLength', 0x100073e0, 'encoded integer sample ID;returns decoded frame count'),
        ('Sample_GetFileLengthMs', 0x10007410, 'encoded integer sample ID;returns frames*1000/rate using integer64 helpers'),
        ('Sample_GetFileFrequency', 0x10007470, 'encoded integer sample ID;returns WAVEFORMATEX DWORD sample rate')]:
        name = 'SQUALL_' + name
        assert re.search('DLL_APIint'+name+r'\(int\w+\);', header), name
        stack = audit.function(start);assert stack['explicit_argument_slots'] == [1]
        result, extra = prove_integer_word(audit, start,
            numeric_calls=[0x10002480, 0x10007e40, 0x1002e140, 0x1003d330, 0x1003d280],
            numeric_memory=[0x10007449, 0x1000748e, 0x10006139])
        evidence.update(extra)
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        assert all(ins[int(a,16)]['raw'] == b'\xc3' for a in result['all_native_return_sites'])
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=1,
            argument_role=dict(arguments_in_public_order=role, source_header_matches=True),
            return_role='Integer32 frame/millisecond/rate value or negative error in EAX on every balanced C3 return',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(r['all_native_return_sites']) for r in rows) == 12
    count = publish('squall-length-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        decoder_virtual_dispatch=decoders, decoder_count=8, decoder_slot=5,
        corrected_linear_disassembly_entry=dict(address='1002bdb0', bytes='6aff', reason='Direct factory CALL target inside linear jump-table decode'),
        sdk_header_path=str(sdk), sdk_header_sha256=digest(sdk), wave_sample_rate_offset=4,
        integer_word_proof_sha256=digest(ROOT/'tools/squall_integer_word_flow.py'),
        decoder_payload_initialized_on_every_internal_error_path_claimed=False,
        wma_duration_hresult_checked_by_original=False,
        ogg_int64_frame_result_publicly_truncated_to_32_bits=True,
        new_runtime_audio_or_decoder_test_claimed=False))
    print(f'Proved5 native length/rate signatures,12 public returns and8 decoder vtables;{count}/97 ABI semantics verified.')


if __name__ == '__main__':main()
