"""Owned integer Layer-I bitstreams; no encoder or original game data is copied."""
class Bits:
    def __init__(self):self.bits=[]
    def put(self,value,count):
        assert 0<=value<(1<<count)
        self.bits.extend((value>>n)&1 for n in range(count-1,-1,-1))
    def bytes(self):
        bits=self.bits+[0]*((-len(self.bits))%8)
        return bytes(sum(bits[i+n]<<(7-n) for n in range(8)) for i in range(0,len(bits),8))


def generate(folder):
    rows=[]
    settings=[(1,44100,3,0,True),(1,44100,3,0,False),(1,44100,0,0,False),
        (1,44100,1,1,False),(1,48000,2,0,False),(1,32000,3,0,False),
        (2,22050,3,0,False),(2,24000,0,0,False)]
    for index,(version,rate,mode,extension,silent) in enumerate(settings):
        channels=1 if mode==3 else 2
        bitrate=(192 if channels==1 else 384) if version==1 else (64 if channels==1 else 128)
        table=[0,32,64,96,128,160,192,224,256,288,320,352,384,416,448] if version==1 else [0,32,48,56,64,80,96,112,128,144,160,176,192,224,256]
        rate_index=([44100,48000,32000] if version==1 else [22050,24000,16000]).index(rate)
        word=0xffe00000|((3 if version==1 else 2)<<19)|(3<<17)|(1<<16)|(table.index(bitrate)<<12)|(rate_index<<10)|(mode<<6)|(extension<<4)
        size=(12*bitrate*1000//rate)*4;payload=bytearray()
        bound=4*(extension+1) if mode==1 else 32
        for frame in range(24):
            bits=Bits();alloc={}
            for sb in range(32):
                common=channels==2 and sb>=bound
                for channel in range(1 if common else channels):
                    value=0 if silent or sb not in [0,5,20,31] else 1+(frame+sb+5*channel)%14
                    bits.put(value,4)
                    alloc[sb,channel]=value
                    if common:alloc[sb,1]=value
            for sb in range(32):
                for channel in range(channels):
                    if alloc[sb,channel]:bits.put(8+(frame+sb+7*channel)%32,6)
            for slot in range(12):
                for sb in range(32):
                    for channel in range(1 if channels==2 and sb>=bound else channels):
                        value=alloc[sb,channel]
                        if value:
                            width=value+1;levels=(1<<width)-1
                            bits.put((frame*13+slot*17+sb*11+channel*7)%levels,width)
            body=bits.bytes();assert len(body)<=size-4
            payload+=word.to_bytes(4,'big')+body+bytes(size-4-len(body))
        name=f'owned-layer1-{index}-v{version}-{rate}-mode{mode}'+('-silent' if silent else '')+'.mp1'
        (folder/name).write_bytes(payload)
        rows.append(dict(name=name,version=version,rate=rate,channels=channels,mode=mode,joint_extension=extension,
            bitrate_kbps=bitrate,frame_bytes=size,mpeg_frames=24,decoded_frames=24*384,silent=silent,
            owned_integer_bitstream=True,allocated_subbands=[] if silent else [0,5,20,31],bitallocation_values=list(range(1,15)) if not silent else [0]))
    return rows
