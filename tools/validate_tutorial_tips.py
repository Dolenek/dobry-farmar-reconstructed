#!/usr/bin/env python3
"""Compare five recovered tutorial windows against original instructions and pixels."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/tutorial-tips-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(objects=5,states=1737,letters=82860,starts=250,steps=1300,frames=32)
    for precision,mode in [('PC53','tutorial-tips'),('PC24','tutorial-tips-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Tutorial tips {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['tutorial_tip_'+key]==value,(key,state['tutorial_tip_'+key])
        assert state['hud_frames']==32 and state['hud_pixels']==15360000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/'CMakeLists.txt',
        ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/game_menu_oracle_cases.inc',ROOT/'tools/game_menu_oracle_services.h',
        ROOT/'tools/tutorial_tips_oracle_cases.inc',ROOT/'tools/tutorial_tips_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_five_tutorial_tip_windows_and_input_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},pixels=30720000,
        original_functions=['004cdde0','004cc150','004cc700','004ca6e0','004cac90','004cd400','004cb8f0',
            '004cc780','004cb4d0','004cc7c0','004cb7e0','004cc290','004ca7d0','004caf70',
            '004cbd60','004ca690','004cac10','004cda00','004cc320','004ccc40','004cd020','00413bf0','00413c10','004f6630','004f6e80'],
        coverage=['Five actual tutorial windows and their native owned control trees, including the retained first ArrowTip image subtree',
            'All 50 actual Russian tutorial texts in every window: 500 starts across both x87 precision contexts',
            'Anchor quadrants, null anchors where valid, arrow inversion, original integer coordinate truncation, carried animation phase and 2600 steps',
            'Action cancellation and predicate calls, repeated-action double completion, actual GameMenu pause flag and disabled-anchor restoration',
            '64 actual D3D8 frames, ordered sprites and 30720000 matching RGB pixels; actual hover/press/release OK input'],
        test_adapters=['Original mapped instructions only in the diagnostic executable; no production original code calls',
            'Explicit XML/resources/old CRT adapters, reconstructed timer/cursor/input and actual recovered GameMenu',
            'TutorialTipHost finish notification is an observed diagnostic boundary; actual TutorialManager/profile/world integration remains pending'],
        pending=['TutorialManager, installed gameplay predicates, profile progression and actual world integration',
            'Disabled anchor already disabled/parent-disabled cases, default multiplier/interlinear override and XML reload matrix',
            'NaN/overflow/zero vector coordinates, null-predicate exception side effects and actual repeated completion profile effects',
            'Full game EXE, bidirectional saves, complete Squall and isolated uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Tutorial tips: 3474 states, 500 starts, 2600 steps, 64 GPU frames/30720000 pixels; 17 originals unchanged')
if __name__=='__main__':main()
