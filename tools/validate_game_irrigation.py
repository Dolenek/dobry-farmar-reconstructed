#!/usr/bin/env python3
"""Buy three actual sprinklers, observe field water, reject invalid drag and restore placement."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/irrigation-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/irrigation'/stamp;work.mkdir(parents=True)
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
report=dict(status='running',work_directory=work.relative_to(r).as_posix(),sessions=[],reference_executable_sha256=digest(data.parent/'Dobrý farmář.exe'),source_file_snapshots={'tools/validate_game_irrigation.py':(work/'driver.py').relative_to(r).as_posix()},os_boundary_only=True,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),launcher_path=launcher_path.relative_to(r).as_posix(),launcher_sha256=digest(launcher_path),source_files={p.relative_to(r).as_posix():digest(p) for p in sorted((r/'src/game').rglob('*')) if p.is_file()},implementation_files={n:digest(runtime/n) for n in ['FarmarSourceInterop.exe','JNGLoad.dll','Squall.dll']})
for n in ['tools/validate_game_irrigation.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'analysis/tool-upgrades-state-validation.json').read_text(encoding='utf-8'));assert fixture['status']=='passed_for_actual_paid_all_farm_tool_upgrades_and_natural_award7_only'
baseline=r/fixture['work_directory']/'0-persisted/Profiles'
from PIL import Image
prior=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
expected_hud=roi(Image.open(r/prior['work_directory']/'0-04-game.png'),(10,8,55,28))
templates={}
u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]
def observe(step,name,style=None):
 filename=f'{cycle}-{name}.png';deadline=time.monotonic()+60
 while observer.step()!=step:
  focus();check_process()
  if observer.step() in [11,19,24,25,26,27,28,39] and observer.step()!=step:
   service_tip();deadline=time.monotonic()+60;continue
  if time.monotonic()>=deadline:
   capture(f'{cycle}-timeout-{step}.png');raise RuntimeError(('Tutorial step timeout',step,observer.step()))
  time.sleep(.1)
 time.sleep(1.5);image=capture(filename)
 style=style or 'Action'
 controls=observer.snapshot(style);geometry=controls['text']['geometry'];ok_at=[int(v) for v in controls['ok']['geometry'][:2]]
 x,y,width,height=geometry;half=min(130,int(width/2)-10);rect=[int(x)-half,int(y)-55,int(x)+half,int(y)+55]
 row=dict(step=step,name=name,text_geometry=geometry,ok_at=ok_at,rect=rect,screenshot=filename,controls=controls)
 if kind=='native':templates[step]=row
 else:
  import numpy as np
  reference=templates[step];rect=reference['rect'];left=np.array(Image.open(work/reference['screenshot']).crop(rect)).astype(int);right=np.array(image.crop(rect)).astype(int);delta=np.abs(left-right)
  row['pixel_comparison']=dict(rect=rect,pixels=left.shape[0]*left.shape[1],different_pixels=int(np.any(delta!=0,axis=2).sum()),max_channel_difference=int(delta.max()))
 session.setdefault('stages',[]).append(row);print(json.dumps(dict(kind=kind,step=step,pixel_comparison=row.get('pixel_comparison'))),flush=True)
 return ok_at

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
   if expected_hud is None:
    deadline=start+60
    while time.monotonic()<deadline and not list(common.rglob('*.save')):check_process();time.sleep(.2)
    assert list(common.rglob('*.save')),'Native game did not create save';time.sleep(1);game=capture(f'{cycle}-04-game.png');expected_hud=roi(game,(10,8,55,28));assert len(set(expected_hud))>10
   else:game=wait_match(f'{cycle}-04-game.png',(10,8,55,28),expected_hud)
   session['loading_seconds']=time.monotonic()-start;session['actions'].append('Created/restored full game and matched native Menu HUD pixels')
   val=observer.property
   def service_tip():
    step=observer.step()
    if step in [11,19,24,25,26,27,28,29,46,47,49]:
     at=observe(step,f'auto-{step}','Arrow');click(hwnd,*at);return True
    if step==39:
     observe(39,'auto-journal');click(hwnd,200,18);time.sleep(1)
     capture(f'{cycle}-journal.png');click(hwnd,*button('ui/gameui/journal.xml','OkButton'));return True
    return False
   positions=[tuple(map(float,row.get('Center').split())) for row in resource('UI/gameUI/farm.xml').find('FieldArray')]
   active=[n for n,row in enumerate(resource('UI/gameUI/farm.xml').find('FieldArray')) if int(row.get('UpgradeIndex'))<=val('ints','fcu')]
   def state():
    return dict(tutorial=observer.step(),money=val('ints','Money'),held=val('uints','PCarry'),carried=[val('uints',f'PCC{n}') for n in range(8)],game_time=val('floats','GameTime'),run_time=val('floats','RunTime'),sprinklers=[dict(id=n,exists=val('booleans',f'SPExist{n}'),status=val('uints',f'SPStatus{n}'),x=val('floats',f'SPXPos{n}'),y=val('floats',f'SPYPos{n}')) for n in range(3)],fields=[dict(id=f,crop=val('uints',f'FFC{f}'),status=val('uints',f'FFS{f}'),water=val('floats',f'FFW{f}'),fertility=val('floats',f'FFF{f}'),progress=val('floats',f'FFP{f}')) for f in active])
   def wait_state(label,test,timeout=60):
    deadline=time.monotonic()+timeout
    while True:
     focus();check_process();at=state()
     if test():at=state();break
     if service_tip():deadline=time.monotonic()+timeout;continue
     if at['tutorial'] not in [-1,None]:
      capture(f'{cycle}-unexpected-{label}.png');raise RuntimeError(('Unexpected natural tutorial',label,at))
     if time.monotonic()>=deadline:
      capture(f'{cycle}-timeout-{label}.png');raise RuntimeError(('Work timeout',label,at))
     time.sleep(.1)
    capture(f'{cycle}-work-{label}.png');session.setdefault('work_steps',[]).append(dict(label=label,state=at));print(json.dumps(dict(kind=kind,label=label,state=at)),flush=True)
   initial=state();assert initial['money']==5134 and val('ints','Upgrade33')==2 and not any(row['exists'] for row in initial['sprinklers']),initial
   session['initial_state']=initial;session['fixture_report_sha256']=digest(r/'analysis/tool-upgrades-state-validation.json')
   while service_tip():pass
   click(hwnd,400,550);time.sleep(1)
   while service_tip():pass
   targets=[(400,270),(530,260),(390,380)]
   session['targets']=targets;session['active_fields']=active;session['affected_fields']=[]
   for index,(x,y) in enumerate(targets):
    cash=val('ints','Money');click(hwnd,720,150)
    wait_state(f'sprinkler-{index}-purchase-preview',lambda n=index:val('uints',f'SPStatus{n}')==2 and not val('booleans',f'SPExist{n}'),timeout=10)
    probe.move(hwnd,x,y);time.sleep(.2);click(hwnd,x,y)
    wait_state(f'sprinkler-{index}-paid-installed',lambda n=index:val('uints',f'SPStatus{n}')==0 and val('booleans',f'SPExist{n}'),timeout=10)
    assert val('ints','Money')==cash-200
    affected=[n for n in active if (positions[n][0]-x)**2+(positions[n][1]-y)**2<=100**2]
    session['affected_fields'].append(affected)
    wait_state(f'sprinkler-{index}-watered-fields',lambda a=affected:all(val('floats',f'FFW{n}')>118 for n in a),timeout=15)
   assert val('ints','Money')==4534
   scene=observer.visible_scene(hwnd)
   water_buttons=[row for row in scene if (row['texture'] or '').lower().startswith('farm/actionareas/sprinkle/water_button_')]
   assert not water_buttons,water_buttons
   click(hwnd,720,150);time.sleep(.5)
   wait_state('fourth-purchase-ui-hidden',lambda:val('ints','Money')==4534 and all(val('booleans',f'SPExist{n}') for n in range(3)))
   def mouse(flags):
    focus();item=INPUT();item.type=0;item.mouse=MOUSE(0,0,0,flags,0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.15)
   probe.move(hwnd,400,270);mouse(2);probe.move(hwnd,120,100)
   wait_state('installed-invalid-drag-preview',lambda:val('uints','SPStatus0')==2)
   session['invalid_preview_scene']=observer.visible_scene(hwnd);mouse(4)
   wait_state('invalid-release-stays-moving',lambda:val('uints','SPStatus0')==2 and abs(val('floats','SPXPos0')-400)<.01)
   probe.move(hwnd,130,280);click(hwnd,130,280)
   wait_state('valid-click-drop-empty-radius',lambda:val('uints','SPStatus0')==0 and abs(val('floats','SPXPos0')-130)<.01)
   time.sleep(7)
   wait_state('old-radius-water-decays',lambda:val('floats','FFW14')<119)
   probe.move(hwnd,130,280);mouse(2);probe.move(hwnd,400,270);time.sleep(.25);mouse(4)
   wait_state('valid-drag-release-restores-radius',lambda:val('uints','SPStatus0')==0 and abs(val('floats','SPXPos0')-400)<.01)
   union=sorted(set().union(*(set(row) for row in session['affected_fields'])))
   wait_state('all-active-fields-automatically-watered',lambda:all(val('floats',f'FFW{n}')>118 for n in union),timeout=15)
   assert val('ints','Money')==4534
   wait_state('original-time-cutoff1070',lambda:val('floats','GameTime')>=1070,timeout=90)
   assert val('floats','GameTime')<1075
   session['final_state']=state();session['final_scene']=observer.visible_scene(hwnd);capture(f'{cycle}-three-paid-sprinklers.png')
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
 report['status']='explored_actual_three_sprinklers_purchase_watering_invalid_drag_and_restore_only'
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
