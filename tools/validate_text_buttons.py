#!/usr/bin/env python3
"""Compare owned text buttons and native menu XML methods with explicit service boundaries."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/text-buttons-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision in ('preserved','original-device-0x20'):
        command=[str(executable),str(original),str(original.parent/'Data')]
        if precision=='original-device-0x20':command.append('device-precision')
        result=subprocess.run(command,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Owned text button oracle failed ({result.returncode}, {precision}): {result.stderr}')
        context=json.loads(result.stdout)
        assert context['scenarios']==1080 and context['lines_destroyed']==9 and context['letter_comparisons']>4000
        assert context['menu_views_destroyed']==3 and context['decoration_comparisons']==15
        assert context['menu_content_comparisons']==27
        assert context['menu_frames']==36 and context['menu_pixels']==17280000
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in ('scenarios','letter_comparisons','lines_destroyed','decoration_comparisons','menu_views_destroyed','menu_content_comparisons','menu_frames','menu_pixels')}
    report['precision_contexts']=contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['0051fe90','0041e8d0','0051f5a0','0041ed10','0051f570','0051f9b0','0041f120',
            '0051dbf0','0041f5f0','004dd980','004d43a0','004eded0','004d22a0','004f1860','004f4b80','004f20f0','00508560',
            '005202d0','004fe470','0051f540','0046bea0','00508120','004f5480','004da840','004d58a0',
            '004fb040','004fa8c0','004fafc0','00481e10','00481770','00531b20','00506a10','0052fe50','00515830'],
        test_adapters=['Allocation/free and String/WString operations use diagnostic CRT/recovered ABI strings',
            'XML virtual attribute/section calls wrap recovered trees parsed from original packages',
            'Typed XML scalar/vector/color conversion uses controlled modern stream adapters; old CRT extraction is unverified',
            'Native font lookups execute original function over original-layout fixtures populated from shipped font records',
            'Resource factories return recovered GPU textures and diagnostic sound references; audible playback is unverified',
            'Native click-mask factory boundary uses recovered mask loading, separately compared over all shipped JNGs',
            'Cursor and timer are controlled; native input registration uses recovered binding sets',
            'Three texture-switching parents use a separate base XML pass from the shipped Options/SoundVolume/Slider definition',
            'Main-menu object construction follows 00484da0 control allocations without executing its singleton/action wiring',
            'Existing fixed-background singleton exercises the guard of 00481990; generic MenuView hooks replace MainMenu hooks in this fixture',
            'Profile names use an original-layout WString fixture; real profile persistence is unverified'],
        coverage=['Six shipped MainMenu animated buttons: constructor, native XML, localized owned letters, parent state/alpha/position/text changes, sprites and owned destruction',
            'Three texture-switching owned text buttons: native parent/text construction and XML, nonnull texture switches, per-texture reference deltas and child destruction',
            'MenuView MainMenu/FixedBack/Options decorations: repeated XML loads, native sprites, owned destruction and per-texture reference deltas',
            'Whole main-menu content: seven owned text buttons, two text lines, three backgrounds, native 00481e10 XML bindings and 20 profile-name refresh sequences per context',
            '36 whole-scene menu RGB comparisons per context: independent source/native hierarchies and GPU buffers; native 00531b20 step/transition frame, 00506a10 traversal, 0052fe50 submission and 00515830 draw; transparent hide/show through completion',
            'Both original GPU x87 precision contexts'],
        pending=['Original XML parser and old CRT typed conversion ABI','Full object/exception ABI',
            'MenuView BackText/BackMultitext branches; original MainMenu loader 00481e10 ignores legacy submenu sections',
            'MainMenu singleton, navigation callbacks and unusual enable/disable hooks','Whole menu physical input/frame service integration and actual navigation','Actual sound loading/playback','Complete main game EXE'])
    report['test_adapters'] += ['Test-only monotonic allocation preserves matching address ranks: original equal-depth UI comparator orders by object pointer; production comparators remain unchanged',
        'Native graphics virtual submission boundary uses recovered SpriteBatches; native batching routines themselves are not executed by this scene test',
        'Source scene processes reconstructed GameTimer over controlled milliseconds and uses its smoothed elapsed value in UI motion/transitions; timer is separately compared over 120000 native frames',
        'Native cursor update, timer advance, input process and graphics begin/finish are controlled empty test services; native elapsed getter freezes the same recovered timer value after the source advance; sound/mesh/extra-drawable/discard registries are empty',
        'Native draw fixture uses sprite-only linked batches and previously checked recovered render-state setup; device creation, reset, mesh and complete graphics singleton ABI remain separate work',
        'GPU readback ignores only the undefined X byte of X8R8G8B8; compares all RGB bytes exactly',
        'Draw API trace normalizes only distinct source/native vertex/index buffer identities; texture identities and draw parameters remain exact']
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Owned text buttons: {report['scenarios']} scenarios, {report['letter_comparisons']} letters/sprites, {report['lines_destroyed']} owned trees destroyed")
if __name__=='__main__':main()
