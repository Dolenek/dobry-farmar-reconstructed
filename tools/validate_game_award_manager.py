#!/usr/bin/env python3
"""Compare eighteen gameplay award predicates and actual rendered award lifecycles."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_eighteen_award_predicates_actual_effect_panel_return_and_source_constructor_cleanup'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/game-award-manager-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=[(176,165),(174,163),(191,180),(172,161),(182,171),(177,166)]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        for group,(states,frames) in enumerate(counts):
            mode=f'award-manager-{group}{suffix}'
            run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=120)
            if run.returncode:raise RuntimeError(f'Award manager {mode}: {run.returncode} {run.stderr}')
            result=json.loads(run.stdout)
            for key,value in dict(states=states,actions=10,frames=frames,pixels=frames*480000).items():assert result['award_manager_'+key]==value,(mode,key,result['award_manager_'+key])
            assert result['ui_constructor_failure_checks']==(4 if group==0 else 0)
            assert result['device_flags']==(0x22 if precision=='PC53' else 0x20)
            contexts.append(dict(precision=precision,group=group,result=result));unchanged()
    extra=['CMakeLists.txt','tools/validate_game_award_manager.py','tools/text_buttons_reference_test.cpp',
           'tools/build_sprite_oracle_fixups.py','tools/text_buttons_oracle_fixups.inc','tools/monotonic_ui_test_heap.h',
           'tools/game_award_manager_oracle_cases.inc','tools/ui_constructor_failure_cases.inc',
           'tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h','tools/main_menu_oracle_services.h',
           'tools/awards_oracle_services.h','tools/contract_status_oracle_services.h','tools/loading_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        contexts=contexts,states=2144,explicit_cpu_actions=120,gpu_frames=2012,gpu_rgb_pixels=965760000,
        gameplay_award_effect_lifetimes=36,actual_award_panels=36,award_screen_lifetimes=12,
        source_constructor_failure_checks=8,native_exception_abi_compared=False,
        predicate_indices=[0,1,2,3,4,5,6,8,10,11,12,13,14,15,16,17,19,20],
        unverified_predicates=[dict(index=7,reason='Actual Warehouse/Water/Dig/Mow level composition'),
            dict(index=9,reason='Exactly seven actual built Manufacture slot flags, not upgrade levels'),
            dict(index=18,reason='Actual Market stock0..18 world composition')],
        original_functions=['004c7d00','004c8630','004c8650','004c6d20','004c70e0',
            '004c74b0','004c7540','004c75d0','004c7660','004c76f0','004c7780','004c7810',
            '004c7890','004c78c0','004c7950','004c79e0','004c7a80','004c7b20','004c7bc0',
            '004c6cd0','004c6ce0','004c7c40','004c7cd0'],
        coverage=[
            'Actual gameplay award manager constructor/destructor/singleton, all21 cached condition snapshots and awardShow profile initialization',
            'Eighteen predicates compare below/exact thresholds against actual original callbacks, with signed/unsigned/float/boolean/profile-key semantics',
            'Actual ViewRegistry stepping automatically creates real independent AwardEffect and actual Awards screen',
            'Actual effect grow/animation, whole game normal/absolute suspension, screen transition and actual AwardPanel construction',
            'Actual mouse OK commits condition flag and phase4, actual moving effect reaches target, manager sets awardShow, marks owned and deletes effect',
            'Actual Awards Return mouse path discards the screen and resumes the whole game',
            '2012 complete D3D8 frames compare draw commands and965760000 exact RGB pixels',
            'Cached conditions, pending effect state/geometry/destination, screen ownership, all profile maps, ordered HUD/audio calls and texture teardown match',
            'Source-only missing-index-entry injection repeats HelpView/Journal failures twice in both precisions and verifies singleton/View/input/interaction/resource cleanup'],
        test_adapters=[
            'Complete cGame remains pending; surrounding game is the actual View and documented HUD/minimal StateView world with remaining services observed',
            'Predicates7/9/18 are explicit pre-owned input conditions and are not evaluated in this report. Their source implementations remain for full world verification',
            'Controlled predicate inputs and temporary manager disable between rewards isolate threshold observations; every award lifecycle itself runs through normal frame dispatch',
            'Six coherent processes per precision preserve the fixed805306368-byte monotonic diagnostic heap. Earlier six-award process exhausted it after four complete awards; the limit was not enlarged',
            'Existing reference CRT atexit, decimal String/concatenation and deferred discard adapters preserve actual original manager/panel callback bodies',
            'Clock/cursor, profile adapters, shared resource cache and disabled UI audio retain prior diagnostic roles; physical audio has separate audits',
            'Constructor failure checks alter only the source in-memory FileIndex table, restored by scope cleanup; no original/archive/common file is changed',
            'Native exceptional CRT/unwind/throw ABI is not claimed. Singleton lifetime order is supported by original ctor/dtor/EH analysis and source failure observations',
            'Production implementation contains reconstructed source only; original mapped code runs solely as diagnostic reference'],
        pending=['Full tool/Manufacture/Market composition for predicates7/9/18','Complete cGame factory/bindings, startup and main game EXE',
            'Full actor/tool cycles on actual Farm and bidirectional whole saves','Complete97-export Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Game awards: 18 predicates, 36 actual award/panel cycles; 2144 states / 120 operations, 2012 frames / 965760000 identical RGB pixels; 8 source constructor failure checks; 3 world predicates pending')
if __name__=='__main__':main()
