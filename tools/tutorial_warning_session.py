"""Isolated complete-game sessions for actual tutorial warning inputs;read-only observation."""
import ctypes as C
import datetime
import json
import shutil
import subprocess
import time
import xml.etree.ElementTree as ET
from contextlib import contextmanager
from ctypes import wintypes as W
from pathlib import Path
from PIL import Image
from record_live_production_verification import ROOT,digest,read
from windows_game_gui import u,k,INPUT,KEY,MOUSE
from windows_game_memory import TutorialMemory
from windows_game_nonzero_mouse import NonzeroMouseGui

u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]
u.IsWindowVisible.argtypes=[W.HWND];u.IsWindowVisible.restype=W.BOOL
k.CloseHandle.argtypes=[W.HANDLE]


class WarningGui(NonzeroMouseGui):
    def click(self,hwnd,x,y):
        self.move(hwnd,round(x),round(y))
        event=INPUT();event.type=0;event.mouse=MOUSE(0,0,0,2,0,0)
        assert u.SendInput(1,C.byref(event),C.sizeof(event))==1
        try:time.sleep(1)
        finally:
            event=INPUT();event.type=0;event.mouse=MOUSE(0,0,0,4,0,0)
            assert u.SendInput(1,C.byref(event),C.sizeof(event))==1
            time.sleep(.1)


class NativeChild:
    def __init__(self,pid):
        self.pid=pid
        k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE
        k.GetExitCodeProcess.argtypes=[W.HANDLE,C.POINTER(W.DWORD)]
        self.handle=k.OpenProcess(0x100000|0x1000,False,pid);assert self.handle
    def poll(self):
        code=W.DWORD();assert k.GetExitCodeProcess(self.handle,C.byref(code))
        return None if code.value==259 else code.value
    def wait(self,timeout=20):
        deadline=time.monotonic()+timeout
        while time.monotonic()<deadline:
            code=self.poll()
            if code is not None:return code
            time.sleep(.05)
        raise RuntimeError('Owned reference process did not exit')


