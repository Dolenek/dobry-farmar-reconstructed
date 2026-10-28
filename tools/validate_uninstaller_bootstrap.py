"""Compare real NSIS self-copy trees under the owned diagnostic process broker."""
from __future__ import annotations
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import subprocess
import uuid
from validate_uninstaller_runtime import ROOT, sha, lines, normalize_private_registry, normalized_removal
from validate_uninstaller_ui import fixture_files, application_registry

FLOWS = (('empty-fallback', ''), ('named-nonempty', ''), ('player-nonempty', ''),
         ('bootstrap-copy-busy-a', ''), ('empty-fallback', 'language-cancel'),
         ('empty-fallback', 'cancel'), ('empty-fallback', 'complete'),
         ('stored-language', 'complete'))


def normalize(root, events):
    raw_root = str(root).encode('mbcs').decode('latin1')
    return [dict(entry, path=entry['path'].replace(raw_root, '<ROOT>')) for entry in events]


def normalize_input_file(rows, executable):
    rows = [dict(row) for row in rows]
    images = [row for row in rows if row['path'] == r'payload\Uninstall.exe']
    if images:
        assert len(images) == 1 and bytes.fromhex(images[0]['bytes_hex']) == executable.read_bytes()
        images[0]['bytes_hex'] = '<OWN-INPUT-UNINSTALLER>'
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT/'.local/uninstaller-bootstrap-validation.json')
    args = parser.parse_args()
    build = ROOT/'build/uninstall-sandbox/Release'
    helper = build/'containment-probe.exe'
    source = ROOT/'build/windows/Release/Uninstall.exe'
    originals = ROOT.parent/'farmář/Dobrý farmář'
    reference = originals/'Uninstall.exe'
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    for row in inventory['files']:
        assert sha(originals/row['path']) == row['sha256'], row['path']
    parent = ROOT/'.local/uninstall-isolation'
    parent.mkdir(parents=True, exist_ok=True)

    def run(mode, label, extra):
        root = parent/f'{label}-{uuid.uuid4()}'
        process = subprocess.run([str(helper), mode, str(root), str(build), *map(str, extra)],
                                 cwd=ROOT, capture_output=True, text=True, timeout=90)
        (root/'harness-stdout.txt').write_text(process.stdout, encoding='utf-8')
        (root/'harness-stderr.txt').write_text(process.stderr, encoding='utf-8')
        assert process.returncode == 0, (root, process.returncode, process.stderr)
        proof = json.loads(process.stdout)
        assert proof['bootstrap_process_tree_verified']
        assert proof['application_job_active_limit'] == 2 and not proof['child_job_limit_one']
        assert proof['outside_markers_unchanged'] and proof['private_registry_keys_removed']
        assert proof['station_dacl_byte_exact_after_restore']
        tree = json.loads((root/'bootstrap-tree-result.json').read_text())
        for key in ('same_input_image', 'same_owned_job', 'same_restricted_primary_preserved',
                    'child_hives_mapped_before_entry', 'child_application_entry_guarded'):
            assert tree[key], key
        assert tree['job_active_process_limit'] == 2 and tree['integrity_rid'] == 0
        for name in ('entry-boundary.json', 'entry-boundary-child.json'):
            assert json.loads((root/name).read_text())['entry_held_until_guard_setup']
        return root, proof, tree

    safety = []
    for index in range(3):
        root, proof, tree = run('--bootstrap-client', f'bootstrap-safety-{index}', ())
        assert proof['bootstrap_owned_client_verified'] and not proof['uninstaller_executed']
        assert (root/'bootstrap-client-proof.txt').read_bytes() == b'restricted-descendant-before-entry'
        assert not tree['creation_brokered_for_error740'] and tree['child_exit_code'] == 0
        safety.append(dict(work_directory=root.relative_to(ROOT).as_posix(), tree=tree))
    results = []
    parent_events_compared = 0
    removal_events_compared = 0
    pending_total = 0
    for case, action in FLOWS:
        pair = {}
        for role, executable in [('source', source), ('reference', reference)]:
            mode = '--uninstall-bootstrap-ui' if action else '--uninstall-bootstrap'
            extra = (executable, case, action) if action else (executable, case)
            root, proof, tree = run(mode, f'{role}-bootstrap-{case}-{action or "silent"}', extra)
            assert proof['uninstaller_executed'] and not proof['bootstrap_owned_client_verified']
            assert json.loads((root/'guarded-exit-code.json').read_text())['exit_code'] == 0
            assert tree['child_exit_code'] == {'language-cancel': 2, 'cancel': 1}.get(action, 0)
            letter = 'B' if case == 'bootstrap-copy-busy-a' else 'A'
            assert tree['copy_letter'] == letter
            raw_parent = lines(root/'guard-events.jsonl')
            # Actual 740 is recorded, then the real same-token CreateProcessAsUser
            # broker supplies genuine handles. Other denied operations are errors.
            denied = [row for row in raw_parent if not row['allowed']]
            if tree['creation_brokered_for_error740']:
                assert len(denied) == 1 and denied[0]['api'] == 'CreateProcessA.bootstrap-api-result' and denied[0]['error'] == 740
                assert any(row['api'] == 'CreateProcessA.bootstrap-created-by-owner-with-same-restricted-token' for row in raw_parent)
            else:
                assert not denied
            child_events = lines(root/'guard-child-events.jsonl')
            assert not [row for row in child_events if not row['allowed']], (root, 'denied child operation')
            events = normalize(root, raw_parent)
            start = next(i for i, row in enumerate(events) if row['api'] == 'CreateDirectoryA' and row['path'] == '<ROOT>\\temp\\~nsu.tmp')
            bootstrap_trace = events[start:]
            deletes = [row['path'] for row in bootstrap_trace if row['api'] == 'DeleteFileA']
            assert deletes == [f'<ROOT>\\temp\\~nsu.tmp\\{chr(65+i)}u_.exe' for i in range(26)]
            copies = [row for row in bootstrap_trace if row['api'] == 'CopyFileA.destination']
            assert len(copies) == (2 if letter == 'B' else 1)
            pending = [row for row in events if row['api'] == 'MoveFileExA.delayed-intent-only']
            assert pending == [dict(api='MoveFileExA.delayed-intent-only', path=path, allowed=True, error=5)
                               for path in (f'<ROOT>\\temp\\~nsu.tmp\\{letter}u_.exe', '<ROOT>\\temp\\~nsu.tmp')]
            assert not any(row['api'] == 'MoveFileExA.delayed-intent-only' for row in child_events)
            copy = root/'temp/~nsu.tmp'/f'{letter}u_.exe'
            assert sha(copy) == sha(executable) and sha(root/'runner/Uninstall.exe') == sha(executable)
            assert not list((root/'temp').rglob('*.manifest'))
            assert sorted(path.relative_to(root/'temp').as_posix() for path in (root/'temp').rglob('*')) == ['~nsu.tmp', f'~nsu.tmp/{letter}u_.exe']
            before = normalize_input_file(fixture_files(root/'fixture-files-before.jsonl'), executable)
            after = normalize_input_file(fixture_files(root/'fixture-files-after.jsonl'), executable)
            before_reg = lines(root/'private-registry-before.jsonl')
            after_reg = lines(root/'private-registry-after.jsonl')
            reg_normalizations = normalize_private_registry(root, after_reg)
            if action in ('cancel', 'language-cancel'):
                assert before == after and application_registry(before_reg) == application_registry(after_reg)
                removal = []
            else:
                removal = [row for row in normalized_removal(root, child_events) if not row['path'].startswith('<ROOT>\\temp\\')]
            controls = {}
            if action:
                for phase in ('language', 'confirm', 'completed'):
                    path = root/f'ui-{phase}.json'
                    if path.exists():
                        snapshot = json.loads(path.read_text())
                        for control in snapshot['controls']:
                            control['text'] = control['text'].replace(str(root), '<ROOT>')
                        controls[phase] = snapshot
            # Never touch the actual OS delayed-delete queue. Apply its two
            # recorded intents only after both owned processes have exited,
            # using ordinary nonrecursive deletes of these exact owned paths.
            assert copy.resolve().is_relative_to(root.resolve())
            copy.unlink()
            directory = root/'temp/~nsu.tmp'
            assert directory.resolve().is_relative_to(root.resolve()) and not any(directory.iterdir())
            directory.rmdir()
            assert not any((root/'temp').iterdir())
            normalized_tree = {key: value for key, value in tree.items() if key not in ('parent_pid', 'child_pid')}
            pair[role] = dict(work_directory=root.relative_to(ROOT).as_posix(), tree=normalized_tree,
                source_input_sha256=sha(executable), copied_image_exactly_matches_own_input=True,
                parent_trace=bootstrap_trace, child_removal_trace=removal,
                files_before=before, files_after=after,
                application_registry_after=application_registry(after_reg),
                normalized_private_registry_sha256=hashlib.sha256(json.dumps(after_reg, sort_keys=True).encode()).hexdigest(),
                private_registry_normalizations=reg_normalizations, controls=controls,
                delayed_intents=pending, owned_intents_applied_with_nonrecursive_deletes=True,
                final_owned_temp_empty=True,
                raw_parent_trace_sha256=sha(root/'guard-events.jsonl'), raw_child_trace_sha256=sha(root/'guard-child-events.jsonl'))
        for field in ('tree', 'parent_trace', 'child_removal_trace', 'files_before', 'files_after',
                      'application_registry_after', 'normalized_private_registry_sha256',
                      'controls', 'delayed_intents', 'final_owned_temp_empty'):
            assert pair['source'][field] == pair['reference'][field], (case, action, field)
        parent_events_compared += len(pair['source']['parent_trace'])
        removal_events_compared += len(pair['source']['child_removal_trace'])
        pending_total += len(pair['source']['delayed_intents'])
        results.append(dict(case=case, action=action or 'silent', exact_observed_bootstrap_and_effects=True, runs=pair))
        print(f'{case}/{action or "silent"}: native self-copy tree, intents and effects match', flush=True)
    for row in inventory['files']:
        assert sha(originals/row['path']) == row['sha256'], row['path']
    paths = [path for path in (ROOT/'tools/uninstall_sandbox').iterdir() if path.is_file()]
    paths += [Path(__file__).resolve(), ROOT/'tools/validate_uninstaller_runtime.py', ROOT/'tools/validate_uninstaller_ui.py']
    report = dict(status='passed_for_guarded_real_bootstrap_process_trees_and_owned_delayed_intents',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        source_files={path.relative_to(ROOT).as_posix(): sha(path) for path in sorted(paths)},
        diagnostic_binaries={path.relative_to(ROOT).as_posix(): sha(path) for path in
            (helper, build/'uninstall-application-guard.dll', build/'uninstall-bootstrap-client.exe')},
        source_uninstaller_sha256=sha(source), original_reference_sha256=sha(reference),
        own_safety_process_trees=safety, fixture_pairs=len(results), actual_uninstaller_processes=len(results)*4,
        exact_parent_trace_events=parent_events_compared, exact_child_removal_events=removal_events_compared,
        exact_paired_delayed_intents=pending_total, flows=results,
        no_original_program_code_used_as_implementation=True,
        test_os_broker_for_untrusted_installer_error740=True,
        same_restricted_token_and_real_parent_process_preserved=True,
        bootstrap_children_held_before_entry_until_private_hives_and_guards_ready=True,
        actual_system_reboot_queue_never_used=True,
        actual_reboot_not_tested=True,
        owned_delayed_intents_applied_after_both_processes_exit=True,
        ordinary_elevated_host_uninstall_not_tested=True,
        pixel_comparison_verified=False, reference_files_unchanged=len(inventory['files']), whole_game_complete=False)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8', newline='\n')
    print(f'Passed {len(results)} native bootstrap pairs ({len(results)*4} uninstall processes); real reboot queue untouched.')


if __name__ == '__main__':
    main()
