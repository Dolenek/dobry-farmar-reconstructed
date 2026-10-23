#!/usr/bin/env python3
"""Compare Person appearance/UI, selectors, reloads and a connected Field scene."""
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
    parser.add_argument('--output', type=Path, default=ROOT/'.local/person-visuals-validation.json')
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
        result = subprocess.run([str(oracle), str(original/'Dobrý farmář.exe'), str(original/'Data'), 'person-visuals'+suffix],
                                cwd=ROOT, capture_output=True, text=True, timeout=120)
        if result.returncode:
            raise RuntimeError(f'Person visuals {precision}: {result.returncode} {result.stderr}')
        state = json.loads(result.stdout)
        for key, value in dict(states=4507, actions=4464, frames=20, pixels=9600000).items():
            assert state['person_visual_'+key] == value, (key, state['person_visual_'+key])
        assert state['person_task_states'] == 23 and state['person_task_actions'] == 6
        assert state['device_flags'] == (0x22 if precision == 'PC53' else 0x20)
        contexts.append(dict(precision=precision, result=state))
        unchanged()
    extra = ['CMakeLists.txt', 'tools/text_buttons_reference_test.cpp', 'tools/text_buttons_oracle_fixups.inc',
             'tools/person_visuals_oracle_cases.inc', 'tools/farm_field_oracle_cases.inc',
             'tools/farm_field_oracle_services.h', 'tools/field_sprinkle_oracle_cases.inc',
             'tools/water_source_oracle_cases.inc', 'tools/game_menu_oracle_cases.inc', 'tools/build_sprite_oracle_fixups.py',
             'tools/monotonic_ui_test_heap.h', 'tools/gameplay_runtime_oracle_services.h', 'tools/loading_oracle_services.h',
             'tools/main_menu_oracle_services.h']
    paths = sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report = dict(status='passed_for_person_visuals_ui_selectors_reloads_and_explicit_actor_boundaries',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'), reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False, original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle), source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts, states=9014, actions=8928, gpu_frames=40, gpu_rgb_pixels=19200000,
        xml_reloads=10, connected_person_task_states=46, connected_person_task_actions=12,
        original_functions=['0044b600','00450a30','00449ea0','0044f1a0','0044f250','0044f2e0',
            '0044ee70','0044ed10','0044fe20','0044a330','004497e0','0058106d','00588535'],
        coverage=[
            'Actual Person XML, movement/job/idle/hose appearances, sound names, action times, carry/job height tables and emitter/tip/hotspot offsets',
            'Twelve carry values, four tool levels, five directions, both mirror states and eleven job types with native texture-change-only copy semantics',
            'World/mirrored/raw hotspot getters, exact selected appearance, activity/hose/direction and FieldObject depth',
            'ShowDiscrete byte predicate, zero/current overflow/count/centering/length, mower/product/seed icon ordering and wrapping PCC sum',
            'Walking appearance selection at angular edges, zero/tiny/large/random vectors and every original direction; hose frame selection across a target grid',
            'Original x87 normalization spill points and actual normal CRT acos/fpatan path in both precision contexts',
            'Actual Field and PersonTasks markers use this Person XML appearance and size; FIFO/clear/discard and resource cleanup execute',
            'Five partial/missing/actual XML reloads per context, retained Lid children, sound request names/reference counts and destructor reference deltas',
            'Exact D3D8 calls and RGB pixels in the connected scene plus post-reload UI frames'],
        test_adapters=[
            'FieldObject drawing/input base adapter on both sides; full native Person constructor and actor absoluteProcess do not execute',
            'Actual original Person XML and individual UI/appearance/hotspot/move/hose bodies execute on adapted Person data',
            'Dig/Mow levels are explicit mandatory world observations; carried seed kind/count getters use a data adapter, with actual SeedPackage tested separately',
            'Shipped previously compared GameConfiguration is adapted as data; native product getters execute their original bodies',
            'Actual Field/queue controllers are connected; full movement/action consumers, actor time advancement and walking trajectory are not claimed',
            'Native normal CRT math mode and floor SSE2 flags avoid uninitialized error callbacks; exceptional CRT ABI remains unverified',
            'Unused/missing-XML storage unspecified by the native constructor is not claimed for arbitrary recycled allocations',
            'Mapped original code is diagnostic reference only and is never used in production implementation'],
        pending=['Complete Person constructor, idle/walking/working actions, carry state transitions, sound channels and actor lifecycle',
                 'Complete Farm/cGame startup, full gameplay and bidirectional saved-game comparison',
                 'Complete Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print('Person visuals: 9014 states / 8928 actions, 10 reloads, 40 frames / 19200000 identical RGB pixels; connected Field/markers; 17 originals unchanged')

if __name__ == '__main__':
    main()
