#!/usr/bin/env python3
"""Compare source SeedPackage, ProgressDiscrete and connected purchase/Field paths."""
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
    parser.add_argument('--output', type=Path, default=ROOT/'.local/seed-package-validation.json')
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
        result = subprocess.run([str(oracle), str(original/'Dobrý farmář.exe'), str(original/'Data'), 'seed-package'+suffix],
                                cwd=ROOT, capture_output=True, text=True, timeout=120)
        if result.returncode:
            raise RuntimeError(f'SeedPackage {precision}: {result.returncode} {result.stderr}')
        state = json.loads(result.stdout)
        for key, value in dict(states=275, objects=11, actions=252, frames=16, pixels=7680000).items():
            assert state['seed_package_'+key] == value, (key, state['seed_package_'+key])
        assert state['progress_discrete_states'] == 519
        assert state['progress_discrete_frames'] == 16 and state['progress_discrete_pixels'] == 7680000
        assert state['device_flags'] == (0x22 if precision == 'PC53' else 0x20)
        contexts.append(dict(precision=precision, result=state))
        unchanged()
    extra = ['CMakeLists.txt', 'tools/text_buttons_reference_test.cpp', 'tools/text_buttons_oracle_fixups.inc',
             'tools/seed_package_oracle_cases.inc', 'tools/seed_package_controller_oracle_cases.inc',
             'tools/farm_field_oracle_cases.inc', 'tools/farm_field_oracle_services.h', 'tools/field_sprinkle_oracle_cases.inc',
             'tools/water_source_oracle_cases.inc', 'tools/game_menu_oracle_cases.inc', 'tools/build_sprite_oracle_fixups.py',
             'tools/monotonic_ui_test_heap.h', 'tools/gameplay_runtime_oracle_services.h', 'tools/loading_oracle_services.h',
             'tools/main_menu_oracle_services.h']
    paths = sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report = dict(status='passed_for_seed_package_progress_purchase_field_and_explicit_person_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'), reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False, original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle), source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts, states=550, objects=22, actions=504, gpu_frames=32, gpu_rgb_pixels=15360000,
        progress_discrete_states=1038, progress_discrete_gpu_frames=32, progress_discrete_gpu_rgb_pixels=15360000,
        original_functions=['0048d3d0','0048d640','0048c9e0','0048d4e0','0048cbc0','0048dda0','0048c760',
            '0045e130','0045ce00','0045cea0','0045cf50','0045d020','0045e340','0045d660','0045dbe0','0045e8d0','0045e100',
            '0045cec0','0045ced0','0045cee0','0045ceb0','0042c960','0042ce30','0042e7c0','00433420','00403740'],
        coverage=[
            'Actual SeedPackage and Use XML, eight kinds, count/status changes and resource cleanup',
            'ProgressDiscrete resizing/current/spacing, colored XML reload with retained Lid and stale same-count children',
            'Native visual-copy semantics preserve RGB, playing and DoubleAnimRender while copying appearance/material/transform',
            'Accepted placement charges once; exact four-pixel drag threshold; rejected paid placement remains moving',
            'Actual mouse hover/down/drag/release/leave, sounds, tips, shared selection and Farm buy-button state',
            'Actual BuySeeds button callback creates a four-use unpaid SeedPackage on Farm movingPicture; placement charges original price',
            'Rejected unpaid factory package is removed through actual Farm registry and deferred discard without payment',
            'Actual Field collision masks, active/grass/crop tutorial field queries and Field tutorial22 anchored to actual SeedPackage',
            'Exact D3D8 calls and RGB pixels in both original x87 contexts; native boolean calls inspect AL only'],
        test_adapters=[
            'Farm scene retains verified StateView composition; full native Farm constructor/UI is not executed',
            'Person task queue and cGame money effect remain explicit observed boundaries',
            'Tutorial20 DigBuilding anchor is explicitly unsupported in this fixture and disabled',
            'Native constructor leaves status storage unspecified; comparisons use fresh zeroed allocations and purchase mode2',
            'Seed mode retains CPU Field comparisons but omits redundant Field GPU frames to fit the diagnostic monotonic heap; dedicated original-farm-field retains all 140 frames',
            'Mapped original code is diagnostic reference only and is never used in production implementation'],
        pending=['Complete Farm composition, actual Person planting/tasks/animation, tutorial20 and complete scene startup',
                 'Arbitrary recycled-allocation constructor state, malformed XML and full save reload sequences',
                 'Whole-game playthrough, bidirectional saves, complete Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print('SeedPackage: 550 states / 504 actions; ProgressDiscrete: 1038 states; 64 total GPU frames / 30720000 identical RGB pixels; 17 originals unchanged')

if __name__ == '__main__':
    main()
