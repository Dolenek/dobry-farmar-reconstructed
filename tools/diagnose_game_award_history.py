#!/usr/bin/env python3
"""Observe pending original award8 glyph geometry, depth and pixels in both complete processes."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/award-history-visual-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/award-history-visual'/stamp;work.mkdir(parents=True)
shutil.copyfile(__file__,work/'driver.py')
from windows_game_gui import GameGui,u,k,INPUT,KEY,MOUSE
from windows_game_memory import TutorialMemory
probe=GameGui(work,data)
button=probe.button
capture=probe.capture
check_process=probe.check_process
def click(hwnd,x,y):
 x,y=round(x),round(y)
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
for n in ['tools/diagnose_game_award_history.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
baseline=r/'.local/factory-history/20261031T112724410306Z/0-shared/VirtualFarm/Profiles'
from PIL import Image
prior=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
expected_hud=roi(Image.open(r/prior['work_directory']/'0-04-game.png'),(10,8,55,28))
u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]

try:
 for cycle,(kind,variant,seed_from) in enumerate([('native','seeds',None),('source','seeds',None)]):
  name=None;shared=work/f'{cycle}-shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
  seed_profiles=baseline if seed_from is None else work/f'{seed_from}-persisted/Profiles'
  shutil.copytree(seed_profiles,common/'Profiles')
  settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
  session=dict(kind=kind,variant=variant,seed_from=seed_from,common_directory=common.relative_to(r).as_posix(),actions=[]);report['sessions'].append(session);launcher=None
  if kind=='native':
   launcher=subprocess.Popen([str(launcher_path),str(data.parent/'Dobrý farmář.exe'),str(runtime),str(shared)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
   line=launcher.stdout.readline();assert line,('Native launcher failed',launcher.communicate());event=json.loads(line);assert event['event']=='ready',event;session['launcher']=event;p=Child(event['pid'])
  else:p=subprocess.Popen([str(runtime/'FarmarSourceInterop.exe'),'--data',str(data),'--common-dir',str(common)],cwd=runtime,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  probe.process=p;observer=TutorialMemory(p.pid,kind,r)
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
   settle(f'{cycle}-03-menu.png');start=time.monotonic()
   # Native menu can already be visible while its input transition finishes.
   # Retry only until the actual game manager is constructed; never click a
   # farm coordinate merely because a previous fixed delay elapsed.
   while not observer.manager():
    assert time.monotonic()-start<20,'Main menu Start did not construct game'
    click(hwnd,*button('ui/mainmenu.xml','StartButton'));time.sleep(.5)
   time.sleep(.3);game=capture(f'{cycle}-04-created.png')
   session['loading_seconds']=time.monotonic()-start;session['actions'].append('Restored native-created pending award profile; wait for actual automatic award panel')
   val=observer.property
   def state():
    return dict(tutorial=observer.step(),money=val('ints','Money'),held=val('uints','PCarry'),carried=val('uints','PCC0'),stock=val('uints','WGoodCount0'),milk=val('uints','WGoodCount10'),building_level=val('ints','Upgrade0'),building_loaded=val('uints','ProdCount0'),building_progress=val('floats','ProdPct0'),processed=val('uints','awQProcess'),task=val('ints','task0'),task_remaining=val('floats','taskTime0'),seed_available=val('uints','SeedAvail1'),game_time=val('floats','GameTime'),crop=val('uints','FFC34'),field_state=val('uints','FFS34'),water=val('floats','FFW34'),fertility=val('floats','FFF34'),crop_progress=val('floats','FFP34'),farm_level=val('ints','fcu'),farm_available=val('ints','fua'))
   session['initial_state']=state()
   assert val('ints','Money')==5234 and val('booleans','awManDepotBuild10') and not val('booleans','Award08BuildAllManufactureDepots')
   deadline=time.monotonic()+20
   while True:
    focus();check_process();scene=observer.visible_scene(hwnd)
    panels=[row for row in scene if (row['texture'] or '').lower()=='ui/gameui/avard_new.jng']
    if panels:
     panel=panels[0];alpha=observer.read(observer.read(panel['address']+observer.offsets['alpha'],'<I')[0],'<f')[0];transition=observer.read(panel['address']+observer.offsets['control_state'],'<ffI')
     if alpha>=.999 and transition[2]==0:break
    assert time.monotonic()<deadline,'Pending native-created award did not show';time.sleep(.05)
   session['visual_samples']=[]
   for index in range(4):
    time.sleep(.5);scene=observer.visible_scene(hwnd)
    glyphs=[row for row in scene if (row['texture'] or '').lower()=='ui/menu_font.dds' and row['deep']>3100 and 340<=row['geometry'][1]<=510]
    for row in glyphs:
     texture=observer.read(row['address']+observer.offsets['texture'],'<I')[0]
     row['texture_resource']=texture;row['texture_gpu_and_metrics']=observer.read(texture+0x24,'<9I')
     alpha=observer.read(row['address']+observer.offsets['alpha'],'<I')[0]
     row['alpha_pointer']=alpha;row['alpha_value']=observer.read(alpha,'<f')[0]
     view_flags=0x24 if kind=='native' else observer.layout['control_state']+12
     row['view_boolean_bytes']=observer.read(row['address']+view_flags,'<6B')
    session['visual_samples'].append(dict(index=index,panel=panel,glyphs=glyphs))
    if kind=='native':
     owner=W.DWORD();thread_id=u.GetWindowThreadProcessId(hwnd,C.byref(owner));assert owner.value==p.pid
     thread=k.OpenThread(2,False,thread_id);assert thread
     try:
      assert k.SuspendThread(thread)!=0xffffffff
      graphics=observer.read(0x65ff8c,'<I')[0];head,count=observer.read(graphics+0x180,'<II');node=observer.read(head,'<I')[0]
      batches=[];visited=set()
      while node!=head:
       assert node not in visited and len(visited)<2048;visited.add(node)
       batches.append(observer.read(node+8,'<9I'));node=observer.read(node,'<I')[0]
      assert len(batches)==count
      session['visual_samples'][-1]['native_graphics']=dict(address=graphics,batches=batches,current=observer.read(graphics+0x188,'<9I'))
     finally:
      k.ResumeThread(thread);k.CloseHandle(thread)
    capture(f'{cycle}-award-observation-{index}.png')
   session['final_scene']=observer.visible_scene(hwnd)
   session['final_state']=state()
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
 report['status']='observed_actual_pending_award8_visuals_only'
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