class WarningSuite:
    def __init__(self,work):
        self.work=Path(work).resolve();self.work.mkdir(parents=True,exist_ok=False)
        self.data=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Data'
        self.runtime=self.work/'runtime';self.runtime.mkdir()
        self.inventory=read(ROOT/'analysis/inventory.json');self.unchanged()
        shutil.copytree(self.data,self.runtime/'Data')
        files={'FarmarTutorialWarnings.exe':'Dobr\u00fd farm\u00e1\u0159.exe','JNGLoad.dll':'JNGLoad.dll','Squall.dll':'Squall.dll'}
        for output,source in files.items():shutil.copy2(ROOT/'build/windows/Release'/source,self.runtime/output)
        self.probe=WarningGui(self.work,self.data)
        prior=read(ROOT/'.local/application-runtime-validation.json')
        self.menu_file=ROOT/prior['work_directory']/'0-03-menu.png'
        self.hud_file=ROOT/prior['work_directory']/'0-04-game.png'
        self.menu_marker=self.probe.roi(Image.open(self.menu_file),(5,5,70,70))
        self.hud_marker=self.probe.roi(Image.open(self.hud_file),(10,8,55,28))
        self.launcher=ROOT/'build/native-process/Release/native-process-launcher.exe'
        layout_exe=ROOT/'build/tutorial-warning-layout/Release/tutorial-warning-layout.exe'
        self.layout=json.loads(subprocess.check_output([str(layout_exe)],creationflags=0x08000000))
        self.report=dict(status='running',work_directory=self.work.relative_to(ROOT).as_posix(),sessions=[],
            recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            implementation_files={n:digest(self.runtime/n) for n in files},
            source_files={p.relative_to(ROOT).as_posix():digest(p) for p in (ROOT/'src/game').rglob('*') if p.is_file()},
            launcher=dict(path=self.launcher.relative_to(ROOT).as_posix(),sha256=digest(self.launcher)),
            tutorial_observer_layout=dict(path=layout_exe.relative_to(ROOT).as_posix(),sha256=digest(layout_exe),value=self.layout),
            startup_templates={p.relative_to(ROOT).as_posix():digest(p) for p in [self.menu_file,self.hud_file]},
            mouse_hold_seconds=1,original_program_code_used_in_implementation=False,
            original_profile_fields_or_memory_written=False,whole_game_complete=False)
        self.report['source_file_snapshots']={}
        for name in ['tutorial_warning_session.py','windows_game_nonzero_mouse.py','windows_game_gui.py','windows_game_memory.py']:
            source=ROOT/'tools'/name;target=self.work/name;shutil.copyfile(source,target)
            key=source.relative_to(ROOT).as_posix();self.report['source_files'][key]=digest(source)
            self.report['source_file_snapshots'][key]=target.relative_to(ROOT).as_posix()

    def unchanged(self):
        for row in self.inventory['files']:assert digest(self.data.parent/row['path'])==row['sha256'],row['path']

    def shortcut(self,code):
        for scan,down in [(0x38,True),(code,True),(code,False),(0x38,False)]:
            self.probe.focus();event=INPUT();event.type=1
            event.keyboard=KEY(0,scan&0x7f,8|(1 if scan&0x80 else 0)|(0 if down else 2),0,0)
            assert u.SendInput(1,C.byref(event),C.sizeof(event))==1;time.sleep(.1)

    def click(self,x,y):self.probe.click(self.hwnd,x,y)
    def capture(self,label):return self.probe.capture(f'{self.index}-{label}.png')

    def counters(self):
        offsets=dict(elapsed=0xc0c,capacity_previous=0xc10,capacity_counter=0xc14,mixed_previous=0xc18,mixed_counter=0xc1c) if self.observer.kind=='native' else self.layout
        manager=self.observer.manager();assert manager
        return {name:self.observer.read(manager+offsets[name],'<I' if name.endswith('_counter') else '<f')[0]
            for name in ['elapsed','capacity_previous','capacity_counter','mixed_previous','mixed_counter']}

    def state(self):
        val=self.observer.property
        return dict(game_time=val('floats','GameTime'),money=val('ints','Money'),held=val('uints','PCarry'),
            carried=[val('uints',f'PCC{n}') for n in range(8)],stock=[val('uints',f'WGoodCount{n}') for n in range(19)],
            market=[val('uints',f'MGoodCount{n}') for n in range(19)],collected=val('uints','awQCollect'),
            mower_level=val('ints','Upgrade31'),warehouse_level=val('ints','Upgrade32'),
            well_level=val('ints','Upgrade33'),tutorial=self.observer.step(),
            processed={n:val('booleans',f'tip{n}') for n in [40,41,42,44]},counters=self.counters())

    def wait(self,label,predicate,timeout=60):
        deadline=time.monotonic()+timeout
        while not predicate():
            self.probe.focus();self.probe.check_process()
            assert time.monotonic()<deadline,(label,'Actual game condition did not arrive',self.observer.step())
            time.sleep(.05)
        return self.capture(label)

    def pause(self,value):
        if self.paused==value:return
        self.shortcut(0xc5)
        if value:
            deadline=time.monotonic()+5;previous=None;stable=time.monotonic()
            while True:
                current=(self.observer.property('floats','GameTime'),self.counters()['elapsed'])
                if current!=previous:previous=current;stable=time.monotonic()
                if time.monotonic()-stable>=.35:break
                assert time.monotonic()<deadline,'Original pause did not settle';time.sleep(.05)
        self.paused=value
        self.session.setdefault('actual_pause_changes',[]).append(dict(paused=value,game_time=self.observer.property('floats','GameTime')))

    @contextmanager
    def open(self,kind,case,profiles):
        self.index=len(self.report['sessions']);shared=self.work/f'{self.index}-shared';common=shared/'VirtualFarm'
        common.mkdir(parents=True);profiles=Path(profiles).resolve(strict=True)
        shutil.copytree(profiles,common/'Profiles')
        settings=self.probe.resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0')
        (common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
        self.session=dict(kind=kind,case=case,seed_profile_directory=profiles.relative_to(ROOT).as_posix(),
            input_profile_files={p.name:digest(p) for p in profiles.iterdir() if p.is_file()},
            common_directory=common.relative_to(ROOT).as_posix(),actions=[])
        self.report['sessions'].append(self.session);launcher=None;observer=None;body_ok=False;hwnd=None
        if kind=='native':
            launcher=subprocess.Popen([str(self.launcher),str(self.data.parent/'Dobr\u00fd farm\u00e1\u0159.exe'),str(self.runtime),str(shared),'1800'],
                cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            line=launcher.stdout.readline();assert line,launcher.communicate();event=json.loads(line);assert event['event']=='ready'
            self.session['launcher']=event;process=NativeChild(event['pid'])
        else:
            process=subprocess.Popen([str(self.runtime/'FarmarTutorialWarnings.exe'),'--data',str(self.data),'--common-dir',str(common)],
                cwd=self.runtime,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        self.session['pid']=process.pid;self.probe.process=process;self.paused=False
        try:
            observer=TutorialMemory(process.pid,kind,ROOT);self.observer=observer
            deadline=time.monotonic()+20
            while hwnd is None:
                self.probe.check_process()
                windows=[row for row in self.probe.windows(process.pid) if row[2].lower().startswith('melesta_')]
                if windows:hwnd=W.HWND(windows[0][0]);break
                assert time.monotonic()<deadline;time.sleep(.1)
            self.hwnd=hwnd;self.probe.hwnd=hwnd
            while not u.IsWindowVisible(hwnd):
                self.probe.check_process();assert time.monotonic()<deadline;time.sleep(.05)
            self.probe.focus();time.sleep(1)
            origin=W.POINT();bounds=W.RECT();assert u.ClientToScreen(hwnd,C.byref(origin));assert u.GetWindowRect(hwnd,C.byref(bounds))
            assert u.SetWindowPos(hwnd,None,bounds.left-origin.x,bounds.top-origin.y,0,0,0x15)
            modules=self.probe.modules(process.pid);self.session['loaded_game_dlls']={}
            for name in ['jngload.dll','squall.dll']:
                path=Path(modules[name]).resolve(strict=True)
                assert path.parent==(self.data.parent if kind=='native' else self.runtime).resolve()
                self.session['loaded_game_dlls'][name]=dict(path=path.relative_to(ROOT.parent).as_posix(),sha256=digest(path))
            self.capture('logo')
            for _ in range(4):self.probe.scan(0x3c)
            deadline=time.monotonic()+120;last=-100;attempts=0
            while not observer.manager():
                frame=self.capture('startup-progress');self.probe.focus()
                assert time.monotonic()<deadline,'Actual main menu did not start game'
                if self.probe.roi(frame,(5,5,70,70))==self.menu_marker and time.monotonic()-last>=3:
                    if observer.manager():break
                    assert attempts<6,'Six actual Start inputs did not construct game'
                    self.click(*self.probe.button('ui/mainmenu.xml','StartButton'));attempts+=1;last=time.monotonic()
                else:time.sleep(.1)
            self.probe.wait_match(f'{self.index}-farm.png',(10,8,55,28),self.hud_marker)
            self.session['actions'].append('Actual Start restored the unmodified checkpoint and matched Menu HUD')
            yield self
            body_ok=True
        finally:
            if not body_ok and observer and process.poll() is None:
                try:
                    self.capture('failure');self.session['failure_live_state']=self.state()
                    self.session['failure_scene']=observer.visible_scene(hwnd) if hwnd and observer.manager() else None
                except Exception as error:self.session['failure_snapshot_error']=repr(error)
            if observer:observer.close()
            if process.poll() is None:
                for window,_,_ in self.probe.windows(process.pid):u.PostMessageW(W.HWND(window),0x10,0,0)
                process.wait(timeout=20)
            self.session['exit_code']=process.poll()
            if launcher:
                launcher.wait(timeout=10);out,err=launcher.communicate()
                self.session['launcher_completion']=out.strip();self.session['launcher_stderr']=err
                k.CloseHandle(process.handle)
            else:
                out,err=process.communicate();(self.work/f'{self.index}-stdout.log').write_bytes(out);(self.work/f'{self.index}-stderr.log').write_bytes(err)
            suffix='persisted' if body_ok else 'failure-persisted';shutil.copytree(common,self.work/f'{self.index}-{suffix}')
            self.session['persisted_profile_directory']=(self.work/f'{self.index}-{suffix}/Profiles').relative_to(ROOT).as_posix()
            if body_ok:
                assert self.session['exit_code']==0,self.session
                if launcher:assert json.loads(self.session['launcher_completion'])==dict(event='exit',exit_code=0,watchdog_triggered=False)
                self.session['status']='completed'
            self.unchanged()
