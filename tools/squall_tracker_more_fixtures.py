"""Owned nonzero classic tracker streams from the original loader records."""
import struct
from squall_tracker_format_fixtures import sample_bytes

def mtm(sample):
    header=bytearray(66);header[:4]=b'MTM\x10'
    struct.pack_into('<HBBHBBBB',header,24,4,1,1,0,1,0,64,4);header[34:38]=bytes([0,15,0,15])
    instrument=bytes(22)+struct.pack('<IIIbBB',len(sample),0,len(sample),0,48,0);assert len(instrument)==37
    tracks=bytearray()
    for channel in range(4):
        for row in range(64):tracks+=bytes([(12+[0,4,7,12][channel])*4,0x10,0]) if row%8==0 else bytes(3)
    sequence=(struct.pack('<4H',1,2,3,4)+bytes(56))*2
    return bytes(header+instrument+bytes([0,1])+bytes([255])*126+tracks+sequence+bytes((v+128)&255 for v in sample))

def composer669(sample):
    header=bytearray(497);header[:2]=b'if';header[110:113]=bytes([1,1,0]);header[113:241]=bytes([0])+bytes([255])*127
    header[241:369]=bytes([6])*128;header[369:497]=bytes([63])*128
    instrument=bytes(13)+struct.pack('<III',len(sample),0,len(sample));assert len(instrument)==25
    pattern=bytearray()
    for row in range(64):
        for channel in range(8):pattern+=bytes([(12+[0,4,7,12,0,4,7,12][channel])*4,12,255]) if row%8==0 else bytes([255,0,255])
    assert len(pattern)==1536
    return bytes(header+instrument+pattern+bytes((v+128)&255 for v in sample))

def stm(sample):
    header=bytearray(1168);header[20:28]=b'!SCREAM!';header[28:35]=bytes([0x1a,2,2,21,0x60,1,64])
    struct.pack_into('<HHHHBBH',header,48+14,0,len(sample),0,len(sample),48,0,8363)
    header[1040:1168]=bytes([0])+bytes([99])*127
    pattern=bytearray()
    for row in range(64):
        for channel,note in enumerate([0x10,0x14,0x17,0x20]):pattern+=bytes([note,8,96,0]) if row%8==0 else bytes([255,0,0,0])
    assert len(pattern)==1024
    return bytes(header+pattern+sample)

def ult(sample):
    header=b'MAS_UTrack_V004'+bytes(33);assert len(header)==48
    instrument=bytes(44)+struct.pack('<4iBBH',0,len(sample),0,len(sample),192,8,8363)+bytes(2);assert len(instrument)==66
    pattern=bytearray()
    for channel in range(4):
        for block in range(8):
            pattern+=bytes([13+[0,4,7,12][channel],1,0x0c,192,0])
            pattern+=bytes([0xfc,7,0,0,0,0,0])
    return header+bytes([1])+instrument+bytes([0])+bytes([255])*255+bytes([3,0,0,15,0,15])+pattern+sample

def far(sample):
    header=bytearray(98);header[:4]=b'FAR\xfe';header[44:47]=bytes([13,10,26]);struct.pack_into('<H',header,47,869)
    header[49]=0xd1;header[50:66]=bytes([1])*16;header[75]=6;header[76:92]=bytes([3,12])*8
    orders=bytearray(771);orders[256:259]=bytes([1,0,0]);struct.pack_into('<H',orders,259,4098)
    pattern=bytearray([63,0])
    for row in range(64):
        for channel in range(16):pattern+=bytes([13+[0,4,7,12][channel%4],0,12,0]) if row%8==0 else bytes(4)
    assert len(pattern)==4098
    instrument=bytes(32)+struct.pack('<IBBIIBB',len(sample),0,12,0,len(sample),0,8);assert len(instrument)==48
    return bytes(header+orders+pattern+bytes([1])+bytes(7)+instrument+sample)

def okt(sample):
    def chunk(name,data):return name+struct.pack('>I',len(data))+data
    header=b'OKTASONGCMOD'+struct.pack('>I',8)+bytes(8)+b'SAMP'+struct.pack('>I',32);assert len(header)==32
    instrument=bytes(20)+struct.pack('>IHHBBBB',len(sample),0,len(sample),0,48,0,0);assert len(instrument)==32
    pattern=bytearray(struct.pack('>H',64))
    for row in range(64):
        for channel in range(4):pattern+=bytes([1+[0,4,7,12][channel],0,31,48]) if row%8==0 else bytes(4)
    return header+instrument+chunk(b'SPEE',bytes([0,6]))+chunk(b'SLEN',bytes([0,1]))+chunk(b'PLEN',bytes([0,1]))+chunk(b'PATT',bytes(130))+chunk(b'PBOD',pattern)+chunk(b'SBOD',sample)

def dsm(sample):
    song=bytearray(192);struct.pack_into('<4H',song,36,1,1,1,4);song[44:48]=bytes([64,48,6,125]);song[48:64]=bytes([16,112])*8
    song[64:192]=bytes([0])+bytes([255])*127
    pattern=bytearray(2)
    for row in range(64):
        if row%8==0:
            for channel in range(4):pattern+=bytes([0xe0|channel,37+[0,4,7,12][channel],1,48])
        pattern.append(0)
    instrument=bytes(13)+bytes([3,0,48])+struct.pack('<4I2H',len(sample),0,len(sample),0,8363,0)+bytes(28)+sample
    assert len(instrument)==64+len(sample)
    tail=b'SONG'+struct.pack('<I',192)+song+b'PATT'+struct.pack('<I',len(pattern))+pattern+b'INST'+struct.pack('<I',len(instrument))+instrument+b'JUNK'+struct.pack('<I',8)+bytes(8)
    return b'RIFF'+struct.pack('<I',4+len(tail))+b'DSMF'+tail

