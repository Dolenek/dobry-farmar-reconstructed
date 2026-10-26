#!/usr/bin/env python3
"""Compare real Direct3D8 JNG textures with original DLL pixels, without game boot."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--test',type=Path,default=ROOT/'build/windows/Release/texture-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/texture-validation.json')
    args=parser.parse_args()
    executable=args.test.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    hashes={}
    for name in ('Data/Data.pack','Data/cz.pack','JNGLoad.dll'):
        expected=next(row['sha256'] for row in inventory['files'] if row['path']==name)
        actual=hashlib.sha256((original/name).read_bytes()).hexdigest()
        assert actual==expected,name
        hashes[name]=actual
    result=subprocess.run([str(executable),str(original/'Data'),str(original/'JNGLoad.dll')],
                          capture_output=True,text=True,check=True,timeout=180)
    report=json.loads(result.stdout)
    assert report['textures_compared']==425 and report['native_textures_compared']==13,('Incomplete image texture coverage',report)
    report.update(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  executable_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),reference_sha256=hashes,
                  original_dll_pixels_compared=True,original_graphics_runtime_compared=False,
                  disguised_jng_dds_files_compared=9,native_source_storage_original_compared_in='native-image-validation.json',
                  original_d3dx_gpu_loader_compared=False,jpeg_d3dx_gpu_loader_comparison_in='jpeg-validation.json',
                  native_d3d8_device=True,padding_texels_compared=False,status='passed',
                  pending=['Original cGraphics device setup and runtime comparison','Padding texel state',
                           'Full game screens/fonts','Reset/lost device and engine resource ABI',
                           'Original D3DX GPU texture loader','Non-JNG format fallback and other native devices'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Direct3D8 textures: {report['textures_compared']} uploads, {report['pixels_compared']} pixels exactly match original JNG DLL")

if __name__=='__main__': main()
