"""Create owned deterministic ProTracker MOD fixtures directly from integer samples."""
import struct

def generate(folder):
    rows=[]
    for variant in [0,1,2]:
        sample=bytearray();seed=0x17295183
        for i in range(2048):
            seed=(seed*1664525+1013904223)&0xffffffff
            phase=(i*(3+variant*2))&255;triangle=(phase if phase<128 else 255-phase)*2-127
            noise=((seed>>24)-128)//8;value=max(-128,min(127,triangle+noise));sample.append(value&255)
        title=('Owned MOD test '+str(variant)).encode().ljust(20,b'\0');headers=bytearray()
        for index in range(31):
            headers+=('sample-'+str(index)).encode().ljust(22,b'\0')
            headers+=struct.pack('>HBBHH',1024 if index==0 else 0,variant if index==0 else 0,48 if index==0 else 0,0,1024 if index==0 and variant!=2 else 1)
        pattern=bytearray(1024)
        def event(row,channel,period,sample,effect=0,param=0):
            pos=(row*4+channel)*4;pattern[pos:pos+4]=bytes([(sample&0xf0)|((period>>8)&15),period&255,((sample&15)<<4)|(effect&15),param])
        notes=[428,339,285,214]
        for row in [0,8,16,24,32,40,48,56]:
            for channel in range(4):event(row,channel,notes[(channel+row//8+variant)%4],1)
        if variant==0:event(0,3,notes[3],1,15,6)
        if variant==1:event(0,3,notes[0],1,15,150);event(16,1,0,0,4,0x47);event(32,2,0,0,10,0x04)
        if variant==2:event(8,0,0,0,9,1);event(24,1,0,0,12,16);event(40,2,0,0,0,0x37)
        event(63,3,0,0,13,0)
        name=f'owned-protracker-{variant}.mod';payload=title+headers+bytes([1,0])+bytes(128)+b'M.K.'+pattern+sample;assert len(payload)==4156
        (folder/name).write_bytes(payload);rows.append(dict(name=name,type='ProTracker MOD',channels=4,orders=1,patterns=1,samples=1,sample_bytes=2048,looping_sample=variant!=2,owned_integer_sample=True))
    return rows