def ptm(sample):
    pattern=bytearray()
    for row in range(64):
        if row%8==0:
            for channel in range(4):pattern+=bytes([0xa0|channel,49+[0,4,7,12][channel],1,48])
        pattern.append(0)
    pattern_offset=704;sample_offset=pattern_offset+len(pattern)
    header=bytearray(608);header[28:32]=bytes([26,3,2,0]);struct.pack_into('<4H',header,32,1,1,1,4)
    header[44:48]=b'PTMF';header[64:68]=bytes([0,15,0,15]);header[96:352]=bytes([0])+bytes([255])*255;struct.pack_into('<H',header,352,pattern_offset//16)
    instrument=bytearray(80);instrument[0]=5;instrument[13]=48;struct.pack_into('<H',instrument,14,8363)
    struct.pack_into('<4I',instrument,18,sample_offset,len(sample),0,len(sample));instrument[76:80]=b'PTMS'
    previous=0;delta=bytearray()
    for value in sample:delta.append((value-previous)&255);previous=value
    return bytes(header+instrument+bytes(pattern_offset-688)+pattern+delta)

def dbm(sample):
    def chunk(name,data):return name+struct.pack('>I',len(data))+data
    header=bytearray(132);header[:4]=b'DBM0';struct.pack_into('>H',header,4,0x0215);header[8:12]=b'NAME';struct.pack_into('>I',header,12,44)
    header[60:64]=b'INFO';struct.pack_into('>I5H',header,64,10,1,1,1,1,4);header[78:82]=b'SONG';struct.pack_into('>I',header,82,48);struct.pack_into('>H',header,130,1)
    instrument=bytes(30)+struct.pack('>HHIIIHH',1,48,8363,0,len(sample),128,1);assert len(instrument)==50
    pattern=bytearray()
    for row in range(64):
        if row%8==0:
            for channel,note in enumerate([0x30,0x34,0x37,0x40]):pattern+=bytes([channel+1,0x0f,note,1,0x0c,48])
        pattern.append(0)
    pattern+=bytes(4)
    return bytes(header)+bytes(2)+chunk(b'INST',instrument)+chunk(b'PATT',struct.pack('>HI',64,len(pattern))+pattern)+chunk(b'SMPL',struct.pack('>II',1,len(sample))+sample)

def amf14(sample):
    header=bytearray(41);header[:4]=b'AMF\x0e';header[36:38]=bytes([1,1]);struct.pack_into('<H',header,38,4);header[40]=4
    pan=bytes([208,48])*2+bytes(28)
    instrument=struct.pack('<B32s13sIIHB',1,bytes(32),bytes(13),1,len(sample),8363,48)+struct.pack('<II',0,len(sample));assert len(instrument)==65
    tracks=bytearray()
    for channel in range(4):
        commands=bytearray([0,0x80,0])
        for row in range(0,64,8):commands+=bytes([row,48+[0,4,7,12][channel],48])
        tracks+=struct.pack('<H',len(commands)//3)+bytes(1)+commands
    return bytes(header)+pan+bytes([125,6])+struct.pack('<5H',64,1,2,3,4)+instrument+struct.pack('<4H',1,2,3,4)+tracks+bytes((v+128)&255 for v in sample)

def amf0(sample):
    header=bytearray(294);signature=b'ASYLUM Music Format V1.0';header[:len(signature)]=signature;header[35:37]=bytes([1,1]);header[38:294]=bytes([0])+bytes([255])*255
    instruments=bytearray(64*37);instruments[23]=48;struct.pack_into('<III',instruments,25,len(sample),0,len(sample))
    pattern=bytearray()
    for row in range(64):
        for channel in range(8):pattern+=bytes([36+[0,4,7,12][channel%4],1,0,0]) if row%8==0 else bytes(4)
    return bytes(header+instruments+pattern+sample)

def generate(folder):
    sample=sample_bytes();result=[]
    for extension,function,channels,patterns in [('mtm',mtm,4,2),('669',composer669,8,1),('stm',stm,4,1),('ult',ult,4,1),('far',far,16,1),('okt',okt,4,1),('dsm',dsm,4,1),('ptm',ptm,4,1),('dbm',dbm,4,1),('amf14',amf14,4,1),('amf0',amf0,8,1)]:
        data=function(sample);assert len(data)>=1084
        path=folder/f'owned-nonzero-blank-title.{extension}';path.write_bytes(data)
        result.append(dict(name=path.name,format=extension.upper(),channels=channels,orders=2 if extension=='mtm' else 1,
            patterns=patterns,samples=31 if extension in ['stm','amf0'] else 1,nonempty_sample_count=1,sample_bytes=len(sample),owned_integer_sample=True,title_zeroed=True,
            original_loader_source=f'src/squall/runtime/SoundFile/track/load_{"amf" if extension.startswith("amf") else extension}.cpp'))
    return result
