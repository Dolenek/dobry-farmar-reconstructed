#!/usr/bin/env python3
"""Bounded source restore/new observer/audio probe,only under the independent watchdog."""
import json
import argparse
import os
import ctypes as C
from ctypes import wintypes as W
import shutil
import subprocess
import time
from desktop_runtime_hold import require_desktop_runtime_available
from read_only_scene_tree import SceneSnapshotChanged
from record_live_production_verification import ROOT,digest,read,write
from tutorial_warning_session import WarningSuite
from windows_game_gui import u
from live_farm_fields import read_live_farm_fields


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manual-focus-wait',action='store_true',help='Wait up to20s for real user activation;never bypass a denied foreground request')
    args=parser.parse_args()
    context=require_desktop_runtime_available();assert context
    work=context['directory']
    if args.manual_focus_wait:
        write(work/'manual-focus-request.json',dict(driver_pid=os.getpid(),seconds=20))
    suite=WarningSuite(work/'source-restore')
    for name in ['tools/monitored_restore_probe.py','tools/read_only_scene_tree.py','tools/desktop_runtime_hold.py','tools/live_farm_fields.py']:
        target=suite.work/name.replace('/','_');shutil.copyfile(ROOT/name,target)
        suite.report['source_files'][name]=digest(ROOT/name)
        suite.report['source_file_snapshots'][name]=target.relative_to(ROOT).as_posix()
    layout_path=ROOT/'analysis/no-dry-award-state-validation.json'
    field_layout=read(layout_path)['field_layout']['metadata']
    suite.report['field_layout_report']=dict(path=layout_path.relative_to(ROOT).as_posix(),sha256=digest(layout_path))
    u.SendMessageTimeoutW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM,W.UINT,W.UINT,C.POINTER(C.c_size_t)]
    u.SendMessageTimeoutW.restype=W.LPARAM
    checkpoints=read(ROOT/'analysis/contract-streak-forty-checkpoints.json')
    checkpoint=next(row for row in checkpoints['profile_sets'] if row['kind']=='native')
    directory=ROOT/checkpoint['profile_directory']
    assert {p.name:digest(p) for p in directory.iterdir() if p.is_file()}==checkpoint['files']
    success=False
    try:
        with suite.open('source','monitored_restore_audio',directory) as active:
            initial=active.state();active.session['initial_state']=initial
            active.capture('monitored-restored-farm')
            active.pause(True)
            try:
                scene=active.observer.visible_scene(active.hwnd)
                fields=read_live_farm_fields(active.observer,ROOT,field_layout,scene)
                assert len(scene)>100 and len(fields)==60 and all(field['active'] for field in fields)
                active.session['scene_observation']=dict(stable_topology_observed=True,controls=len(scene),
                    all60_actual_field_identities_and_active_flags_observed=True,fields=fields,atomic_frame_claimed=False,
                    ordinary_game_pause_used=True,render_or_UI_thread_suspended=False)
            except SceneSnapshotChanged as error:
                active.session['scene_observation']=dict(stable_topology_observed=False,error=str(error),atomic_frame_claimed=False)
            finally:
                active.pause(False)
            audio=ROOT/'build/process-audio-capture/Release/process-audio-capture.exe'
            active.probe.progress('before-owned-process-audio-capture')
            with (work/'source-audio.log').open('wb') as output,(work/'source-audio-stderr.log').open('wb') as errors:
                capture=subprocess.Popen([str(audio),str(active.session['pid']),'4',str(work/'source-audio.pcm')],stdout=output,stderr=errors)
                deadline=time.monotonic()+6;last_health=0;last_vblank=None
                active.session['bounded_window_and_DWM_health']=[]
                try:
                    while capture.poll() is None:
                        active.probe.progress('waiting-for-bounded-process-audio-capture')
                        active.probe.check_process();assert time.monotonic()<deadline,'Process audio capture exceeded6s'
                        if time.monotonic()-last_health>=.5:
                            active.probe.progress('before-bounded-restored-window-health')
                            result=C.c_size_t();assert u.SendMessageTimeoutW(active.hwnd,0,0,0,2|32,250,C.byref(result)),'Restored game window stopped responding'
                            observed=subprocess.run([str(ROOT/'build/desktop-timing-probe/Release/desktop-timing-probe.exe')],
                                capture_output=True,text=True,timeout=1,creationflags=0x08000000)
                            assert observed.returncode==0;timing=json.loads(observed.stdout)
                            assert last_vblank is None or timing['qpc_vblank']>last_vblank,'DWM vertical-blank time did not advance'
                            last_vblank=timing['qpc_vblank'];last_health=time.monotonic()
                            active.session['bounded_window_and_DWM_health'].append(timing)
                        time.sleep(.2)
                    assert capture.returncode==0
                finally:
                    if capture.poll() is None:capture.kill();capture.wait(timeout=1)
            active.session['audio_capture_log']=(work/'source-audio.log').relative_to(ROOT).as_posix()
            active.session['final_state']=active.state()
            for key in ['stock','market','carried','collected','held','money']:
                assert active.session['final_state'][key]==initial[key],key
        success=True;suite.report['status']='passed_actual_source_restored_checkpoint_and_process_audio_capture_only'
    except Exception as error:
        suite.report['status']='failed';suite.report['error']=repr(error)
        import traceback;suite.report['traceback']=traceback.format_exc()
    finally:
        suite.unchanged();suite.report['reference_files_unchanged']=17
        suite.report['screenshots']={p.name:digest(p) for p in suite.work.glob('*.png')}
        suite.report['watchdog_lease']=context['lease']
        write(suite.work/'report.json',suite.report)
    print(json.dumps(dict(status=suite.report['status'],work_directory=suite.report['work_directory'],error=suite.report.get('error'))),flush=True)
    return 0 if success else 1


if __name__=='__main__':raise SystemExit(main())
