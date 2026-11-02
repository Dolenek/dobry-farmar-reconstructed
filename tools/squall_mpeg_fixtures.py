"""Encode owned deterministic integer waveforms into MP2/MP3 test inputs."""
import struct,subprocess
from pathlib import Path


def generate(folder,encoder):
    rows=[]
    settings=[('mp2',44100,1,'192k'),('mp2',44100,2,'192k'),('mp2',22050,2,'96k'),
              ('mp3',44100,1,'128k'),('mp3',44100,2,'128k'),('mp3',22050,1,'64k'),('mp3',22050,2,'64k'),('mp3',11025,1,'32k')]
    for index,(codec,rate,channels,bitrate) in enumerate(settings):
        frames=11520;pcm=bytearray();seed=0x17295183
        for i in range(frames):
            for channel in range(channels):
                seed=(seed*1664525+1013904223)&0xffffffff
                phase=(i*(37+channel*16))&1023;triangle=(phase if phase<512 else 1023-phase)*64-16384
                noise=((seed>>16)-32768)//8
                impulse=(23000 if (i+channel*11)%193==0 else -23000 if (i+channel*7)%211==0 else 0)
                value=max(-32768,min(32767,triangle+noise+impulse));pcm+=struct.pack('<h',value)
        name=f'mpeg-{index}-{codec}-{rate}-{channels}.{codec}';raw=folder/f'{name}.s16le';raw.write_bytes(pcm)
        command=[str(encoder),'-hide_banner','-loglevel','error','-nostdin','-y','-f','s16le','-ar',str(rate),'-ac',str(channels),'-i',str(raw),
                 '-map_metadata','-1','-threads','1','-c:a','mp2' if codec=='mp2' else 'libmp3lame','-b:a',bitrate]
        if codec=='mp3':command+=['-write_xing','0','-id3v2_version','0','-write_id3v1','0']
        command+=['-f',codec,str(folder/name)]
        result=subprocess.run(command,capture_output=True,timeout=45);assert result.returncode==0,result.stderr.decode('utf-8',errors='replace')
        rows.append(dict(name=name,codec=codec,rate=rate,channels=channels,source_frames=frames,raw_name=raw.name,encoder_command=command))
    return rows
