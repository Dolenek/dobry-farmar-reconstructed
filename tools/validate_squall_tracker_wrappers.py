#!/usr/bin/env python3
"""Compare actual tracker wrappers with controlled zero/ended/empty-mix states."""
import argparse
import datetime
import json
from pathlib import Path
import shutil
import subprocess

from validate_squall_tracker_stages import ROOT, digest, symbol


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--candidate', type=Path, default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output', type=Path, default=ROOT/'.local/squall-tracker-wrappers-validation.json')
    args = parser.parse_args()
    work = ROOT/'.local/squall-tracker-wrappers'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    binaries = {'native.dll': ROOT.parent/'farmář/Dobrý farmář/Squall.dll',
                'source.dll': args.candidate.resolve(), 'source.map': args.candidate.resolve().with_suffix('.map'),
                'probe.exe': ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-wrappers-probe.exe'}
    for name, path in binaries.items():
        shutil.copyfile(path, work/name)
    sources = ['tools/validate_squall_tracker_wrappers.py', 'tools/validate_squall_tracker_stages.py',
               'tools/squall_tracker_wrappers_probe.cpp', 'tools/squall_tracker_probe/CMakeLists.txt']
    sources += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.cpp', '.h']]
    for name in sources:
        target = work/'snapshots'/name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT/name, target)
    report = dict(status='actual_wrapper_controlled_state_comparison_not_full_tracker_equivalence',
                  work_directory=work.relative_to(ROOT).as_posix(),
                  binaries={name:digest(work/name) for name in binaries},
                  source_files={name:digest(work/'snapshots'/name) for name in sources}, runs=[])
    source_rvas = [symbol(work/'source.map', f'?Get{mode}Samples@CDecompressTrack@@') for mode in ['Mono', 'Stereo']]
    exact_bytes = pairs = 0
    for precision in [53, 24]:
        results = []
        for backend, rvas in [('native', ['2bfe0', '2c000']), ('source', source_rvas)]:
            folder = work/f'{backend}-pc{precision}'
            folder.mkdir()
            run = subprocess.run([str(work/'probe.exe'), str(work/f'{backend}.dll'), *rvas, backend, str(precision)],
                                 cwd=folder, capture_output=True, timeout=45)
            (folder/'stdout.json').write_bytes(run.stdout)
            (folder/'stderr.log').write_bytes(run.stderr)
            row = dict(backend=backend, precision=precision, exit_code=run.returncode,
                       directory=folder.relative_to(ROOT).as_posix(),
                       artifacts={p.name:digest(p) for p in folder.iterdir()})
            report['runs'].append(row)
            (work/'report.json').write_bytes((json.dumps(report, indent=2)+'\n').encode('utf-8'))
            assert run.returncode == 0, (row, run.stderr.decode('utf-8', 'replace'))
            state = json.loads(run.stdout)
            assert state['case_count'] == 216 and state['guards_intact'] and state['precision_unchanged']
            results.append((state, (folder/'destinations.bin').read_bytes()))
        assert results[0] == results[1], f'Actual native/source difference PC{precision}'
        pairs += results[0][0]['case_count']
        exact_bytes += len(results[0][1])
    for name, value in report['source_files'].items():
        assert digest(ROOT/name) == value, name
    report.update(status='passed_scoped_actual_wrappers_only', exact_case_pairs=pairs,
                  exact_guarded_destination_bytes=exact_bytes,
                  source_rvas=source_rvas, full_tracker_equivalence=False,
                  controlled_states=['zero module channels', 'song end reached', 'pending mix with zero active voices'])
    encoded = (json.dumps(report, indent=2)+'\n').encode('utf-8')
    (work/'report.json').write_bytes(encoded)
    args.output.write_bytes(encoded)
    print(json.dumps({key:report[key] for key in ['status','work_directory','exact_case_pairs','exact_guarded_destination_bytes']}))


if __name__ == '__main__':
    main()
