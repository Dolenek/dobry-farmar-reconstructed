#!/usr/bin/env python3
"""Compare shipped extended animation text buttons and their real owned tooltips."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/extended-buttons-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','extended-buttons'),('PC24','extended-buttons-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Extended buttons {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=38862,objects=18,reloads=54,letters=1950398,frames=72,pixels=34560000,steps=38664,sound_residues=180).items():
            assert state['extended_'+key]==value,(key,state['extended_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/extended_button_oracle_cases.inc',Path(__file__).resolve()])
    report=dict(status='passed_for_extended_animation_text_button_and_tooltip_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        states=77724,objects=36,reloads=108,letters=3900796,frames=144,pixels=69120000,steps=77328,sound_residues=360,
        original_functions=['00428370','004284c0','0042a1b0','00428590','00429bb0','004297c0','00428a90','00428ff0','004293e0','00499ca0','00499cc0','0049a340','004d2160','004cdee0'],
        coverage=['16 shipped buttons from eight farm/game menu/manufacture XML files plus explicit Texts/Pics and missing-Tip fixtures',
            'Actual owned TextLine, MultilineText, Picture, four resource appearances and button text; complete hierarchy and resource reference comparison',
            'Repeated XML appends image and tooltip entries and saved layouts without clearing old entries; retained overwritten sound references',
            'All screen quadrants including fixed 400/300 boundaries and offscreen positions; original appearance assignment preserves current play/time',
            'Hover/unhover/press/release/enable/disable, tipsDisabled, native tutorial +bfc guard; wait/fade boundary arithmetic with nonstandard directions, zero durations and NaN',
            '2048 deterministic wait/fade states per object and precision; x87 arithmetic executes in native and recovered implementations',
            '144 complete Direct3D8 frames and ordered draw API/submissions match; 128 use shipped definitions and 16 explicit fixture definitions'],
        test_adapters=['Original EXE mapped only for diagnosis without entry point; reconstructed implementation never loads original program code',
            'Native XML and old CRT typed/string services remain explicit adapters; native tooltip algorithms, constructors and destructors execute',
            'Native 004cdee0 runs against controlled tutorial +bfc fixture; production requires a tutorial-state binding and does not implement the full tutorial here',
            'Mouse callback positions and frame clock are controlled; lifecycle/action methods called directly, physical device input unverified',
            'Sound callbacks compare requests and retained references; this component test does not play physical audio',
            'Original constructor leaves waitRemaining uninitialized; diagnostic zeroed heap represents it as zero, source initializes zero without claiming normal allocator defaults'],
        pending=['Other extended-button template specializations, action area/building controllers and real GameMenu/world integration',
            'Full tutorial state machine, physical mouse/key input, complete engine frame and main EXE',
            'Native invalid allocation, exception and locale behavior, 3D resource paths, complete game/save compatibility'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Extended buttons: 77724 states, 77328 steps, 144 GPU frames/69120000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
