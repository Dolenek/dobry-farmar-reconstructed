#!/usr/bin/env python3
"""Audit original tutorial entry arguments from file-verified native instructions."""
import argparse
import collections
import datetime
import re
import struct
from pathlib import Path
from pe import PE
from record_live_production_verification import ROOT,digest,read,write

ENTRIES={0x4ce650:'Common',0x4ceb00:'Arrow',0x4cef60:'DisabledAnchor',
         0x4cf3c0:'Action',0x4cf7d0:'RepeatedAction'}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--disassembly',type=Path,default=ROOT/'.local/game-tutorial-native-disassembly.txt')
    args=parser.parse_args();binary=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    pe=PE(binary);inventory=read(ROOT/'analysis/inventory.json')
    assert digest(binary)==next(r['sha256'] for r in inventory['files'] if r['path']==binary.name)
    rows={};previous=None
    for line in args.disassembly.read_text(encoding='utf-8-sig').splitlines():
        match=re.match(r'^\s*([0-9A-F]{8}):((?:\s+[0-9A-F]{2})+)\s+([a-z][a-z0-9]*)\s*(.*?)\s*$',line)
        if match:
            address,raw,op,operands=match.groups();previous=int(address,16)
            rows[previous]=dict(raw=bytes.fromhex(raw),op=op,args=operands)
        elif previous is not None and re.fullmatch(r'\s+[0-9A-F]{2}(?:\s+[0-9A-F]{2})*\s*',line):
            rows[previous]['raw']+=bytes.fromhex(line)
    incoming=collections.defaultdict(set);calls=[];tail_jumps=[]
    for address,row in rows.items():
        op,operands=row['op'],row['args'];end=address+len(row['raw'])
        if op not in ['ret','int','jmp'] and end in rows:incoming[end].add(address)
        if (op.startswith('j') or op.startswith('loop')) and re.fullmatch('[0-9A-F]{8}',operands):
            incoming[int(operands,16)].add(address)
        if op in ['call','jmp'] and re.fullmatch('[0-9A-F]{8}',operands) and int(operands,16) in ENTRIES:
            (calls if op=='call' else tail_jumps).append(address)
    assert not tail_jumps,tail_jumps
    encoded_calls=set()
    for section in pe.sections:
        if section['name']!='.text':continue
        block=pe.data[section['raw_offset']:section['raw_offset']+section['raw_size']]
        offset=0
        while True:
            offset=block.find(b'\xe8',offset)
            if offset<0:break
            if offset+5<=len(block):
                address=pe.image_base+section['rva']+offset
                target=(address+5+struct.unpack_from('<i',block,offset+1)[0])&0xffffffff
                if target in ENTRIES:encoded_calls.add(address)
            offset+=1
    assert encoded_calls==set(calls),('Byte-level relative call scan differs from disassembly',encoded_calls^set(calls))
    def verified(address):
        row=rows[address];offset=pe.offset(address-pe.image_base,len(row['raw']))
        assert pe.data[offset:offset+len(row['raw'])]==row['raw'],hex(address)
        return dict(address=f'{address:08x}',bytes=row['raw'].hex(),op=row['op'],args=row['args'])
    result=[];witnesses={}
    for address in calls:
        todo=list(incoming[address]);seen=set();pushes=[]
        while todo:
            at=todo.pop()
            if at in seen:continue
            seen.add(at);assert len(seen)<1000,('Unbounded backward argument search',hex(address))
            row=rows[at];witnesses[at]=verified(at)
            if row['op']=='push':
                assert re.fullmatch('[0-9A-F]+h|[0-9]+',row['args']),('Nonconstant tutorial index',hex(address),hex(at),row)
                value=int(row['args'][:-1],16) if row['args'].endswith('h') else int(row['args'])
                pushes.append(dict(instruction=f'{at:08x}',step=value));continue
            assert row['op'] not in ['call','pop','ret','int'],('Argument crosses unhandled stack operation',hex(address),hex(at))
            assert incoming[at],('Argument path without predecessor',hex(address),hex(at))
            todo.extend(incoming[at])
        assert pushes
        witnesses[address]=verified(address)
        result.append(dict(call=f'{address:08x}',entry=f"{int(rows[address]['args'],16):08x}",
            style=ENTRIES[int(rows[address]['args'],16)],steps=sorted({p['step'] for p in pushes}),
            constant_index_pushes=sorted(pushes,key=lambda r:r['instruction'])))
    # No absolute function-address payload can feed an ordinary indirect call.
    absolute={}
    for entry in ENTRIES:
        pattern=struct.pack('<I',entry);offset=0;found=[]
        while True:
            offset=pe.data.find(pattern,offset)
            if offset<0:break
            found.append(offset);offset+=1
        absolute[f'{entry:08x}']=found
    assert all(not values for values in absolute.values()),absolute
    indices=sorted({step for row in result for step in row['steps']})
    assert not ({0,10}&set(indices))
    report=dict(status='passed_native_compiled_tutorial_callers_have_no_index0_or10',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reference_sha256=digest(binary),disassembly=dict(path=args.disassembly.relative_to(ROOT).as_posix(),sha256=digest(args.disassembly)),
        entry_points={f'{key:08x}':value for key,value in ENTRIES.items()},
        direct_call_count=len(result),calls=result,compiled_caller_indices=indices,
        independent_text_section_relative_call_scan_exact=True,
        absolute_entry_address_occurrences=absolute,tail_entry_jumps=tail_jumps,
        instruction_witnesses=[witnesses[key] for key in sorted(witnesses)],
        unused_data_indices=[0,10],all50_data_record_oracle_report='analysis/tutorial-manager-validation.json',
        conclusion_scope='Original compiled ordinary call paths to all five manager show entries;not a claim about memory corruption or arbitrary synthetic external calls',
        production_changed=False,original_binary_changed=False,whole_game_complete=False)
    write(ROOT/'analysis/tutorial-native-callers-audit.json',report)
    print('Original tutorial calls:',len(result),'indices:',indices)


if __name__=='__main__':main()
