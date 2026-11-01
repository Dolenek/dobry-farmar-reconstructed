#!/usr/bin/env python3
"""Follow the actual world timeline and observe the first three natural crop-destruction events."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/field-disasters-validation.json')
parser.add_argument('--resume-report',type=Path,help='Retain only completed, normally exited segments from this exact failed diagnostic report')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/field-disasters'/stamp;work.mkdir(parents=True)
shutil.copyfile(__file__,work/'driver.py')
from windows_game_gui import GameGui,u,k,INPUT,KEY
from windows_game_memory import TutorialMemory
from stable_live_scene import read_stable_scene
probe=GameGui(work,data)
button=probe.button
capture=probe.capture
check_process=probe.check_process
def click(hwnd,x,y):
 x,y=round(x),round(y)
 for attempt in range(4):
  try:
   probe.move(hwnd,x,y)
   if observer.step() in [1,2,3,4,5]:
    info=observer.anchor_info(x,y);session['actions'].append(info);print(json.dumps(info),flush=True)
   probe.click(hwnd,x,y);return
  except AssertionError as error:
   detail=error.args[0] if error.args else None
   if not isinstance(detail,tuple) or not detail or detail[0]!='Injected pointer missed owned window target':raise
   report.setdefault('diagnostic_pointer_retries',[]).append(dict(kind=kind,pid=p.pid,attempt=attempt+1,actual=detail[1],expected=detail[2],mouse_button_injected=False))
   if attempt==3:raise
   time.sleep(.1)

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
report=dict(status='running',work_directory=work.relative_to(r).as_posix(),sessions=[],reference_executable_sha256=digest(data.parent/'Dobrý farmář.exe'),source_file_snapshots={'tools/validate_game_field_disasters.py':(work/'driver.py').relative_to(r).as_posix()},os_boundary_only=True,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),launcher_path=launcher_path.relative_to(r).as_posix(),launcher_sha256=digest(launcher_path),source_files={p.relative_to(r).as_posix():digest(p) for p in sorted((r/'src/game').rglob('*')) if p.is_file()},implementation_files={n:digest(runtime/n) for n in ['FarmarSourceInterop.exe','JNGLoad.dll','Squall.dll']})
report['diagnostic_tree_retries']=[]
report['diagnostic_foreground_retries']=[]
report['source_files']['tools/stable_live_scene.py']=digest(r/'tools/stable_live_scene.py')
def safe_scene():return read_stable_scene(observer,hwnd,report['diagnostic_tree_retries'])
def polling_focus():
 try:
  if u.IsIconic(hwnd):u.ShowWindow(hwnd,9)
  focus();return True
 except AssertionError as error:
  if str(error)!='Source application lost foreground; no global input was injected':raise
  report['diagnostic_foreground_retries'].append(dict(kind=kind,pid=p.pid,foreground=int(u.GetForegroundWindow() or 0),global_input_injected=False))
  return False
for n in ['tools/validate_game_field_disasters.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'analysis/farm-expansions-state-validation.json').read_text(encoding='utf-8'));assert fixture['status']=='passed_for_actual_paid_two_farm_expansions_sixty_active_fields_and_thirty_watered_fields_only'
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

resume_count=0
if args.resume_report:
 prior_path=args.resume_report.resolve(strict=True);prior=json.loads(prior_path.read_text(encoding='utf-8'));prior_work=r/prior['work_directory']
 assert prior['status']=='failed' and len(prior['sessions'])==4
 assert prior['sessions'][3].get('status')!='completed' and 'Injected pointer missed owned window target' in prior['error']
 for name,value in prior['source_files'].items():
  if name!='tools/validate_game_field_disasters.py':assert digest(r/name)==value,name
 assert digest(prior_work/'driver.py')==prior['source_files']['tools/validate_game_field_disasters.py']
 assert prior['implementation_files']==report['implementation_files']
 for index,old_session in enumerate(prior['sessions'][:3]):
  assert old_session['status']=='completed' and old_session['exit_code']==0
  shutil.copytree(prior_work/f'{index}-persisted',work/f'{index}-persisted')
  for screenshot_path in prior_work.glob(f'{index}-*.png'):
   assert digest(screenshot_path)==prior['screenshots'][screenshot_path.name];shutil.copy2(screenshot_path,work/screenshot_path.name)
  old_session['retained_from_report']=prior_path.relative_to(r).as_posix();report['sessions'].append(old_session)
 report['retained_completed_segments']=dict(report_path=prior_path.relative_to(r).as_posix(),report_sha256=digest(prior_path),driver_snapshot_sha256=digest(prior_work/'driver.py'),completed_cycles=[0,1,2],original_diagnostic_error=prior['error'],old_production_and_helper_hashes_verified=True)
 resume_count=3

try:
 for cycle,(kind,variant,seed_from) in enumerate([('native','prefix',None),('source','prefix',None),('native','disasters',0),('source','disasters',1)]):
  if cycle<resume_count:continue
  name=None;shared=work/f'{cycle}-shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
  seed_profiles=(r/fixture['work_directory']/f'{1-cycle}-persisted/Profiles') if variant=='prefix' else work/f'{seed_from}-persisted/Profiles'
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
     if not polling_focus():
      assert time.monotonic()<deadline,('Foreground was unavailable throughout diagnostic deadline',label)
      time.sleep(.1);continue
     check_process();at=state()
     if test():at=state();break
     if service_award() or service_tip():deadline=time.monotonic()+timeout;continue
     if at['tutorial'] not in [-1,None]:
      capture(f'{cycle}-unexpected-{label}.png');raise RuntimeError(('Unexpected natural tutorial',label,at))
     if time.monotonic()>=deadline:
      capture(f'{cycle}-timeout-{label}.png');raise RuntimeError(('Work timeout',label,at))
     time.sleep(.1)
    capture(f'{cycle}-work-{label}.png');session.setdefault('work_steps',[]).append(dict(label=label,state=at));print(json.dumps(dict(kind=kind,label=label,state=at)),flush=True)
   initial=state();assert initial['current_upgrade']==3 and initial['available_upgrade']==3,initial
   if variant=='prefix':assert initial['money']==3004,initial
   else:
    assert initial['money']==report['sessions'][seed_from]['final_state']['money'] and 3550<=initial['game_time']<3580 and val('booleans','awardShow13'),initial
    session['award_returned_to_game']=True
    session['restart_clock_begin']=dict(wall=time.monotonic(),game_time=initial['game_time'])
   session['initial_state']=initial;session['fixture_report_sha256']=digest(r/'analysis/farm-expansions-state-validation.json');session['restore_from_kind']=('source' if kind=='native' else 'native') if variant=='prefix' else kind;session['seed_profile_hashes']={name:digest(seed_profiles/name) for name in ['00000001.xml','00000001.save']}
   def key(code,down):
    focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
   def fast():
    key(0x38,True);key(0xd2,True);key(0xd2,False);key(0x38,False)
   session['awards']=[]
   def shows():return [bool(val('booleans',f'awardShow{n}')) for n in range(21)]
   def service_award():
    if session.get('award_returned_to_game'):return False
    scene=safe_scene()
    panels=[row for row in scene if (row['texture'] or '').lower()=='ui/gameui/avard_new.jng']
    if panels:
     panel=panels[0];deadline=time.monotonic()+12
     while True:
      alpha=observer.read(observer.read(panel['address']+observer.offsets['alpha'],'<I')[0],'<f')[0]
      transition=observer.read(panel['address']+observer.offsets['control_state'],'<ffI')
      glyphs=[row for row in safe_scene() if (row['texture'] or '').lower()=='ui/menu_font.dds' and 6400<=row['deep']<6401 and 340<=row['geometry'][1]<=480]
      if alpha>=.999 and transition[2]==0 and glyphs:break
      focus();check_process();assert time.monotonic()<deadline,'Automatic award panel/glyph readiness';time.sleep(.05)
     time.sleep(.5);ordinal=len(session['awards']);capture(f'{cycle}-award-{ordinal}.png');before=shows()
     click(hwnd,*button('ui/awardpanel.xml','OkButton'))
     session['awards'].append(dict(before=before,game_time=val('floats','GameTime'),run_time=val('floats','RunTime'),grow_mask=val('ints','awGrowType'),glyphs=glyphs))
     time.sleep(.5);return True
    expected=[]
    if val('floats','RunTime')>=1800:expected.append(13)
    if val('ints','awGrowType')==255:expected.append(11)
    buttons=[row for row in scene if (row['texture'] or '').lower()=='ui/gameui/main_menu_button.jng' and abs(row['geometry'][1]-576)<1 and row['deep']==3080]
    if expected and all(val('booleans',f'awardShow{n}') for n in expected) and buttons:
     offset=0x24 if kind=='native' else observer.layout['control_state']+12
     flags=observer.read(buttons[0]['address']+offset,'<6B')
     if flags[2] and flags[3]:
      click(hwnd,*button('ui/awards.xml','ReturnButton'));time.sleep(.5);session['actions'].append('Actual Return after all currently satisfied natural awards completed');session['award_returned_to_game']=True;return True
    return False
   from live_farm_fields import read_live_farm_fields
   report['source_files']['tools/live_farm_fields.py']=digest(r/'tools/live_farm_fields.py')
   layout_log=r/'.local/live-farm-world-layout-build.log';layout_text=layout_log.read_text(encoding='utf-8',errors='replace')
   import re
   match=re.search(r'(\d+)\s*\| harvested',layout_text);assert match and int(match[1])==2056
   session['harvested_metadata']=dict(native_offset=0x944,source_offset=int(match[1]),compiler_layout_sha256=digest(layout_log),reference_guard_file='tools/farm_field_oracle_cases.inc',reference_guard_sha256=digest(r/'tools/farm_field_oracle_cases.inc'))
   map_lines=(r/'build/windows/Release/DobryFarmar.map').read_text(encoding='latin-1').splitlines()
   def preferred(part):
    rows=[line.split() for line in map_lines if part in line];assert len(rows)==1;return int(rows[0][2],16)
   if kind=='native':vtable=0x5f1870;identity=0x508;harvested_offset=0x944
   else:
    base=observer.singleton-preferred('3PAVTutorialManager@')+0x400000;vtable=base+preferred('??_7FarmField@')-0x400000;identity=field_layout['id'];harvested_offset=2056
   scene=safe_scene();field_rows=read_live_farm_fields(observer,r,field_layout,scene);assert all(f['active'] for f in field_rows)
   crop_ids=[f['id'] for f in field_rows if f['culture']!=0];assert crop_ids==[14,15,16,17,18,23,27,34]
   addresses={}
   for c in scene:
    if observer.read(c['address'],'<I')[0]==vtable:addresses[observer.read(c['address']+identity,'<I')[0]]=c['address']
   assert sorted(addresses)==list(range(60))
   def crops():
    return [dict(id=n,culture=val('uints',f'FFC{n}'),status=val('uints',f'FFS{n}'),water=val('floats',f'FFW{n}'),progress=val('floats',f'FFP{n}'),marked=bool(observer.read(addresses[n]+harvested_offset,'<B')[0])) for n in crop_ids]
   session['original_crop_ids']=crop_ids;session['timeline']=[]
   while service_tip():pass
   click(hwnd,400,550);time.sleep(.5)
   if variant=='prefix':
    fast();session['actions'].append('Actual original Alt+Insert15x through the later world timeline; no profile/time/RNG writes')
    for cutoff in [1462,1820,2120,2670,3150,3550]:
     wait_state(f'actual-timeline{cutoff}',lambda cutoff=cutoff:val('floats','GameTime')>=cutoff,timeout=65)
     session['timeline'].append(dict(target=cutoff,state=state(),crops=crops(),market_prices=[val('ints',f'MEffPrice{n}') for n in range(19)]))
    session['actions'].append('Ordinary close/save before the destruction window; the next full process restores default1x')
   else:
    wait_state('before-natural-destruction',lambda:val('floats','GameTime')>=3584,timeout=45)
    before=crops();assert all(row['culture']!=0 and row['status']==1 and row['water']>118 and not row['marked'] for row in before),before
    session['before_disasters']=dict(state=state(),crops=before)
    session['event_windows']=[];previous=None;deadline=time.monotonic()+20
    while val('floats','GameTime')<3589:
     assert time.monotonic()<deadline
     if not polling_focus():time.sleep(.1);continue
     check_process()
     at=state();rows=crops();signature=[(r['culture'],r['status'],r['marked']) for r in rows]
     if signature!=previous:
      row=dict(state=at,crops=rows);session['event_windows'].append(row);print(json.dumps(dict(kind=kind,destruction_change=row)),flush=True);previous=signature
     if service_tip():deadline=time.monotonic()+20
     time.sleep(.025)
    session['after_disasters']=dict(state=state(),crops=crops());capture(f'{cycle}-after-three-natural-destruction-events.png')
    wait_state('original-time-cutoff3600',lambda:val('floats','GameTime')>=3600,timeout=25)
    assert val('floats','GameTime')<3605 and val('booleans','awardShow13')
    session['restart_clock_end']=dict(wall=time.monotonic(),game_time=val('floats','GameTime'))
    assert (session['restart_clock_end']['game_time']-session['restart_clock_begin']['game_time'])/(session['restart_clock_end']['wall']-session['restart_clock_begin']['wall'])<2
   session['final_fields']=read_live_farm_fields(observer,r,field_layout,safe_scene());session['final_state']=state();session['final_scene']=safe_scene();capture(f'{cycle}-persisted-after-field-disasters.png')
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
 report['status']='explored_natural_world_progression_and_first_three_crop_destruction_events_only'
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
