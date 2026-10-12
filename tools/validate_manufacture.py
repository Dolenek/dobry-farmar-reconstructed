#!/usr/bin/env python3
"""Compare the real Manufacture scene, purchases, placement and demolition."""
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
    parser.add_argument('--output', type=Path, default=ROOT/'.local/manufacture-validation.json')
    args = parser.parse_args()
    oracle = args.oracle.resolve(strict=True)
    reference = ROOT.parent/'farmář/Dobrý farmář'
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))

    def unchanged():
        for row in inventory['files']:
            assert digest(reference/row['path']) == row['sha256'], row['path']

    unchanged()
    contexts = []
    counts = dict(states=1605, objects=1, factories=11, placements=4096, removals=10,
                  purchases=2, builds=2, frames=85, pixels=40800000, effects=1, invalid_slots=3, reloads=5)
    for precision, suffix in [('PC53', ''), ('PC24', '-device')]:
        mode = 'manufacture'+suffix
        result = subprocess.run([str(oracle), str(reference/'Dobrý farmář.exe'), str(reference/'Data'), mode],
                                cwd=ROOT, capture_output=True, text=True, timeout=60)
        if result.returncode:
            raise RuntimeError(f'{mode}: {result.returncode} {result.stderr}')
        state = json.loads(result.stdout)
        for key, value in counts.items():
            assert state['manufacture_'+key] == value, (mode, key, state['manufacture_'+key], value)
        assert state['production_placement_states'] == 24576
        assert state['device_flags'] == (0x22 if precision == 'PC53' else 0x20)
        assert state['production_house_objects'] == 0  # independently required house oracle
        contexts.append(dict(precision=precision, **state))
        unchanged()

    paths = sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()] + [ROOT/name for name in [
        'CMakeLists.txt', 'tools/text_buttons_reference_test.cpp', 'tools/text_buttons_oracle_fixups.inc',
        'tools/build_sprite_oracle_fixups.py', 'tools/manufacture_oracle_cases.inc',
        'tools/production_placement_oracle_cases.inc', 'tools/production_house_oracle_cases.inc',
        'tools/production_house_oracle_services.h', 'tools/game_menu_oracle_cases.inc',
        'tools/game_menu_oracle_services.h', 'tools/contract_status_oracle_services.h',
        'tools/tutorial_manager_oracle_services.h',
    ]] + [Path(__file__).resolve()])
    report = dict(
        status='passed_for_manufacture_scene_and_explicit_remaining_world_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(reference/'Dobrý farmář.exe'), reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False, original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle), source_files={p.relative_to(ROOT).as_posix(): digest(p) for p in paths},
        precision_contexts=contexts, manufacture_states=counts['states']*2,
        manufacture_placements=8192, geometry_comparisons=49152,
        frames=170, pixels=81600000, stencil_format='D24S8',
        original_functions=['00499730', '00496fa0', '004979a0', '00494390', '00498400', '00494a20',
            '00493b70', '004957c0', '00495bd0', '00494e70', '00495fd0', '00497070', '00497480',
            '00497870', '00493c60', '00496440', '00493bb0', '0049f7d0', '00499630', '00498c60',
            '00498840', '00493310', '00493400', '00493820', '004178c0', '00417d10', '00417b90', '004f0510'],
        coverage=[
            'Real Manufacture constructor and eleven saved house types, all twenty owned slots, eleven factory calls and exact full-count gate',
            'All eleven focused house kinds, four levels, three availability values and three signed Money values; repeated selection and null selection',
            'Actual upgrade callbacks, three levels per type and inactive/blocked/guide-34 price branches; profile values and ordered warehouse/SetMoney effects',
            '4096 actual house mask placement/control guard comparisons per precision, plus 24576 standalone rectangle/mask comparisons per precision',
            'Thirty-two local control-enable combinations, HUD suspension/resumption, real warehouse labels and enable/show/hide propagation',
            'Actual BuyBuilding open/cancel/choose creates Manufacture-owned houses; two real valid constructions persist level, position, money and award values',
            'Whole scene movement, mouse/frame/render dispatch, actual selection and protected outside clicks; exact sprites, D3D calls and pixels',
            'Actual built-house drag and right-press cancellation restore saved position; nine unfinished and one completed deferred removals',
            'Completed destruction creates a real demolition Picture, executes its animation-ended callback, preserves both origins and verifies texture ownership teardown',
            'Three invalid requested slots retain the native count increment without allocation; five XML reloads preserve resize/append/missing-section semantics',
        ],
        corrections=[
            'Constructor final show/hide targets Build and price, not the scene itself',
            'Selection copies the image before showing, sets the title before showing, and preserves the original control ordering',
            'Russian localization keys use Unicode escapes independent of the build machine code page',
            'Rectangle parsing consumes one separator between components, matching original vector extraction',
            'Owned house destruction retains the stable moving-pointer address instead of virtual dispatch through a destroyed service base',
        ],
        test_adapters=[
            'Only the diagnostic maps reference methods; production classes never load or call original code',
            'Warehouse product/free/transfers and SetMoney are explicit controlled observations; real warehouse and cGame still required',
            'Actual source/native GameMenu, TutorialManager, BuyBuilding, Manufacture and ProductionBuilding controllers execute; Farm/WareDetail remain diagnostic StateView bases',
            'Profiles are real source maps and native ABI map adapters; complete whole-game save compatibility still pending',
            'Native CRT strings, XML/resource interfaces and deferred-map ABI are explicit adapters; atexit registrations are recorded because the mapped reference has no CRT startup',
            'The fixed 512 MiB monotonic diagnostic heap preserves pointer order; Manufacture uses an independent process from the separately required complete house oracle',
            'Sound request names and arguments are compared with a disabled backend; audible complete-world playback remains pending',
        ],
        pending=[
            'Complete Manufacture enabled-step tutorial playthrough and actual warehouse notifications, Farm/WareDetail and cGame world providers',
            'Malformed XML, factory exceptions, nonstandard masks and reused-process padding behavior',
            'Main source-built game EXE, all world systems, bidirectional whole-world saves, complete Squall behavior and isolated source uninstaller',
        ],
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(f"Manufacture: {counts['states']*2} states, 170 frames/81,600,000 pixels; 17 originals unchanged")


if __name__ == '__main__':
    main()
