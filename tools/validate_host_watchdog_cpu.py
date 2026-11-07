#!/usr/bin/env python3
"""Exercise watchdog timeout/tree cleanup with CPU fixtures, never a game or GPU."""
import ctypes as C
from ctypes import wintypes as W
import datetime
import json
import subprocess
import sys
import time
from record_live_production_verification import ROOT,digest,read,write


def main():
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    base=ROOT/'.local/monitored-host-runtime'/('cpu-selftest-'+stamp);base.mkdir(parents=True)
    exe=ROOT/'build/host-test-watchdog/Release/host-test-watchdog.exe';worker=ROOT/'tools/host_watchdog_cpu_worker.py'
    cases=[]
    for mode,seconds,stale,expected in [('healthy',4,1500,0),('stall',5,1000,125),('heartbeat',2,1000,124)]:
        directory=base/mode
        args=[str(exe),str(directory),str(seconds),str(stale),sys.executable,str(worker),mode]
        result=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=10)
        assert result.returncode==expected,(mode,result.returncode,result.stdout,result.stderr)
        rows=[json.loads(line) for line in (directory/'watchdog.jsonl').read_text(encoding='utf-8').splitlines()]
        assert rows[0]['event']=='started' and rows[-1]['event']=='finished'
        assert rows[-1]['watchdog_exit_code']==expected
        if expected:assert any(r['event']=='terminated_owned_job' for r in rows)
        cases.append(dict(mode=mode,return_code=result.returncode,stdout=result.stdout,stderr=result.stderr,
            log=dict(path=(directory/'watchdog.jsonl').relative_to(ROOT).as_posix(),sha256=digest(directory/'watchdog.jsonl')),events=rows))
    directory=base/'guardian-exit';args=[str(exe),str(directory),'30','30000',sys.executable,str(worker),'guardian_exit']
    guardian=subprocess.Popen(args,cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    k=C.WinDLL('kernel32',use_last_error=True)
    k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE
    k.WaitForSingleObject.argtypes=[W.HANDLE,W.DWORD];k.CloseHandle.argtypes=[W.HANDLE]
    handles=[]
    try:
        deadline=time.monotonic()+5
        while not (directory/'fixture-child.json').exists():
            assert guardian.poll() is None and time.monotonic()<deadline;time.sleep(.02)
        pids=[read(directory/'lease.json')['driver_pid'],read(directory/'fixture-child.json')['pid']]
        for pid in pids:
            handle=k.OpenProcess(0x100000,False,pid);assert handle;handles.append(handle)
            assert k.WaitForSingleObject(handle,0)==258
        guardian.terminate();guardian.wait(timeout=5)
        for handle in handles:assert k.WaitForSingleObject(handle,3000)==0,'Owned child survived independent guardian exit'
        cases.append(dict(mode='guardian_exit',actual_owned_processes_confirmed_exited=pids,
            guardian_exit_code=guardian.returncode,job_kill_on_close_verified=True))
    finally:
        if guardian.poll() is None:guardian.terminate();guardian.wait(timeout=5)
        guardian.communicate()
        for handle in handles:k.CloseHandle(handle)
    sources=['tools/host_test_watchdog.cpp','tools/host_test_watchdog/CMakeLists.txt','tools/host_watchdog_cpu_worker.py',
        'tools/desktop_runtime_hold.py','tools/validate_host_watchdog_cpu.py']
    report=dict(status='passed_CPU_only_watchdog_healthy_progress_stall_absolute_timeout_and_guardian_death_tree_cleanup',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=base.relative_to(ROOT).as_posix(),
        source_files={name:digest(ROOT/name) for name in sources},watchdog_sha256=digest(exe),cases=cases,
        games_or_graphics_devices_started=False,physical_input_injected=False,host_freeze_root_cause_proven=False,whole_game_complete=False)
    write(ROOT/'analysis/host-watchdog-cpu-validation.json',report)
    print('Watchdog CPU checks passed: healthy,stalled progress,absolute timeout,guardian death and owned child cleanup.')


if __name__=='__main__':main()
