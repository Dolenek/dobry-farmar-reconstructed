#!/usr/bin/env python3
"""Exercise native and reconstructed tutorial49 from the same genuine farm save."""
import time
import xml.etree.ElementTree as ET

from desktop_runtime_hold import require_desktop_runtime_available
from record_live_production_verification import ROOT, digest, read, write
from tutorial_warning_session import WarningSuite


def properties(path):
    return {entry.get('Name'): entry.get('Val') for entry in ET.parse(path).getroot()}


def main():
    context = require_desktop_runtime_available()
    assert context, 'Use the independent host test watchdog'
    work = context['directory']
    suite = WarningSuite(work / 'fertility')
    suite.report['watchdog_lease'] = context['lease']
    driver = ROOT / 'tools/validate_game_fertility_tip.py'
    snapshot = suite.work / driver.name
    snapshot.write_bytes(driver.read_bytes())
    key = driver.relative_to(ROOT).as_posix()
    suite.report['source_files'][key] = digest(driver)
    suite.report['source_file_snapshots'][key] = snapshot.relative_to(ROOT).as_posix()

    seed_report = ROOT / 'analysis/remaining-crops-growth-state-validation.json'
    seed = read(seed_report)
    assert seed['sessions'][1]['kind'] == 'source' and seed['sessions'][1]['exit_code'] == 0
    profiles = ROOT / seed['work_directory'] / '1-persisted/Profiles'
    starting = properties(profiles / '00000001.xml')
    fertility = [float(starting[f'FFF{n}']) for n in range(60)]
    assert starting['tip49'] == '0' and fertility.index(min(fertility)) == 34
    assert 0 < fertility[34] < .2 and starting['FFS34'] == '0'
    suite.report['fixture'] = dict(report=seed_report.relative_to(ROOT).as_posix(), sha256=digest(seed_report),
        profiles=profiles.relative_to(ROOT).as_posix(), files={p.name: digest(p) for p in profiles.iterdir()},
        field34_initial_fertility=fertility[34], initial_game_time=float(starting['GameTime']))
    samples = {}
    ok = False
    try:
        for kind in ('native', 'source'):
            with suite.open(kind, 'fertility49', profiles) as active:
                assert active.session['input_profile_files'] == suite.report['fixture']['files']
                initial = active.state()
                assert not active.observer.property('booleans', 'tip49')
                active.session['initial_state'] = initial
                deadline = time.monotonic() + 40
                while True:
                    active.probe.check_process()
                    step = active.observer.step()
                    assert step in (-1, 49), ('Unexpected tutorial', step)
                    if step == 49:
                        controls = active.observer.snapshot('Arrow')
                        if all(row['alpha'] >= .999 and row['state'][2] == 0 and
                               all(row['sprite'][slot] >> 24 == 255 for slot in (2, 5, 8, 11))
                               for row in controls.values()):
                            break
                    assert time.monotonic() < deadline, 'Natural fertility tip did not settle'
                    time.sleep(.05)
                zeros = [n for n in range(60) if active.observer.property('floats', f'FFF{n}') == 0]
                assert zeros == [34] and active.observer.property('uints', 'FFS34') == 0
                state = active.state()
                assert state['game_time'] > suite.report['fixture']['initial_game_time']
                x, y, _, _ = controls['text']['geometry']
                rect = (int(x) - 130, int(y) - 55, int(x) + 130, int(y) + 55)
                active.probe.move(active.hwnd, 780, 580)
                frame = active.capture('fertility-tip49')
                pixels = active.probe.roi(frame, rect)
                assert len(set(pixels)) > 8
                samples[kind] = (rect, pixels)
                active.session['stages'] = [dict(step=49, screenshot=f'{active.index}-fertility-tip49.png',
                    depleted_fields=zeros, rect=rect, controls=controls, state=state)]
                active.click(*controls['ok']['geometry'][:2])
                active.wait('fertility-tip49-dismissed', lambda: active.observer.step() == -1 and
                            active.observer.property('booleans', 'tip49'), timeout=15)
                final = active.state()
                for field in ['stock', 'market', 'carried', 'held', 'collected']:
                    assert final[field] == initial[field], field
                active.session['final_state'] = final
            saved = ROOT / active.session['persisted_profile_directory'] / '00000001.xml'
            stored = properties(saved)
            assert stored['tip49'] == '1' and float(stored['FFF34']) == 0
            active.session['persisted_tip49'] = True
            active.session['profile_xml_sha256'] = digest(saved)
            print(kind, 'natural field34 tutorial49 displayed, dismissed and saved', flush=True)
        assert samples['native'] == samples['source']
        native, source = [row['stages'][0]['controls'] for row in suite.report['sessions']]
        for name in native:
            for key in ['geometry', 'sprite', 'flags']:
                assert native[name][key] == source[name][key], (name, key)
        suite.report['exact_text_rgb_pixels'] = len(samples['source'][1]) // 3
        suite.report['status'] = 'passed_actual_native_source_natural_field_fertility_warning49'
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
