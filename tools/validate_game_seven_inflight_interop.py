#!/usr/bin/env python3
"""Restore and optionally complete fourteen inputs across seven real cowsheds."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path)
parser.add_argument('--crash-trace',type=Path,help='Attach source-only read-only exception diagnostic before Start')
parser.add_argument('--fixture-report',type=Path,default=r/'.local/seven-inflight-state-validation.json')
parser.add_argument('--suite',choices=['seven-inflight-interop','seven-completion'],default='seven-inflight-interop')
parser.add_argument('--complete',action='store_true')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
if args.complete:args.suite='seven-completion'
if args.crash_trace:args.crash_trace=args.crash_trace.resolve(strict=True)
if args.output is None:args.output=r/'.local'/f'{args.suite}-validation.json'
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local'/args.suite/stamp;work.mkdir(parents=True)
shutil.copyfile(__file__,work/'driver.py')
from windows_game_gui import GameGui,u,k,INPUT,KEY
from windows_game_memory import TutorialMemory
probe=GameGui(work,data)
button=probe.button
capture=probe.capture
check_process=probe.check_process
def click(hwnd,x,y):
 probe.move(hwnd,x,y)
 if observer.step() in [1,2,3,4,5]:
  info=observer.anchor_info(x,y);session['actions'].append(info);print(json.dumps(info),flush=True)
 probe.click(hwnd,x,y)
errors=probe.errors
focus=probe.focus
modules=probe.modules
resource=probe.resource
roi=probe.roi
scan=probe.scan
screenshot=probe.screenshot
settle=probe.settle
wait_match=probe.wait_match
windows=probe.windows
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
inventory=json.loads((r/'analysis/inventory.json').read_text(encoding='utf-8'))
def unchanged():
 for row in inventory['files']:assert digest(data.parent/row['path'])==row['sha256'],row['path']
unchanged();runtime=work/'runtime';runtime.mkdir();shutil.copytree(data,runtime/'Data')
for a,b in [('Dobrý farmář.exe','FarmarSourceInterop.exe'),('JNGLoad.dll','JNGLoad.dll'),('Squall.dll','Squall.dll')]:shutil.copy2(r/'build/windows/Release'/a,runtime/b)
k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE;k.CloseHandle.argtypes=[W.HANDLE];k.GetExitCodeProcess.argtypes=[W.HANDLE,C.POINTER(W.DWORD)]
class Child:
 def __init__(self,pid):self.pid=pid;self.handle=k.OpenProcess(0x100000|0x1000,False,pid);assert self.handle
 def poll(self):
  code=W.DWORD();assert k.GetExitCodeProcess(self.handle,C.byref(code));return None if code.value==259 else code.value
 def wait(self,timeout=20):
  end=time.monotonic()+timeout
  while time.monotonic()<end:
   code=self.poll()
   if code is not None:return code
   time.sleep(.05)
  raise RuntimeError('Owned native process did not exit')
report=dict(status='running',work_directory=work.relative_to(r).as_posix(),sessions=[],reference_executable_sha256=digest(data.parent/'Dobrý farmář.exe'),os_boundary_only=True,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),launcher_path=launcher_path.relative_to(r).as_posix(),launcher_sha256=digest(launcher_path),source_files={p.relative_to(r).as_posix():digest(p) for p in sorted((r/'src/game').rglob('*')) if p.is_file()},implementation_files={n:digest(runtime/n) for n in ['FarmarSourceInterop.exe','JNGLoad.dll','Squall.dll']})
for n in ['tools/validate_game_seven_inflight_interop.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Restore only unmodified profiles produced by actual maximum-capacity production play.
fixture=json.loads((args.fixture_report).read_text(encoding='utf-8'));assert fixture['status'].startswith('passed_for_actual_seven_simultaneous_inflight_buildings')
fixture_work=r/fixture['work_directory']
from game_save_state import load_save
import struct
from PIL import Image
prior=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
expected_hud=roi(Image.open(r/prior['work_directory']/'0-04-game.png'),(10,8,55,28))
u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]

try:
 for cycle,(kind,variant,seed_from) in enumerate(([('source','restore',0),('native','restore',1)] if args.complete else [('source','restore',0),('native','restore',1),('native','restore',2),('source','restore',4)]),start=2):
  name=None;shared=work/f'{cycle}-shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
  seed_profiles=(fixture_work if seed_from<2 else work)/f'{seed_from}-persisted/Profiles'
  shutil.copytree(seed_profiles,common/'Profiles')
  settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
  session=dict(kind=kind,variant=variant,seed_from=seed_from,common_directory=common.relative_to(r).as_posix(),actions=[]);report['sessions'].append(session);launcher=None
  if kind=='native':
   launcher=subprocess.Popen([str(launcher_path),str(data.parent/'Dobrý farmář.exe'),str(runtime),str(shared)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
   line=launcher.stdout.readline();assert line,('Native launcher failed',launcher.communicate());event=json.loads(line);assert event['event']=='ready',event;session['launcher']=event;p=Child(event['pid'])
  else:p=subprocess.Popen([str(runtime/'FarmarSourceInterop.exe'),'--data',str(data),'--common-dir',str(common)],cwd=runtime,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  probe.process=p;observer=TutorialMemory(p.pid,kind,r);trace=None
  try:
   deadline=time.monotonic()+20;hwnd=None
   while time.monotonic()<deadline:
    check_process();found=[row for row in windows(p.pid) if row[2].lower().startswith('melesta_')]
    if found:hwnd=W.HWND(found[0][0]);break
    time.sleep(.1)
   assert hwnd;probe.hwnd=hwnd
   u.IsWindowVisible.argtypes=[W.HWND];u.IsWindowVisible.restype=W.BOOL
   ready=time.monotonic()+20
   while not u.IsWindowVisible(hwnd):
    check_process();assert time.monotonic()<ready,'Owned game window never became visible';time.sleep(.05)
   focus();time.sleep(1)
   origin=W.POINT();u.ClientToScreen(hwnd,C.byref(origin));bounds=W.RECT();u.GetWindowRect(hwnd,C.byref(bounds));session['desktop']=[u.GetSystemMetrics(0),u.GetSystemMetrics(1)]
   assert min(session['desktop'])>=600 and session['desktop'][0]>=800
   assert u.SetWindowPos(hwnd,None,bounds.left-origin.x,bounds.top-origin.y,0,0,0x15)
   loaded=modules(p.pid);session['loaded_game_dlls']={}
   for n in ['jngload.dll','squall.dll']:
    path=Path(loaded[n]).resolve(strict=True);assert path.parent==(data.parent if kind=='native' else runtime).resolve(),(n,str(path));session['loaded_game_dlls'][n]=dict(path=path.relative_to(r.parent).as_posix(),sha256=digest(path))
   capture(f'{cycle}-01-logo.png')
   for _ in range(4):scan(0x3c)
   time.sleep(2);capture(f'{cycle}-02-startup.png')
   settle(f'{cycle}-03-menu.png');attach_start=time.monotonic()
   if kind=='source' and args.crash_trace:
    trace_file=(work/f'{cycle}-crash-trace.jsonl').open('w',encoding='utf-8')
    trace=subprocess.Popen([str(args.crash_trace.resolve(strict=True)),str(p.pid),str(runtime/'FarmarSourceInterop.exe')],cwd=r,stdout=subprocess.PIPE,stderr=trace_file,text=True)
    while True:
     trace_ready=trace.stdout.readline();trace_file.write(trace_ready);trace_file.flush();assert trace_ready,'Debugger exited before ready'
     if json.loads(trace_ready)['event']=='ready':break
    # Pipe remaining output directly to the diagnostic artifact without blocking UI.
    import threading
    def copy_trace():
     for line in trace.stdout:trace_file.write(line);trace_file.flush()
    trace_reader=threading.Thread(target=copy_trace,daemon=True);trace_reader.start()
    session['debugger_attach_seconds']=time.monotonic()-attach_start
   start=time.monotonic()
   # Native menu can already be visible while its input transition finishes.
   # Retry only until the actual game manager is constructed; never click a
   # farm coordinate merely because a previous fixed delay elapsed.
   while not observer.manager():
    check_process();assert time.monotonic()-start<60,'Main menu Start did not construct game within60s after debugger readiness'
    click(hwnd,*button('ui/mainmenu.xml','StartButton'));time.sleep(.5)
   if expected_hud is None:
    deadline=start+60
    while time.monotonic()<deadline and not list(common.rglob('*.save')):check_process();time.sleep(.2)
    assert list(common.rglob('*.save')),'Native game did not create save';time.sleep(1);game=capture(f'{cycle}-04-game.png');expected_hud=roi(game,(10,8,55,28));assert len(set(expected_hud))>10
   else:game=wait_match(f'{cycle}-04-game.png',(10,8,55,28),expected_hud)
   session['loading_seconds']=time.monotonic()-start;session['actions'].append('Created/restored full game and matched native Menu HUD pixels')
   clock_before=[repr(observer.property('floats',f'MSellTime{i}')) for i in range(19)]
   deadline=time.monotonic()+8
   while not all(observer.property('floats',f'MSellTime{i}')==float('inf') for i in range(19)):
    focus();check_process();assert time.monotonic()<deadline,('Unsold grass clock did not recompute',clock_before)
    time.sleep(.05)
   session['clock_after_restore']=dict(first_observed=clock_before,recomputed=[repr(observer.property('floats',f'MSellTime{i}')) for i in range(19)])
   def key(code,down):
    focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
   value=observer.property
   def multi():return dict(counts=[value('uints',f'ProdCount{n}') for n in range(7)],progress=[value('floats',f'ProdPct{n}') for n in range(7)],stopped=[value('uints',f'ProdStop{n}') for n in range(7)],milk=value('uints','WGoodCount10'),processed=value('uints','awQProcess'),warehouse=sum(value('uints',f'WGoodCount{n}') for n in range(19)),game_time=value('floats','GameTime'))
   session['first_observed_production']=multi()
   if args.complete:
    deadline=time.monotonic()+45;previous=None
    while True:
     focus();check_process();at=multi()
     if at['counts']!=previous:
      session.setdefault('completion_observations',[]).append(at);previous=at['counts']
     if at['counts']==[0]*7 and at['processed']==20 and at['milk']==20:break
     assert time.monotonic()<deadline,('Fourteen restored units did not finish',at);time.sleep(.03)
    while value('floats','GameTime')<1130:
     focus();check_process();assert time.monotonic()<deadline,'Post-completion canonical cutoff1130 timed out';time.sleep(.04)
    assert value('floats','GameTime')<1135
    session['actions'].append('All fourteen restored inputs really produced fourteen milk units across seven simultaneously active cowsheds at1x')
   else:
    key(0x38,True);key(0xc5,True);key(0xc5,False);key(0x38,False)
    session['actions'].append('Actual Alt+Pause after restoring seven nonzero progress values and fourteen inputs')
   time.sleep(.2);capture(f'{cycle}-10-restored-seven-inflight.png')
   session['final_production_state']=multi()
   u.PostMessageW(hwnd,0x10,0,0);assert p.wait(timeout=20)==0
   shutil.copytree(common,work/f'{cycle}-persisted')
   session['exit_code']=p.poll();session['status']='completed'
   if launcher:
    launcher.wait(timeout=10);out,err=launcher.communicate();session['launcher_completion']=out.strip();assert launcher.returncode==0,(launcher.returncode,err)
   else:
    out,err=p.communicate();(work/f'{cycle}-stdout.log').write_bytes(out);(work/f'{cycle}-stderr.log').write_bytes(err)
  finally:
   observer.close()
   if p.poll() is None:
    u.PostMessageW(hwnd,0x10,0,0);p.wait(timeout=15)
   if kind=='native':k.CloseHandle(p.handle)
   if trace is not None:
    assert trace.wait(timeout=10)==0
    trace_reader.join(timeout=5);assert not trace_reader.is_alive();trace_file.close()
 report['fixture_report_sha256']=digest(args.fixture_report)
 report['completed_remaining_units']=args.complete
 report['status']='explored_actual_seven_restored_completion_only' if args.complete else 'explored_actual_seven_inflight_bidirectional_restores_only'
except Exception as e:
 report['status']='failed';report['error']=repr(e)
 import traceback;report['traceback']=traceback.format_exc()
finally:
 unchanged();report['reference_files_unchanged']=17
 report['whole_game_complete']=False
 report['screenshots']={p.name:digest(p) for p in work.glob('*.png')}
 encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
 print(json.dumps(dict(status=report['status'],work_directory=str(work),error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
