"""Actual source/reference uninstall dialogs on a private noninteractive desktop."""
from __future__ import annotations
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import subprocess
import uuid
from PIL import Image
from validate_uninstaller_runtime import ROOT, sha, lines, normalize_private_registry, normalized_removal

FLOWS = (('empty-fallback', 'language-cancel'), ('empty-fallback', 'cancel'),
         ('empty-fallback', 'complete'), ('stored-language', 'cancel'),
         ('stored-language', 'complete'), ('invalid-stored-language', 'complete'),
         ('named-nonempty', 'complete'), ('player-nonempty', 'complete'))


def fixture_files(path):
    return sorted(lines(path), key=lambda row: row['path'])


def application_registry(rows):
    roots = (r'Software\Alawar', r'Software\Microsoft\Windows\CurrentVersion\Uninstall',
             r'Software\UnrelatedFixture', r'Software\FarmarContainedChild')
    result = [row for row in rows if row['key'].startswith(roots)]
    for row in result:
        row['values'].sort(key=lambda value: value['name'])
    return sorted(result, key=lambda row: (row['hive'], row['key']))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT/'.local/uninstaller-ui-validation.json')
    args = parser.parse_args()
    build = ROOT/'build/uninstall-sandbox/Release'
    source = ROOT/'build/windows/Release/Uninstall.exe'
    original_root = ROOT.parent/'farmář/Dobrý farmář'
    reference = original_root/'Uninstall.exe'
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    for row in inventory['files']:
        assert sha(original_root/row['path']) == row['sha256'], row['path']
    parent = ROOT/'.local/uninstall-isolation'
    parent.mkdir(parents=True, exist_ok=True)
    results = []
    total_controls = 0
    for case, action in FLOWS:
        pair = {}
        for role, executable in [('source', source), ('reference', reference)]:
            root = parent/f'{role}-ui-{case}-{action}-{uuid.uuid4()}'
            command = [str(build/'containment-probe.exe'), '--uninstall-ui', str(root),
                       str(build), str(executable), case, action]
            proc = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=90)
            (root/'harness-stdout.txt').write_text(proc.stdout, encoding='utf-8')
            (root/'harness-stderr.txt').write_text(proc.stderr, encoding='utf-8')
            assert proc.returncode == 0, (root, proc.returncode, proc.stderr)
            proof = json.loads(proc.stdout)
            assert proof['private_registry_keys_removed'] and proof['station_dacl_byte_exact_after_restore']
            assert proof['outside_markers_unchanged'] and proof['uninstaller_executed']
            boundary = json.loads((root/'entry-boundary.json').read_text())
            assert boundary['entry_held_until_guard_setup']
            driver = json.loads((root/'ui-driver-result.json').read_text())
            assert driver['only_private_owned_hwnds']
            assert driver['language_seen'] == (case not in ('stored-language', 'invalid-stored-language'))
            assert driver['confirm_seen'] == (action != 'language-cancel')
            assert driver['completed_seen'] == (action == 'complete')
            code = json.loads((root/'guarded-exit-code.json').read_text())['exit_code']
            assert code == {'language-cancel': 2, 'cancel': 1, 'complete': 0}[action]
            events = lines(root/'guard-events.jsonl')
            assert not [row for row in events if not row['allowed']], (root, 'denied UI application call')
            assert not any(row['api'].endswith('delayed-intent-only') for row in events)
            lang_calls = [row for row in events if row['api'] == 'GetProcAddress.allowed' and row['path'] == 'LangDialog']
            assert bool(lang_calls) == driver['language_seen']
            before = fixture_files(root/'fixture-files-before.jsonl')
            after = fixture_files(root/'fixture-files-after.jsonl')
            before_reg = lines(root/'private-registry-before.jsonl')
            after_reg = lines(root/'private-registry-after.jsonl')
            registry_normalizations = normalize_private_registry(root, after_reg)
            if action != 'complete':
                assert before == after, (root, 'cancel changed owned files or attributes')
                assert application_registry(before_reg) == application_registry(after_reg), (root, 'cancel changed app registry')
            snapshots = {}
            for phase in ('language', 'confirm', 'completed'):
                path = root/f'ui-{phase}.json'
                if not path.exists():
                    continue
                row = json.loads(path.read_text())
                for control in row['controls']:
                    control['text'] = control['text'].replace(str(root), '<ROOT>')
                snapshots[phase] = row
            captures = {}
            for phase in snapshots:
                path = root/f'ui-{phase}.bmp'
                rgb = Image.open(path).convert('RGB')
                colors = len(rgb.getcolors(rgb.width*rgb.height))
                captures[phase] = dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(path),
                    width=rgb.width, height=rgb.height, rgb_colors=colors,
                    status='flat_noninteractive_capture_not_pixel_evidence' if colors <= 1 else 'raster_available')
            pair[role] = dict(work_directory=root.relative_to(ROOT).as_posix(), exit_code=code,
                entry_boundary=boundary, stages=driver, controls=snapshots, captures=captures,
                files_before=before, files_after=after,
                application_registry_before=application_registry(before_reg),
                application_registry_after=application_registry(after_reg),
                normalized_private_registry_sha256=hashlib.sha256(json.dumps(after_reg, sort_keys=True).encode()).hexdigest(),
                registry_normalizations=registry_normalizations,
                raw_private_registry_sha256=sha(root/'private-registry-after.jsonl'),
                application_removal_trace=normalized_removal(root, events) if action == 'complete' else [],
                raw_trace_sha256=sha(root/'guard-events.jsonl'), runner_sha256=sha(root/'runner/Uninstall.exe'))
            assert pair[role]['runner_sha256'] == sha(executable)
        # Temp/plugin cleanup has random paths; all application file/registry
        # operations through the completion point precede that cleanup.
        for role in pair:
            pair[role]['application_removal_trace'] = [row for row in pair[role]['application_removal_trace']
                if not row['path'].startswith('<ROOT>\\temp\\')]
        for field in ('exit_code', 'stages', 'controls', 'files_before', 'files_after',
                      'application_registry_before', 'application_registry_after',
                      'normalized_private_registry_sha256', 'application_removal_trace'):
            assert pair['source'][field] == pair['reference'][field], (case, action, field)
        count = sum(len(row['controls']) for row in pair['source']['controls'].values())
        total_controls += count
        results.append(dict(case=case, action=action, exact_control_records=count,
                            exact_observed_flow_and_effects=True, runs=pair))
        print(f'{case}/{action}: UI controls, exit and effects match ({count} controls)', flush=True)
    for row in inventory['files']:
        assert sha(original_root/row['path']) == row['sha256'], row['path']
    sources = [path for path in (ROOT/'tools/uninstall_sandbox').iterdir() if path.is_file()]
    sources += [Path(__file__).resolve(), ROOT/'tools/validate_uninstaller_runtime.py',
                ROOT/'tools/prepare_nsis_toolchain.py', ROOT/'analysis/nsis-build-config.json']
    report = dict(status='passed_for_private_native_uninstall_dialog_flows_controls_and_effects',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        source_files={path.relative_to(ROOT).as_posix(): sha(path) for path in sorted(sources)},
        source_uninstaller_sha256=sha(source), original_reference_sha256=sha(reference),
        helper_sha256=sha(build/'containment-probe.exe'), guard_sha256=sha(build/'uninstall-application-guard.dll'),
        fixture_pairs=len(results), uninstall_processes=len(results)*2,
        exact_control_records=total_controls, flows=results,
        language_plugin_runtime_tested=True, stored_language_and_invalid_language_tested=True,
        pixel_equality_verified=False,
        pixel_capture_limitation='Private noninteractive station yields flat PrintWindow/WM_PRINT images; no pixel fidelity claimed',
        private_desktop_never_switched_to_user=True, registry_overrides_before_entry=True,
        bootstrap_self_copy_verified=False, reference_files_unchanged=len(inventory['files']), whole_game_complete=False)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8', newline='\n')
    print(f'Passed {len(results)} native UI pairs, {total_controls} control records; pixel capture unavailable; bootstrap pending.')


if __name__ == '__main__':
    main()
