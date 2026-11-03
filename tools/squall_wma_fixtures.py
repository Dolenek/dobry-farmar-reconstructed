"""Encode owned nonzero integer PCM into test-only WMA/ASF inputs."""
import struct,subprocess

def generate(folder,encoder):
    rows=[]
    for index,(codec,rate,channels,bitrate) in enumerate([('wmav1',44100,1,'64k'),('wmav1',44100,2,'128k'),('wmav2',22050,1,'32k'),('wmav2',44100,2,'128k'),('wmav2',48000,2,'192k')]):
        frames=16384;pcm=bytearray();seed=0x17295183
        for i in range(frames):
            for channel in range(channels):
                seed=(seed*1664525+1013904223)&0xffffffff;phase=(i*(37+channel*16))&1023
                triangle=(phase if phase<512 else 1023-phase)*64-16384;noise=((seed>>16)-32768)//8
                impulse=23000 if (i+channel*11)%193==0 else -23000 if (i+channel*7)%211==0 else 0
                pcm+=struct.pack('<h',max(-32768,min(32767,triangle+noise+impulse)))
        name=f'wma-{index}-{codec}-{rate}-{channels}.wma';raw=folder/(name+'.s16le');raw.write_bytes(pcm)
        command=[str(encoder),'-hide_banner','-loglevel','error','-nostdin','-y','-f','s16le','-ar',str(rate),'-ac',str(channels),'-i',str(raw),'-map_metadata','-1','-threads','1','-c:a',codec,'-b:a',bitrate,'-f','asf',str(folder/name)]
        result=subprocess.run(command,capture_output=True,timeout=45);assert result.returncode==0,result.stderr.decode('utf-8',errors='replace')
        rows.append(dict(name=name,codec=codec,rate=rate,channels=channels,source_frames=frames,raw_name=raw.name,encoder_command=command))
    return rows
