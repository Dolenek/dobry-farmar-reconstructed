#!/usr/bin/env python3
"""Compare real Win32 JNG exports, guarded malformed inputs and non-game PNG fixtures."""
import argparse,datetime,hashlib,json,struct,subprocess,zlib,zipfile,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data))
def png(w,h,depth,color,raw,extra=b'',interlace=0):
    return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,depth,color,0,0,interlace))+extra+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b'')
def write(p,v):p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--probe',type=Path,default=ROOT/'build/windows/Release/jng-error-probe.exe')
    p.add_argument('--source-dll',type=Path,default=ROOT/'build/windows/Release/JNGLoad.dll')
    p.add_argument('--output',type=Path,default=ROOT/'.local/jng-error-abi-validation.json')
    args=p.parse_args();probe=args.probe.resolve(strict=True);source=args.source_dll.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář/JNGLoad.dll'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original.parent/row['path'])==row['sha256'],row['path']
    unchanged()
    work=ROOT/'.local/jng-error-abi'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    fixtures=[]
    def add(name,data,valid):
        path=work/(name+'.png');path.write_bytes(data);fixtures.append(dict(name=name,path=path,valid=valid,sha256=digest(path)))
    rgba=bytes([255,0,0,255,0,255,0,128,0,0,255,0,17,33,65,254])
    valid=png(2,2,8,6,b'\0'+rgba[:8]+b'\0'+rgba[8:]);add('rgba8',valid,True)
    add('rgb8',png(2,2,8,2,b'\0\xff\0\0\0\xff\0\0\0\0\xff\x11\x21\x41'),True)
    for depth in [1,2,4,8,16]:
        rowbytes=(2*depth+7)//8;raw=b'\0'+bytes([0x55]*rowbytes)+b'\0'+bytes([0xaa]*rowbytes)
        add('gray'+str(depth),png(2,2,depth,0,raw),True)
    add('grayalpha8',png(2,2,8,4,b'\0\0\xff\x55\x80\0\xaa\0\xff\xff'),True)
    for color,channels in [(2,3),(4,2),(6,4)]:
        pixels=bytes((n*37)&255 for n in range(4*channels*2))
        add('color'+str(color)+'depth16',png(2,2,16,color,b'\0'+pixels[:2*channels*2]+b'\0'+pixels[2*channels*2:]),True)
    palette=chunk(b'PLTE',bytes([255,0,0,0,255,0]))+chunk(b'tRNS',bytes([255,64]))
    add('palette1-transparency',png(2,2,1,3,b'\0\x40\0\x80',palette),True)
    def paeth(a,b,c):
        q=a+b-c;d=[abs(q-a),abs(q-b),abs(q-c)];return [a,b,c][d.index(min(d))]
    for filt in range(1,5):
        raw=bytearray()
        for y in range(2):
            raw.append(filt)
            for x in range(8):
                i=y*8+x;a=rgba[i-4] if x>=4 else 0;b=rgba[i-8] if y else 0;c=rgba[i-12] if y and x>=4 else 0
                pred=[0,a,b,(a+b)//2,paeth(a,b,c)][filt];raw.append((rgba[i]-pred)&255)
        add('rgba8-filter'+str(filt),png(2,2,8,6,raw),True)
    raw=bytearray();pixels=bytes((n*19)&255 for n in range(3*3*4))
    for x,y,dx,dy in [(0,0,8,8),(4,0,8,8),(0,4,4,8),(2,0,4,4),(0,2,2,4),(1,0,2,2),(0,1,1,2)]:
        if x>=3 or y>=3:continue
        for yy in range(y,3,dy):
            raw.append(0)
            for xx in range(x,3,dx):raw.extend(pixels[(yy*3+xx)*4:(yy*3+xx+1)*4])
    add('rgba8-adam7',png(3,3,8,6,raw,interlace=1),True)
    opaque=[]
    for color in [bytes([255,0,0]),bytes([0,0,255])]:
        opaque.append(png(2,2,8,2,b'\0'+color*2+b'\0'+color*2)[8:])
    def mng(frames,delay):
        header=chunk(b'MHDR',struct.pack('>7I',2,2,100,len(frames),len(frames),delay*len(frames),1))
        fram=chunk(b'FRAM',bytes([1,0,2,0,0,0])+struct.pack('>I',delay))
        return b'\x8aMNG\r\n\x1a\n'+header+b''.join(fram+image for image in frames)+chunk(b'MEND',b'')
    # MHDR/FRAM fields follow the pinned primary libmng headers and chunk reader.
    add('mng-single-opaque',mng(opaque[:1],0),True)
    add('mng-two-opaque-delay20',mng(opaque,20),True)
    add('mng-two-opaque-delay0',mng(opaque,0),True)
    with zipfile.ZipFile(original.parent/'Data/Data.pack') as archive:
        asset='Manufacture/building_mask.jng'
        entry=next(name for name in archive.namelist() if name.lower()==asset.lower())
        add('actual-building-mask-jng',archive.read(entry),True)
    add('bad-signature',bytes(8)+valid[8:],False)
    bad=bytearray(valid);bad[29]^=1;add('bad-ihdr-crc',bad,False)
    bad=bytearray(valid);bad[-1]^=1;add('bad-iend-crc',bad,False)
    add('unknown-critical',valid[:33]+chunk(b'ABCD',b'abc')+valid[33:],False)
    add('unknown-ancillary',valid[:33]+chunk(b'abCD',b'abc')+valid[33:],True)
    bad=bytearray(chunk(b'abCD',b'abc'));bad[-1]^=1
    add('bad-ancillary-crc',valid[:33]+bad+valid[33:],False)
    add('invalid-color-depth',png(2,2,16,3,b'\0\0\0\0\0'),False)
    add('zero-width',png(0,2,8,6,b'\0\0'),False)
    add('invalid-filter',png(2,2,8,6,b'\5'+rgba[:8]+b'\5'+rgba[8:]),False)
    idat=valid.find(b'IDAT');bad=bytearray(valid);bad[idat+4]^=0xff
    # Recompute chunk CRC so the failure comes from compressed contents, not CRC.
    size=struct.unpack('>I',bad[idat-4:idat])[0];bad[idat+4+size:idat+8+size]=struct.pack('>I',zlib.crc32(bad[idat:idat+4+size]))
    add('invalid-zlib',bad,False)
    for length in [1,7,8,16,32,len(valid)//2,len(valid)-1]:add('truncated-'+str(length),valid[:length],False)
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),python_version=sys.version,zlib_version=zlib.ZLIB_RUNTIME_VERSION,
        probe_path=probe.relative_to(ROOT).as_posix(),source_dll_path=source.relative_to(ROOT).as_posix(),
        probe_sha256=digest(probe),original_dll_sha256=digest(original),source_dll_sha256=digest(source),source_files={name:digest(ROOT/name) for name in ['tools/jng_error_probe.cpp','tools/validate_jng_error_abi.py','src/jngload/JNGLoad.cpp','src/jngload/JNGLoad.h','CMakeLists.txt']},cases=[],whole_game_complete=False)
    try:
        def run(label,inputs):
            runs=[]
            for kind,dll in [('native',original),('source',source)]:
                out=work/(label+'-'+kind);out.mkdir()
                result=subprocess.run([str(probe),str(dll),str(out)]+[str(row['path']) for row in inputs],capture_output=True,text=True,timeout=20)
                (out/'stdout.jsonl').write_text(result.stdout,encoding='utf-8');(out/'stderr.log').write_text(result.stderr,encoding='utf-8')
                assert result.returncode==0,(label,kind,result.returncode,result.stderr,result.stdout)
                rows=[json.loads(line) for line in result.stdout.splitlines()];assert rows[0]==dict(event='free_null',unchanged=True)
                runs.append(dict(kind=kind,events=rows,pixels={p.name:digest(p) for p in out.glob('*.bgra')}))
            equality=runs[0]['events']==runs[1]['events']
            valid_pixels=all(not row['valid'] or runs[0]['pixels'].get(str(n)+'.bgra')==runs[1]['pixels'].get(str(n)+'.bgra') for n,row in enumerate(inputs))
            known={name:bytes([0,0,255,255,0,255,0,128,0,0,0,0,65,33,17,254]) for name in ['rgba8']+[f'rgba8-filter{i}' for i in range(1,5)]+['bad-ihdr-crc','bad-iend-crc','bad-ancillary-crc','unknown-ancillary']}
            known.update({'mng-single-opaque':bytes([0,0,255,255])*4,'mng-two-opaque-delay20':bytes([0,0,255,255])*4,'mng-two-opaque-delay0':bytes([255,0,0,255])*4,'zero-width':b''})
            for n,row in enumerate(inputs):
                if row['valid']:
                    for one in runs:
                        assert one['events'][n+1]['result']=='returned' and one['events'][n+1]['free_preserves_pointer'] and str(n)+'.bgra' in one['pixels'],(label,one)
                if row['name'] in known:
                    for kind in ['native','source']:
                        assert (work/(label+'-'+kind)/(str(n)+'.bgra')).read_bytes()==known[row['name']],(label,kind,row['name'])
            report['cases'].append(dict(name=label,inputs=[{k:v for k,v in row.items() if k!='path'} for row in inputs],runs=runs,events_equal=equality,valid_pixels_equal=valid_pixels,invalid_returned_pixels_compared=False))
            write(work/'report.json',report)
        for row in fixtures:run(row['name'],[row])
        run('valid-invalid-valid',[fixtures[0],next(row for row in fixtures if row['name']=='bad-signature'),fixtures[0]])
        select=lambda name:next(row for row in fixtures if row['name']==name)
        run('png-mng-jng-errors-png',[fixtures[0],select('mng-two-opaque-delay20'),select('actual-building-mask-jng'),select('unknown-critical'),select('invalid-filter'),select('invalid-zlib'),fixtures[0]])
        report['comparison']=dict(case_process_pairs=len(report['cases']),all_events_equal=all(c['events_equal'] for c in report['cases']),all_valid_pixels_equal=all(c['valid_pixels_equal'] for c in report['cases']),invalid_pixels_claimed_equal=False)
        assert report['comparison']['all_events_equal'] and report['comparison']['all_valid_pixels_equal'],report['comparison']
        report['status']='passed_for_actual_jng_error_abi_png_and_selected_mng_inputs_only'
    except Exception as e:
        import traceback
        report['status']='failed';report['error']=repr(e);report['traceback']=traceback.format_exc()
    finally:
        unchanged();report['original_files_unchanged']=len(inventory['files'])
        report['pending']=['Any recorded event or valid-pixel differences','Allocation/registration failures, concurrency and all remaining nonasset LIBMNG inputs','Entire reconstruction goal']
        write(work/'report.json',report);write(args.output,report)
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],comparison=report.get('comparison'),error=report.get('error'))))
    if report['status']=='failed':raise SystemExit(1)
if __name__=='__main__':main()
