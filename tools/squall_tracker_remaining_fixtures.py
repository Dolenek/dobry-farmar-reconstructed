"""Owned containers for remaining actual tracker loader branches."""
import struct
from squall_tracker_format_fixtures import sample_bytes,it,xm,s3m
from squall_tracker_fixtures import generate as generate_mod

def rip_container(payload):
    prefix=bytearray(128);total=len(prefix)+len(payload)
    struct.pack_into('<I',prefix,0x18,total-256);struct.pack_into('<I',prefix,0x20,len(prefix))
    return bytes(prefix)+payload

def med(sample,version):
    song=bytearray(788);struct.pack_into('>HHBBBB',song,0,0,len(sample)//2,0,0,48,0)
    struct.pack_into('>HH',song,504,1,1);song[508:764]=bytes([0])+bytes([255])*255
    struct.pack_into('>H',song,764,125);song[766:770]=bytes([0,0x10,0x23,6]);song[770:786]=bytes([64])*16;song[786:788]=bytes([64,1])
    pattern=bytearray(bytes([4,63]) if version==0 else struct.pack('>HHI',4,63,0))
    for row in range(64):
        for channel in range(4):
            if row%8==0:pattern+=bytes([1+[0,4,7,12][channel],0x10,0]) if version==0 else bytes([1+[0,4,7,12][channel],1,0,0])
            else:pattern+=bytes(3 if version==0 else 4)
    song_offset=52;block_array=840;sample_array=844;pattern_offset=848;sample_offset=pattern_offset+len(pattern)
    length=sample_offset+6+len(sample);header=bytearray(52);header[:4]=b'MMD'+bytes([ord('0')+version])
    struct.pack_into('>II',header,4,length,song_offset);struct.pack_into('>I',header,16,block_array);struct.pack_into('>I',header,24,sample_array)
    return bytes(header+song)+struct.pack('>II',pattern_offset,sample_offset)+pattern+struct.pack('>IH',len(sample),0)+sample

def mdl(sample):
    def chunk(name,data):return name+struct.pack('<I',len(data))+data
    info=bytearray(347);struct.pack_into('<HHBBB',info,52,1,0,128,6,125)
    info[59:91]=bytes([16,96,16,96])+bytes([128])*28;info[91:347]=bytes([0])+bytes([255])*255
    pattern=bytes([1,4,63])+bytes(16)+struct.pack('<4H',1,2,3,4)
    tracks=bytearray(struct.pack('<H',4))
    for channel in range(4):
        data=bytearray()
        for _ in range(8):data+=bytes([31,37+[0,4,7,12][channel],1,192,24])
        tracks+=struct.pack('<H',len(data))+data
    mapping=bytes([1,108,192,0,64,0,0,4,0,0,0,0,0,0])
    instrument=bytes([1,1,1])+bytes(32)+mapping
    sample_info=bytearray(60);sample_info[:2]=bytes([1,1]);struct.pack_into('<4I',sample_info,42,8363,len(sample),0,len(sample))
    return b'DMDL\x10'+chunk(b'IN',info)+chunk(b'PA',pattern)+chunk(b'TR',tracks)+chunk(b'II',instrument)+chunk(b'IS',sample_info)+chunk(b'SA',sample)

def delta8(sample):
    return bytes((v-(sample[i-1] if i else 0))&255 for i,v in enumerate(sample))

def psm(sample):
    def chunk(name,data):return name+struct.pack('<I',len(data))+data
    pattern=bytearray()
    for row in range(64):
        if row%8==0:
            for channel,note in enumerate([0x20,0x24,0x27,0x30]):pattern+=bytes([0x60,channel,note,0,96])
        pattern+=bytes([0,0])
    pattern+=bytes(2)
    body=chunk(b'PBOD',struct.pack('<I4sHH',len(pattern),b'P000',64,0)+pattern)
    oplh=bytearray(32);oplh[4:9]=bytes([0,6,0,125,0]);oplh[9:13]=b'P000'
    body+=chunk(b'SONG',b'MAINSONG'+bytes([1,2,4])+chunk(b'OPLH',oplh))
    header=bytearray(96);header[0]=0x80;header[1:9]=b'MAINSONG';header[9:13]=b'S000'
    struct.pack_into('<III',header,54,len(sample),0,len(sample));header[68]=95;struct.pack_into('<I',header,73,8363)
    body+=chunk(b'DSMP',header+delta8(sample))
    return b'PSM '+struct.pack('<I',len(body))+b'FILE'+body

def dmf(sample):
    def chunk(name,data):return name+struct.pack('<I',len(data))+data
    header=b'DDMF\x04XTRACKER'+bytes(53);assert len(header)==66
    # seqsize includes the two loop words in this loader's chunk advance;
    # it reads three orders, so terminate orders with two explicit 0xffff words.
    sequence=struct.pack('<5H',0,0,0,0xffff,0xffff)
    # Loader advances size+8, hence six bytes of payload followed by the
    # remaining terminators, which its unknown-chunk scan skips in four bytes.
    body=b'SEQU'+struct.pack('<I',6)+sequence
    pattern=bytearray()
    for row in range(64):
        pattern.append(0)
        for channel in range(4):
            pattern+=bytes([0x70,1,13+[0,4,7,12][channel],192]) if row%8==0 else bytes([0])
    body+=chunk(b'PATT',struct.pack('<HB',1,4)+struct.pack('<BBHI',4,0x44,64,len(pattern))+pattern)
    info=bytes([1,0])+struct.pack('<IIIHBB',len(sample),0,len(sample),8363,191,1)+bytes(6)
    body+=chunk(b'SMPI',info)+chunk(b'SMPD',struct.pack('<I',len(sample))+sample)+chunk(b'ENDE',bytes(4))
    return header+body

def mt2(sample):
    header=bytearray(382);header[:4]=b'MT20';struct.pack_into('<H',header,8,0x202)
    struct.pack_into('<5HBBIHH',header,106,1,0,1,4,882,6,4,0,1,1);header[126:382]=bytes([0])+bytes([255])*255
    pattern=bytearray()
    for row in range(64):
        for channel in range(4):pattern+=bytes([25+[0,4,7,12][channel],1,0x70,0,0,0,0]) if row%8==0 else bytes(7)
    # The unpacked loop requires remaining length strictly greater than seven.
    pattern+=bytes(8)
    instrument=bytearray(152);struct.pack_into('<I',instrument,32,112);struct.pack_into('<H',instrument,36,1);struct.pack_into('<H',instrument,138,1024)
    sample_header=bytearray(62);struct.pack_into('<III',sample_header,32,26,len(sample),8363)
    sample_header[44:48]=bytes([1,1,0,1]);struct.pack_into('<IIHBBH',sample_header,48,0,len(sample),192<<7,0x80,49,0)
    return bytes(header)+bytes(14)+struct.pack('<HI',64,len(pattern))+pattern+instrument+bytes(254*36)+sample_header+bytes(255*36)+bytes([0,128,0,0,0,0,0,0])+delta8(sample)+bytes(16)

def ams(sample):
    header=b'Extreme'+struct.pack('<BBBBHHBH',0,1,3,1,1,1,0,0)
    instrument=struct.pack('<IIIBHBB',len(sample),0,len(sample),0x80,8363,96,0)
    pattern=bytearray()
    for row in range(64):
        if row%8==0:
            for channel in range(4):pattern+=bytes([channel|(0x80 if channel==3 else 0),12+[0,4,7,12][channel],1])
        else:pattern+=bytes([0x80,0,0])
    pattern+=bytes(3)
    return header+instrument+bytes(7)+bytes(2)+struct.pack('<H',0)+struct.pack('<I',len(pattern))+pattern+sample

def generate(folder):
    sample=sample_bytes();rows=[]
    mod_rows=generate_mod(folder);mod=bytearray((folder/mod_rows[0]['name']).read_bytes());mod[:20]=bytes(20)
    for kind,payload in [('IT',it(sample)),('XM',xm(sample)),('S3M',s3m(sample)),('MOD',bytes(mod))]:
        path=folder/f'owned-rip-container-{kind.lower()}.umx';path.write_bytes(rip_container(payload))
        rows.append(dict(name=path.name,format='UMX_'+kind,channels=4,orders=1,patterns=1,samples=31 if kind=='MOD' else 1,nonempty_sample_count=1,sample_bytes=2048,
            owned_integer_sample=True,title_zeroed=True,original_loader_source='src/squall/runtime/SoundFile/track/load_umx.cpp',
            container_scope='Actual original ripper header bounds and embedded-signature dispatch;not claimed to be a complete Unreal package'))
    for kind,payload,source in [('MMD0',med(sample,0),'med'),('MMD1',med(sample,1),'med'),('MDL',mdl(sample),'mdl'),
                                ('PSM',psm(sample),'psm'),('DMF',dmf(sample),'dmf'),('MT2',mt2(sample),'mt2'),('AMS',ams(sample),'ams')]:
        path=folder/f'owned-nonzero-blank-title.{kind.lower()}';path.write_bytes(payload)
        rows.append(dict(name=path.name,format=kind,channels=4,orders=1,patterns=1,samples=1,nonempty_sample_count=1,sample_bytes=2048,
            owned_integer_sample=True,title_zeroed=True,original_loader_source=f'src/squall/runtime/SoundFile/track/load_{source}.cpp'))
        if kind=='AMS':rows[-1]['sample_scope']='Actual original RS_AMS8 default raw-copy branch;general AMS compression compatibility is not claimed'
        if kind=='DMF':rows[-1]['sequence_scope']='Original seqsize advance and unknown-chunk skip with explicit terminating words;broader file conventions remain open'
    return rows
