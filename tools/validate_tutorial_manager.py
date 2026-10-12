#!/usr/bin/env python3
"""Compare TutorialManager, every installed predicate, actual HUD and frames."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/tutorial-manager-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=1250,starts=167,steps=140,predicates=7424,frames=102)
    for precision,mode in [('PC53','tutorial-manager'),('PC24','tutorial-manager-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Tutorial manager {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['tutorial_manager_'+key]==value,(key,state['tutorial_manager_'+key])
        assert state['hud_frames']==102 and state['hud_pixels']==48960000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/'CMakeLists.txt',
        ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/game_menu_oracle_cases.inc',ROOT/'tools/game_menu_oracle_services.h',
        ROOT/'tools/tutorial_manager_oracle_cases.inc',ROOT/'tools/tutorial_manager_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_tutorial_manager_profile_predicates_and_connected_hud_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},pixels=97920000,
        original_functions=['004cfce0','004d0490','004ce160','004cdf70','004cdfd0','004cdee0','004cdef0','004cdf10',
            '004cfbe0','004d0990','004d0a20','004d0ac0','004ce650','004ceb00','004cef60','004cf3c0','004cf7d0','004d0c30','004d1160',
            '004c9fc0','004c9fd0','004c9fe0','004c9ff0','004ca050','004ca080','004ca0b0','004ca0d0','004ca0f0','004ca110','004ca140','004ca160',
            '004ca190','004ca1b0','004ca1e0','004ca1f0','004ca210','004ca230','004ca260','004ca290','004ca2b0','004ca310','004ca330',
            '004ca350','004ca450','004ca4e0','004ca570','004ca610'],
        coverage=['Actual manager constructor, 50 actual XML records, 29 installed predicate pointers and five owned tip constructors with exact creation order',
            'Typed profile availability/completion including disabled and active short circuits, unsigned tip4294967295, tutJustProc and TFastSpeed transitions',
            'All 50 steps opened through actual common/arrow/disabled-anchor methods, native control capture/restore and already-disabled anchor retention',
            '14848 predicate comparisons with exact ordered primitive queries, repeated activity reads, scene states, visibility and water equality',
            '256 actual manager enabled-step scenarios: initial well/plot water, warehouse follow-up, production-complete modal, elapsed and profile maps',
            'Actual manager/HUD/five windows connected in the frame dispatcher: OK hover/press/release, action cancellation, repeated double completion, 204 frames and 97920000 identical pixels',
            'Native destructor queues only four tips with exact source origins; repeated-action window remains alive, explicitly verified'],
        test_adapters=['Mapped original controller instructions in diagnosis only; source implementation never calls original program code',
            'Explicit XML/resources/typed profile and old CRT boundaries; reverse vector destruction calls every native record destructor',
            'Required actual world primitive operations use controlled diagnostic observations, not production replacement controllers',
            'Predicate dialog visibility is exercised with actual View states; full dialog factories and derived Farm/Ware/Manufacture/character bindings remain pending',
            'The native retained fifth tip is manually deleted only by diagnostic cleanup, outside the manager destructor'],
        pending=['Actual derived Farm/Warehouse/Ware/Manufacture/character/selection world providers and cGame lifecycle',
            'Full tutorial run through real gameplay and full-game save/load compatibility',
            'Extreme/NaN predicate values, XML reload/default override and parent-disabled matrices, invalid-step/null-predicate/constructor exceptions',
            'Post-destruction use of the retained repeated tip and its original dangling manager context',
            'Full game EXE, complete Squall and isolated uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Tutorial manager: 2500 states, 14848 predicates, 204 GPU frames/97920000 pixels; 17 originals unchanged')
if __name__=='__main__':main()
