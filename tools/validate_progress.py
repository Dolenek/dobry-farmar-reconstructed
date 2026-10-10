#!/usr/bin/env python3
"""Compare native ProgressBar geometry and owned loading-screen controls."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sprite-oracle',type=Path,default=ROOT/'build/windows/Release/progress-sprite-reference-test.exe')
    parser.add_argument('--ui-oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/progress-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert digest(original)==expected
    sprite=args.sprite_oracle.resolve(strict=True);ui=args.ui_oracle.resolve(strict=True)
    contexts=[]
    for precision,mode in [('PC53','progress'),('PC24','progress-device')]:
        command=[str(sprite),str(original)]+(['device'] if precision=='PC24' else [])
        result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode: raise RuntimeError(f'Progress geometry {precision}: {result.returncode} {result.stderr}')
        geometry=json.loads(result.stdout);assert geometry['sprite_comparisons']==100000
        result=subprocess.run([str(ui),str(original),str(original.parent/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode: raise RuntimeError(f'Progress control {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['progress_states']==3684 and state['progress_objects']==12 and state['progress_reloads']==24
        assert state['progress_frames']==48 and state['progress_pixels']==23040000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,geometry=geometry,control=state))
    assert digest(original)==expected
    paths=[*sorted((ROOT/'src/game').rglob('*.h')),*sorted((ROOT/'src/game').rglob('*.cpp')),
        ROOT/'tools/progress_sprite_reference_test.cpp',ROOT/'tools/progress_oracle_cases.inc',
        ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/progress_sprite_oracle_fixups.inc',
        ROOT/'tools/text_buttons_oracle_fixups.inc',Path(__file__).resolve(),ROOT/'CMakeLists.txt']
    report=dict(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=expected,reference_files_modified=False,original_entry_point_executed=False,
        original_functions=['004f91e0','004f9230','004f9270','004f9290','004f9b20','004f9f80'],
        source_files={str(path.relative_to(ROOT)).replace('\\','/'):digest(path) for path in paths},
        oracle_sha256=dict(sprite=digest(sprite),ui=digest(ui)),precision_contexts=contexts,
        sprite_comparisons=200000,control_states=7368,owned_objects=24,xml_reloads=48,
        gpu_frames=96,gpu_rgb_pixels=46080000,
        coverage=['Entire 76-byte transformed sprite and clamped current value; four sides and invalid sides, signed sizes/angles, atlas frames, zero/negative maximum and parent alpha',
            'Actual shipped UI/Load.xml and Progress texture, owned nullable-parent native/source construction, XML load, Lid geometry, missing Side overwrite, repeated XML retained child ownership, release deltas',
            'Original native full UI frame and recovered frame traversal over actual D3D8 device; exact ordered submissions, draw calls and all RGB pixels'],
        test_adapters=['Original PE mapped only inside diagnostic executables, entry point never called',
            'Allocator and XML typed scalar/resource service boundaries use existing documented text-buttons diagnostic adapters',
            'Explicit synthetic Lid XML uses original shipped texture; no synthetic content is installed in game data',
            'Separate pure transform fixture selects the native virtual frame result; full control comparison uses native animation/frame methods',
            'Controlled WinMM clock/input/cursor; production graphics initialization and full game navigation remain pending'],
        pending=['LoadingFromLog controller and resource log loading/music lifecycle',
            'Original object/exception ABI, malformed typed XML and NaN/infinity control lifecycle',
            'Entire game runtime and new-game transition'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('ProgressBar: 200000 sprites, 7368 states, 96 GPU frames / 46080000 identical RGB pixels')
if __name__=='__main__':main()
