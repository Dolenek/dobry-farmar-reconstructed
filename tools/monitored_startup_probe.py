#!/usr/bin/env python3
"""Short full-source startup with a live independent Job watchdog; no injected input."""
import ctypes as C
from ctypes import wintypes as W
import json
from pathlib import Path
import shutil
import subprocess
import time
import xml.etree.ElementTree as ET
from desktop_runtime_hold import require_desktop_runtime_available
from record_live_production_verification import ROOT,digest,read,write
from windows_game_gui import GameGui,u


def main():
    context=require_desktop_runtime_available();assert context,'Independent watchdog required'
    work=context['directory'];data=ROOT.parent/'farmář/Dobrý farmář/Data';probe=GameGui(work,data)
    runtime=work/'runtime';runtime.mkdir();common=work/'common';(common/'sys').mkdir(parents=True)
    source={'FarmarMonitoredStartup.exe':'Dobrý farmář.exe','JNGLoad.dll':'JNGLoad.dll','Squall.dll':'Squall.dll'}
    for name,current in source.items():shutil.copy2(ROOT/'build/windows/Release'/current,runtime/name)
    settings=probe.resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0')
    ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),backend='source',
        implementation_files={name:digest(runtime/name) for name in source},samples=[],
        injected_input=False,other_UI_queues_joined=False,running_threads_suspended=False,
        entered_farm=False,whole_game_complete=False)
    sources=['tools/monitored_startup_probe.py','tools/windows_game_gui.py','tools/desktop_runtime_hold.py','tools/desktop_timing_probe.cpp']
    report['source_files']={name:digest(ROOT/name) for name in sources};report['source_file_snapshots']={}
    for name in sources:
        target=work/name.replace('/','_');shutil.copyfile(ROOT/name,target)
        report['source_file_snapshots'][name]=target.relative_to(ROOT).as_posix()
    report['watchdog_lease']=context['lease']
    report['watchdog_sha256']=digest(ROOT/'build/host-test-watchdog/Release/host-test-watchdog.exe')
    report['DWM_liveness_metric']='qpc_vblank;per-probe-process frame/refresh counters are retained but not used as global liveness'
    process=None;window=None;normal=False
    try:
        probe.progress('before-owned-source-launch')
        out=(work/'game-stdout.log').open('wb');err=(work/'game-stderr.log').open('wb')
        process=subprocess.Popen([str(runtime/'FarmarMonitoredStartup.exe'),'--data',str(data),'--common-dir',str(common)],
            cwd=runtime,stdout=out,stderr=err);out.close();err.close();probe.process=process
        report['pid']=process.pid;deadline=time.monotonic()+4
        while window is None:
            probe.progress('waiting-for-owned-startup-window');probe.check_process()
            windows=[row for row in probe.windows(process.pid) if row[2].lower().startswith('melesta_')]
            if windows:window=W.HWND(windows[0][0]);break
            assert time.monotonic()<deadline,'Source startup window was not constructed promptly';time.sleep(.1)
        probe.hwnd=window
        u.SendMessageTimeoutW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM,W.UINT,W.UINT,C.POINTER(C.c_size_t)]
        u.SendMessageTimeoutW.restype=W.LPARAM
        start=time.monotonic();failed=0;last_change=start;previous=None
        while time.monotonic()-start<8:
            probe.progress('before-bounded-owned-WM_NULL');probe.check_process();result=C.c_size_t()
            responsive=bool(u.SendMessageTimeoutW(window,0,0,0,2|32,250,C.byref(result)))
            failed=0 if responsive else failed+1
            assert failed<2,'Two consecutive bounded owned-window responsiveness probes failed'
            probe.progress('before-bounded-DWM-counter-query')
            capture=subprocess.run([str(ROOT/'build/desktop-timing-probe/Release/desktop-timing-probe.exe')],
                capture_output=True,text=True,timeout=1)
            assert capture.returncode==0,('DWM timing query unavailable',capture.stdout,capture.stderr)
            timing=json.loads(capture.stdout);signature=timing['qpc_vblank']
            if signature!=previous:previous=signature;last_change=time.monotonic()
            assert time.monotonic()-last_change<3,'DWM vertical-blank timestamps did not advance during animated startup'
            report['samples'].append(dict(elapsed=time.monotonic()-start,owned_window_responsive=responsive,dwm=timing))
            probe.progress('startup-health-observed');time.sleep(.3)
        probe.progress('before-owned-normal-close');assert u.PostMessageW(window,0x10,0,0)
        report['game_exit_code']=process.wait(timeout=3);assert report['game_exit_code']==0
        normal=True;report['status']='passed_bounded_monitored_source_startup_without_input_only'
    except Exception as error:
        report['status']='failed';report['error']=repr(error)
    finally:
        if process and process.poll() is None:
            probe.progress('cleanup-owned-source-window')
            if window:u.PostMessageW(window,0x10,0,0)
            try:process.wait(timeout=2)
            except subprocess.TimeoutExpired:process.kill();process.wait(timeout=2)
        report['normal_close']=normal;report['game_exit_code']=process.poll() if process else None
        for row in read(ROOT/'analysis/inventory.json')['files']:
            assert digest(data.parent/row['path'])==row['sha256'],row['path']
        report['original_files_unchanged']=17
        write(work/'probe-report.json',report);probe.progress('probe-report-written',status=report['status'])
    print(json.dumps(dict(status=report['status'],normal_close=normal,samples=len(report['samples']),error=report.get('error'))),flush=True)
    return 0 if normal else 1


if __name__=='__main__':raise SystemExit(main())
