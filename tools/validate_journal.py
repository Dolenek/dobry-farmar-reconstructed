#!/usr/bin/env python3
"""Compare actual Journal, HUD messages and MultilineText copies in full UI frames."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_actual_journal_hud_messages_multiline_copy_layout_reload_and_input'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/journal-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'journal'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=120)
        if run.returncode:raise RuntimeError(f'Journal {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        for key,value in dict(states=215,actions=51,frames=181,pixels=86880000).items():assert state['journal_'+key]==value,(precision,key,state['journal_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_journal.py','tools/text_buttons_reference_test.cpp',
           'tools/build_sprite_oracle_fixups.py','tools/text_buttons_oracle_fixups.inc','tools/monotonic_ui_test_heap.h',
           'tools/journal_oracle_cases.inc','tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h',
           'tools/main_menu_oracle_services.h','tools/contract_status_oracle_services.h','tools/loading_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        contexts=contexts,states=430,explicit_cpu_actions=102,gpu_frames=362,gpu_rgb_pixels=173760000,
        original_functions=['00446aa0','00446d10','00445880','00446420','00446740','00446d50',
            '00445780','004457e0','00446a60','00446a80','004f6000','004f5db0','004f5cb0','004db5e0'],
        coverage=[
            'Actual Journal constructor/destructor/singleton, real up/down/OK controls and actual MultilineText prototype',
            'Actual HUD message vectors with1,2,6,12,28 records, mixed read flags and wrapped/multiple lines',
            'Actual HUD mouse button dispatches actual Journal.open, disables normal/absolute HUD processing and lays out latest messages',
            'Actual mouse up/down, both original layout directions and explicit negative/middle/final/beyond-end index clamping',
            'Actual OK returns to HUD, marks all messages read and resets the journal button to normalColor',
            'Actual full XML reload retains original decoration behavior and another actual HUD open/close cycle',
            'Fresh MultilineText copies own copied prototypes but initially empty text/line pools; original measuredSize and post-generation color ordering are preserved',
            'All displayed text/prototype geometry, line pools, measurements, profile maps and ordered HUD observations match',
            '362 whole D3D8 frames compare commands and173760000 exact RGB pixels; texture lifetime and singleton cleanup match'],
        test_adapters=[
            'Actual GameMenuWorld.openMessages resolves actual Journal in this mode; native00446d50 executes its actual relocated body',
            'The complete cGame remains pending. Other HUD scene/stock/gameplay/tutorial operations retain documented observers',
            'Original TextLine.textHeight is reached through a virtual call; its004db603 cookie operand is now explicitly relocated. Cookie checks remain enabled',
            'Clock/cursor, profile and numeric configuration adapters, minimal surrounding StateViews and disabled UI audio retain existing diagnostic roles',
            'The original undefined empty-message open and exceptional input paths are not claimed by this valid-message comparison',
            'GamePause help dependency is a mandatory deferred provider, allowing cGame to preserve original construction order; old Pause/actual Help regressions must pass',
            'Production Journal contains reconstructed source only; original mapped instructions run solely as comparison reference'],
        pending=['Actual gameplay award manager and complete cGame factory/bindings','Main EXE startup and full actor/tool cycles on actual Farm',
            'Bidirectional full save files','Complete97-export Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Journal: 430 states / 102 operations; actual HUD/mouse/read flags, text copy/layout/reload; 362 frames / 173760000 identical RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
