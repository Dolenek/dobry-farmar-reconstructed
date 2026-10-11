#!/usr/bin/env python3
"""Compare the recovered Awards tree, all 21 AwardButtons and controller callbacks."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/awards-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','awards'),('PC24','awards-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Awards {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['awards_states']==8316 and state['awards_objects']==3 and state['awards_reloads']==3
        assert state['awards_frames']==78 and state['awards_pixels']==37440000
        assert state['awards_returns']==3 and state['awards_panel_callbacks']==63 and state['awards_sound_residues']==12
        assert state['awards_letters']==287496 and state['awards_decorations']==465
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/awards_oracle_cases.inc',ROOT/'tools/awards_oracle_services.h',ROOT/'tools/loading_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_awards_controller_buttons_and_shipped_2d_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        control_states=16632,owned_screens=6,bulk_xml_reloads=6,return_callbacks=6,award_panel_callbacks=126,
        gpu_frames=156,gpu_rgb_pixels=74880000,letter_comparisons=574992,retained_button_sound_references=24,
        original_functions=['004728c0','00472d50','00474da0','00474dc0','00473150','00474910','00474520','00473d50','00474130','00473520','00473940',
            '004787e0','00478dc0','00478da0','004763a0','004776b0','00477ab0','00476ae0','00477e80','0040ab50'],
        coverage=['Actual UI/Awards.xml with all 21 owned AwardButtons, TooltipFrame/MultilineText and AwardEffect children, CaptionText and ReturnButton',
            'Native/recovered singleton construction/destruction, nullable parent, actual typed profile ownership refresh, enabled/disabled transitions',
            'Raw ownership bytes including 252 preserve button input state; hover/unhover, owned effect phase and tooltip fade, all sprite bytes and resource reference deltas',
            'Whole native/recovered frame traversal and identical GPU RGB plus ordered draw calls for every button hover and actual ReturnButton press/release',
            'XML reload description-before-button ordering; original retained state texture references and 24 overwritten ReturnButton sound references',
            'Native indexed condition String table, all 21 keys including skipped Award18 name; panel parent discard origin, real typed condition boolean commit, effect phase 4, pending clear and screen disable'],
        test_adapters=['Original PE mapped in diagnostics with entry point disabled; production never calls original code',
            'Native typed profile getter and decimal key formatting use explicit service boundaries; source accesses real recovered profile properties',
            'Native allocation, immutable XML wrappers, resource/font/mask extraction, controlled cursor/input and discard queue adapters',
            'Disabled recovered SoundManager owns source Effect sounds; native Effect sounds and ReturnButton sound requests use diagnostic resources/recorders. Audible playback is not covered',
            'Original indexed key initializer executes with CRT atexit registration captured, without executing original CRT shutdown',
            'Award-panel callback uses a real generic View/ButtonPrototype tree; the gameplay AwardPanel factory and game award manager are not exercised',
            'Diagnostic monotonic 512 MiB heap preserves equal-depth allocation order'],
        pending=['MainMenu navigation and actual gameplay AwardPanel/award manager integration, award condition detection in game world',
            'Actual sound playback in this screen, full input/device lifecycle, RTL and non-800x600 screen cases',
            'Malformed XML, missing pending effect, out-of-range keys and original C++ exception/object ABI',
            'Full engine/gameplay and saved-game compatibility'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('Awards: 16632 states, 126 award callbacks, 156 GPU frames / 74880000 identical RGB pixels')
if __name__=='__main__':main()
