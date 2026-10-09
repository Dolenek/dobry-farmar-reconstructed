#!/usr/bin/env python3
"""Extract only the original icon artwork, never code, for the recovered window."""
import argparse
import hashlib
import json
import struct
from pathlib import Path
from pe import PE

ROOT=Path(__file__).resolve().parents[1]

def extract_icon(pe, identifier=101):
    rva,size=pe.directory(2)
    if not rva: raise ValueError('No resource directory')
    base=pe.offset(rva)
    def entries(offset):
        count=sum(pe.unpack('<HH',base+offset+12))
        return [pe.unpack('<II',base+offset+16+i*8) for i in range(count)]
    def find(offset,number):
        return next(target for name,target in entries(offset) if name==number)
    def resource(kind,number):
        type_directory=find(0,kind)
        name_directory=find(type_directory&0x7fffffff,number)
        languages=entries(name_directory&0x7fffffff)
        language,data=min(languages)
        if data&0x80000000: raise ValueError('Unexpected resource nesting')
        payload_rva,length,codepage,reserved=pe.unpack('<IIII',base+data)
        return pe.data[pe.offset(payload_rva,length):pe.offset(payload_rva,length)+length],language
    group,language=resource(14,identifier)
    reserved,kind,count=struct.unpack_from('<HHH',group)
    if reserved or kind!=1 or len(group)!=6+14*count: raise ValueError('Unexpected icon group')
    header=bytearray(struct.pack('<HHH',0,1,count))
    offset=6+16*count
    payloads=[]
    for index in range(count):
        entry=group[6+index*14:6+(index+1)*14]
        icon_id=struct.unpack_from('<H',entry,12)[0]
        payload,icon_language=resource(3,icon_id)
        length=struct.unpack_from('<I',entry,8)[0]
        if len(payload)!=length: raise ValueError('Icon length mismatch')
        header.extend(entry[:12]+struct.pack('<I',offset))
        payloads.append(payload); offset+=length
    return bytes(header)+b''.join(payloads),dict(group_id=identifier,language=language,image_count=count)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'.local/game-artwork/game.ico')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    row=next(row for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==row['sha256']
    icon,metadata=extract_icon(PE(original))
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_bytes(icon)
    metadata.update(original_sha256=row['sha256'],ico_sha256=hashlib.sha256(icon).hexdigest(),
                    kind='Original icon artwork only; no executable code extracted')
    (ROOT/'analysis/game-icon.json').write_text(json.dumps(metadata,indent=2)+'\n',encoding='utf-8')
    print(f"Extracted {metadata['image_count']} icon artwork images ({len(icon)} bytes)")

if __name__=='__main__': main()
