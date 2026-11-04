#!/usr/bin/env python3
"""Retain actual tracker loader/order states, including native cleanup failures."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess

from squall_tracker_fixtures import generate

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def symbol(path, name):
    for line in path.read_text(encoding='latin-1').splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1].startswith(name):
            return f'{int(parts[2], 16) - 0x10000000:x}'
    raise AssertionError(name)


FIELDS = dict(samples=0xb7a8, order_first_dword=0xb7ac, default_speed=0xd04c,
              default_tempo=0xd050, default_global_volume=0xd054, song_flags=0xd058,
              channels=0xd05c, mix_channels=0xd060, mix_status=0xd064,
              buffer_count=0xd068, module_type=0xd06c, sample_count=0xd070,
              instrument_count=0xd074, pattern_name_pointer=0xd0e4,
              title_first_dword=0xd0e8, max_pattern_samples=0xeef8)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--candidate', type=Path, default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output', type=Path, default=ROOT/'.local/squall-tracker-stages-validation.json')
    parser.add_argument('--all-mod-fixtures', action='store_true')
    args = parser.parse_args()
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work = ROOT/'.local/squall-tracker-stages'/stamp
    work.mkdir(parents=True)
    snapshots = work/'snapshots'
    sources = [Path(__file__).resolve(), ROOT/'tools/squall_tracker_fixtures.py',
               ROOT/'tools/squall_tracker_stage_probe.cpp', ROOT/'tools/squall_tracker_probe/CMakeLists.txt']
    sources += sorted((ROOT/'src/squall').rglob('*.cpp')) + sorted((ROOT/'src/squall').rglob('*.h'))
    source_hashes = {}
    for path in sources:
        name = path.relative_to(ROOT).as_posix()
        dest = snapshots/name
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, dest)
        source_hashes[name] = digest(dest)
    probe = ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-stage-probe.exe'
    reference = ROOT.parent/'farmář/Dobrý farmář/Squall.dll'
    binaries = {'native.dll': reference, 'source.dll': args.candidate.resolve(),
                'source.map': args.candidate.resolve().with_suffix('.map'), 'probe.exe': probe}
    for name, path in binaries.items():
        shutil.copyfile(path, work/name)
    hashes = {name: digest(work/name) for name in binaries}
    fixtures = work/'fixtures'
    fixtures.mkdir()
    generate(fixtures)
    titled = fixtures/'owned-protracker-0.mod'
    blank = fixtures/'owned-protracker-blank-title.mod'
    blank.write_bytes(bytes(20) + titled.read_bytes()[20:])
    inputs = [titled, blank]
    if args.all_mod_fixtures:
        inputs = []
        for index in [0, 1, 2]:
            path = fixtures/f'owned-protracker-{index}.mod'
            blank_path = fixtures/f'owned-protracker-{index}-blank-title.mod'
            blank_path.write_bytes(bytes(20)+path.read_bytes()[20:])
            inputs.extend([path, blank_path])
    native = ['2c100', '26910', '2a590', '2c5f0']
    names = ['?Create@CDecompressTrack@@', '?CreateOrderInfo@CDecompressTrack@@',
             '?Read@CDecompressTrack@@', '?Destroy@CDecompressTrack@@']
    own = [symbol(work/'source.map', name) for name in names]
    report = dict(status='actual_methods_on_zeroed_64k_owned_objects_not_factory_equivalence',
                  work_directory=work.relative_to(ROOT).as_posix(), source_files=source_hashes,
                  binaries=hashes, sources_snapshotted_before_run=True,
                  fixture_hashes={p.name: digest(p) for p in fixtures.glob('*.mod')},
                  native_rvas=native, source_rvas=own, cases=[])
    for fixture in inputs:
        for precision in [53, 24]:
            for stage in ['create', 'orders']:
                for backend, rvas in [('native', native), ('source', own)]:
                    folder = work/f'{fixture.stem}-pc{precision}-{stage}-{backend}'
                    folder.mkdir()
                    command = [str(work/'probe.exe'), str(work/f'{backend}.dll'), str(fixture), *rvas, stage, str(precision)]
                    run = subprocess.run(command, cwd=folder, capture_output=True, timeout=30)
                    (folder/'stdout.json').write_bytes(run.stdout)
                    (folder/'stderr.log').write_bytes(run.stderr)
                    row = dict(fixture=fixture.name, precision=precision, stage=stage, backend=backend,
                               directory=folder.relative_to(ROOT).as_posix(), exit_code=run.returncode,
                               stdout=run.stdout.decode('utf-8', 'replace'), stderr=run.stderr.decode('utf-8', 'replace'), states={})
                    for path in folder.glob('after-*.bin'):
                        data = path.read_bytes()
                        assert len(data) == 65536
                        row['states'][path.stem] = {name: struct.unpack_from('<I', data, offset)[0] for name, offset in FIELDS.items()}
                    row['artifacts'] = {path.name: digest(path) for path in folder.iterdir()}
                    report['cases'].append(row)
                    (work/'report.json').write_bytes((json.dumps(report, indent=2)+'\n').encode('utf-8'))
    for name, value in source_hashes.items():
        assert digest(ROOT/name) == value, name
    report['production_implementation_changed'] = False
    report['normal_factory_load_claimed_safe'] = False
    encoded = (json.dumps(report, indent=2)+'\n').encode('utf-8')
    (work/'report.json').write_bytes(encoded)
    args.output.write_bytes(encoded)
    print(json.dumps(dict(work_directory=report['work_directory'], cases=len(report['cases']),
                          exit_codes={str(code):sum(row['exit_code']==code for row in report['cases']) for code in sorted({r['exit_code'] for r in report['cases']})})))


if __name__ == '__main__':
    main()
