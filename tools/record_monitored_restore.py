#!/usr/bin/env python3
"""Audit the bounded restored-farm scene and the preceding monitored trials."""
import argparse
import datetime
import json
from pathlib import Path

from current_production_regression import current_regression
from record_live_production_verification import ROOT, digest, read, write


def json_lines(path):
    return [json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]


def reference(path):
    return dict(path=path.relative_to(ROOT).as_posix(), sha256=digest(path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('run_directories', nargs='+', type=Path)
    args = parser.parse_args()
    trials = []
    for directory in args.run_directories:
        work = directory.resolve(strict=True)
        raw_path = work / 'source-restore/report.json'
        raw = read(raw_path)
        session, = raw['sessions']
        assert session['exit_code'] == 0 and raw['reference_files_unchanged'] == 17
        assert not raw['original_profile_fields_or_memory_written']
        for name, sha in raw['source_files'].items():
            assert digest(ROOT / raw['source_file_snapshots'].get(name, name)) == sha, name
        for name, sha in raw['implementation_files'].items():
            actual = 'Dobr\u00fd farm\u00e1\u0159.exe' if name == 'FarmarTutorialWarnings.exe' else name
            assert digest(ROOT / 'build/windows/Release' / actual) == sha, name
        for name, sha in raw['screenshots'].items():
            assert digest(raw_path.parent / name) == sha, name

        watchdog_path = work / 'watchdog.jsonl'
        watchdog = json_lines(watchdog_path)
        started, finished = watchdog[0], watchdog[-1]
        assert started['maximum_seconds'] == 45 and started['stale_progress_ms'] == 4000
        assert finished['event'] == 'finished' and finished['watchdog_exit_code'] == 0
        assert finished['live_owned_processes_before_final_close'] == 0
        assert started['driver_pid'] == raw['watchdog_lease']['driver_pid']
        samples = [row for row in watchdog if row['event'] == 'health']
        window_path = work / 'owned-window-health.jsonl'
        window = json_lines(window_path)
        assert window and all(row['pid'] == session['pid'] and row['consecutive_misses'] < 3 for row in window)
        input_path = work / 'input-api-timings.jsonl'
        inputs = json_lines(input_path)
        assert inputs and max(row['seconds'] for row in inputs) <= 1
        presses = [row for row in inputs if row.get('input_flags') == [2]]
        releases = [row for row in inputs if row.get('input_flags') == [4]]
        assert presses and len(presses) == len(releases)
        passed = session.get('status') == 'completed'
        assert finished['driver_exit_code'] == (0 if passed else 1)
        scene = session.get('scene_observation', {})
        if passed:
            assert session['actions'] == ['Actual Start restored the unmodified checkpoint and matched Menu HUD']
            for key in ['stock', 'market', 'carried', 'collected', 'held', 'money']:
                assert session['initial_state'][key] == session['final_state'][key], key
            times = [row['qpc_vblank'] for row in session['bounded_window_and_DWM_health']]
            assert len(times) >= 2 and all(a < b for a, b in zip(times, times[1:]))
        else:
            assert 'Injected pointer missed owned window target' in raw['error']
        if scene.get('stable_topology_observed'):
            assert scene['ordinary_game_pause_used'] and not scene['render_or_UI_thread_suspended']
            assert not scene['atomic_frame_claimed'] and scene['controls'] > 100
            assert sorted(field['id'] for field in scene['fields']) == list(range(60))
            assert all(field['active'] for field in scene['fields'])
            assert [row['paused'] for row in session['actual_pause_changes']] == [True, False]
        trials.append(dict(raw_report=reference(raw_path), watchdog_log=reference(watchdog_path),
            window_log=reference(window_path), input_log=reference(input_path),
            driver_exit=finished['driver_exit_code'], game_exit=session['exit_code'],
            watchdog_exit=finished['watchdog_exit_code'], live_owned_processes_at_end=0,
            window_responses=sum(row['responsive'] for row in window), window_observations=len(window),
            host_cpu_percent_range=[min(row['host_cpu_percent'] for row in samples),
                                    max(row['host_cpu_percent'] for row in samples)],
            minimum_host_available_physical_bytes=min(row['host_available_physical_bytes'] for row in samples),
            mouse_press_release_pairs=len(presses), maximum_input_api_seconds=max(row['seconds'] for row in inputs),
            restored_farm=passed, scene={key: val for key, val in scene.items() if key != 'fields'},
            observed_fields=len(scene.get('fields', [])), error=raw.get('error')))

    assert [row['restored_farm'] for row in trials] == [False, True, True]
    assert [row['observed_fields'] for row in trials] == [0, 0, 60]
    full, _ = current_regression()
    inventory = read(ROOT / 'analysis/inventory.json')['files']
    originals = ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    assert len(inventory) == 17 and all(digest(originals / row['path']) == row['sha256'] for row in inventory)
    name = 'analysis/monitored-source-restore-validation.json'
    write(ROOT / name, dict(status='passed_monitored_actual_restore_and_paused_read_only_scene',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        trials=trials, revised_scene_tree_runtime_validated=True, physical_display_proven_by_counter_alone=False,
        host_freeze_root_cause_proven=False, incident_resolved=False, original_files_unchanged=17,
        production_changed=False, current_full_ctest_report=full, whole_game_complete=False))
    verification_path = ROOT / 'analysis/verification.json'
    verification = read(verification_path)
    if name not in verification['reports']:
        verification['reports'].append(name)
    write(verification_path, verification)
    print('Audited two genuine restores, 60 observed active fields, and the retained pointer failure.')


if __name__ == '__main__':
    main()
