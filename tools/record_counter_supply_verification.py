#!/usr/bin/env python3
"""Record actual thousand-counter play, unmodified checkpoint continuation and rich saves."""
import datetime
import hashlib
from record_live_production_verification import ROOT, digest, read, write, checked
from record_harvest_hundred_verification import traces_for


def main():
    state = read(ROOT/'.local/counter-supply-state-validation.json')
    checked(state)
    assert state['status'] == 'passed_actual_native_source_thousand_counter_supply_only'
    assert state['auditor_sha256'] == digest(ROOT/'tools/validate_counter_supply_state.py')
    comparison = state['comparison']
    assert comparison['actual_harvest_actions_per_executable'] == 936
    assert comparison['actual_productions_per_executable'] == 1005
    assert comparison['exact_rgb_pixels'] == 1920000
    assert not comparison['native_prefix_exit_code_captured']
    assert not any(s.get('feed_retry_observations') for s in state['sessions'])
    quiescent = read(ROOT/'.local/counter-quiescence-state-validation.json')
    checked(quiescent)
    assert quiescent['status'] == 'passed_actual_price99_quiescence_and_exact_original_sale_replay_only'
    assert quiescent['auditor_sha256'] == digest(ROOT/'tools/validate_counter_quiescence_state.py')
    for name, sha in quiescent['audit_files'].items():
        assert digest(ROOT/name) == sha
    assert [b['binary']['original_replay_removed_sales'] for b in quiescent['comparison']['branches']] == [0, 70]
    interop = read(ROOT/'.local/counter-supply-interop-validation.json')
    work = checked(interop)
    assert interop['status'] == 'passed_thousand_counter_awards_and_four_bidirectional_rich_saves_only'
    assert interop['fixture_report_sha256'] == digest(ROOT/'.local/counter-quiescence-validation.json')
    assert len(interop['sessions']) == 4
    assert all(s['status'] == 'completed' and s['exit_code'] == 0 for s in interop['sessions'])
    transfers = interop['comparison']['transfer_comparisons']
    assert [t['binary']['complete_expected_output_bytes'] for t in transfers] == [23475, 24115, 23475, 23475]
    assert all(t['binary']['whole_binary_unchanged'] and not t['normalizations'] for t in transfers)
    traces = traces_for(work)
    full = read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    for name, sha in full['production_source_files'].items():
        assert digest(ROOT/name) == sha, name
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path']) == row['sha256']
    observation = dict(status='native_bounded_feed_observation_retained_and_real_checkpoint_completed',
        retained_native_prefix=state['retained_native_prefix'],
        actual_pending_income_evidence=comparison['short_feed_interval_explanation'],
        native_prefix_process_exit_code_captured=False,
        successful_native_and_source_process_exit_codes=[s['exit_code'] for s in state['sessions']],
        finite_eight_second_feed_retry_branch_exercised=False,
        successful_paths_needed_retry=False, checkpoint_profile_fields_modified=False,
        source_started_from_original_common_baseline=True,
        source_primary_process_debugger_trace_captured=False,
        price_click_observation='Both original and source use14 accelerated clicks, increments1..14 clamped99;97 is total price increase, not click count',
        whole_game_complete=False)
    reports = {}
    for name, report in [('counter-supply-state-validation.json', state),
                         ('counter-quiescence-state-validation.json', quiescent),
                         ('counter-supply-interop-validation.json', interop),
                         ('counter-supply-initial-observations.json', observation)]:
        write(ROOT/'analysis'/name, report)
        reports[name] = digest(ROOT/'analysis'/name)
    result = dict(status='passed_actual_thousand_harvest_production_income_awards_and_four_rich_saves_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__), reports=reports,
        original_award_predicates='004c8650:awQCollect>99/>999;awQProcess>99/>999;awEarn',
        original_sale_replay='0040c337 successor-skipping erase;restore replays actual records before restoring saved cutoff',
        actual_harvest_actions_per_executable=936, actual_productions_per_executable=1005,
        final_counters=[1036, 1011], complete_award_panel_exact_rgb_pixels=1920000,
        actual_new_award_indices=[1, 2, 3, 6], paid_cowshed_level3_cost=1300,
        exact_grass_milk_money_conservation=True,
        independent_final_outcomes_preserved=dict(native_money=3180, source_money=3200, native_market_milk=10, source_market_milk=0),
        native_resumed_real_unmodified_checkpoint=True, native_prefix_exit_code_captured=False,
        source_completed_full_continuous_original_baseline_workload=True,
        quiescent_source_save_original_replay_removed_sales=70,
        actual_bidirectional_transfers=4, whole_identical_save_bytes=[23475, 24115, 23475, 23475],
        source_restore_debugger_traces=traces, normalizations=[],
        production_implementation_changed=False, original_files_unchanged=17,
        current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json', new_full_ctest_run=False,
        whole_game_complete=False,
        pending=['Natural20/40 contract streaks,high-price sale,planning queue and final all-awards condition',
                 'Remaining native ABI semantics for79 Squall exports and broader behavior',
                 'Remaining JNG/uninstaller and whole original goal scopes'])
    name = 'counter-supply-regression.json'
    write(ROOT/'analysis'/name, result)
    verification = read(ROOT/'analysis/verification.json')
    for entry in list(reports)+[name]:
        if 'analysis/'+entry not in verification['reports']:
            verification['reports'].append('analysis/'+entry)
    for entry in ['counter-supply-state-validation.json', 'counter-quiescence-state-validation.json', 'counter-supply-interop-validation.json']:
        if 'analysis/'+entry not in verification['original_game_runtime']['scope_reports']:
            verification['original_game_runtime']['scope_reports'].append('analysis/'+entry)
    verification['latest_scoped_regression'] = dict(report='analysis/'+name,
        recorded_at_utc=result['recorded_at_utc'], whole_game_complete=False)
    paths = [ROOT/'CMakeLists.txt']+[p for folder in ['src', 'tools', 'tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows = [p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths, key=lambda p: p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/'+name
    write(ROOT/'analysis/verification.json', verification)
    print('Recorded1872 actual harvests,2010 actual productions,four exact award panels and four rich whole-save restores.')


if __name__ == '__main__':
    main()
