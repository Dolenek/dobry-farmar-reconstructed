#!/usr/bin/env python3
"""Compare real incoming contract letters, deadlines, tutorial and mouse input."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_contract_letter_lifetime_deadlines_reload_tutorial_and_input'

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/contract-letter-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'contract-letter'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=120)
        if run.returncode:raise RuntimeError(f'Contract letter {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        for key,value in dict(states=1702,assignments=274,actions=1330,frames=80,pixels=38400000).items():
            assert state['contract_letter_'+key]==value,(precision,key,state['contract_letter_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_contract_letter.py','tools/text_buttons_reference_test.cpp',
           'tools/build_sprite_oracle_fixups.py','tools/text_buttons_oracle_fixups.inc','tools/monotonic_ui_test_heap.h',
           'tools/contract_letter_oracle_cases.inc','tools/production_house_oracle_cases.inc',
           'tools/production_house_oracle_services.h','tools/tutorial_manager_oracle_services.h',
           'tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h',
           'tools/contract_status_oracle_services.h','tools/loading_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/p for p in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        contexts=contexts,states=3404,assignments=548,explicit_cpu_actions=2660,
        gpu_frames=160,gpu_rgb_pixels=76800000,owned_letter_lifetimes=6,xml_reloads=24,
        original_functions=['00424ba0','00421760','00424320','004250d0','00425770','00425d90','00421520'],
        coverage=[
            'Three actual owned Farm letter slots with original XML, independent read/unread appearances, child AnimationButton/ProgressBar, two sound resources and matching teardown deltas',
            'All131 shipped contract definitions: incoming animation, unsigned letter duration, profile letter/letState/letTime, unread/read assignment, actual arrival and countdown',
            'Restored deadline/state, direct restored position without farm offset, warning flag, blink clock, near/expiry sound calls and observed Farm removal request',
            'Zero/negative elapsed, zero-distance and exact arrival (remaining0 stays disabled), expiry at exactly0, and tutorial-active elapsed freeze',
            'Partial, empty and repeated XML loads retain omitted properties and sound pointers; original reload appends its tooltip/lid children and retains overwritten resource references',
            'Real mouse hover/press/release opens actual ContractProvide from an incoming letter, records read state and suspends/resumes the actual GameMenu',
            'Actual letter deadline starts TutorialManager action17; its predicate observes this exact actor, and actual mouse input reads it, finishes the tutorial and opens the offer',
            'All160 D3D8 frames compare commands and76800000 RGB pixels, plus controls, deadline/progress/blink scalars, all profile maps, actual tutorial state and ordered HUD/money/sound observations'],
        test_adapters=[
            'Full Farm/cGame remain pending; existing StateView provides real moving Farm hierarchy. Rendered Farm is parented under actual GameMenu as in the original constructor',
            'Farm004341b0 removal is an observed call boundary; complete Farm slot count/clear implementation and offer acceptance through it are not claimed here',
            'GameMenu warehouse/money/gameplay providers retain their recorded diagnostic boundaries; both sides use the same providers on offer suspension/resume',
            'Actual source TutorialManager and ContractProvide execute; tutorial anchorState resolves the actual letter rather than a synthetic fixture state',
            'Native Ghidra ctor body stops at00424c4a; diagnostic fixups explicitly include the actual allocation/XML tail through RET8 at00424d63',
            'Native second vtable contains two pointers followed by null; the null is preserved and not treated as a relocation',
            'Controlled GameTimer/cursor and disabled UI audio backend; physical sound/DLL behavior has separate reports',
            'Mapped original instructions execute only as test reference; production classes contain no calls into original code'],
        pending=['Complete Farm constructor, slot count/clear and offer acceptance through actual world',
                 'Other Person arrival/cancel/tutorial/award queue branches','Full Farm/cGame EXE and bidirectional complete saves',
                 'Complete97-export Squall behavior, exceptional CRT ABI and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Contract letters: 3404 states / 548 assignments / 2660 CPU operations; 160 frames / 76800000 identical RGB pixels; actual tutorial17 and offer input; 17 originals unchanged')
if __name__=='__main__':main()
