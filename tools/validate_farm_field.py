#!/usr/bin/env python3
"""Compare actual Field/actions and Farm geometry with connected irrigation."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle', type=Path, default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output', type=Path, default=ROOT/'.local/farm-field-validation.json')
    args = parser.parse_args()
    oracle = args.oracle.resolve(strict=True)
    original = ROOT.parent/'farmář/Dobrý farmář'
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:
            assert digest(original/row['path']) == row['sha256'], row['path']
    unchanged()
    contexts = []
    for precision, suffix in [('PC53', ''), ('PC24', '-device')]:
        result = subprocess.run([str(oracle), str(original/'Dobrý farmář.exe'), str(original/'Data'), 'farm-field'+suffix],
                                cwd=ROOT, capture_output=True, text=True, timeout=120)
        if result.returncode:
            raise RuntimeError(f'Field {precision}: {result.returncode} {result.stderr}')
        state = json.loads(result.stdout)
        for key, value in dict(states=2236, objects=40, actions=2112, frames=70, pixels=33600000).items():
            assert state['farm_field_'+key] == value, (key, state['farm_field_'+key])
        assert state['field_tip_states'] == 2499 and state['field_tip_frames'] == 70 and state['field_tip_pixels'] == 33600000
        assert state['device_flags'] == (0x22 if precision == 'PC53' else 0x20)
        contexts.append(dict(precision=precision, result=state))
        unchanged()
    extra = ['CMakeLists.txt', 'tools/text_buttons_reference_test.cpp', 'tools/text_buttons_oracle_fixups.inc',
             'tools/farm_field_oracle_cases.inc', 'tools/farm_field_oracle_services.h', 'tools/field_sprinkle_oracle_cases.inc',
             'tools/water_source_oracle_cases.inc', 'tools/game_menu_oracle_cases.inc', 'tools/build_sprite_oracle_fixups.py',
             'tools/monotonic_ui_test_heap.h', 'tools/gameplay_runtime_oracle_services.h', 'tools/loading_oracle_services.h',
             'tools/main_menu_oracle_services.h']
    paths = sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report = dict(status='passed_for_field_actions_farm_geometry_and_connected_irrigation',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'), reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False, original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle), source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts, states=4472, objects=80, actions=4224, gpu_frames=140, gpu_rgb_pixels=67200000, field_tip_states=4998, field_tip_xml_reloads=4,
        original_functions=['00441360','004415e0','004408d0','004405e0','00440380','004402a0','004402d0','0040c750','00415320','00415340','00415350','00415360','00415370','00415790','004153b0',
            '0043c120','00436340','004388e0','0043ac90','0043b0d0','0043ba70','00438200','00438740','004387f0',
            '00438620','004386d0','00436880','00451680','00437dc0','004380c0','0043b5f0','0043c3e0',
            '0043cc80','0043f6a0','0043d0b0','0043fd80','0043da40','0043dee0','0043c440','0043f180',
            '0043c830','0043e550','0043e960','0043ed60','00431530','0042df60','0042e3a0','004326c0','0042d290'],
        coverage=[
            'Actual FarmField XML, all eight cultures and all four saved states; 32 short/long absolute steps per combination',
            'Locked, purchasable and active constructors at eight current/available upgrade combinations',
            'All eight processed-flag combinations for tutorial21/22/23; actual empty seed-list destruction and visible tutorial23',
            'Growing/Ready/Dead/Ploughed action begin/step/finish, processor replacement, counts and ownership; exact typed profiles and shared RNG',
            'Water/fertil exhaustion, progress, sign/appearance/mask updates, resource reference deltas and cleanup',
            'Actual Farm top/first queries and radius selection; real Warehouse/Sprinkle masks reject overlap',
            'Actual mouse hover/down/release/leave and native Farm held flag accessors',
            'Actual Sprinkle, WaterSource capacity, Field water/preview/growth, emitter and particles in one scene',
            'Actual FieldTip selection/hide from Field mouse input, timers and all eight culture names/icons/three bars; partial and actual XML reload retain original owned labels and backgrounds; full cleanup/reference deltas',
            'Exact D3D8 calls and all RGB pixels across 140 GPU frames in both original x87 contexts'],
        test_adapters=[
            'Farm scene retains the verified StateView composition; full native Farm constructor/UI is not executed',
            'Remaining Farm field indication is an ordered observation; actual FieldTip controller and UI execute',
            'Person action queue and task primitives are explicit observations; complete Person actions/animation remain pending',
            'Product configuration uses previously verified numeric configuration; original product getters execute their real bodies',
            'Seed-package query in this scene is empty; seed-package interaction and related tutorial branches remain pending',
            'Native cGame particle discard is observed; actual source ownership and cleanup execute',
            'Mapped original code is diagnostic reference only and is never used in production implementation'],
        pending=['Complete Farm XML composition, SeedPackage controller, Person actions and whole scene startup',
                 'Additional tutorial/upgrade callbacks and save reload sequences in the complete game',
                 'Whole-game playthrough, bidirectional saves, complete Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print('Field: 4472 Field + 4998 FieldTip states, 4224 actions, 140 frames / 67200000 identical RGB pixels; connected irrigation; 17 originals unchanged')

if __name__ == '__main__':
    main()
