#!/usr/bin/env python3
"""Compare actual seed and hose input/action/particle cycles with the original."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATUS = 'passed_for_person_seed_sow_drop_and_hose_particle_input_cycles'

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle', type=Path, default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output', type=Path, default=ROOT/'.local/person-flows-validation.json')
    args = parser.parse_args()
    oracle = args.oracle.resolve(strict=True)
    original = ROOT.parent/'farmář/Dobrý farmář'
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:
            assert digest(original/row['path']) == row['sha256'], row['path']
    unchanged()
    contexts = []
    for flow, counts in [('seed', (98, 24, 72)), ('hose', (115, 48, 64))]:
        for precision, suffix in [('PC53', ''), ('PC24', '-device')]:
            run = subprocess.run([str(oracle), str(original/'Dobrý farmář.exe'), str(original/'Data'),
                                  'person-'+flow+'-flows'+suffix], cwd=ROOT, capture_output=True, text=True, timeout=160)
            if run.returncode:
                raise RuntimeError(f'Person {flow}/{precision}: {run.returncode} {run.stderr}')
            state = json.loads(run.stdout)
            for key, expected in zip(('states', 'actions', 'frames'), counts):
                assert state['person_controller_'+key] == expected, (flow, precision, key, state)
            assert state['person_controller_pixels'] == counts[2]*800*600
            assert state['device_flags'] == (0x22 if precision == 'PC53' else 0x20)
            contexts.append(dict(flow=flow, precision=precision, result=state))
            unchanged()
    extra = ['CMakeLists.txt', 'tools/validate_person_flows.py', 'tools/text_buttons_reference_test.cpp',
             'tools/text_buttons_oracle_fixups.inc', 'tools/build_sprite_oracle_fixups.py',
             'tools/monotonic_ui_test_heap.h', 'tools/person_controller_oracle_cases.inc',
             'tools/person_flows_oracle_cases.inc', 'tools/person_seed_scene_oracle_cases.inc',
             'tools/farm_field_oracle_cases.inc', 'tools/mow_building_oracle_cases.inc',
             'tools/game_menu_oracle_cases.inc', 'tools/warehouse_oracle_cases.inc']
    paths = sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    total = lambda key: sum(c['result']['person_controller_'+key] for c in contexts)
    report = dict(status=STATUS, recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'), reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False, original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle), source_files={p.relative_to(ROOT).as_posix(): digest(p) for p in paths},
        contexts=contexts, states=total('states'), explicit_cpu_actions=total('actions'),
        gpu_frames=total('frames'), gpu_rgb_pixels=total('pixels'),
        original_functions=['0044f340', '004bd820', '0045a730', '00456470', '00456c50',
                            '004590f0', '0045c5a0', '00459590', '00459a20', '0045c9c0', '00459eb0', '0042f5a0'],
        coverage=[
            'Actual Person ctor discovers status3 SeedPackage and restores carry10; four Sow actions consume all four seeds, plant actual Fields and defer exhausted package deletion',
            'Real mouse pickup of visible seed, ordinary walk and arrival, real Field click and automatic Sow, then Dig tool pickup drops seed through actual kind9 ElasticAnimation and reveals it at completion',
            'Actual actor no-op mouse callbacks match original RET004bd820; ButtonPrototype frame-switch callbacks must not run on Person hover',
            'Hose work at Well levels1/2 with empty, almost full and full Field; zero, negative and positive elapsed; cancellation clears and watering continues',
            'Full mouse Field click from hose idle automatically installs hose work, fills actual Field, stops emission and returns to hose idle; actual live particles drain and are deferred-deleted',
            'Hose target uses second point argument as original0042f5a0 instructions, correcting a misleading decompilation alias',
            'All272 whole D3D8 frames compare commands and130560000 RGB pixels; profile maps, task/marker ownership, sound requests, emitter/particle motion, RNG, resources and deferred deletion also match'],
        test_adapters=[
            'Existing actual StateView/FarmAreas/tools/Field/SeedPackage/Person/HoseEmitter/Elastic objects compose the scene; full Farm/cGame still pending',
            'Farm indication and Money/HUD services retain documented diagnostic boundaries from person-controller; actual bodies execute for all exercised actors, actions, tools, fields and effects',
            'Separate seed and hose processes bound the monotonic diagnostic heap while preserving pointer ordering; production has neither the split nor that allocator',
            'Controlled GameTimer and cursor drive actual UI processing; source SoundManager uses existing disabled diagnostic backend, with physical audio covered separately',
            'Original instructions execute only as a diagnostic reference, never in production reconstruction'],
        pending=['Other arrival branches, cancellation with current marker, repeated tutorial predicates and long queued award branches',
                 'Full Farm/cGame EXE and bidirectional complete saves',
                 'Complete97-export Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(f"Person flows: {total('states')} states / {total('actions')} CPU operations, {total('frames')} frames / {total('pixels')} identical RGB pixels; 17 originals unchanged")

if __name__ == '__main__':
    main()
