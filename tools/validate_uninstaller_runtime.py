"""Compare known source/reference NSIS removal in owned restricted fixtures only."""
from __future__ import annotations
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import re
import subprocess
import uuid
from pe import PE

ROOT = Path(__file__).resolve().parents[1]
CASES = ('empty-fallback', 'missing-registration', 'named-empty', 'named-nonempty',
         'player-empty', 'player-nonempty', 'missing-player-menu',
         'subkey-keeps-virtualfarm', 'empty-menu-value', 'registry-values-only')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def lines(path):
    return [json.loads(row) for row in path.read_text(encoding='ascii').splitlines()]


def manifest(root):
    result = []
    for part in ('payload', 'shell', 'temp'):
        base = root/part
        if not base.exists():
            result.append(dict(path=part, exists=False))
            continue
        for path in [base, *sorted(base.rglob('*'))]:
            assert not path.is_symlink()
            row = dict(path=path.relative_to(root).as_posix(), directory=path.is_dir())
            if path.is_file():
                row.update(bytes=path.stat().st_size, sha256=sha(path))
            result.append(row)
    return result


def normalized_removal(root, events):
    ansi_root = str(root).encode('mbcs').decode('latin1')
    result = []
    started = False
    for entry in events:
        entry = dict(entry)
        entry['path'] = entry['path'].replace(ansi_root, '<ROOT>')
        if entry['path'].startswith('<ROOT>\\payload\\'):
            started = True
        if started:
            result.append(entry)
    assert result and result[0]['api'] == 'SetFileAttributesA'
    return result


