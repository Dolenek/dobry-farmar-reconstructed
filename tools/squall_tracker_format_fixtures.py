"""Owned nonzero XM/IT/S3M fixtures assembled from the original loader structures."""
import struct


def sample_bytes():
    result=bytearray();seed=0x17295183
    for index in range(2048):
        seed=(seed*1664525+1013904223)&0xffffffff
        phase=(index*5)&255
        triangle=(phase if phase<128 else 255-phase)*2-127
        value=max(-128,min(127,triangle+((seed>>24)-128)//8))
        result.append(value&255)
    return bytes(result)


def xm(sample):
    header=bytearray(b'Extended Module: '+bytes(20)+b'\x1a'+b'Owned fixture writer'+struct.pack('<H',0x104))
    assert len(header)==60
    header+=struct.pack('<I8H',276,1,0,4,1,1,0,6,125)+bytes(256)
    assert len(header)==336
    pattern=bytearray()
    for row in range(64):
        for channel in range(4):
            pattern+=bytes([49+[0,4,7,12][channel],1,0,0,0]) if row%8==0 else bytes(5)
    pattern_header=struct.pack('<IBHH',9,0,64,len(pattern))
    instrument=struct.pack('<I',263)+bytes(22)+struct.pack('<BH',0,1)
    instrument+=struct.pack('<I',40)+bytes(230)
    assert len(instrument)==263
    sample_header=struct.pack('<IIIBbBBbB',len(sample),0,len(sample),48,0,1,128,0,0)+bytes(22)
    assert len(sample_header)==40
    previous=0;delta=bytearray()
    for value in sample:
        delta.append((value-previous)&255);previous=value
    return bytes(header+pattern_header+pattern+instrument+sample_header+delta)


def it(sample):
    header=bytearray(192)
    header[:4]=b'IMPM'
    struct.pack_into('<9H',header,30,0x1004,1,0,1,1,0x214,0x214,1,0)
    header[48:54]=bytes([128,48,6,125,128,0])
    header[64:128]=bytes([0,64,0,64])+bytes([255])*60
    header[128:192]=bytes([64])*4+bytes(60)
    pattern=bytearray()
    for row in range(64):
        if row%8==0:
            for channel in range(4):pattern+=bytes([0x80|channel+1,7,60+[0,4,7,12][channel],1,48])
        pattern.append(0)
    pattern_header=struct.pack('<HHI',len(pattern),64,0)
    sample_offset=208;pattern_offset=288;pcm_offset=pattern_offset+len(pattern_header)+len(pattern)
    sample_header=bytearray(80);sample_header[:4]=b'IMPS'
    sample_header[17:20]=bytes([64,0x11,48]);sample_header[46:48]=bytes([1,0xa0])
    struct.pack_into('<7I',sample_header,48,len(sample),0,len(sample),8363,0,0,pcm_offset)
    data=header+bytes([0])+struct.pack('<II',sample_offset,pattern_offset)
    data+=bytes(sample_offset-len(data))+sample_header
    assert len(data)==pattern_offset
    return bytes(data+pattern_header+pattern+sample)


def s3m(sample):
    header=bytearray(96);header[28:30]=bytes([0x1a,0x10])
    struct.pack_into('<6H',header,32,2,1,1,0,0x1320,2);header[44:48]=b'SCRM'
    header[48:54]=bytes([64,6,125,0x80|48,0,0])
    header[64:96]=bytes([0,8,1,9])+bytes([255])*28
    pattern=bytearray()
    for row in range(64):
        if row%8==0:
            for channel,note in enumerate([0x40,0x44,0x47,0x50]):pattern+=bytes([0x60|channel,note,1,48])
        pattern.append(0)
    sample_offset=112;pattern_offset=192;pcm_offset=(pattern_offset+2+len(pattern)+15)&~15
    sample_header=bytearray(80);sample_header[0]=1
    struct.pack_into('<HIII',sample_header,14,pcm_offset>>4,len(sample),0,len(sample))
    sample_header[28:32]=bytes([48,0,0,1]);struct.pack_into('<I',sample_header,32,8363)
    sample_header[76:80]=b'SCRS'
    data=header+bytes([0,255])+struct.pack('<HH',sample_offset>>4,pattern_offset>>4)
    data+=bytes(sample_offset-len(data))+sample_header
    assert len(data)==pattern_offset
    data+=struct.pack('<H',len(pattern)+2)+pattern
    data+=bytes(pcm_offset-len(data))
    return bytes(data+bytes((value+128)&255 for value in sample))


def generate(folder):
    sample=sample_bytes();rows=[]
    for extension,function in [('xm',xm),('it',it),('s3m',s3m)]:
        path=folder/f'owned-nonzero-blank-title.{extension}'
        path.write_bytes(function(sample))
        rows.append(dict(name=path.name,format=extension.upper(),channels=4,orders=1,patterns=1,
                         samples=1,sample_bytes=len(sample),owned_integer_sample=True,title_zeroed=True))
    return rows
