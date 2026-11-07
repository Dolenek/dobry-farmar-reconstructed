#!/usr/bin/env python3
"""Audit bounded no-input source startup and independent watchdog health evidence."""
import argparse
import datetime
from pathlib import Path
from current_production_regression import current_regression
from record_live_production_verification import ROOT,digest,read,write


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--run-directory',type=Path,required=True)
    args=parser.parse_args();work=args.run_directory.resolve(strict=True);raw=read(work/'probe-report.json')
    assert raw['status']=='passed_bounded_monitored_source_startup_without_input_only'
    assert raw['normal_close'] and raw['game_exit_code']==0 and not raw['injected_input']
    assert not raw['running_threads_suspended'] and not raw['other_UI_queues_joined']
    assert raw['original_files_unchanged']==17 and raw['samples'][-1]['elapsed']>=7
    misses=[n for n,row in enumerate(raw['samples']) if not row['owned_window_responsive']]
    assert not misses or misses==[0],('Unexplained responsiveness deadline miss after startup',misses)
    times=[row['dwm']['qpc_vblank'] for row in raw['samples']]
    assert all(a<b for a,b in zip(times,times[1:]))
    import json
    health=[json.loads(line) for line in (work/'watchdog.jsonl').read_text(encoding='utf-8').splitlines()]
    assert health[0]['event']=='started' and health[-1]['event']=='finished'
    assert health[-1]['driver_exit_code']==health[-1]['watchdog_exit_code']==0
    assert health[-1]['live_owned_processes_before_final_close']==0
    assert raw['watchdog_sha256']==digest(ROOT/'build/host-test-watchdog/Release/host-test-watchdog.exe')
    for name,sha in raw['source_files'].items():assert digest(ROOT/raw['source_file_snapshots'][name])==sha
    for name,sha in raw['implementation_files'].items():
        current='Dobrý farmář.exe' if name=='FarmarMonitoredStartup.exe' else name
        assert digest(ROOT/'build/windows/Release'/current)==sha
    samples=[row for row in health if row['event']=='health'];assert samples
    full,_=current_regression()
    report=dict(status='passed_eight_second_independently_monitored_source_startup_without_input_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        raw_report=dict(path=(work/'probe-report.json').relative_to(ROOT).as_posix(),sha256=digest(work/'probe-report.json')),
        health_log=dict(path=(work/'watchdog.jsonl').relative_to(ROOT).as_posix(),sha256=digest(work/'watchdog.jsonl')),
        authorization_report=dict(path='analysis/host-runtime-monitoring-authorization.json',sha256=digest(ROOT/'analysis/host-runtime-monitoring-authorization.json')),
        implementation_files=raw['implementation_files'],owned_window_observations=len(raw['samples']),
        owned_window_healthy_observations=len(raw['samples'])-len(misses),
        bounded250ms_WM_NULL_deadline_miss_indices=misses,
        DWM_vblank_timestamps_strictly_advanced=True,physical_display_proven_by_counter_alone=False,
        host_cpu_percent_range=[min(r['host_cpu_percent'] for r in samples),max(r['host_cpu_percent'] for r in samples)],
        minimum_host_available_physical_bytes=min(r['host_available_physical_bytes'] for r in samples),
        independent_watchdog_exit=0,game_exit=0,injected_input=False,entered_farm=False,
        revised_scene_tree_runtime_validated=False,host_freeze_root_cause_proven=False,incident_resolved=False,
        original_files_unchanged=17,production_changed=False,current_full_ctest_report=full,
        current_full_ctest_source_hashes_match=True,new_full_ctest_run=False,whole_game_complete=False)
    write(ROOT/'analysis/monitored-source-startup-validation.json',report)
    verification=read(ROOT/'analysis/verification.json')
    for name in ['analysis/host-runtime-monitoring-authorization.json','analysis/host-watchdog-cpu-validation.json',
        'analysis/monitored-source-startup-validation.json']:
        if name not in verification['reports']:verification['reports'].append(name)
    verification['host_runtime_incident'].update(authorized_monitored_host_tests=True,
        host_interactive_runtime_tests_held=False,unmonitored_host_tests_held=True,
        latest_monitored_report='analysis/monitored-source-startup-validation.json',resolved=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Audited bounded8s source startup,advancing DWM timestamps,normal0 and independent health log;incident not resolved.')


if __name__=='__main__':main()
