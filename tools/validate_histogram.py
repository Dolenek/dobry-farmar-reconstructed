#!/usr/bin/env python3
"""Compare actual Market histogram values, XML ownership and D3D8 frames."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/histogram-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','histogram'),('PC24','histogram-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Histogram {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=2696,objects=4,reloads=20,frames=32,pixels=15360000).items():assert state['histogram_'+key]==value,(key,state['histogram_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/histogram_oracle_cases.inc',ROOT/'tools/build_sprite_oracle_fixups.py',Path(__file__).resolve()])
    report=dict(status='passed_for_histogram_values_xml_ownership_and_frames',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        states=5392,objects=8,xml_reloads=40,gpu_frames=64,gpu_rgb_pixels=30720000,
        original_functions=['0046d420','0046d4a0','0046d4c0','0046d560','0046b730','0046b6f0'],
        coverage=['Actual Market.xml PriceGrowHistogram and original market texture; 0/1/3/17 synthetic bar counts with owned Lid controls',
            'Empty, short, excess, zero and negative value sequences; maximum selection and ordered updateLid calls; separate maximum override retains Threshold',
            'Full child geometry, bar/lid sprites, dirty flags, clamp side effects, visibility and reload deletion; destruction reference deltas',
            'Native full UI frame traversal compared with recovered traversal, draw submissions and every RGB pixel in two x87 precision contexts'],
        test_adapters=['Original program mapped only in diagnostic executable; no original entry point execution',
            'Existing explicit XML/typed resource and allocator boundaries; no replacement histogram methods',
            'Synthetic XML is confined to ignored diagnostic files; shipped game data remains unchanged'],
        pending=['Integration into actual Market scene and its real forecasts, sales and full cGame loop',
            'Missing mandatory Bar section, malformed XML, NaN/infinity input lifecycle and exception/allocation ABI',
            'Normal allocator identities and other locales/x87 rounding modes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Market Histogram: 5392 states, 40 XML reloads, 64 frames / 30720000 identical RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
