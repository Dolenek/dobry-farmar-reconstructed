#!/usr/bin/env python3
"""Compare the original pause menu and actual Options/Awards/YesNo/Loading/MainMenu navigation."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/game-pause-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    counts=dict(states=23,letters=1064,frames=132,pixels=63360000,actions=14,reloads=2,loading_steps=48,sound_residues=20)
    for precision,mode in [('PC53','game-pause'),('PC24','game-pause-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Game pause {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in counts.items():assert state['pause_'+key]==value,(key,state['pause_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/game_pause_oracle_cases.inc',ROOT/'tools/game_pause_oracle_services.h',ROOT/'tools/main_menu_oracle_services.h',ROOT/'tools/loading_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_game_pause_menu_and_connected_navigation_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        **{key:2*value for key,value in counts.items()},
        original_functions=['00487330','00487980','00485680','00485d90','00486170','00485560','00485590','00486550','00485660','00485630','00487210','00486d90','004870a0','00489e80'],
        coverage=['Real pause menu constructor, singleton lifetime, five owned TextAnimationButton controls, actual UI/Menu.xml and owned decorations',
            'Actual Continue mouse release and Escape/Enter input handlers restore cGame enable/absoluteEnable before hiding pause',
            'Actual Options construction and reuse of existing singleton, assigned return view and real Options OK navigation',
            'Actual Awards construction, existing-singleton guard, game/pause hiding and real Awards return callback restoring visibility while remaining paused',
            'Required Help View showTransparently/enable calls; full Help controller remains pending',
            'Real YesNo construction, original raw Russian confirmation text, No discard/restore and Yes discard-game request with exact source-line origins',
            'Original Yes hides whole cGame, not pause; source repaired against native instruction 00486ed1',
            'Original and source LoadingFromLog use menu resource log, batch 3 and 96 actual loading steps; completion constructs real source/native MainMenu and FixedBack',
            '264 complete D3D8 frames, ordered submissions/API and 126720000 exactly matching RGB pixels, 28 actual navigation actions',
            '40 verified source/native retained sound references from full XML reload are recorded then diagnostically released; original overwrites without releasing'],
        test_adapters=['Original mapped only for diagnosis without entry point; implementation never calls original code',
            'Original constructors, callbacks, input handlers, Loading steps/completion and MainMenu construction execute native instructions',
            'XML/old CRT/resource interfaces, source-line strings and native profile services have explicit diagnostic adapters',
            'Profiles use real isolated source ProfileSession; native profile properties use established diagnostic bindings',
            'Game/help are real recovered Views in this component fixture; required save-and-discard 00486970 is a trace boundary, so full cGame save/unload is not validated',
            'Control click/hover sounds use typed diagnostic resources and compare requests/lifetime; real disabled SoundManager/MusicPlayback are used for Loading music; audible output remains pending'],
        pending=['Full GameMenu, Help, Farm, tutorial, actual save-and-discard game operation and complete cGame scene integration',
            'Physical input devices, audible output, main EXE, full Squall behavior and bidirectional saves',
            'Allocation/exception and invalid XML paths, corrupted profile/engine globals, in-flight context destruction'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Game pause: 46 states, 28 actions, 264 GPU frames/126720000 RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
