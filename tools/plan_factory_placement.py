#!/usr/bin/env python3
"""Plan seven real building placements from shipped geometry and decoded alpha; GUI validation remains required."""
import argparse,hashlib,json,struct,subprocess,zipfile,xml.etree.ElementTree as ET
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def f32(value):return struct.unpack('<f',struct.pack('<f',value))[0]
def contains(rect,point):return rect[0]<=point[0]<=rect[2] and rect[1]<=point[1]<=rect[3]
def corners(rect):return [(rect[0],rect[1]),(rect[2],rect[1]),(rect[2],rect[3]),(rect[0],rect[3])]
def touches(a,b):return any(contains(a,p) for p in corners(b)) or any(contains(b,p) for p in corners(a))

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,default=ROOT/'.local/seven-factory-placement-plan.json')
    args=p.parse_args();work=ROOT/'.local/placement-packing';work.mkdir(exist_ok=True)
    archive=ROOT.parent/'farmář/Dobrý farmář/Data/Data.pack'
    with zipfile.ZipFile(archive) as z:
        def resource(name):return z.read(next(key for key in z.namelist() if key.lower()==name.lower()))
        scene=ET.fromstring(resource('UI/gameUI/Manufacture.xml'));house=ET.fromstring(resource('Manufacture/CowShed.xml'))
        name=house.get('texture_mask');(work/'building_mask.jng').write_bytes(resource(name))
    decoder=ROOT/'build/windows/Release/jng-decode.exe';raw=work/'building_mask.bgra'
    result=subprocess.run([str(decoder),str(work/'building_mask.jng'),str(raw)],check=True,capture_output=True,text=True)
    w,h=map(int,result.stdout.split());data=raw.read_bytes();assert len(data)==w*h*4
    width,height=map(int,house.get('size').split());assert width==height==200
    # 004f0510 samples each mask with (count-1)*relative*(1/extent),
    # rounded at the original PC24 spill points. All proposed positions are integral.
    xx=[int(f32(f32((w-1)*x)*f32(1/width))) for x in range(width)]
    yy=[int(f32(f32((h-1)*y)*f32(1/height))) for y in range(height)]
    rows=[sum((1<<x) for x in range(width) if data[(yy[y]*w+xx[x])*4+3]) for y in range(height)]
    allowed=tuple(map(float,scene.get('DepotArea').split()))
    forbidden=[tuple(map(float,n.get('Rect').split())) for n in scene.find('AntiDepotAreaList')]
    positions=[(317,124),(482,124),(235,269),(400,269),(564,269),(317,414),(482,414)]
    def bounds(at):return (at[0]-100,at[1]-100,at[0]+100,at[1]+100)
    def overlap(a,b):
        a,b=bounds(a),bounds(b);left=min(a[0],b[0]);low=max(a[1],b[1]);high=min(a[3],b[3])
        if min(a[2],b[2])<=max(a[0],b[0]) or low>=high:return False
        return any((rows[y-a[1]]<<(a[0]-left))&(rows[y-b[1]]<<(b[0]-left)) for y in range(low,high))
    for at in positions:
        box=bounds(at);assert all(contains(allowed,point) for point in corners(box)),at
        assert not any(touches(rect,box) for rect in forbidden),at
    collisions=[(i,j) for i in range(7) for j in range(i) if overlap(positions[i],positions[j])]
    assert not collisions,collisions
    report=dict(status='offline_candidate_only_requires_actual_native_and_source_placement',factory_kind=2,geometry=[width,height],allowed=allowed,forbidden=forbidden,
        positions=positions,checked_pairs=21,mask_overlap_pairs=collisions,mask_asset=name,mask_sha256=digest(work/'building_mask.jng'),decoded_sha256=digest(raw),
        decoder_sha256=digest(decoder),driver_sha256=digest(__file__),original_archive_sha256=digest(archive),whole_game_complete=False)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report))
if __name__=='__main__':main()
