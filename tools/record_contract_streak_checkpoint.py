#!/usr/bin/env python3
"""Reference independently earned native/source checkpoints without editing saves."""
import datetime
import hashlib
from record_live_production_verification import ROOT, digest, read, write
from validate_counter_supply_state import props


def main():
    inputs = [('native', 'analysis/contract-streak-retained-prefix-state-validation.json'),
              ('source', 'analysis/contract-streak-source-restored-ten-state-validation.json')]
    profile_sets = []
    for kind, name in inputs:
        report = read(ROOT / name)
        index, session = next((n, s) for n, s in enumerate(report['sessions']) if s['kind'] == kind)
        branch = next(b for b in report['comparison']['branches'] if b['kind'] == kind)
        assert session['status'] == 'completed' and session['exit_code'] == 0
        assert branch['final_streak'] == 10 and branch['requested_target_reached']
        directory = ROOT / report['work_directory'] / f'{index}-persisted/Profiles'
        values = props(directory / '00000001.xml')
        assert values['awQTask'] == ('UInt', '10')
        assert all(values[f'task{n}'] == ('Int', '-1') for n in range(3))
        assert values['Award04Make20Task'] == values['Award05Make40Task'] == ('Bool', '0')
        assert digest(directory / '00000001.save') == branch['complete_saved_sha256']
        assert digest(directory / '00000001.xml') == branch['saved_profile_sha256']
        profile_sets.append(dict(kind=kind, profile_directory=directory.relative_to(ROOT).as_posix(),
            files={p.name: digest(p) for p in sorted(directory.iterdir()) if p.is_file()},
            source_session_index=index, source_report=name, source_report_sha256=digest(ROOT / name),
            streak=10, money=int(values['Money'][1]), complete_expected_saved_bytes=branch['complete_expected_saved_bytes']))
    full = read(ROOT / 'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT / name) == sha for name, sha in full['production_source_files'].items())
    for row in read(ROOT / 'analysis/inventory.json')['files']:
        assert digest(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159' / row['path']) == row['sha256']
    result = dict(status='verified_actual_contract_streak_checkpoint_profiles',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        work_directory='.', profile_sets=profile_sets, ordinary_farming_supply_claimed=False,
        profile_or_counter_fields_written=False, original_files_unchanged=17, whole_game_complete=False,
        source_includes_actual_restore_and_completion_of_its_retained_active_contract=True,
        native_and_source_random_streams_claimed_matched=False, normalizations=[])
    checkpoint_path = ROOT / 'analysis/contract-streak-ten-checkpoints.json'
    if checkpoint_path.exists():
        # A live driver may already reference these exact metadata bytes.
        # Publishing another scope must not rewrite its timestamp or SHA.
        previous = read(checkpoint_path)
        assert previous['status'] == result['status'] and previous['profile_sets'] == profile_sets
    else:
        write(checkpoint_path, result)
    source = read(ROOT / inputs[1][1])
    native_name = 'analysis/contract-streak-native-active-restore-state-validation.json'
    native = read(ROOT / native_name)
    ns, ss = native['sessions'][0], source['sessions'][0]
    assert ns['kind'] == 'native' and ss['kind'] == 'source'
    assert ns['input_profile_files'] == ss['input_profile_files']
    assert ns['seed_profile_directory'] == ss['seed_profile_directory']
    assert ns['exit_code'] == ss['exit_code'] == 0
    left, right = ns['contract_receipts'][0], ss['contract_receipts'][0]
    assert left['offer'] == right['offer'] == 341
    assert left['model'] == right['model']
    assert left['model']['requirements'] == [dict(product=4, initial_count=30, count=29)]
    assert left['model']['bonus'] == 940 and left['model']['penalty'] == 380
    assert left['before_acceptance'] is right['before_acceptance'] is None
    for key in ['stock', 'market', 'money', 'streak', 'tasks']:
        assert left['after_completion'][key] == right['after_completion'][key]
    for receipt in [left, right]:
        assert receipt['restored_active_state']['money'] == 4944
        assert receipt['before_completion']['money'] == 4944
        assert receipt['after_completion']['money'] == 5884 and receipt['after_completion']['streak'] == 6
    nb = native['comparison']['branches'][0]
    assert nb['entire_binary_unchanged'] and nb['complete_expected_saved_bytes'] == 15991
    prior = read(ROOT / inputs[0][1])['comparison']['branches'][1]['retained_active_contract']
    assert prior['model']['requirements'] == [dict(product=4, initial_count=30, count=33)]
    assert prior['model']['bonus'] == 1070 and prior['paid_deposit'] == 430
    comparison = dict(status='passed_original_and_source_actual_restore_and_completion_of_identical_active_contract_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        reports={name: digest(ROOT / name) for name in [inputs[0][1], inputs[1][1], native_name]},
        identical_input_profile_files=ns['input_profile_files'], offer=341, original_paid_deposit_before_restore=430,
        cumulative_quantity_before_restore=33, quantity_after_actual_restore_both=29,
        cumulative_bonus_before_restore=1070, bonus_after_actual_restore_both=940,
        model_penalty_after_restore_both=380, additional_deposit_charged_on_restore=0,
        actual_money_before_completion=4944, actual_money_after_completion=5884, final_streak_both=6,
        all19_stock_and_market_words_identical_after_completion=True,
        entire_native15991_byte_save_unchanged=True, source_intermediate_binary_save_claimed_captured=False,
        source_completed_five_receipts_to_streak10=True, production_changed=False, normalizations=[], whole_game_complete=False)
    comparison_name = 'contract-streak-active-restore-comparison.json'
    write(ROOT / 'analysis' / comparison_name, comparison)
    startup_name = '.local/contract-streak/20261106T182123042718Z/report.json'
    startup = read(ROOT / startup_name)
    assert startup['status'] == 'failed' and 'Diagnostic timeout must be30..1800' in startup['error']
    assert not any('launcher' in s for s in startup['sessions'])
    original_path = ROOT / '.local/contract-streak-ten-checkpoints-original.json'
    rewritten_path = ROOT / '.local/contract-streak-ten-checkpoints-rewritten.json'
    original, rewritten = read(original_path), read(rewritten_path)
    expected = startup['baseline_report']['sha256']
    assert digest(original_path) == digest(checkpoint_path) == expected
    changed = [key for key in original if original[key] != rewritten[key]]
    assert sorted(changed) == ['recorded_at_utc', 'recorder_sha256']
    original_recorder = ROOT / '.local/contract-streak-ten-checkpoint-original-recorder.py'
    assert digest(original_recorder) == original['recorder_sha256']
    observation = dict(status='restored_exact_original_checkpoint_metadata_and_retained_launcher_argument_failure',
        recorder_sha256=digest(__file__), startup_report=startup_name, startup_report_sha256=digest(ROOT / startup_name),
        startup_failed_before_launcher_ready=True, invalid_requested_watchdog_seconds=3700, native_maximum_watchdog_seconds=1800,
        subsequent_phases_use_five_contract_increments=True,
        metadata_rewrite_cause='Recorder rerun changed only publication timestamp and recorder hash after a live driver captured original metadata hash',
        metadata_changed_keys=changed, original_complete_bytes=original_path.stat().st_size,
        original_sha256=expected, restored_current_sha256=digest(checkpoint_path),
        original_metadata_snapshot=original_path.relative_to(ROOT).as_posix(),
        rewritten_metadata_snapshot=rewritten_path.relative_to(ROOT).as_posix(), rewritten_sha256=digest(rewritten_path),
        original_recorder_snapshot=original_recorder.relative_to(ROOT).as_posix(), original_recorder_sha256=digest(original_recorder),
        exact_original_timestamp_recovered_against_preexisting_sha256=True,
        profile_sets_changed=False, profiles_or_counters_written=False, existing_checkpoint_metadata_now_immutable=True,
        whole_game_complete=False)
    observation_name = 'contract-streak-checkpoint-metadata-observations.json'
    write(ROOT / 'analysis' / observation_name, observation)
    verification = read(ROOT / 'analysis/verification.json')
    names = ['contract-streak-ten-checkpoints.json', comparison_name, observation_name,
             'contract-streak-source-restored-ten-state-validation.json', 'contract-streak-source-restored-ten-regression.json',
             'contract-streak-native-active-restore-state-validation.json', 'contract-streak-native-active-restore-regression.json']
    for name in names:
        if 'analysis/' + name not in verification['reports']:
            verification['reports'].append('analysis/' + name)
    verification['latest_scoped_regression'] = dict(report='analysis/' + comparison_name,
        recorded_at_utc=comparison['recorded_at_utc'], whole_game_complete=False)
    paths = [ROOT / 'CMakeLists.txt'] + [p for folder in ['src', 'tools', 'tests'] for p in (ROOT / folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows = [p.relative_to(ROOT).as_posix() + '\0' + digest(p) + '\n' for p in sorted(paths, key=lambda p: p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(rows).encode('utf8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/' + comparison_name
    write(ROOT / 'analysis/verification.json', verification)
    print('Linked two independently earned streak10 checkpoints;Money8274/8124,complete16194-byte saves;no profile writes.')


if __name__ == '__main__':main()
