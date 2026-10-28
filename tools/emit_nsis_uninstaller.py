#!/usr/bin/env python3
"""Emit NSIS2.28 WriteUninstaller output offline from a source-built builder.

Matches NSIS2.28 Source/exehead/exec.c EW_WRITEUNINSTALLER: copy own PE
header, apply compiler-produced icon patches, then append generated payload.
No installer or uninstall instruction is executed; no registry/UI is touched.
"""
import argparse,hashlib,json,struct
from pathlib import Path
from nsis_solid import load_archive,MAX_DECODED
from nsis_extract import OPCODES,extract

def emit(builder,output):
    archive=load_archive(builder,require_uninstaller=False)
    if archive.flags&1:raise ValueError('Expected source-built generation helper, not an uninstaller')
    entry_offset,count=struct.unpack_from('<II',archive.header,20)
    if entry_offset+count*28>len(archive.header):raise ValueError('Instruction table outside header')
    writes=[]
    for i in range(count):
        opcode,*arguments=struct.unpack_from('<7i',archive.header,entry_offset+i*28)
        if opcode==OPCODES.index('WRITEUNINSTALLER'):writes.append(arguments)
    if len(writes)!=1:raise ValueError('Expected exactly one WriteUninstaller instruction')
    _,offset,icon_size,*_=writes[0];block=archive.datablock
    def chunk(position):
        if position<0 or position+4>len(block):raise ValueError('Data chunk header outside datablock')
        size=struct.unpack_from('<I',block,position)[0]
        if size>MAX_DECODED or position+4+size>len(block):raise ValueError('Invalid solid data chunk')
        return block[position+4:position+4+size],position+4+size
    icons,next_offset=chunk(offset)
    if len(icons)!=icon_size:raise ValueError('Icon data length differs from WriteUninstaller')
    payload,_=chunk(next_offset)
    if len(payload)<28 or not struct.unpack_from('<I',payload)[0]&1:raise ValueError('Not an uninstall payload')
    head=bytearray(archive.pe.data[:archive.firstheader_offset]);position=0;patches=[]
    while position<len(icons) and icons[position]: # exact NSIS2.28 byte test
        if position+8>len(icons):raise ValueError('Truncated icon patch')
        size,destination=struct.unpack_from('<II',icons,position);position+=8
        if position+size>len(icons) or destination+size>len(head):raise ValueError('Icon patch outside source PE header')
        head[destination:destination+size]=icons[position:position+size]
        patches.append(dict(offset=destination,bytes=size));position+=size
    if position+4!=len(icons) or struct.unpack_from('<I',icons,position)[0]!=0:raise ValueError('Invalid icon patch terminator')
    output.parent.mkdir(parents=True,exist_ok=True);output.write_bytes(head+payload)
    decoded=extract(output)
    return dict(method='offline_source_framework_EW_WRITEUNINSTALLER',builder_sha256=hashlib.sha256(archive.pe.data).hexdigest(),output_sha256=hashlib.sha256(output.read_bytes()).hexdigest(),output_bytes=output.stat().st_size,pe_header_bytes=len(head),icon_patches=patches,instructions=len(decoded['instructions']),pages=decoded['blocks']['pages']['count'],uninstall_or_install_code_executed=False)

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--builder',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--metadata',type=Path);a=p.parse_args();report=emit(a.builder,a.output)
    if a.metadata:a.metadata.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report))
if __name__=='__main__':main()
