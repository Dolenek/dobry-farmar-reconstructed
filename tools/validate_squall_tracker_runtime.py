#!/usr/bin/env python3
"""Actual MOD factory and owned-constructor tests; retain undefined native failures."""
import argparse
import datetime
import json
from pathlib import Path
import shutil
import subprocess
import wave

from validate_squall_tracker_stages import ROOT, digest, symbol
from squall_tracker_fixtures import generate
from squall_tracker_format_fixtures import generate as generate_formats


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--candidate', type=Path, default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output', type=Path, default=ROOT/'.local/squall-tracker-runtime-validation.json')
    parser.add_argument('--only-owned', action='store_true')
    parser.add_argument('--wrapped-reader', action='store_true')
    parser.add_argument('--additional-formats', action='store_true')
    args = parser.parse_args()
    work = ROOT/'.local/squall-tracker-runtime'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    binaries = {'native.dll': ROOT.parent/'farmář/Dobrý farmář/Squall.dll',
                'source.dll': args.candidate.resolve(), 'source.map': args.candidate.resolve().with_suffix('.map')}
    for short in ['probe', 'ctor-probe']:
        binaries[short+'.exe'] = ROOT/f'build/squall-tracker-probe-vs/Release/squall-tracker-{short}.exe'
    for name, path in binaries.items():
        shutil.copyfile(path, work/name)
    sources = ['tools/validate_squall_tracker_runtime.py', 'tools/validate_squall_tracker_stages.py',
               'tools/squall_tracker_fixtures.py', 'tools/squall_tracker_format_fixtures.py', 'tools/squall_tracker_probe.cpp',
               'tools/squall_tracker_ctor_probe.cpp', 'tools/squall_tracker_probe/CMakeLists.txt']
    sources += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.cpp', '.h']]
    for name in sources:
        target = work/'snapshots'/name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT/name, target)
    fixtures = work/'fixtures'
    fixtures.mkdir()
    generate(fixtures)
    inputs = []
    for index in [0, 1, 2]:
        titled = fixtures/f'owned-protracker-{index}.mod'
        blank = fixtures/f'owned-protracker-{index}-blank-title.mod'
        blank.write_bytes(bytes(20)+titled.read_bytes()[20:])
        inputs.extend([titled, blank])
    if args.additional_formats:
        inputs.extend(fixtures/item['name'] for item in generate_formats(fixtures))
    wrapped_files={}
    if args.wrapped_reader:
        for orders in [5, 8]:
            path=fixtures/f'owned-protracker-orders{orders}-blank-title.mod'
            data=bytearray(inputs[1].read_bytes());data[950]=orders
            path.write_bytes(data);inputs.append(path)
        for path in [p for p in inputs if 'blank-title' in p.name]:
            container=fixtures/(path.name+'.reader.wav') if args.additional_formats else path.with_suffix('.wav')
            wrapped_files[path.name]=container.name
            with wave.open(str(container),'wb') as out:
                out.setparams((1,1,44100,0,'NONE','not compressed'));out.writeframes(path.read_bytes())
    report = dict(status='observed_actual_mod_factory_and_owned_constructors_not_general_tracker_equivalence',
                  work_directory=work.relative_to(ROOT).as_posix(),
                  binaries={name:digest(work/name) for name in binaries},
                  source_files={name:digest(work/'snapshots'/name) for name in sources},
                  fixture_hashes={p.name:digest(p) for p in fixtures.iterdir()}, cases=[])
    report['owned_reader_mod_payload_in_real_wav_data'] = args.wrapped_reader
    report['wrapped_reader_files'] = wrapped_files
    report['owned_constructor_return_unit'] = 'sample frames; legacy raw probe field returned_bytes retained verbatim'
    file_rva = symbol(work/'source.map', '?api_SampleGetFile@@')
    seek_rva = symbol(work/'source.map', '?seek@CReader@@')
    ctor_rva = symbol(work/'source.map', '??0CDecompressTrack@@')

    def run_case(folder, command, details):
        folder.mkdir()
        run = subprocess.run(command, cwd=folder, capture_output=True, timeout=30)
        (folder/'stdout.json').write_bytes(run.stdout)
        (folder/'stderr.log').write_bytes(run.stderr)
        row = dict(**details, exit_code=run.returncode, directory=folder.relative_to(ROOT).as_posix(),
                   stdout=run.stdout.decode('utf-8','replace'), stderr=run.stderr.decode('utf-8','replace'),
                   artifacts={p.name:digest(p) for p in folder.iterdir()})
        report['cases'].append(row)
        (work/'report.json').write_bytes((json.dumps(report, indent=2)+'\n').encode('utf-8'))
        return row

    for fixture in ([] if args.only_owned else inputs):
        for precision in [53, 24]:
            extra = ['pc24'] if precision == 24 else []
            for memory in [0, 1]:
                for mode in ['metadata', 'mono', 'stereo']:
                    for backend in ['native', 'source']:
                        folder = work/f'factory-{fixture.name}-pc{precision}-mem{memory}-{mode}-{backend}'
                        command = [str(work/'probe.exe'), str(work/f'{backend}.dll'),
                                   '8300' if backend == 'native' else file_rva,
                                   str(fixture), mode, '0', '4096', str(memory), *extra]
                        run_case(folder, command, dict(kind='factory', fixture=fixture.name, precision=precision,
                                                      memory=memory, mode=mode, backend=backend))
    for fixture in [p for p in inputs if 'blank-title' in p.name]:
        for precision in [53, 24]:
            for slot in [1, 2]:
                for backend in ['native', 'source']:
                    folder = work/f'owned-{fixture.name}-pc{precision}-slot{slot}-{backend}'
                    command = [str(work/'ctor-probe.exe'), str(work/'native.dll'), str(work/'source.dll'),
                               str(fixtures/wrapped_files[fixture.name] if args.wrapped_reader else fixture), file_rva, seek_rva, '2bdb0' if backend == 'native' else ctor_rva,
                               str(slot), backend, *(['pc24'] if precision == 24 else [])]
                    run_case(folder, command, dict(kind='owned-constructor', fixture=fixture.name,
                                                  precision=precision, slot=slot, backend=backend))
    for name, value in report['source_files'].items():
        assert digest(ROOT/name) == value, name
    report['normal_factory_safe_claimed'] = False
    report['source_rvas'] = dict(file=file_rva, seek=seek_rva, constructor=ctor_rva)
    encoded = (json.dumps(report, indent=2)+'\n').encode('utf-8')
    (work/'report.json').write_bytes(encoded)
    args.output.write_bytes(encoded)
    print(json.dumps(dict(work_directory=report['work_directory'], processes=len(report['cases']),
                          exits={kind:{backend:{str(code):sum(r['kind']==kind and r['backend']==backend and r['exit_code']==code for r in report['cases']) for code in sorted({r['exit_code'] for r in report['cases'] if r['kind']==kind and r['backend']==backend})} for backend in ['native','source']} for kind in ['factory','owned-constructor']})))


if __name__ == '__main__':
    main()
