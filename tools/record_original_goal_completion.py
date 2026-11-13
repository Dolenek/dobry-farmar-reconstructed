#!/usr/bin/env python3
"""Record the six original acceptance criteria against the current artifacts and evidence."""
import datetime
import subprocess

from audit_completion_readiness import audit_evidence
from reconcile_goal_status import current_implementation, verified_gate
from record_live_production_verification import ROOT, digest, read, write


def main():
    base = audit_evidence()
    evidence = base['supporting_reports']
    # This registry is rewritten by reconciliation after the audit.
    evidence.pop('analysis/verification.json', None)

    def report(name):
        path = ROOT / 'analysis' / (name + '.json')
        evidence[path.relative_to(ROOT).as_posix()] = digest(path)
        return read(path)

    def check_reference(ref):
        assert digest(ROOT / ref['path']) == ref['sha256'], ref['path']

    verification = read(ROOT / 'analysis/verification.json')
    assert verified_gate(verification, 'current_contract_streak40_gate',
        'passed_real40_award5_full_pixels_and_four_whole_cross_restores_only')
    assert verified_gate(verification, 'current_award_runtime_gate',
        'passed_current21_requested_panels_effects_returns_saved_flags_and_automatic_final_award_only')
    assert verification['current_tutorial_warning_gate']['complete']
    assert verification['current_game_audio_output_gate']['complete']
    assert base['existing_full_regression']['passed'] == 102 and base['existing_full_regression']['failed'] == 0

    source_build = report('production-source-only-build-v4')
    assert source_build['actual_source_process_exits'] == [0, 0]
    assert not source_build['staged_source_manifest']['original_game_binaries_present']
    interop = report('native-application-interop-validation')
    assert len(interop['sessions']) == 5 and all(row['exit_code'] == 0 for row in interop['sessions'])
    assert all(row['binary_save_identical_bytes'] == 1981 for row in interop['comparisons'])
    shortcuts = report('runtime-shortcut-gui-validation')
    assert shortcuts['exact_reset_rgb_pixels'] == 2553600
    assert [row['exit_code'] for row in shortcuts['sessions']] == [0, 0]
    options = report('game-options-runtime-regression')
    current_implementation(options['implementation_files'])
    assert options['status'] == 'passed_actual_options_volume_restart_and_cross_restore'
    assert options['persisted_volumes'] == {'Sound': '50', 'Music': '80'}
    assert options['exact_slider_rgb_pixels'] == 19488 and options['exact_slider_comparison_processes'] == 6
    for run in ('change_and_restart', 'cross_restart'):
        for ref in options[run].values():
            check_reference(ref)
    for row in options['sessions']:
        assert row['exit_code'] == 0
        check_reference(row['settings'])

    coverage = report('current-tutorial-runtime-coverage')
    assert coverage['status'] == 'passed_all_48_compiled_tutorial_steps_in_both_complete_games'
    reachable = set(range(50)) - {0, 10}
    for indices in coverage['actual_observed_indices_by_program'].values():
        assert set(indices) == reachable
    for path, sha in coverage['reports'].items():
        assert digest(ROOT / path) == sha, path
    warnings = report('tutorial-warning-runtime-regression')
    assert warnings['verified_steps'] == [40, 41, 42, 44]
    current_implementation(warnings['implementation_files'])
    for path, sha in warnings['reports'].items():
        assert digest(ROOT / path) == sha, path
    pixels = [row['text_composite_pixel_comparison'] for row in warnings['comparisons']]
    assert all(row['identical'] for row in pixels) and sum(row['pixels'] for row in pixels) == 65520
    fertility = report('tutorial-fertility49-runtime-regression')
    assert fertility['natural_depletion_and_persisted_tip_verified'] and fertility['exact_text_rgb_pixels'] == 28600
    current_implementation(fertility['implementation_files'])
    for ref in fertility['runtime'].values():
        check_reference(ref)
    assert all(row['exit_code'] == 0 for row in fertility['sessions'])

    supply = report('counter-supply-regression')
    assert supply['actual_harvest_actions_per_executable'] == 936
    assert supply['actual_productions_per_executable'] == 1005
    assert supply['exact_grass_milk_money_conservation']
    assert report('factory-catalog-regression')['factory_kinds'] == list(range(11))
    factory_levels = report('factory-levels-regression')
    assert factory_levels['factory_kinds'] == list(range(11)) and factory_levels['paid_upgrades_per_executable'] == 33
    assert report('factory-history-regression')['full_process_bidirectional_transfers'] == 4
    assert report('farm-expansions-regression')['restored_live_field_records'] == 240
    assert report('market-assortment-regression')['all19_market_clock_spellings_equal']
    assert report('field-disasters-regression')['further_bidirectional_restores'] == 4
    assert report('late-timeline-regression')['actual_natural_award'] == 14
    campaign = report('campaign-loop-regression')
    assert (campaign['main_game_end_event'], campaign['clock_wrap_from'], campaign['clock_wrap_to']) == (312, 14400, 11190)
    assert campaign['further_bidirectional_restores'] == 4 and not campaign['normalizations']
    resources = report('original-resource-scope-audit')
    assert resources['effective_xml_count'] == 66 and len(resources['click_mask_references']) == 153
    assert not any(resources[key] for key in ['parse_errors', 'nonempty_mesh_attributes', 'nonempty_animation_attributes'])
    audio = report('current-game-loopback-audio-validation')
    assert [row['kind'] for row in audio['captures']] == ['native', 'source']
    assert audio['process_tree_only'] and not audio['microphone_or_other_application_audio_captured']
    current_implementation(audio['implementation_files'])
    for row in audio['captures']:
        assert row['actual_game_normal_exit'] == 0 and row['nonzero_samples'] > 44100
        for key in ('runtime_report', 'capture_log', 'captured_pcm'):
            check_reference(row[key])

    restore = report('monitored-source-restore-validation')
    assert restore['revised_scene_tree_runtime_validated']
    assert any(row['restored_farm'] and row['observed_fields'] == 60 for row in restore['trials'])
    for trial in restore['trials']:
        for key in ('raw_report', 'watchdog_log', 'window_log', 'input_log'):
            check_reference(trial[key])
        assert trial['game_exit'] == trial['watchdog_exit'] == trial['live_owned_processes_at_end'] == 0

    rich = report('contract-streak-forty-interop-validation')
    current_implementation(rich['implementation_files'])
    assert [row['kind'] for row in rich['sessions']] == ['source', 'native', 'native', 'source']
    assert all(row['exit_code'] == 0 and len(row['live_fields']) == 60 for row in rich['sessions'])
    assert report('contract-streak-forty-regression')['four_cross_restore_bytes'] == [17369] * 4
    dll = report('squall-loader-game-regression')
    assert dll['abi_exports'] == 97 and dll['native_public_return_sites'] == 223
    assert report('squall-loader-regression')['all45_game_sound_pcm_bytes_per_build'] == 122527752

    tracked_programs = subprocess.check_output(['git', 'ls-files', '--', ':(glob)**/*.exe',
        ':(glob)**/*.dll', ':(glob)**/*.gpr', ':(glob)**/*.rep/**'], cwd=ROOT, text=True)
    assert not tracked_programs.strip()
    reference_project = ROOT.parent / 'farm\u00e1\u0159 - Ghidra'
    working_project = ROOT / 'work/ghidra'
    assert reference_project.resolve(strict=True) != working_project.resolve(strict=True)
    project_files = {path.relative_to(reference_project).as_posix(): digest(path)
                     for path in sorted(reference_project.rglob('*')) if path.is_file()}
    assert any(name.endswith('.gpr') for name in project_files) and any('.rep/' in name for name in project_files)
    for name in ('README.md', 'README.en.md', 'README.agent.md', 'docs/SOURCE_BUILD.md',
                 'docs/VERIFICATION_LIMITS.md', 'docs/COMPLETION_AUDIT.md', 'docs/RUNTIME_TESTING.md',
                 'docs/media/menu-logo.png', 'docs/media/in-game.webp',
                 'docs/media/loading.webp', 'docs/media/menu-clean.webp'):
        evidence[name] = digest(ROOT / name)

    criteria = [
        dict(index=1, completed=True, proof='Four independently staged and built Win32 executables and DLLs',
             artifacts=base['artifacts']),
        dict(index=2, completed=True, proof='Actual gameplay with shipped data in both programs',
             compiled_tutorial_steps_per_program=48, settings_sessions=6, market_rows=19, factory_kinds=11,
             running_game_audio_captures=2, verified_original_data_xml_files=66),
        dict(index=3, completed=True, proof='Native comparisons and whole bidirectional save restores',
             current_rich_save_bytes=[17369] * 4, exact_warning_text_rgb_pixels=65520,
             exact_fertility_text_rgb_pixels=28600, limits_document='docs/VERIFICATION_LIMITS.md'),
        dict(index=4, completed=True, proof='All original DLL export contracts and editable implementations',
             exports=base['dll_exports'], squall_abi_exports=97, native_public_return_sites=223,
             exact_jng_images_per_build=416, whole_DLL_behavior_claimed_equal=False),
        dict(index=5, completed=True, proof='Source-built uninstaller with verified isolated removal effects',
             test_processes=base['isolated_uninstaller_processes']),
        dict(index=6, completed=True, proof='Editable source, documented reproducible build and retained originals',
             staged_source_files=base['staged_files_rehashed'], current_production_and_build_files=base['current_source_files_matching_staged_production'],
             original_data_and_program_files_verified=17, original_ghidra_reference_file_digests=project_files,
             tracked_original_programs_or_ghidra_projects=0),
    ]
    result = dict(status='passed_all_six_original_completion_criteria',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        criteria=criteria, supporting_reports=evidence,
        accepted_tracker_exception='analysis/native-tracker-fidelity-decision.json',
        host_runtime_validated_with_independent_monitoring=True,
        scope='Shipped game data, the original public DLL interfaces, editable implementations and documented verification limits')
    write(ROOT / 'analysis/original-goal-completion-audit.json', result)
    print('All six original criteria verified against current evidence.')


if __name__ == '__main__':
    main()
