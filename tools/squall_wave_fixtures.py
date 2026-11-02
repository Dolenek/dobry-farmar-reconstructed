"""Generate owned nonzero PCM/IMA/MS ADPCM WAV fixtures; no original audio bytes."""
import struct


def chunk(name,data):
    return name+struct.pack('<I',len(data))+data+(b'\0' if len(data)%2 else b'')


def wav(fmt,data):
    contents=b'WAVE'+chunk(b'fmt ',fmt)+chunk(b'data',data)
    return b'RIFF'+struct.pack('<I',len(contents))+contents


def generate(folder):
    rows=[]
    def add(name,fmt,data,frames,rate,channels,tag):
        (folder/name).write_bytes(wav(fmt,data));rows.append(dict(name=name,frames=frames,rate=rate,channels=channels,format_tag=tag))
    for bits in [8,16]:
        for channels in [1,2]:
            frames=256;rate=11025;align=channels*bits//8
            fmt=struct.pack('<HHIIHH',1,channels,rate,rate*align,align,bits)
            if bits==8:data=bytes((n*37+c*113)&255 for n in range(frames) for c in range(channels))
            else:
                values=[-32768,-32767,-16385,-257,-256,-129,-128,-127,-1,0,1,127,128,129,255,256,16383,32766,32767]
                data=b''.join(struct.pack('<h',values[(n*7+c*13)%len(values)]) for n in range(frames) for c in range(channels))
            add(f'pcm{bits}-{channels}.wav',fmt,data,frames,rate,channels,1)
    predictions=[-32768,-32640,-1,0,127,128,255,256,32767]
    indices=[0,1,44,88,1,20,40,80,0]
    for channels in [1,2]:
        for pattern in range(3):
            rate=8000;samples=9;align=8*channels
            fmt=struct.pack('<HHIIHHHH',0x11,channels,rate,rate*align//samples,align,4,2,samples)
            blocks=[]
            for block in range(9):
                headers=b''.join(struct.pack('<hBB',predictions[(block+c*3)%9],indices[(block+c*4)%9],0) for c in range(channels))
                payload=b''.join(bytes(((block*29+pattern*83+c*71+n*53)&255 for n in range(4))) for c in range(channels))
                blocks.append(headers+payload)
            add(f'ima-{channels}-{pattern}.wav',fmt,b''.join(blocks),9*samples,rate,channels,0x11)
    coefficients=[(256,0),(512,-256),(0,0),(192,64),(240,0),(460,-208),(392,-232)]
    for channels in [1,2]:
        for predictor in range(7):
            rate=8000;samples=4;align=8*channels
            extra=struct.pack('<HH',samples,len(coefficients))+b''.join(struct.pack('<hh',a,b) for a,b in coefficients)
            fmt=struct.pack('<HHIIHHH',2,channels,rate,rate*align//samples,align,4,len(extra))+extra
            blocks=[]
            for block in range(9):
                predictors=bytes((predictor if c==0 else 6-predictor) for c in range(channels))
                steps=b''.join(struct.pack('<h',[16,17,1000,32767][(block+c)%4]) for c in range(channels))
                current=b''.join(struct.pack('<h',predictions[(block+c*3)%9]) for c in range(channels))
                previous=b''.join(struct.pack('<h',predictions[(block+c*3+5)%9]) for c in range(channels))
                payload=bytes((block*43+predictor*17+n*109)&255 for n in range(channels))
                blocks.append(predictors+steps+current+previous+payload)
            add(f'msadpcm-{channels}-{predictor}.wav',fmt,b''.join(blocks),9*samples,rate,channels,2)
    (folder/'files.txt').write_text(''.join(f"{row['name']} {row['frames']} {row['rate']}\n" for row in rows),encoding='ascii')
    assert len(rows)==24
    return rows
