#!/usr/bin/env python3
"""Verify all shipped JPEG storage, exact MMX colors and complete D3DX GPU textures."""
import argparse,datetime,hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_all_shipped_jpeg_storage_all_mmx_color_triples_and_whole_gpu_textures'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/jpeg-reference-test.exe');p.add_argument('--output',type=Path,default=ROOT/'.local/jpeg-validation.json');args=p.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář';oracle=args.oracle.resolve(strict=True);inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',[]),('PC24',['device'])]:
        r=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data')]+suffix,cwd=ROOT,capture_output=True,text=True,timeout=90)
        if r.returncode:raise RuntimeError(f'JPEG {precision}: {r.returncode} {r.stderr}')
        d=json.loads(r.stdout)
        for key,value in dict(jpeg_images=3,color_triples=16777216,native_mmx=1,source_bytes=782080,gpu_texture_pixels=195520,reference_relocated=True).items():assert d[key]==value,(precision,key,d[key])
        assert d['device_flags']==(0x22 if precision=='PC53' else 0x20);contexts.append(dict(precision=precision,result=d));unchanged()
    extra=['CMakeLists.txt','analysis/codecs.lock.json','tools/cmake/OriginalCodecs.cmake','tools/fetch_codecs.py',
           'tools/validate_jpeg.py','tools/jpeg_reference_test.cpp','tools/click_mask_oracle_fixups.inc','tools/build_sprite_oracle_fixups.py']
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[ROOT/name for name in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=17,
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,oracle_sha256=digest(oracle),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},contexts=contexts,
        shipped_jpeg_images=['logo.jpg','logo/logo2.jpg','logo/logo3.jpg'],source_bytes_per_context=782080,gpu_texture_pixels_per_context=195520,
        mmx_color_triples_per_context=16777216,original_functions=['005975c7','005a1b47','005a7b11','005a79d4','00592e55'],
        decoder_dependency='IJG6a, JPEG_LIB_VERSION61, jpeg_decompress_struct424 bytes; pinned separately from JNGLoad IJG6b',
        coverage=['Every decoded byte including zero X byte, format22, dimensions/pitch for all three original logo JPEGs',
            'All16777216 Y/Cb/Cr triples compared with actual original MMX coefficients, arithmetic shifts and saturation',
            'Original CPUID MMX path retained and matched; no reference decoder/IDCT/color callbacks replaced',
            'Whole allocated GPU textures match original D3DX: dimensions, format, pool, mips and all4bytes per texel, PC53/PC24',
            'Original image forcibly mapped away from preferred address; missing DQT, main-buffer, indexed Huffman and X8 converter relocations repaired'],
        test_adapters=['Original mapped without entry point; only old CRT allocation/free/memmove adapted',
            'Real Direct3D8 HAL software vertex processing; GPU/window are diagnostic fixtures',
            'Exhaustive direct MMX color call clears MMX tags after each batch, matching surrounding decoder boundary; source conversion is integer C++',
            'Native byte storage readers and native D3DX algorithms execute actual bodies; source implementation uses reconstructed wrapper and pinned source library'],
        pending=['Complete startup/engine/main EXE','Non-MMX native decoder path (diagnostic attempt failed before comparison)',
            'JPEG inputs outside shipped assets, scalar tails, grayscale/CMYK/progressive/corrupt/truncated cases and native exceptional ABI',
            'General D3DX format fallback/filtering beyond covered formats/caps','Bidirectional full-game saves, complete Squall behavior and isolated uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('JPEG: all3 logos / 782080 source bytes / 195520 whole GPU texels and16777216 color triples per PC53/PC24;17 originals unchanged')
if __name__=='__main__':main()
