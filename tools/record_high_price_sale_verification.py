#!/usr/bin/env python3
"""Record natural price80 award, exact save encoding and real original rendering variants."""
import datetime
import hashlib
from record_live_production_verification import ROOT, digest, read, write, checked
from record_harvest_hundred_verification import traces_for


def main():
    state = read(ROOT/'.local/high-price-sale-state-validation.json')
    checked(state)
    assert state['status'] == 'passed_actual_price80_sales_natural_award15_and_complete_encoded_saves_only'
    assert state['auditor_sha256'] == digest(ROOT/'tools/validate_high_price_sale_state.py')
    comparison = state['comparison']
    rendering = comparison['actual_original_rendering_variants']
    assert rendering['identical_nonshader_pointer_sprite_words'] == 504
    assert comparison['actual_award_panel_pixels']['different_pixels'] == 1
    assert not comparison['actual_award_panel_pixels']['exact']
    assert len(rendering['whole_frame_original_source_matches']) == 2
    for match in rendering['whole_frame_original_source_matches']:
        assert match['entire_identical_rgb_pixels'] == 480000 and not match['normalizations']
        for key in ['source','original']:
            assert digest(ROOT/match[key+'_frame']) == match[key+'_sha256']
    diagnostic_path = ROOT/rendering['diagnostic_report']
    assert digest(diagnostic_path) == rendering['diagnostic_report_sha256']
    diagnostic = read(diagnostic_path)
    checked(diagnostic)
    interop = read(ROOT/'.local/high-price-sale-interop-validation.json')
    work = checked(interop)
    assert interop['status'] == 'passed_natural_price80_award_and_four_bidirectional_complete23694_saves_only'
    assert interop['fixture_report_sha256'] == digest(ROOT/'.local/high-price-sale-state-validation.json')
    assert len(interop['sessions']) == 4 and all(s['status'] == 'completed' and s['exit_code'] == 0 for s in interop['sessions'])
    assert all(t['binary']['whole_binary_unchanged'] and t['binary']['complete_expected_output_bytes'] == 23694 for t in interop['comparison']['transfer_comparisons'])
    traces = traces_for(work)
    first_path = ROOT/'.local/high-price-sale-interop/20261105T211309333986Z/report.json'
    first = read(first_path)
    assert first['status'] == 'failed' and 'MSellTime10' in first['error']
    assert len(first['sessions']) == 4 and all(s['exit_code'] == 0 for s in first['sessions'])
    first_work = ROOT/first['work_directory']
    assert digest(first_work/'driver.py') == first['source_files']['tools/validate_game_high_price_sale_interop.py']
    parser = interop['original_float_parser_evidence']
    assert digest(ROOT/parser['path']) == parser['sha256'] and parser['actual_original_parsed_float'] == 1
    full = read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name) == sha for name,sha in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path']) == row['sha256']
    observation = dict(status='initial_one_pixel_assertion_resolved_by_two_actual_original_rendering_variants',
        initial_paired_full_frame_comparison=comparison['actual_award_panel_pixels'],
        actual_original_to_original_difference=rendering['original_to_original_raw_difference'],
        original_pixel_values=rendering['observed_original_pixel_values'],
        original_source_whole_frame_matches=rendering['whole_frame_original_source_matches'],
        live_scene_comparison=rendering, images_modified=False, normalizations=[],
        production_changed=False, primary_source_debugger_trace_captured=False,
        primary_native_and_source_normal_exit_codes=[s['exit_code'] for s in state['sessions']],
        diagnostic_native_and_source_normal_exit_codes=[s['exit_code'] for s in diagnostic['sessions']],
        initial_interop_diagnostic=dict(report=first_path.relative_to(ROOT).as_posix(),sha256=digest(first_path),
            actual_completed_processes=4,source_debugger_traces=traces_for(first_work),
            rejected_assumption='Serialized MSellTime10 infinity must remain infinity throughout the first1s of a reload',
            actual_original_parser_evidence=parser,
            actual_source_cache_after_first_restore='0.00126662',
            resolution='Original old-CRT parser reads1.#INF as1;finite1s cache then expires and recalculates infinity at price99 without any sale',
            production_changed=False),
        whole_game_complete=False)
    reports = {}
    for name,value in [('high-price-sale-state-validation.json',state),
                       ('high-price-sale-rendering-observation.json',observation),
                       ('high-price-sale-interop-validation.json',interop)]:
        write(ROOT/'analysis'/name,value)
        reports[name] = digest(ROOT/'analysis'/name)
    result = dict(status='passed_natural_price80_sale_award15_complete_save_encoding_and_four_cross_restores_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        audit_files={name:digest(ROOT/name) for name in ['tools/audit_award_pixel_order.py','tools/counter_save_audit.py','tools/validate_high_price_sale_state.py']},
        original_high_sale_predicate='Market::absoluteStep signed sold>79;GameAwardManager004c8650 reads awHighSold',
        actual_sale_price=80, actual_primary_sales_per_executable=2,
        actual_primary_money_before=3180,actual_primary_money_after=3340,
        actual_primary_market_milk_before=10,actual_primary_market_milk_after=8,
        original_price_spike_game_time=13380, original_award_index=15,
        complete_expected_save_bytes=23694, original_journal_note_added=1, unchanged_coefficient_words=383,
        paired_raw_pixels_different=1, paired_raw_difference_normalized=False,
        original_native_runs_also_differ_by_same_pixel=True,
        entire_source_frames_matched_to_actual_original_frames=2, exact_rgb_pixels_in_those_matches=960000,
        captured_nonshader_pointer_sprite_words_equal=504,
        original_equal_depth_pointer_order_preserved=True,
        actual_bidirectional_restores=4, entire_identical_save_transfer_bytes=[23694]*4,
        source_restore_debugger_traces=traces, independent_sale_timestamps_preserved=True,
        original_profile_fields_modified_by_probe=False, normalizations=[],production_changed=False,
        current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',
        new_full_ctest_run=False,original_files_unchanged=17,whole_game_complete=False,
        pending=['Natural20/40 contract streaks and final all-awards condition',
                 'Original paused warehouse click cause/source comparison',
                 'Remaining Squall ABI/behavior,JNG,uninstaller and whole original goal scopes'])
    name = 'high-price-sale-regression.json'
    write(ROOT/'analysis'/name,result)
    verification = read(ROOT/'analysis/verification.json')
    for entry in list(reports)+[name]:
        if 'analysis/'+entry not in verification['reports']:
            verification['reports'].append('analysis/'+entry)
    for entry in ['high-price-sale-state-validation.json','high-price-sale-interop-validation.json']:
        if 'analysis/'+entry not in verification['original_game_runtime']['scope_reports']:
            verification['original_game_runtime']['scope_reports'].append('analysis/'+entry)
    verification['latest_scoped_regression'] = dict(report='analysis/'+name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths = [ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows = [p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded actual price80 award15,two reference-observed full-frame variants,and four complete23694-byte restores.')


if __name__ == '__main__':
    main()
