#!/usr/bin/env python3
"""Compare all original AwardEffect states and all shipped award image definitions."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/award-effect-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','effect'),('PC24','effect-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'AwardEffect {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['effect_states']==5439 and state['effect_objects']==21 and state['effect_reloads']==126
        assert state['effect_steps']==5040 and state['effect_sounds']==21
        assert state['effect_frames']==84 and state['effect_pixels']==40320000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/award_effect_oracle_cases.inc',ROOT/'tools/loading_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_award_effect_state_machine_and_shipped_2d_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        control_states=10878,owned_objects=42,bulk_xml_reloads=252,steps=10080,sound_callbacks=42,
        gpu_frames=168,gpu_rgb_pixels=80640000,
        original_functions=['004c6a40','004c61e0','004c60e0','004c61c0','004c5b70','004c5bc0','00421520'],
        coverage=['All 21 actual UI/Awards.xml Effect/Button definitions, labels, constructor XML deletion and sound ownership',
            'Native constructor/loadXML/state setter/steps/destructor, nullable parent, phases 0..8 and unknown 9..11, random scale/time and hover flag',
            'Exact whole sprite bytes, state/flags/size/position/depth, appearance, timed animation, move completion and hover grow/shrink under PC53/PC24',
            'Native/recovered whole-frame traversal and ordered GPU draw: all 21 actual textures across phase paths, 168 exact RGB frames',
            'Explicit root Sound XML using a shipped sound, completion callback dispatch and destructor reference release'],
        test_adapters=['Original PE mapped in diagnostics with entry point disabled; production never calls original code',
            'XML/resource/font/typed extraction and allocation use existing explicit diagnostic boundaries',
            'Recovered SoundManager deliberately initialized disabled; click callback recorder calls its real disabled path. Audible playback is not verified here',
            'Real recovered GameTimer with controlled WinMM clock, controlled input and cursor',
            'Diagnostic monotonic 512 MiB heap preserves equal-depth allocation order; four GPU frames per award cover every image and each phase across the suite'],
        pending=['AwardButton, Awards controller, MainMenu navigation and gameplay award manager',
            'Actual root Sound playback with source Squall; shipped Awards root has no Sound and the branch uses explicit XML fixture',
            'Repeated nonempty Sound reload retains earlier references in original; not claimed verified here',
            'Degenerate move denominator, nonfinite values, malformed XML and original exception/object ABI',
            'Full engine/gameplay and saved-game compatibility'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('AwardEffect: 10878 states, 10080 steps, 168 GPU frames / 80640000 identical RGB pixels')
if __name__=='__main__':main()
