#!/usr/bin/env python3
"""Actually buy both remaining field expansions and verify all60 live activation flags and sprinkler coverage."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/farm-expansions-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/farm-expansions'/stamp;work.mkdir(parents=True)
shutil.copyfile(__file__,work/'driver.py')
from windows_game_gui import GameGui,u,k,INPUT,KEY
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
report=dict(status='running',work_directory=work.relative_to(r).as_posix(),sessions=[],reference_executable_sha256=digest(data.parent/'Dobrý farmář.exe'),source_file_snapshots={'tools/validate_game_farm_expansions.py':(work/'driver.py').relative_to(r).as_posix()},os_boundary_only=True,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),launcher_path=launcher_path.relative_to(r).as_posix(),launcher_sha256=digest(launcher_path),source_files={p.relative_to(r).as_posix():digest(p) for p in sorted((r/'src/game').rglob('*')) if p.is_file()},implementation_files={n:digest(runtime/n) for n in ['FarmarSourceInterop.exe','JNGLoad.dll','Squall.dll']})
for n in ['tools/validate_game_farm_expansions.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'analysis/contract-expiry-state-validation.json').read_text(encoding='utf-8'));assert fixture['status']=='passed_for_opposite_program_contract37_restore_natural_expiry_and_failure_save_only'
baseline=r/fixture['work_directory']/'0-persisted/Profiles'
field_layout_path=r/'build/live-farm-field-layout-vs/Release/farm-field-live-layout.exe'
field_layout=json.loads(subprocess.check_output([str(field_layout_path)]))
report['field_layout']=dict(executable_sha256=digest(field_layout_path),metadata=field_layout,source_sha256=digest(r/'tools/live_farm_field_layout.cpp'),cmake_sha256=digest(r/'tools/live_farm_field_layout/CMakeLists.txt'))
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
   def state():
    return dict(current_upgrade=val('ints','fcu'),available_upgrade=val('ints','fua'),tutorial=observer.step(),money=val('ints','Money'),held=val('uints','PCarry'),stock_roses=val('uints','WGoodCount4'),game_time=val('floats','GameTime'),run_time=val('floats','RunTime'),tasks=[dict(id=val('ints',f'task{n}'),remaining=val('floats',f'taskTime{n}'),pass_cancel=val('booleans',f'taskPassCancel{n}')) for n in range(3)],letters=[dict(id=val('ints',f'letter{n}'),remaining=val('floats',f'letTime{n}')) for n in range(3)],sprinklers=[val('booleans',f'SPExist{n}') for n in range(3)])
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
   initial=state();assert initial['money']==4504 and initial['current_upgrade']==1 and initial['available_upgrade']==3,initial
   session['initial_state']=initial;session['fixture_report_sha256']=digest(r/'analysis/contract-expiry-state-validation.json')
   farm_xml=resource('UI/gameUI/farm.xml');field_xml=list(farm_xml.find('FieldArray'))
   prices=[int(n.get('Price')) for n in farm_xml.find('UpgradePrices')];places=[tuple(map(float,n.get('Pos').split())) for n in farm_xml.find('FieldsBuyPlaces')]
   assert prices==[0,70,500,1000]
   map_rows=(r/'build/windows/Release/DobryFarmar.map').read_text(encoding='latin-1').splitlines()
   def preferred(part):
    rows=[line.split() for line in map_rows if part in line];assert len(rows)==1,(part,len(rows));return int(rows[0][2],16)
   if kind=='native':field_vtable=0x5f1870;offsets=dict(id=0x508,upgrade_index=0x50c,active=0x510)
   else:
    base=observer.singleton-preferred('3PAVTutorialManager@')+0x400000;field_vtable=base+preferred('??_7FarmField@')-0x400000;offsets=field_layout
   session['field_observer']=dict(vtable=field_vtable,offsets=offsets,native_constructor='0043c120',native_refresh_purchase='0043ba70')
   centers=[tuple(map(float,node.get('Center').split())) for node in field_xml]
   groups=[int(node.get('UpgradeIndex')) for node in field_xml]
   source_positions=[(400.,270.),(530.,260.),(390.,380.)]
   def fields():
    scene=observer.visible_scene(hwnd);rows=[]
    for control in scene:
     address=control['address']
     if observer.read(address,'<I')[0]!=field_vtable:continue
     index=observer.read(address+offsets['id'],'<I')[0];upgrade=observer.read(address+offsets['upgrade_index'],'<i')[0];active=bool(observer.read(address+offsets['active'],'<B')[0]);assert 0<=index<60
     assert tuple(control['geometry'][:2])==centers[index] and upgrade==groups[index],(index,control['geometry'],centers[index],upgrade,groups[index])
     rows.append(dict(id=index,upgrade=upgrade,active=active,position=centers[index],culture=val('uints',f'FFC{index}'),status=val('uints',f'FFS{index}'),water=val('floats',f'FFW{index}'),progress=val('floats',f'FFP{index}')))
    rows.sort(key=lambda row:row['id']);assert [row['id'] for row in rows]==list(range(60)),len(rows)
    coverage=[row['id'] for row in rows if row['active'] and any((row['position'][0]-x)**2+(row['position'][1]-y)**2<=10000 for x,y in source_positions)]
    overlays={name:sum((c['texture'] or '').lower()==name for c in scene) for name in ['farm/actionareas/field/field-1.jng','farm/actionareas/field/field-2.jng','ui/gameui/farm/up_field_button.jng']}
    return dict(fields=rows,coverage_ids=coverage,overlays=overlays)
   def checkpoint(label):
    at=state();snapshot=fields();current=at['current_upgrade'];assert [row['id'] for row in snapshot['fields'] if row['active']]==[i for i,g in enumerate(groups) if g<=current]
    session.setdefault('expansion_snapshots',[]).append(dict(label=label,state=at,**snapshot));capture(f'{cycle}-expansion-{label}.png');print(json.dumps(dict(kind=kind,label=label,money=at['money'],current=current,active=sum(row['active'] for row in snapshot['fields']),coverage=snapshot['coverage_ids'],overlays=snapshot['overlays'])),flush=True)
   while service_tip():pass
   click(hwnd,400,550);time.sleep(.4)
   checkpoint('initial-level1')
   for level in [2,3]:
    before=state()['money'];click(hwnd,*places[level]);wait_state(f'paid-expansion{level}',lambda:val('ints','fcu')==level and val('ints','Money')==before-prices[level],timeout=15);time.sleep(.4);probe.move(hwnd,760,490);checkpoint(f'paid-level{level}')
   current=fields();new_covered=[n for n in current['coverage_ids'] if groups[n]>1];assert new_covered
   wait_state('new-fields-receive-actual-sprinkler-water',lambda:all(val('floats',f'FFW{n}')>1 for n in new_covered),timeout=12)
   checkpoint('expanded-live-watering')
   wait_state('original-time-cutoff1400',lambda:val('floats','GameTime')>=1400,timeout=20)
   assert val('floats','GameTime')<1405 and val('floats','RunTime')<1800
   checkpoint('persisted-level3')
   session['final_state']=state();session['final_scene']=observer.visible_scene(hwnd);session['new_covered_field_ids']=new_covered
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
 report['status']='explored_actual_paid_farm_expansions_and_expanded_sprinkler_watering_only'
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
