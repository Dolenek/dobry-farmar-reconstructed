#!/usr/bin/env python3
"""Compare owned native AwardTip, appearance assignment, text and real GPU frames."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/tooltip-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','tooltip'),('PC24','tooltip-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Tooltip {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['tooltip_states']==3660 and state['tooltip_objects']==12 and state['tooltip_reloads']==360
        assert state['tooltip_placements']==1440 and state['tooltip_letters']==88558
        assert state['tooltip_frames']==48 and state['tooltip_pixels']==23040000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/tooltip_oracle_cases.inc',Path(__file__).resolve()])
    report=dict(status='passed_for_owned_award_tooltip_and_shipped_2d_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        control_states=7320,owned_objects=24,xml_reloads=720,placements=2880,letter_comparisons=177116,
        gpu_frames=96,gpu_rgb_pixels=46080000,
        original_functions=['004c8d10','004c8e00','004c8e80','004c95c0','004c9a80','004d2080','004d2130','004d2160','004d22a0'],
        coverage=['Actual UI/AwardTip.xml, all 21 actual UI/Awards.xml descriptions, owned MultilineText/lines/letters and four cAnimable resources',
            'Original constructor/XML/placement/step/destructor; nullable parent, show/hide propagation, half-screen ties, outside-screen correction',
            'Both x87 precisions, full sprite bytes and exact texture reference deltas across changes, XML reloads and destruction',
            'Alternate 2D atlas/length/loop XML, missing Podlogka, retained fade/time/playing, timer unscaledElapsed with non-unit multiplier',
            'Native/recovered whole frame traversal and ordered GPU draw calls: 96 exact RGB frames'],
        test_adapters=['Original PE mapped in diagnostics with entry point disabled; implementation contains no original-code calls',
            'XML/resource/font/typed extraction and allocation use existing explicit diagnostic boundaries',
            'Graphics size callbacks use 800x600; real recovered GameTimer with controlled WinMM clock, controlled input and cursor'],
        pending=['Animated AwardEffect, AwardButton/controller callbacks and actual MainMenu navigation',
            'cAnimable 3D mesh/animation resources explicitly rejected pending their actual resource factory reconstruction',
            'Malformed/nonfinite geometry, huge unsigned graphics sizes, XML parse failures and exception/object ABI',
            'Full engine, gameplay and saved-game compatibility'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('AwardTip: 7320 states, 720 XML reloads, 96 GPU frames / 46080000 identical RGB pixels')
if __name__=='__main__':main()
