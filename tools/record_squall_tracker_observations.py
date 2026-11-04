#!/usr/bin/env python3
"""Record tracker differences without claiming broken native loads passed."""
import datetime
import json
from pathlib import Path

from record_live_production_verification import ROOT, digest, read, write


def main():
    initial = ROOT/'.local/squall-tracker-first/20261104T175858399323Z'
    ctor = ROOT/'.local/squall-tracker-ctor/20261104T180419739546Z'
    first = read(initial/'report.json')
    assert len(first['cases']) == 18
    for row in first['cases']:
        folder = ROOT/row['directory']
        for name, value in row['artifacts'].items():
            assert digest(folder/name) == value, (folder, name)
        if row['backend'] == 'native':
            assert row['exit_code'] == 0xc0000374
            assert '"frames":-26' in row['stdout']
            if row['mode'] != 'metadata':
                assert '"returned_bytes":4096' in row['stdout'] and '"nonzero_bytes":0' in row['stdout']
        else:
            assert row['exit_code'] == 0
            state = json.loads(row['stdout'])
            assert state['frames'] in [338688, 282240]
            if row['mode'] != 'metadata':
                assert state['returned_bytes'] == 4096*(2 if row['mode'] == 'mono' else 4)
                assert state['guards_intact'] and state['nonzero_bytes'] > 0
    stage_path = ROOT/'.local/squall-tracker-stages-validation.json'
    stages = read(stage_path)
    work = ROOT/stages['work_directory']
    assert len(stages['cases']) == 16
    for name, value in stages['source_files'].items():
        assert digest(work/'snapshots'/name) == value
        assert digest(ROOT/name) == value
    for name, value in stages['binaries'].items():
        assert digest(work/name) == value
    for row in stages['cases']:
        for name, value in row['artifacts'].items():
            assert digest(ROOT/row['directory']/name) == value
        state = row['states']['after-create']
        titled = row['fixture'] == 'owned-protracker-0.mod'
        if row['backend'] == 'native':
            assert row['exit_code'] == (0xc0000374 if titled else 0)
            assert state['default_speed'] == 125 and state['channels'] == 0 and state['song_flags'] == 4
            assert state['samples'] == 0xffffff00 and state['order_first_dword'] == 0xffffffff
            assert state['module_type'] == 31 and state['sample_count'] == 0
            assert state['pattern_name_pointer'] == (0x656e774f if titled else 0)
            if row['stage'] == 'orders':
                assert row['states']['after-orders']['samples'] == 0
        else:
            assert row['exit_code'] == 0 and state['channels'] == 4 and state['default_speed'] == 6
            assert state['module_type'] == 1 and state['sample_count'] == 31
            assert state['pattern_name_pointer'] == 0
            if row['stage'] == 'orders':
                assert row['states']['after-orders']['samples'] == 338688
    previous = read(ROOT/'.local/squall-tracker-baseline-20261104/manifest.json')
    for name, value in previous['files'].items():
        assert digest(ROOT/'.local/squall-tracker-baseline-20261104'/name) == value
    assert stages['binaries']['source.dll'] == previous['files']['build/windows/squall-candidate/Release/Squall.dll']
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path']) == row['sha256']
    for name in ['constructor', 'factory', 'read-methods', 'destroy', 'stream-read', 'mod-loader']:
        source = ROOT/f'.local/squall-tracker-{name}-exact.txt'
        (ROOT/f'analysis/squall-tracker-{name}-instructions.txt').write_bytes(source.read_bytes())
    write(ROOT/'analysis/squall-tracker-initial-process-observations.json', first)
    write(ROOT/'analysis/squall-tracker-staged-process-observations.json', stages)
    result = dict(status='actual_tracker_layout_inconsistency_and_native_cleanup_failures_not_equivalent',
                  recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  production_implementation_changed=False, candidate_sha256=stages['binaries']['source.dll'],
                  reference_sha256=stages['binaries']['native.dll'],
                  normal_load_processes=18, native_heap_corruption_processes=9,
                  source_successful_normal_load_processes=9,
                  staged_processes=16, native_titled_cleanup_failures=4,
                  native_blank_title_successful_cleanup=4, source_staged_successes=8,
                  original_files_unchanged=17,
                  initial_source_provenance_limit='First exploratory probes were snapshotted after their runs; no claim of pre-run source hashing. Production remained at70d62ec. New staged runs snapshot all sources/binaries before execution.',
                  initial_baseline=previous,
                  initial_report_sha256=digest(initial/'report.json'),
                  constructor_report=read(ctor/'report.json'),
                  constructor_artifacts={p.relative_to(ROOT).as_posix():digest(p) for p in ctor.rglob('*') if p.is_file()},
                  staged_report_sha256=digest(stage_path),
                  findings=[
                      'Native factory1002dd9a allocates0xeef8 bytes; constructor1002bed2 writes FileBuffer at+0xef08 and pattern pointer at+0xef04. Owned64KiB storage removes that underallocation but titled input still corrupts heap during cleanup.',
                      'Constructor/Create/Destroy use newer fields: Order+b7ac, channels+d05c, pattern-name pointer+d0e4. MOD loader10021250 and stream Read1002a590 use older fields four bytes earlier afterOrderInfo.',
                      'After native Create, title Owne occupies+d0e4 (0x656e774f). Native Destroy1002c61a loads this field and1002c62b calls delete. The same bytes with a zero20-byte title clean up successfully on owned large storage in PC53/PC24.',
                      'Native CreateOrderInfo10026910 clears+b7a8 as SamplesInFile, overwriting the old loader order. It sees newer Order+b7ac still0xff, yielding0 frames rather than338688.',
                      'Native GetMonoSamples1002bfe0 and GetStereoSamples1002c000 invoke Read(buffer,length*2,1) / Read(buffer,length*4,2), ignore start/loop and return frames directly. Public source instead copies cached pattern bytes.'
                  ],
                  limitations=[
                      'No broad tracker-format or actual physical playback equivalence established.',
                      'Owned zeroed objects validate isolated actual methods, not safe behavior of the unmodified native factory.',
                      'Pointer addresses and heap corruption are not normalized into successful PCM equivalence.',
                      'Full reconstructed tracker implementation still differs; no production workaround or decoder replacement introduced.'
                  ], whole_game_complete=False, whole_squall_complete=False)
    write(ROOT/'analysis/squall-tracker-initial-observations.json', result)
    verification = read(ROOT/'analysis/verification.json')
    verification['squall-tracker-observations'] = dict(
        report='analysis/squall-tracker-initial-observations.json',
        status=result['status'], production_implementation_changed=False,
        native_normal_loads_failed=9, native_staged_titled_cleanup_failed=4,
        native_staged_blank_title_cleanup_passed=4, complete=False)
    for name in ['initial-observations', 'initial-process-observations', 'staged-process-observations']:
        name = f'analysis/squall-tracker-{name}.json'
        if name not in verification['reports']:
            verification['reports'].append(name)
    write(ROOT/'analysis/verification.json', verification)
    print('Recorded18 actual loads,16 staged processes, mixed field offsets and native cleanup failures; production unchanged.')


if __name__ == '__main__':
    main()
