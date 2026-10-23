#!/usr/bin/env python3
"""Compare actual Person queue/marks connected to Field and SeedPackage input."""
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
    parser.add_argument('--output', type=Path, default=ROOT/'.local/person-tasks-validation.json')
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
        result = subprocess.run([str(oracle), str(original/'Dobrý farmář.exe'), str(original/'Data'), 'person-tasks'+suffix],
                                cwd=ROOT, capture_output=True, text=True, timeout=120)
        if result.returncode:
            raise RuntimeError(f'Person tasks {precision}: {result.returncode} {result.stderr}')
        state = json.loads(result.stdout)
        for key, value in dict(states=51, actions=42, frames=8, pixels=3840000).items():
            assert state['person_task_'+key] == value, (key, state['person_task_'+key])
        assert state['seed_package_states'] == 280
        assert state['device_flags'] == (0x22 if precision == 'PC53' else 0x20)
        contexts.append(dict(precision=precision, result=state))
        unchanged()
    extra = ['CMakeLists.txt', 'tools/text_buttons_reference_test.cpp', 'tools/text_buttons_oracle_fixups.inc',
             'tools/person_tasks_oracle_cases.inc', 'tools/seed_package_oracle_cases.inc', 'tools/seed_package_controller_oracle_cases.inc',
             'tools/farm_field_oracle_cases.inc', 'tools/farm_field_oracle_services.h', 'tools/field_sprinkle_oracle_cases.inc',
             'tools/water_source_oracle_cases.inc', 'tools/game_menu_oracle_cases.inc', 'tools/build_sprite_oracle_fixups.py',
             'tools/monotonic_ui_test_heap.h', 'tools/gameplay_runtime_oracle_services.h', 'tools/loading_oracle_services.h',
             'tools/main_menu_oracle_services.h']
    paths = sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report = dict(status='passed_for_person_queue_markers_field_seed_input_and_explicit_actor_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'), reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False, original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle), source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts, states=102, actions=84, gpu_frames=16, gpu_rgb_pixels=7680000,
        original_functions=['0044d2f0','0044d790','0044dd80','0044e300','0044f870','00449930','00449a30'],
        coverage=[
            'Actual Field enqueue and SeedPackage click/release create actual Picture markers using shipped Person XML appearance/size',
            'FIFO and completion, empty excludes-current semantics, exact record pointers/targets and marker transform/appearance/state',
            'Completion repositions remaining same-area markers after moving Field, preserving stored record targets',
            'Removing queued records preserves current task and does not decrement clickIndex',
            'Explicit cancellation/clear, exact Person.cpp506/536/551 deferred discard origins and resource reference cleanup',
            'Unsigned clickIndex extremes/wrap and unspilled x87 depth arithmetic',
            'Actual mouse Field and SeedPackage input appends tasks; exact D3D8 calls and all RGB pixels in both x87 contexts'],
        test_adapters=[
            'Native Person is a data adapter for marker appearance/size, queue and current record; full native constructor/actor/animation does not execute',
            'Native original queue bodies execute with actual Field, SeedPackage, Picture and View ownership',
            'Farm retains the verified StateView composition; remaining indication/gate and cGame effects retain explicit observations',
            'This mode includes earlier SeedPackage/Field CPU and GPU prerequisites; the 16 reported frames are additional queue/input frames only',
            'Mapped original code is diagnostic reference only and is never used in production implementation'],
        pending=['Full Person constructor/XML, walking, carrying, working, animation/sound and actual action consumers',
                 'Full Farm/cGame startup, gameplay and bidirectional saved-game comparison',
                 'Complete Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print('Person tasks: 102 states / 84 actions, 16 frames / 7680000 identical RGB pixels; actual Field/Seed mouse input and markers; 17 originals unchanged')

if __name__ == '__main__':
    main()
