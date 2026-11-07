#!/usr/bin/env python3
"""Bind actual21-panel lifecycle evidence to current source binaries and saved flags."""
import argparse
import datetime
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image
from current_production_regression import current_regression
from record_live_production_verification import ROOT, digest, read, write


def properties(path):
    return {n.get('Name'): (n.get('Type'), n.get('Val')) for n in ET.parse(path).getroot()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report', type=Path, required=True)
    args = parser.parse_args()
    path = args.runtime_report.resolve(strict=True)
    raw = read(path)
    assert raw['status'] == 'passed_for_game_input_requested_awards_full_process_panel_return'
    assert raw['award_indices'] == list(range(21))
    assert raw['forced_game_input_awards'] == 20 and raw['automatic_collect_all_award']
    assert raw['reference_files_unchanged'] == 17
    work = ROOT / raw['work_directory']
    for name, sha in raw['source_files'].items():
        actual = ROOT / raw['source_file_snapshots'][name] if name in raw.get('source_file_snapshots', {}) else ROOT / name
        assert digest(actual) == sha, name
    for name, sha in raw['screenshots'].items():
        assert digest(work / name) == sha
    for name, sha in raw['implementation_files'].items():
        assert digest(work / 'runtime' / name) == sha
        current = 'Dobr\u00fd farm\u00e1\u0159.exe' if name == 'FarmarSourceInterop.exe' else name
        assert digest(ROOT / 'build/windows/Release' / current) == sha
    prefix = raw.get('native_prefix')
    prefix_work = None
    prior = None
    if prefix:
        assert prefix['normal_exit'] and digest(ROOT / prefix['report']) == prefix['sha256']
        prior = read(ROOT / prefix['report'])
        prefix_work = ROOT / prior['work_directory']
    panels = []
    region = tuple(raw['panel_exact_roi'])
    assert region == (250, 370, 550, 465)
    for index in range(21):
        if prefix and index in prefix['completed_indices']:
            native = prefix_work / f'0-award{index:02d}-panel.png'
            assert digest(native) == prior['screenshots'][native.name]
            native_states = prior['sessions'][0]['fully_appeared_panels']
        else:
            native = work / f'0-award{index:02d}-panel.png'
            native_states = raw['sessions'][0]['fully_appeared_panels']
        source = work / f'1-award{index:02d}-panel.png'
        for states in [native_states, raw['sessions'][1]['fully_appeared_panels']]:
            state = next(row for row in states if row['index'] == index)
            assert state['alpha'] >= .999 and state['transition'][2] == 0 and state['visible_text_lines'] > 0
        a, b = [Image.open(p).convert('RGB') for p in [native, source]]
        assert a.size == b.size == (800, 600)
        assert a.crop(region).tobytes() == b.crop(region).tobytes(), index
        panels.append(dict(index=index, region=list(region), different_rgb_pixels=0,
            compared_rgb_pixels=28500, files={p.relative_to(ROOT).as_posix(): digest(p) for p in [native, source]}))
    profiles = [work / f'{i}-persisted/Profiles' for i in range(2)]
    saved = [properties(p / '00000001.xml') for p in profiles]
    assert set(saved[0]) == set(saved[1])
    keys = sorted(k for k in saved[0] if k.startswith('Award'))
    assert len(keys) == 21
    for values in saved:
        for index, key in enumerate(keys):
            assert values[key] == values[f'awardShow{index}'] == ('Bool', '1')
    changes = {key: [saved[0][key], saved[1][key]] for key in saved[0] if saved[0][key] != saved[1][key]}
    assert set(changes) <= {'GameTime', 'RunTime'}
    binaries = [(p / '00000001.save').read_bytes() for p in profiles]
    assert binaries[0] == binaries[1]
    for index, session in enumerate(raw['sessions']):
        assert session['kind'] == ['native', 'source'][index]
        assert session['status'] == 'passed' and session['exit_code'] == 0
        assert session['actions'][-1].startswith('automatic collect-all predicate')
        assert not any(action.startswith('a21') for action in session['actions'])
    import json
    assert json.loads(raw['sessions'][0]['launcher_exit']) == dict(event='exit', exit_code=0, watchdog_triggered=False)
    full_name, _ = current_regression()
    for row in read(ROOT / 'analysis/inventory.json')['files']:
        assert digest(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159' / row['path']) == row['sha256']
    name = 'current-awards-runtime-regression.json'
    result = dict(status='passed_current21_requested_panels_effects_returns_saved_flags_and_automatic_final_award_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        runtime_report=dict(path=path.relative_to(ROOT).as_posix(), sha256=digest(path)),
        panels=panels, exact_panel_roi_rgb_pixels=sum(p['compared_rgb_pixels'] for p in panels),
        diagnostic_requested_conditions=20, automatic_collect_all_condition=True,
        all21_naturally_earned_in_one_profile_claimed=False, saved_owned_and_shown_flags=42,
        complete_identical_binary_bytes=len(binaries[0]), profile_properties=len(saved[0]),
        changed_profile_properties=changes, original_files_unchanged=17, production_changed=False,
        current_full_ctest_source_hashes_match=True, current_full_ctest_report=full_name,
        new_full_ctest_run=False, whole_game_complete=False)
    write(ROOT / 'analysis' / name, result)
    verification = read(ROOT / 'analysis/verification.json')
    if 'analysis/' + name not in verification['reports']:
        verification['reports'].append('analysis/' + name)
    verification['current_award_runtime_gate'] = dict(report='analysis/' + name, complete=True,
        diagnostic_requested_conditions=20, automatic_final_condition=True)
    verification['latest_scoped_regression'] = dict(report='analysis/' + name,
        recorded_at_utc=result['recorded_at_utc'], whole_game_complete=False)
    write(ROOT / 'analysis/verification.json', verification)
    print(json.dumps({k: v for k, v in result.items() if k != 'panels'}))


if __name__ == '__main__':
    main()
