"""Bounded read-only decoding of diagnostic copies of the recovered world save."""
from pathlib import Path
import struct

def load_save(path):
    data=Path(path).read_bytes()
    if len(data)>64*1024*1024:
        raise ValueError('Diagnostic save exceeds64MiB')
    offset=0
    def raw(count):
        nonlocal offset
        if count<0 or offset+count>len(data):
            raise ValueError(('Truncated save',offset,count,len(data)))
        value=data[offset:offset+count];offset+=count;return value
    def read(fmt):
        values=struct.unpack('<'+fmt,raw(struct.calcsize('<'+fmt)))
        return values[0] if len(values)==1 else list(values)
    def count():
        value=read('I')
        if value>10000:raise ValueError(('Unbounded diagnostic record count',value))
        return value
    seeds=[];padding_ranges=[]
    for _ in range(count()):
        start=offset;x,y,kind,quantity,status,unpaid=read('ffIIIB')
        padding=raw(3);padding_ranges.append([start+21,start+24])
        seeds.append(dict(x=x,y=y,kind=kind,count=quantity,status=status,unpaid=unpaid,padding_hex=padding.hex()))
    sale_count=count();cutoff_offset=offset;cutoff=read('f');sales=[];sale_time_ranges=[]
    for _ in range(sale_count):
        start=offset;time,amount=read('fI');sales.append(dict(time=time,amount=amount));sale_time_ranges.append([start,start+4])
    messages=[]
    for _ in range(count()):
        flag=read('B');length=count()
        messages.append(dict(read=flag,text=raw(length*2).decode('utf-16-le')))
    coefficients=[read('I') for _ in range(count())]
    if offset!=len(data):raise ValueError(('Trailing save data',offset,len(data)))
    return dict(bytes=len(data),seeds=seeds,sales=sales,cutoff=cutoff,messages=messages,coefficient_bits=coefficients,
                seed_padding_ranges=padding_ranges,sale_clock_ranges=[[cutoff_offset,cutoff_offset+4]]+sale_time_ranges)
