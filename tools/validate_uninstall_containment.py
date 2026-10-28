"""Run only our marker/hive containment probe, never an uninstall executable."""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
from pathlib import Path
import subprocess
import uuid

from pe import PE

ROOT = Path(__file__).resolve().parents[1]


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, default=ROOT/'build/uninstall-sandbox')
    parser.add_argument('--runs', type=int, default=3)
    parser.add_argument('--output', type=Path,
                        default=ROOT/'.local/uninstaller-containment-validation.json')
    args = parser.parse_args()
    assert 1 <= args.runs <= 10
    executable = args.build_dir.resolve()/'Release/containment-probe.exe'
    image = PE(executable)
    assert image.bits == 32 and image.machine == 0x14c
    assert not image.characteristics & 0x2000
    parent = ROOT/'.local/uninstall-isolation'
    parent.mkdir(parents=True, exist_ok=True)
    results = []
    for _ in range(args.runs):
        root = parent/f'primary-validation-{uuid.uuid4()}'
        run = subprocess.run([str(executable), str(root)], cwd=ROOT,
                             capture_output=True, text=True, timeout=45)
        (root/'harness-stdout.txt').write_text(run.stdout, encoding='utf-8')
        (root/'harness-stderr.txt').write_text(run.stderr, encoding='utf-8')
        assert run.returncode == 0, (root, run.returncode, run.stderr)
        proof = json.loads(run.stdout)
        assert proof['status'] == 'passed_for_restricted_primary_child_private_desktop_and_impersonation'
        for flag in ('inside_file_write', 'outside_file_write_denied',
                     'outside_file_delete_denied',
                     'child_outside_medium_and_low_integrity_mutations_denied',
                     'outside_dacl_owner_mutations_denied', 'system_runtime_read',
                     'inside_registry_write', 'outside_registry_write_denied',
                     'outside_markers_unchanged', 'private_registry_keys_removed',
                     'restricted_primary_child', 'child_private_hkcu_hklm',
                     'child_original_hkcu_write_denied', 'private_desktop',
                     'noninteractive_station_acl_restored',
                     'station_dacl_byte_exact_after_restore', 'child_job_limit_one'):
            assert proof[flag] is True, flag
        assert proof['private_window_station'] is False
        assert proof['untrusted_integrity_rid'] == 0
        assert proof['uninstaller_executed'] is False
        assert (root/'child-proof.txt').read_bytes() == b'restricted-primary-private-desktop-and-hives'
        assert sha(root/'probe-child.exe') == sha(executable)
        results.append(dict(work_directory=root.relative_to(ROOT).as_posix(), proof=proof))
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    originals = ROOT.parent/'farmář/Dobrý farmář'
    for entry in inventory['files']:
        assert sha(originals/entry['path']) == entry['sha256'], entry['path']
    sources = sorted(path.relative_to(ROOT).as_posix()
                     for path in (ROOT/'tools/uninstall_sandbox').iterdir() if path.is_file())
    sources.append('tools/validate_uninstall_containment.py')
    report = dict(
        status='passed_for_owned_restricted_primary_marker_hive_and_desktop_probes_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        source_files={name: sha(ROOT/name) for name in sources},
        probe_path=executable.relative_to(ROOT).as_posix(), probe_sha256=sha(executable),
        repeated_lifetimes=len(results), runs=results,
        reference_files_unchanged=len(inventory['files']),
        policy='Full restricted primary token: unique SID, Restricted Code and Builtin Users; '
               'administrators deny-only, privileges removed, Untrusted integrity RID0; '
               'private hive/root/desktop labelled Untrusted; default object ACL grants unique SID',
        noninteractive_station_shared=True,
        noninteractive_station_unique_sid_acl_temporarily_added_and_byte_exact_restored=True,
        application_boundary_guards_not_yet_implemented=True,
        uninstaller_executed=False, uninstaller_effects_verified=False,
        whole_game_complete=False)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n',
                           encoding='utf-8', newline='\n')
    print(f'Passed {len(results)} real restricted primary processes; private HKCU/HKLM, '
          f'desktop and owned guards; {len(inventory["files"])} originals unchanged. '
          'No uninstaller executed.')


if __name__ == '__main__':
    main()
