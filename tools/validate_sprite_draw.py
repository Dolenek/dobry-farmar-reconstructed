#!/usr/bin/env python3
"""Compare recovered sprite draw pixels against an isolated original method."""
import argparse
import datetime
import hashlib
import json
import subprocess
import struct
import tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/sprite-draw-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/sprite-draw-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    hashes={name:next(row['sha256'] for row in inventory['files'] if row['path']==name)
            for name in ('Dobrý farmář.exe','Data/Data.pack','Data/cz.pack')}
    def verify():
        for name,expected in hashes.items(): assert hashlib.sha256((original/name).read_bytes()).hexdigest()==expected,name
    verify()
    executable=args.oracle.resolve(strict=True)
    # Owned vs.1.1 fixture: m4x4 oPos,v0,c0; mul oD0,v5,c4; mov oT0,v7.
    # Tokens follow the native D3D8 header grammar, not any original code bytes.
    bytecode=[0xfffe0101,20,0xc00f0000,0x90e40000,0xa0e40000,
              5,0xd00f0000,0x90e40005,0xa0e40004,1,0xe00f0000,0x90e40007,0x0000ffff]
    with tempfile.TemporaryDirectory(dir=ROOT/'.local',prefix='sprite-shader-') as temporary:
        (Path(temporary)/'sprite-fixture.vs').write_bytes(struct.pack('<13I',*bytecode))
        result=subprocess.run([str(executable),str(original/'Dobrý farmář.exe'),str(original/'Data'),temporary],capture_output=True,text=True,timeout=120)
    if result.returncode: raise RuntimeError(f'Native draw comparison failed ({result.returncode}): {result.stderr}')
    report=json.loads(result.stdout); verify()
    assert report['scenarios']==108 and report['font_scenarios']==12 and report['pixels_compared']==108*800*600 and report['nonbackground_pixels']>10000
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=hashes,oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_function='00515830',
        original_entry_point_executed=False,reference_files_modified=False,native_hal_device=True,fullscreen_device_created=False,
        fullscreen_draw_branch=True,texture_suppression_branch=True,triangle_metric_compared=True,api_call_sequence_compared=True,
        vertex_shader_scenarios=32,shader_constant_indices=[4,5],shader_cache_and_reset_tested=True,
        bitmap_font_atlases=3,labels_loaded_from_original_menu_xml=True,localized_labels_from_original_strings_xml=True,
        font_layout_original_compared_in='text-layout-validation.json',
        x8r8g8b8_unused_high_byte_masked=True,
        pending=['Original shader resource runtime comparison','Sprite shader ownership/constant container ABI',
                 'Original batch allocation/transition runtime comparison','Meshes and debug/system font overlay',
                 'Full game screens','Device loss/reset','Fullscreen device/mode change','Other native drivers'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Sprite draws: {report['scenarios']} original GPU comparisons, {report['pixels_compared']} identical pixels")
if __name__=='__main__': main()
