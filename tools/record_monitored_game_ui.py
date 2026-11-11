#!/usr/bin/env python3
"""Verify the saved settings and fertility tutorial evidence from owned processes."""
import argparse
import datetime
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

from current_production_regression import current_regression
from reconcile_goal_status import current_implementation
from record_live_production_verification import ROOT, digest, read, write
from record_monitored_restore import json_lines, reference


def checked_run(path, status):
    path = path.resolve(strict=True)
    raw = read(path)
    assert raw['status'] == status and raw['reference_files_unchanged'] == 17
    current_implementation(raw['implementation_files'])
    assert not raw['original_profile_fields_or_memory_written']
    for name, sha in raw['source_files'].items():
        assert digest(ROOT / raw['source_file_snapshots'].get(name, name)) == sha, name
    for name, sha in raw['screenshots'].items():
        assert digest(path.parent / name) == sha, name
    assert all(row['status'] == 'completed' and row['exit_code'] == 0 for row in raw['sessions'])
    watchdog_path = path.parent.parent / 'watchdog.jsonl'
    started, *_, finished = json_lines(watchdog_path)
    assert started['driver_pid'] == raw['watchdog_lease']['driver_pid']
    assert finished['event'] == 'finished' and finished['driver_exit_code'] == finished['watchdog_exit_code'] == 0
    assert finished['live_owned_processes_before_final_close'] == 0
    return raw, dict(raw_report=reference(path), watchdog=reference(watchdog_path))


def settings_properties(ref):
    path = ROOT / ref['path']
    assert digest(path) == ref['sha256']
    return {entry.tag: dict(entry.attrib) for entry in ET.parse(path).getroot()}


def cropped(raw, filename, rectangles):
    with Image.open(ROOT / raw['work_directory'] / filename) as frame:
        frame = frame.convert('RGB')
        return tuple(frame.crop(box).tobytes() for box in rectangles)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--options', required=True, type=Path)
    parser.add_argument('--cross-options', required=True, type=Path)
    parser.add_argument('--fertility', required=True, type=Path)
    args = parser.parse_args()
    options, option_refs = checked_run(args.options, 'passed_actual_native_source_options_volume_changes_and_restart')
    cross, cross_refs = checked_run(args.cross_options, 'passed_actual_bidirectional_options_settings_restore')
    assert cross['cross_restore_origin'] == option_refs['raw_report']
    assert [(row['kind'], row['case']) for row in options['sessions']] == [
        ('native', 'change'), ('native', 'restart'), ('source', 'change'), ('source', 'restart')]
    assert [row['kind'] for row in cross['sessions']] == ['native', 'source']
    expected = {'Sound': '50', 'Music': '80'}
    saved = {}
    changed = {}
    seen_pixels = []
    cases = []
    for raw in (options, cross):
        assert raw['slider_rectangles'] == options['slider_rectangles']
        for index, session in enumerate(raw['sessions']):
            kind, case = session['kind'], session['case']
            persisted = session['persisted_settings']
            values = settings_properties(persisted)
            assert values['Sound'] == expected
            if case == 'change':
                changed[kind] = persisted
                saved[kind] = values
                shot = f'{index}-options-volumes-changed.png'
            else:
                writer = kind if case == 'restart' else session['settings_written_by']
                assert session['input_settings_sha256'] == changed[writer]['sha256']
                assert session['restarted_slider_regions_exact']
                shot = f'{index}-options-opened.png'
            seen_pixels.append(cropped(raw, shot, raw['slider_rectangles']))
            cases.append(dict(kind=kind, case=case, exit_code=0, settings=persisted))
    assert all(sample == seen_pixels[0] for sample in seen_pixels)
    options_pixels = sum(map(len, seen_pixels[0])) // 3
    assert options_pixels == 19488
    settings_differences = {}
    for section in saved['native'].keys() | saved['source'].keys():
        native = saved['native'].get(section, {})
        source = saved['source'].get(section, {})
        for key in native.keys() | source.keys():
            if native.get(key) != source.get(key):
                settings_differences[f'{section}.{key}'] = [native.get(key), source.get(key)]
    assert settings_differences == {'graphics.back_color': ['0x0', '0x00000000'],
        'graphics.show_cursor_pos': ['0', ''], 'lightConfig.count': ['0', None]}

    fertility, fertility_refs = checked_run(args.fertility, 'passed_actual_native_source_natural_field_fertility_warning49')
    seed = fertility['fixture']
    assert 0 < seed['field34_initial_fertility'] < .2
    assert digest(ROOT / seed['report']) == seed['sha256']
    assert all(digest(ROOT / seed['profiles'] / name) == sha for name, sha in seed['files'].items())
    assert [row['kind'] for row in fertility['sessions']] == ['native', 'source']
    stages = []
    tips = []
    for session in fertility['sessions']:
        assert session['input_profile_files'] == seed['files']
        stage, = session['stages']
        assert stage['step'] == 49 and stage['depleted_fields'] == [34]
        assert stage['state']['tutorial'] == 49 and session['final_state']['tutorial'] == -1
        saved_xml = ROOT / session['persisted_profile_directory'] / '00000001.xml'
        assert digest(saved_xml) == session['profile_xml_sha256']
        props = {entry.get('Name'): entry.get('Val') for entry in ET.parse(saved_xml).getroot()}
        assert props['tip49'] == '1' and float(props['FFF34']) == 0
        stages.append(stage)
        tips.append(dict(kind=session['kind'], exit_code=0, stages=[dict(step=49, screenshot=stage['screenshot'])],
                         persisted_profile=reference(saved_xml)))
    assert stages[0]['rect'] == stages[1]['rect']
    rect = stages[0]['rect']
    left, right = [cropped(fertility, stage['screenshot'], [rect])[0] for stage in stages]
    assert left == right and len(left) // 3 == 28600
    assert stages[0]['controls'] == stages[1]['controls']

    current, _ = current_regression()
    recorded = dict(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__), implementation_files=options['implementation_files'],
        reference_files_unchanged=17, production_changed=False, current_full_ctest_report=current,
        whole_game_complete=False)
    paths = ['analysis/game-options-runtime-regression.json', 'analysis/tutorial-fertility49-runtime-regression.json']
    write(ROOT / paths[0], dict(recorded, status='passed_actual_options_volume_restart_and_cross_restore',
        change_and_restart=option_refs, cross_restart=cross_refs, sessions=cases,
        persisted_volumes=expected, exact_slider_rgb_pixels=options_pixels, exact_slider_comparison_processes=6,
        differing_unrelated_xml_properties_native_then_source=settings_differences,
        entire_settings_xml_byte_equality_claimed=False))
    write(ROOT / paths[1], dict(recorded, status='passed_actual_natural_field34_tip49_in_both_programs',
        work_directory=fertility['work_directory'], runtime=fertility_refs, fixture=seed, sessions=tips,
        exact_text_rgb_pixels=len(left) // 3, exact_controls=True, natural_depletion_and_persisted_tip_verified=True))
    verification_path = ROOT / 'analysis/verification.json'
    verification = read(verification_path)
    for path in paths:
        if path not in verification['reports']:
            verification['reports'].append(path)
    write(verification_path, verification)
    print('Verified six options sessions and two natural fertility tutorial sessions.')


if __name__ == '__main__':
    main()
