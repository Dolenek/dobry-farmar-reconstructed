#!/usr/bin/env python3
"""Retain bounded monitor false alarm and refused-focus trials without calling them passes."""
import datetime
import json
from record_live_production_verification import ROOT,digest,read,write


def main():
    rows=[]
    names=['source-startup-20261107T2202008646470Z','source-restore-20261107T2214266094620Z',
           'source-restore-20261107T2217088150954Z']
    for name in names:
        work=ROOT/'.local/monitored-host-runtime'/name
        path=work/('probe-report.json' if 'startup' in name else 'source-restore/report.json')
        raw=read(path);assert raw['status']=='failed'
        events=[json.loads(line) for line in (work/'watchdog.jsonl').read_text(encoding='utf-8').splitlines()]
        assert events[-1]['event']=='finished' and events[-1]['driver_exit_code']==1
        assert events[-1]['watchdog_exit_code']==0 and events[-1]['live_owned_processes_before_final_close']==0
        game_exit=raw['game_exit_code'] if 'startup' in name else raw['sessions'][0]['exit_code']
        assert game_exit==0
        info=dict(run_directory=work.relative_to(ROOT).as_posix(),raw_report=dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path)),
            original_driver_status=raw['status'],original_error=raw['error'],actual_game_exit=game_exit,
            watchdog_log=dict(path=(work/'watchdog.jsonl').relative_to(ROOT).as_posix(),sha256=digest(work/'watchdog.jsonl')),
            orphaned_live_owned_processes=0,passed=False)
        if 'startup' in name:
            missed=[i for i,s in enumerate(raw['samples']) if not s['owned_window_responsive']]
            assert not missed or missed==[0]
            info['bounded250ms_WM_NULL_deadline_miss_indices']=missed
            times=[s['dwm']['qpc_vblank'] for s in raw['samples']];assert all(a<b for a,b in zip(times,times[1:]))
            info['correction']='Per-probe-process frame counters reset;use advancing vblank timestamps without claiming physical display proof'
        else:
            timings=[json.loads(line) for line in (work/'input-api-timings.jsonl').read_text(encoding='utf-8').splitlines()]
            assert all(row['api']!='SendInput' for row in timings)
            if 'returned' in timings[0]:assert timings[0]['returned']==0
            info['input_api_timings']=timings;info['input_injected']=False
            info['foreground_refusal_not_bypassed']=True
        rows.append(info)
    write(ROOT/'analysis/monitored-probe-initial-observations.json',dict(
        status='recorded_bounded_DWM_counter_false_alarm_and_two_safe_foreground_refusals',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        trials=rows,production_changed=False,host_freeze_root_cause_proven=False,whole_game_complete=False))
    print('Retained3 failed bounded trials;all games normal0,all owned trees ended,no injected input in focus refusals.')


if __name__=='__main__':main()
