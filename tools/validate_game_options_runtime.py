#!/usr/bin/env python3
"""Compare actual volume controls and settings persistence in both applications."""
import argparse
import time
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

from desktop_runtime_hold import require_desktop_runtime_available
from record_live_production_verification import ROOT, digest, read, write
from tutorial_warning_session import WarningSuite


def volume_settings(path):
    root = ET.parse(path).getroot()
    return {name: int(root.find('Sound').get(name)) for name in ('Sound', 'Music')}


def settled_regions(active, label, rectangles, prior=None):
    deadline = time.monotonic() + 15
    previous = None
    while time.monotonic() < deadline:
        active.probe.check_process()
        image = active.capture(label)
        pixels = tuple(active.probe.roi(image, box) for box in rectangles)
        if pixels == previous and (prior is None or all(a != b for a, b in zip(pixels, prior))):
            return pixels
        previous = pixels
        time.sleep(.25)
    raise AssertionError(('Options regions did not settle', label))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cross-restart-from', type=Path, help='Completed monitored volume change report')
    args = parser.parse_args()
    context = require_desktop_runtime_available()
    assert context, 'Use the independent host test watchdog'
    work = context['directory']
    suite = WarningSuite(work / 'options')
    suite.report['watchdog_lease'] = context['lease']
    driver = ROOT / 'tools/validate_game_options_runtime.py'
    snapshot = suite.work / driver.name
    snapshot.write_bytes(driver.read_bytes())
    key = driver.relative_to(ROOT).as_posix()
    suite.report['source_files'][key] = digest(driver)
    suite.report['source_file_snapshots'][key] = snapshot.relative_to(ROOT).as_posix()
    checkpoints = read(ROOT / 'analysis/contract-streak-forty-checkpoints.json')
    seed = next(row for row in checkpoints['profile_sets'] if row['kind'] == 'native')
    profiles = ROOT / seed['profile_directory']
    assert {path.name: digest(path) for path in profiles.iterdir() if path.is_file()} == seed['files']
    options = suite.probe.resource('UI/Options.xml')
    rectangles = []
    targets = {}
    expected = {'Sound': 50, 'Music': 80}
    for name, value in expected.items():
        slider = options.find(name + 'Volume')
        x, y = map(int, slider.get('position').split())
        width, _ = map(int, slider.get('size').split())
        thumb_width, thumb_height = map(int, slider.find('Slider').get('size').split())
        left = x - width // 2
        assert slider.get('Min') == '0' and slider.get('Max') == '100'
        targets[name] = (left + width * value // 100, y)
        rectangles.append((left - thumb_width // 2 - 2, y - thumb_height // 2 - 2,
                           left + width + thumb_width // 2 + 2, y + thumb_height // 2 + 2))
    suite.report['slider_rectangles'] = rectangles
    suite.report['physical_targets'] = targets
    suite.report['profile_seed'] = seed['files']
    changed_pixels = {}
    menu_button_box = (354, 400, 445, 429)
    ok = False
    try:
        prior_changes = {}
        if args.cross_restart_from:
            previous = args.cross_restart_from.resolve(strict=True)
            prior = read(previous)
            assert prior['status'] == 'passed_actual_native_source_options_volume_changes_and_restart'
            assert prior['implementation_files'] == suite.report['implementation_files']
            assert prior['slider_rectangles'] == [list(box) for box in rectangles]
            suite.report['cross_restore_origin'] = dict(path=previous.relative_to(ROOT).as_posix(),
                                                       sha256=digest(previous))
            for index, session in enumerate(prior['sessions']):
                if session['case'] != 'change':
                    continue
                assert session['exit_code'] == 0
                kind = session['kind']
                persisted = session['persisted_settings']
                prior_changes[kind] = ROOT / persisted['path']
                assert digest(prior_changes[kind]) == persisted['sha256']
                assert volume_settings(prior_changes[kind]) == expected
                name = f'{index}-options-volumes-changed.png'
                image = ROOT / prior['work_directory'] / name
                assert digest(image) == prior['screenshots'][name]
                with Image.open(image) as frame:
                    changed_pixels[kind] = tuple(suite.probe.roi(frame, box) for box in rectangles)
        for kind in ('native', 'source'):
            opposite = 'source' if kind == 'native' else 'native'
            settings_seed = prior_changes[opposite] if prior_changes else None
            phases = ('cross-restart',) if prior_changes else ('change', 'restart')
            for phase in phases:
                with suite.open(kind, phase, profiles, enter_farm=False, settings_seed=settings_seed) as active:
                    settings_path = ROOT / active.session['common_directory'] / 'sys/settings.xml'
                    initial = volume_settings(settings_path)
                    assert initial == ({'Sound': 100, 'Music': 42} if phase == 'change' else expected)
                    active.session['input_volumes'] = initial
                    active.session['input_settings_sha256'] = digest(settings_path)
                    if phase == 'cross-restart':
                        assert active.session['input_settings_sha256'] == digest(prior_changes[opposite])
                        active.session['settings_written_by'] = opposite
                    active.probe.move(active.hwnd, 780, 580)
                    menu_pixels = settled_regions(active, 'menu-before-options', rectangles + [menu_button_box])
                    active.click(*active.probe.button('UI/MainMenu.xml', 'Options'))
                    opened = settled_regions(active, 'options-opened', rectangles, menu_pixels[:2])
                    if phase == 'change':
                        for point in targets.values():
                            active.click(*point)
                        active.probe.move(active.hwnd, 780, 580)
                        changed_pixels[kind] = settled_regions(active, 'options-volumes-changed', rectangles, opened)
                    else:
                        assert opened == changed_pixels[kind], 'Restarted slider positions differ from the changed positions'
                        active.session['restarted_slider_regions_exact'] = True
                    active.click(*active.probe.button('UI/Options.xml', 'OkButton'))
                    active.probe.move(active.hwnd, 780, 580)
                    active.probe.wait_match(f'{active.index}-options-return-to-menu.png', menu_button_box,
                                            menu_pixels[-1], timeout=15)
                    active.session['actions'].append('Options OK restored the main menu button pixels')
                persisted = ROOT / active.session['persisted_profile_directory'] / '../sys/settings.xml'
                persisted = persisted.resolve(strict=True)
                assert volume_settings(persisted) == expected
                active.session['persisted_volumes'] = expected.copy()
                active.session['persisted_settings'] = dict(path=persisted.relative_to(ROOT).as_posix(),
                                                             sha256=digest(persisted))
                if phase == 'change':
                    settings_seed = persisted
                print(kind, phase, 'saved settings match', flush=True)
        assert changed_pixels['native'] == changed_pixels['source']
        suite.report['exact_changed_and_restarted_slider_rgb_pixels'] = sum(
            (right - left) * (bottom - top) for left, top, right, bottom in rectangles)
        suite.report['status'] = ('passed_actual_bidirectional_options_settings_restore' if prior_changes else
                                  'passed_actual_native_source_options_volume_changes_and_restart')
        ok = True
    except Exception as error:
        import traceback
        suite.report.update(status='failed', error=repr(error), traceback=traceback.format_exc())
        print(suite.report['traceback'], flush=True)
    finally:
        suite.unchanged()
        suite.report['reference_files_unchanged'] = 17
        suite.report['screenshots'] = {path.name: digest(path) for path in suite.work.glob('*.png')}
        write(suite.work / 'report.json', suite.report)
    return 0 if ok else 1


if __name__ == '__main__':
    raise SystemExit(main())
