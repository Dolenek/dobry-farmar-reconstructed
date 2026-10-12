#!/usr/bin/env python3
"""Compare all production-house controllers with explicit world boundaries."""
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
    parser.add_argument('--output', type=Path, default=ROOT/'.local/production-houses-validation.json')
    args = parser.parse_args()
    oracle = args.oracle.resolve(strict=True)
    reference = ROOT.parent/'farmář/Dobrý farmář'
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))

    def unchanged():
        for row in inventory['files']:
            assert digest(reference/row['path']) == row['sha256'], row['path']

    unchanged()
    contexts = []
    for component, prefix, counts in [
        ('extended-picture-buttons', 'extended_', dict(states=23749, objects=11, reloads=33, letters=819850, frames=44, pixels=21120000, steps=23628, sound_residues=0)),
        ('production-houses', 'production_house_', dict(states=8094, objects=55, actions=7829, frames=44, pixels=21120000)),
    ]:
        for precision, suffix in [('PC53', ''), ('PC24', '-device')]:
            mode = component+suffix
            result = subprocess.run([str(oracle), str(reference/'Dobrý farmář.exe'), str(reference/'Data'), mode], cwd=ROOT, capture_output=True, text=True, timeout=60)
            if result.returncode:
                raise RuntimeError(f'{mode}: {result.returncode} {result.stderr}')
            state = json.loads(result.stdout)
            for key, value in counts.items():
                assert state[prefix+key] == value, (mode, key, state[prefix+key], value)
            assert state['device_flags'] == (0x22 if precision == 'PC53' else 0x20)
            if component == 'production-houses':
                assert state['production_placement_states'] == 24576
            contexts.append(dict(component=component, precision=precision, **state))
            unchanged()

    paths = sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()] + [ROOT/name for name in [
        'CMakeLists.txt', 'tools/text_buttons_reference_test.cpp', 'tools/text_buttons_oracle_fixups.inc',
        'tools/production_house_oracle_cases.inc', 'tools/production_house_oracle_services.h',
        'tools/production_placement_oracle_cases.inc',
        'tools/game_menu_oracle_cases.inc', 'tools/game_menu_oracle_services.h',
        'tools/contract_status_oracle_services.h', 'tools/tutorial_manager_oracle_services.h',
        'tools/extended_button_oracle_cases.inc', 'tools/extended_button_oracle_run.inc',
    ]] + [Path(__file__).resolve()])
    report = dict(
        status='passed_for_production_house_controller_and_observed_world_boundary_scope',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(reference/'Dobrý farmář.exe'), reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False, original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle), source_files={p.relative_to(ROOT).as_posix(): digest(p) for p in paths},
        precision_contexts=contexts, production_house_states=16188, production_house_objects=110,
        production_house_actions=15658, extended_picture_states=47498, frames=176, pixels=84480000,
        stencil_format='D24S8',
        original_functions=['0049e9f0', '0049b080', '004a0330', '0049cd60', '004a2490', '004a1c90',
            '0049f010', '0049f0c0', '0049f170', '0049f210', '0049f300', '0049f3c0', '0049fdf0',
            '0049f8c0', '0049fc30', '0049d340', '0049d7b0', '00499e30', '00499fe0', '00499f00',
            '00499c00', '00499ad0', '0049ab60', '00499b90', '0049fdb0', '004a0000', '004a0230',
            '004a02c0', '0049a080', '0049a050', '00499ea0'],
        coverage=[
            'All eleven real derived constructors and shipped XML; four saved levels and new level -1 per type, four full level records, exact input/output icon recipes and UI child graph',
            'ProdCount/ProdPct/ProdStop and position properties; unsigned capacity/clamp, nonzero stop values, x quantity text, slider/progress/hover and exact ordered warehouse calls',
            'Real Upgrade::setLevel refresh across levels 0/1/2/upper-clamped 3, appearance/mask/speed/capacity and awDepotUpg changes',
            'New placement click with blocked invalid guide 37, paid/free first construction, build flag, level zero, money argument, award key, focus and placement release',
            'Original press/distance-squared >32 manipulation, next cursor/colour tick, valid release and focus click; observed Manufacture boundaries',
            'Production tick, all three scene movement guards, tutorial pause, full warehouse, near/exact/negative/NaN progress and uint awQProcess; sound branch gates',
            'Native level -1 capacity overlap at +324: loop and three raw padding bytes retained; fresh zero diagnostic heap and four separately seeded overlapping words',
            'Dormant StartStopButton signs distinguished with explicit XML fixture, eight byte selector values and unchanged dirty flag',
            'All eleven controllers execute real mouse/frame/render traversal: 88 GPU frames and matching ordered sprites/draw calls/42,240,000 RGB pixels; base ExtendedPictureButton adds 88 frames',
            'Owned texture reference deltas, sounds and teardown; exact float bits in both default and real-device precision',
        ],
        corrections=[
            '0049cd60 copies primary appearance into its hovered record, not into the building; missing hovered attributes inherit primary values',
            'Native +5a4 is the SensorPanel Button, +5ac is the LoadedSubst Picture; sections and UI offset loop use these actual addresses',
            'lighter_mask is assigned to PictureButton overlay +27c, not Podlogka +620',
            'AddDepotButton dispatches taking from warehouse; AddStockButton dispatches returning to warehouse',
            '0049fdf0 unsigned argument and 004a1c90 float animation-time addition follow instructions instead of incorrect pseudocode casts',
        ],
        test_adapters=[
            'Production libraries do not load original code. Only the diagnostic oracle loads reference instructions; original entry point never executes',
            'Manufacture placement/list/begin/end/focus/remove and warehouse goods/free/transfers, Money and SetMoney are explicitly controlled observations, not a production game world',
            'House uses real source TutorialManager and source/native profile maps; guide-37 field cases test the decision branch without playing the complete guide',
            'HUD/other scene controller frame execution is suspended for house GPU/input scenarios; whole-world frame integration is pending',
            'The monotonic diagnostic heap is fresh zero storage; padding-dependent original behavior across different/reused process heaps remains unproved',
            'Native old CRT allocation/strings/streams/vector cleanup and XML/resource interfaces use explicit existing diagnostic adapters',
            'Sound requests/resources are compared; audible full-game playback is pending',
        ],
        pending=[
            'Complete real Manufacture world behavior is tracked separately in manufacture-validation.json; the controlled house boundaries here remain explicit',
            'Real warehouse and cGame binding; complete tutorial play sequence and scene input/audio integration',
            'ProductionPlacement now has 24,576 direct rectangle/mask comparisons per precision; invalid/nonstandard mask inputs remain unproved',
            'Repeated/malformed XML and invalid upgrades, allocation exceptions, reused-heap raw padding, invalid masks and nonstandard hardware',
            'Main reconstructed game EXE, bidirectional whole-world saves, complete Squall behavior and isolated source uninstaller',
        ],
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print('Production houses: 16,188 controller states, 110 objects, 176 combined frames/84,480,000 pixels; 17 originals unchanged')


if __name__ == '__main__':
    main()
