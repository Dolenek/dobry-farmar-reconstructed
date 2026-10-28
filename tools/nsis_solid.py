"""Bounded NSIS2.28 solid-LZMA archive reader; never executes PE code."""
from dataclasses import dataclass
import lzma
import struct
from pe import PE
MAX_DECODED=64*1024*1024
@dataclass(frozen=True)
class SolidArchive:
    pe: PE
    firstheader_offset: int
    flags: int
    header: bytes
    decoded: bytes
    @property
    def datablock(self):return self.decoded[4+len(self.header):]

def load_archive(path,require_uninstaller=True):
    pe=PE(path);data=pe.data
    if b"Nullsoft Install System v2.28" not in data:raise ValueError('Only NSIS2.28 is supported')
    overlay=max(s['raw_offset']+s['raw_size'] for s in pe.sections)
    magic=struct.pack('<4I',0xDEADBEEF,0x6C6C754E,0x74666F73,0x74736E49)
    signature=data.find(magic,overlay)
    if signature<overlay+4:raise ValueError('NSIS overlay header not found')
    start=signature-4
    if start+33>len(data):raise ValueError('Truncated NSIS firstheader')
    flags,_,_,_,_,header_size,total_size=struct.unpack_from('<7I',data,start)
    if flags&~15 or (require_uninstaller and not flags&1):raise ValueError('Not a supported NSIS uninstaller')
    if start+total_size!=len(data):raise ValueError('Overlay length differs from NSIS declaration')
    prop=data[start+28];quotient,lc=divmod(prop,9);pb,lp=divmod(quotient,5);dictionary=struct.unpack_from('<I',data,start+29)[0]
    if dictionary>MAX_DECODED or lc>8 or pb>4:raise ValueError('Unsupported LZMA properties')
    decoder=lzma.LZMADecompressor(format=lzma.FORMAT_RAW,filters=[dict(id=lzma.FILTER_LZMA1,dict_size=dictionary,lc=lc,lp=lp,pb=pb)])
    end=start+total_size-(0 if flags&4 else 4)
    decoded=decoder.decompress(data[start+33:end],max_length=MAX_DECODED+1)
    if not decoder.eof or decoder.unused_data or len(decoded)>MAX_DECODED:raise ValueError('Truncated, trailing or excessive solid stream')
    if len(decoded)<4 or struct.unpack_from('<I',decoded)[0]!=header_size:raise ValueError('Decoded header size differs from firstheader')
    header=decoded[4:4+header_size]
    if len(header)!=header_size or header_size<300:raise ValueError('Truncated NSIS header')
    return SolidArchive(pe,start,flags,header,decoded)
