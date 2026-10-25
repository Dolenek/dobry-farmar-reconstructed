#!/usr/bin/env python3
"""Compare actual HelpView pages in the rendered GamePauseMenu input flow."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_actual_help_pages_pause_navigation_reload_input_and_frames'

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/help-view-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'help-view'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=120)
        if run.returncode:raise RuntimeError(f'HelpView {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        expected=dict(help_states=15,help_actions=12,pause_states=24,pause_actions=26,pause_frames=188,pause_pixels=90240000,pause_loading_steps=48)
        for key,value in expected.items():assert state[key]==value,(precision,key,state[key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_help_view.py','tools/text_buttons_reference_test.cpp',
           'tools/build_sprite_oracle_fixups.py','tools/text_buttons_oracle_fixups.inc','tools/monotonic_ui_test_heap.h',
           'tools/help_view_oracle_cases.inc','tools/game_pause_oracle_cases.inc','tools/main_menu_oracle_services.h',
           'tools/game_pause_oracle_services.h','tools/loading_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        contexts=contexts,states=30,explicit_cpu_actions=24,gpu_frames=376,gpu_rgb_pixels=180480000,
        pause_states=48,pause_explicit_actions=52,
        original_functions=['0047fa60','0047dde0','0047f370','0047dbe0','0047dc80','0047ed50',
            '0047de80','0047e250','0047e690'],
        coverage=[
            'Actual HelpView constructor/destructor/singleton, all three actual TextAnimationButton controls and both actual HelpPage ownership trees',
            'Shipped pictures and localized MultilineText, generated lines and complete UI graph compared against actual original instructions',
            'Actual GamePauseMenu opens Help through ordinary mouse input; repeated Next/Previous callbacks update page visibility and boundary buttons',
            'Actual Help close returns to actual pause menu while gameplay remains paused',
            'Full XML reload replaces indexed pages while old View-owned page graphs remain children, followed by another real mouse navigation cycle',
            'Whole diagnostic scene also performs actual Options reuse, Awards return, confirmation dialog, loading and MainMenu construction',
            '376 full D3D8 frames compare draw calls and180480000 exact RGB pixels across both precisions',
            'Final singleton, control/input registries and texture reference destruction deltas match'],
        test_adapters=[
            'GamePauseContext now receives actual HelpView; the older pause-only mode retains its documented explicit Help boundary',
            'The complete cGame remains pending; the surrounding game in this comparison is the previously documented View/discard observer',
            'Profile, loading, graphics reset, clock/cursor, disabled UI sound and other existing diagnostic adapters retain their documented roles',
            'The final ctor hide receiver is PrevButton (0047fc5d), not HelpView; this was established from ECX in original instructions',
            'Production HelpView executes only reconstructed source; mapped original code runs solely as the comparison reference'],
        pending=['Actual Journal and gameplay award manager dependencies of cGame','Complete cGame, startup and main EXE',
            'Full actor/tool cycles on actual Farm and bidirectional complete saves','Complete97-export Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('HelpView: 30 states / 24 operations; actual pause/page mouse navigation and reload; 376 frames / 180480000 identical RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