def normalize_private_registry(root, rows):
    """Keep all keys/values, normalize only observed Windows initialization data."""
    changes = []
    for row in rows:
        row['values'].sort(key=lambda item: item['name'])
        for value in row['values']:
            if (row['hive'], row['key'], value['name']) == (
                'HKCU', r'Software\Microsoft\Windows\CurrentVersion\Internet Settings',
                'ZonesSecurityUpgrade'):
                assert value['type'] == 3 and len(bytes.fromhex(value['data_hex'])) == 8
                timestamp = int.from_bytes(bytes.fromhex(value['data_hex']), 'little')/10_000_000 - 11644473600
                assert abs(timestamp-datetime.datetime.now(datetime.timezone.utc).timestamp()) < 120
                changes.append(dict(hive=row['hive'], key=row['key'], name=value['name'],
                                    kind='Windows zone initialization FILETIME', raw=value['data_hex']))
                value['data_hex'] = '<OS-INITIALIZATION-FILETIME>'
            if (row['hive'] == 'HKLM' and row['key'] in (
                r'Software\Microsoft\Advanced INF Setup\IE.HKCUZoneInfo',
                r'Software\Microsoft\Advanced INF Setup\IE40.UserAgent') and
                value['name'] == 'InstallINFFile'):
                assert value['type'] == 1
                raw = bytes.fromhex(value['data_hex']).decode('utf-16-le')
                assert raw.startswith(str(root)+'\\temp\\RGI') and raw.endswith('.tmp\0')
                normalized = re.sub(r'\\RGI[0-9a-fA-F]+\.tmp\0$', r'\\<OS-INF-TEMP>.tmp', raw)
                assert normalized != raw
                changes.append(dict(hive=row['hive'], key=row['key'], name=value['name'],
                                    kind='Windows temporary INF path', raw=raw))
                value['data_hex'] = normalized.replace(str(root), '<ROOT>').encode('utf-16-le').hex()
    rows.sort(key=lambda item: (item['hive'], item['key']))
    return changes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, default=ROOT/'build/uninstall-sandbox')
    parser.add_argument('--source', type=Path, default=ROOT/'build/windows/Release/Uninstall.exe')
    parser.add_argument('--cases', nargs='+', choices=CASES, default=CASES)
    parser.add_argument('--guard-runs', type=int, default=3)
    parser.add_argument('--output', type=Path, default=ROOT/'.local/uninstaller-runtime-validation.json')
    args = parser.parse_args()
    assert 1 <= args.guard_runs <= 10 and len(set(args.cases)) == len(args.cases)
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    originals = ROOT.parent/'farmář/Dobrý farmář'
    for row in inventory['files']:
        assert sha(originals/row['path']) == row['sha256'], row['path']
    build = args.build_dir.resolve()/'Release'
    helper = build/'containment-probe.exe'
    source, reference = args.source.resolve(), originals/'Uninstall.exe'
    assert sha(source) != sha(reference)
    # Every imported application mutation/loader boundary must have a guard.
    guard_source = (ROOT/'tools/uninstall_sandbox/application_guard.cpp').read_text(encoding='utf-8')
    hooks = set(re.findall(r'HOOK\((\w+)\)', guard_source)) - {'api'}
    prefixes = ('CreateFile', 'CreateDirectory', 'RemoveDirectory', 'DeleteFile', 'MoveFile',
                'CopyFile', 'WriteFile', 'SetFileTime', 'SetFileAttributes', 'WritePrivateProfile',
                'GetTempFileName', 'CreateProcess', 'CreateRemoteThread', 'ExitWindowsEx',
                'ShellExecute', 'SHFileOperation', 'RegOpen', 'RegClose', 'RegCreate',
                'RegDelete', 'RegSet', 'RegQuery', 'RegEnum', 'LoadLibrary', 'GetProcAddress',
                'CoCreateInstance', 'SystemParametersInfo', 'SetCurrentDirectory')
    import_audits = {}
    for role, path in [('source', source), ('reference', reference)]:
        image = PE(path)
        assert image.bits == 32 and image.machine == 0x14c
        mutations = sorted({entry['name'] for dll in image.imports() for entry in dll['symbols']
                            if 'name' in entry and entry['name'].startswith(prefixes)})
        assert set(mutations) <= hooks, set(mutations) - hooks
        import_audits[role] = dict(path=str(path), sha256=sha(path), guarded_imports=mutations)
    parent = ROOT/'.local/uninstall-isolation'
    parent.mkdir(parents=True, exist_ok=True)

    def run(mode, name, extra=()):
        root = parent/f'{name}-{uuid.uuid4()}'
        command = [str(helper), mode, str(root), str(build), *map(str, extra)]
        proc = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
        (root/'harness-stdout.txt').write_text(proc.stdout, encoding='utf-8')
        (root/'harness-stderr.txt').write_text(proc.stderr, encoding='utf-8')
        assert proc.returncode == 0, (root, proc.returncode, proc.stderr)
        proof = json.loads(proc.stdout)
        assert proof['station_dacl_byte_exact_after_restore']
        assert proof['private_registry_keys_removed'] and proof['outside_markers_unchanged']
        boundary = json.loads((root/'entry-boundary.json').read_text())
        assert boundary['entry_held_until_guard_setup'] and boundary['guard_ready'] == 0x4641524d
        events = lines(root/'guard-events.jsonl')
        assert events[0]['api'] == 'setup-before-application-entry'
        return root, proof, boundary, events

    guards = []
    for index in range(args.guard_runs):
        root, proof, boundary, events = run('--guard-client', f'guard-validation-{index}')
        assert proof['guarded_entry_client_verified'] and not proof['uninstaller_executed']
        denied = {entry['api'] for entry in events if not entry['allowed']}
        assert {'CreateFileA', 'DeleteFileA', 'CopyFileA.destination', 'MoveFileA.destination',
                'WriteFile', 'SetFileTime', 'CreateDirectoryA', 'LoadLibraryExA',
                'SHGetSpecialFolderLocation.unknown', 'GetProcAddress.unknown'} <= denied
        assert any(entry['api'] == 'MoveFileExA.delayed-intent-only' for entry in events)
        guards.append(dict(work_directory=root.relative_to(ROOT).as_posix(),
                           denied_apis=sorted(denied), entry_boundary=boundary))
    results = []
    for case in args.cases:
        pair = {}
        for role, executable in [('source', source), ('reference', reference)]:
            root, proof, boundary, events = run('--uninstall-quiet', f'{role}-{case}', (executable, case))
            assert proof['uninstaller_executed'] and not proof['guarded_entry_client_verified']
            assert not [entry for entry in events if not entry['allowed']], (root, 'denied application operation')
            assert not any(entry['api'].endswith('delayed-intent-only') for entry in events)
            registry = lines(root/'private-registry-after.jsonl')
            registry_normalizations = normalize_private_registry(root, registry)
            pair[role] = dict(work_directory=root.relative_to(ROOT).as_posix(),
                entry_boundary=boundary,
                fixture=json.loads((root/'uninstall-fixture-result.json').read_text()),
                manifest=manifest(root), private_registry=registry,
                raw_private_registry_sha256=sha(root/'private-registry-after.jsonl'),
                private_registry_normalizations=registry_normalizations,
                removal_trace=normalized_removal(root, events),
                trace_events=len(events), raw_trace_sha256=sha(root/'guard-events.jsonl'),
                runner_sha256=sha(root/'runner/Uninstall.exe'))
            assert pair[role]['runner_sha256'] == sha(executable)
        for field in ('fixture', 'manifest', 'private_registry', 'removal_trace'):
            assert pair['source'][field] == pair['reference'][field], (case, field)
        results.append(dict(case=case, exact_final_tree_and_normalized_private_registry=True,
                            exact_removal_trace_events=len(pair['source']['removal_trace']), runs=pair))
        print(f'{case}: source/reference effects and removal trace match', flush=True)
    for row in inventory['files']:
        assert sha(originals/row['path']) == row['sha256'], row['path']
    sources = [path for path in (ROOT/'tools/uninstall_sandbox').iterdir() if path.is_file()]
    sources += [Path(__file__).resolve()]
    report = dict(status='passed_for_guarded_silent_uninstall_fixture_pairs_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        source_files={path.relative_to(ROOT).as_posix(): sha(path) for path in sorted(sources)},
        import_audits=import_audits,
        diagnostic_binaries={str(path.relative_to(ROOT)): sha(path) for path in
            (helper, build/'uninstall-application-guard.dll', build/'uninstall-guard-client.exe')},
        guard_client_lifetimes=len(guards), guard_clients=guards,
        uninstall_processes=len(results)*2, cases=results,
        private_registry_overrides_before_entry=True,
        boundary='Untrusted restricted primary token, private desktop, single-process Job; '
                 'known ANSI application imports guarded; source guard DLL injected before PE entry',
        windows_installer_detection='Per-child __COMPAT_LAYER=RunAsInvoker; no elevation',
        startup_mode='/S _?=<owned payload>; self-copy/bootstrap intentionally bypassed for these cases',
        temporary_decompression_write_chunk_counts_not_compared=True,
        final_temporary_directory_compared=True,
        reference_files_unchanged=len(inventory['files']),
        isolated_silent_removal_effects_verified=True,
        interactive_ui_verified=False, bootstrap_self_copy_verified=False,
        actual_system_delayed_delete_queue_never_used=True, whole_game_complete=False)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n',
                           encoding='utf-8', newline='\n')
    print(f'Passed {len(results)} source/reference fixture pairs, {len(guards)} guard clients; '
          f'{len(inventory["files"])} originals unchanged. UI/bootstrap remain pending.')


if __name__ == '__main__':
    main()
