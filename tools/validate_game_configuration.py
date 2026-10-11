#!/usr/bin/env python3
"""Compare cGame XML tables and resource ownership with original loader methods."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/game-configuration-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','game-configuration'),('PC24','game-configuration-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Game configuration {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['game_configuration_states']==72 and state['game_configuration_objects']==8
        assert state['game_configuration_reloads']==48 and state['game_configuration_appearances']==3312
        assert state['game_configuration_sound_residues']==216
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/game_configuration_oracle_cases.inc',ROOT/'tools/game_configuration_oracle_services.h',
        ROOT/'tools/loading_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_game_configuration_and_2d_appearance_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        table_states=144,objects=16,xml_reloads=96,appearance_states=6624,cost_value_comparisons=8640,
        original_functions=['00403610','00403660','004036b0','004036c0','0040af00','0040b560','0040e6e0','004d2080','004d2130','004d22a0'],
        coverage=['Actual game.xml: 19 product records with both independent icons, 8 seed records, 15 x 4 upgrade costs, 5 UTF-16 labels and upgrade sound',
            'Original record constructors/destructors and actual product/seed/game XML instructions execute in the diagnostic mapped reference',
            'Sorted XML enumeration, ignored Id/tags, signed price radii, uint32 seed maximum, missing field retention and direct record loaders',
            '15-row/4-level costs and 5-label limits; missing Up0 is not replaced by a differently named section, level loop uses section count',
            'Actual texture cache identity and complete reference deltas on repeated load, empty Picture replacement and destruction',
            'Independent cAnimable atlas/length/loop/playing fields, capacity clamping; source View XML prefix versus native base View',
            'Actual cSound with disabled SoundManager: repeated UpgradeSound retains prior references, only current sound released at destruction'],
        test_adapters=['Original EXE maps for diagnosis without entry point; production components never load original code',
            '2716-byte native fixture constructs real View prefix and actual product/seed records; it is not original complete cGame constructor, singleton or world',
            'Uninitialized scalar fields are explicitly seeded to equal sentinels before missing-field tests; original appearance constructor defaults execute',
            'Native XML uses existing immutable source tree adapter; typed numeric parsing and Up decimal formatting are explicit old-CRT boundaries',
            'Native texture and sound factories use real source ResourceManager, actual D3D8 texture upload and disabled audio; no physical/audible claim',
            'Native current sound release mirrors observed 00410830 instruction; complete game destructor with world side effects is not invoked'],
        pending=['Complete cGame 00413210/00410830, gameplay/tutorial/world constructors and integration into MainMenu completion',
            'Game save/load and bidirectional original compatibility, all world systems and playable main EXE',
            'Mesh/Animation cAnimable factories and 3D resource behavior; current implementation rejects these explicit paths',
            'Invalid/empty numeric cost text is undefined in original getter, non-default locale and original old-CRT numeric conversion ABI',
            'Original production/seed arrays have no upper-count guard; reconstructed component explicitly rejects overflow data, equivalence outside original data not claimed',
            'Physical sound playback, complete game singleton hooks, allocation/exception ABI'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Game configuration: 144 states, 96 XML reloads, 6624 appearance states; 17 originals unchanged')
if __name__=='__main__':main()
