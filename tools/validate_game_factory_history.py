#!/usr/bin/env python3
"""Build every remaining factory kind in one actual persistent profile and earn natural award8."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/factory-history-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/factory-history'/stamp;work.mkdir(parents=True)
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
for n in ['tools/validate_game_factory_history.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'analysis/squall-channel-vectors-game-interop-validation.json').read_text(encoding='utf-8'));assert fixture['status']=='passed_for_seven_simultaneous_buildings_bidirectional_saves_only'
baseline=r/fixture['work_directory']/'3-persisted/Profiles'
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
    if step in [11,19,24,25,26,27,28,29,46,47]:
     at=observe(step,f'auto-{step}','Arrow');click(hwnd,*at);return True
    if step==39:
     observe(39,'auto-journal');click(hwnd,200,18);time.sleep(1)
     capture(f'{cycle}-journal.png');click(hwnd,*button('ui/gameui/journal.xml','OkButton'));return True
    return False
   def state():
    return dict(tutorial=observer.step(),money=val('ints','Money'),held=val('uints','PCarry'),carried=val('uints','PCC0'),stock=val('uints','WGoodCount0'),milk=val('uints','WGoodCount10'),building_level=val('ints','Upgrade0'),building_loaded=val('uints','ProdCount0'),building_progress=val('floats','ProdPct0'),processed=val('uints','awQProcess'),task=val('ints','task0'),task_remaining=val('floats','taskTime0'),seed_available=val('uints','SeedAvail1'),game_time=val('floats','GameTime'),crop=val('uints','FFC34'),field_state=val('uints','FFS34'),water=val('floats','FFW34'),fertility=val('floats','FFF34'),crop_progress=val('floats','FFP34'),farm_level=val('ints','fcu'),farm_available=val('ints','fua'))
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
   session['initial_state']=state()
   def command(text):
    for ch in text:
     focus();assert u.PostMessageW(hwnd,0x102,ord(ch),0);time.sleep(.02)
    time.sleep(.2);session['actions'].append('Actual InputDevices/GameInput WM_CHAR command: '+text)
   def service_award():
    scene=observer.visible_scene(hwnd)
    if any((row['texture'] or '').lower()=='ui/gameui/avard_new.jng' for row in scene):
     panel=next(row for row in scene if (row['texture'] or '').lower()=='ui/gameui/avard_new.jng')
     deadline=time.monotonic()+12
     while True:
      alpha=observer.read(observer.read(panel['address']+observer.offsets['alpha'],'<I')[0],'<f')[0]
      transition=observer.read(panel['address']+observer.offsets['control_state'],'<ffI')
      if alpha>=.999 and transition[2]==0:break
      focus();check_process();assert time.monotonic()<deadline,('Award panel did not finish appearing',alpha,transition);time.sleep(.05)
     session.setdefault('award_panel_states',[]).append(dict(alpha=alpha,transition=transition))
     ordinal=len(session.get('awards',[]))
     capture(f'{cycle}-award-{ordinal}-root-transition-finished.png')
     # MultilineText creates its dirty lines in enabledStep. Root transition0
     # can be observed before that next normal dispatch; observe the lines,
     # rather than treating root opacity alone as completed text rendering.
     deadline=time.monotonic()+6
     while True:
      text_scene=observer.visible_scene(hwnd)
      # Root XML is loaded after the child XML, adding its 3100 depth to
      # the child's 3300. Native-created late frames contain 79 glyphs on
      # four rows at depths 6400..6400.26; the OK label starts at 6401.
      lines=[row for row in text_scene if (row['texture'] or '').lower()=='ui/menu_font.dds' and 6400<=row['deep']<6401 and 340<=row['geometry'][1]<=480]
      if len(lines)==79 and len({row['geometry'][1] for row in lines})==4:
       session.setdefault('award_visible_congratulation_lines',[]).append(lines);time.sleep(.5);break
      focus();check_process()
      if time.monotonic()>=deadline:
       session['late_award_scene']=text_scene;capture(f'{cycle}-award-text-still-empty.png');raise RuntimeError('Award congratulation did not create visible text lines after transition')
      time.sleep(.05)
     capture(f'{cycle}-award-{len(session.get("awards",[]))}.png');time.sleep(.5)
     click(hwnd,*button('ui/awardpanel.xml','OkButton'));time.sleep(1)
     session.setdefault('awards',[]).append('Actual natural award panel and OK; remain for queued next award');return True
    if val('booleans','awardShow8') and any((row['texture'] or '').lower()=='ui/gameui/main_menu_button.jng' and abs(row['geometry'][1]-576)<1 and row['deep']==3080 for row in scene):
     click(hwnd,*button('ui/awards.xml','ReturnButton'));time.sleep(1)
     session['actions'].append('Return after natural all-factory-kind history award8 completed');return True
    return False
   def wait_for(label,predicate,timeout=35):
    deadline=time.monotonic()+timeout
    while True:
     focus();check_process()
     if service_award() or service_tip():deadline=time.monotonic()+timeout;continue
     if predicate():break
     if time.monotonic()>deadline:
      session['timeout_scene']=observer.visible_scene(hwnd);capture(f'{cycle}-timeout-{label}.png');raise RuntimeError(('Catalog timeout',label,state()))
     time.sleep(.15)
    at=dict(money=val('ints','Money'),levels=[val('ints',f'Upgrade{n}') for n in range(20)],types=[val('uints',f'ProdType{n}') for n in range(7)],loaded=[val('uints',f'ProdCount{n}') for n in range(7)],history=[val('booleans',f'awManDepotBuild{n}') for n in range(11)],destroyed=val('uints','awQDestroy'),award=val('booleans','Award08BuildAllManufactureDepots'),award_shown=val('booleans','awardShow8'),game_time=val('floats','GameTime'))
    session.setdefault('catalog_steps',[]).append(dict(label=label,state=at));print(json.dumps(dict(kind=kind,label=label,state=at)),flush=True);capture(f'{cycle}-{label}.png');return at
   assert val('ints','Money')==7084 and all(val('ints',f'Upgrade{n}')==0 and val('uints',f'ProdType{n}')==2 and val('uints',f'ProdCount{n}')==0 for n in range(7))
   assert [bool(val('booleans',f'awManDepotBuild{n}')) for n in range(11)]==[True]*4+[False]*7
   session['initial_destroyed']=val('uints','awQDestroy');session['initial_money']=val('ints','Money')
   session['fixture_report_sha256']=digest(r/'analysis/squall-channel-vectors-game-interop-validation.json')
   definitions=['SheepFold','HenHouse','CowShed','PigPen','CheeseDairy','Creamery','Weavery','Incubator','TextileDep','Bakery','Mill']
   pictures=set()
   for name in definitions:
    xml=resource(f'Manufacture/{name}.xml');assert list(map(float,xml.get('size').split()))==[200.,200.]
    for level in xml.find('Upgrades'):
     pictures.add(level.get('Picture').lower())
     if level.find('Hovered') is not None:pictures.add(level.find('Hovered').get('Picture',level.get('Picture')).lower())
   session['all_purchase_alias_words']=[]
   import struct
   def snapshot_aliases():
    scene=observer.visible_scene(hwnd)
    houses=[row for row in scene if (row['texture'] or '').lower() in pictures and row['geometry'][2:]==[200.0,200.0]]
    assert len(houses)==7,(building,houses)
    words=[];expected={}
    for house in houses:
     offset=0x24 if kind=='native' else observer.layout['control_state']+12
     property_id,limit=observer.read(house['address']+offset,'<Ii');key='Upgrade'+str(property_id)
     try:previous=val('ints',key)
     except KeyError:previous=0
     current=expected.get(key,previous)
     expected[key]=current if current<limit else struct.unpack('<i',struct.pack('<I',(limit-1)&0xffffffff))[0]
     words.append(dict(position=house['geometry'][:2],property_id=property_id,level_count=limit,previous=previous))
    return words,expected
   click(hwnd,500,550);time.sleep(.8)
   places=[tuple(map(float,row.get('place').split())) for row in resource('UI/gameUI/BuyBuilding.xml').find('ItemPlace')]
   costs=[int(list(row)[0].text) for row in resource('Game.xml').find('Costs')][:11]
   session['costs']=costs;paid=0
   for index,building in enumerate(range(4,11)):
    click(hwnd,482,414);time.sleep(.3)
    before_destroyed=val('uints','awQDestroy')
    click(hwnd,*button('ui/gameui/manufacture.xml','DestroyButton'))
    removed=wait_for(f'before-kind-{building}-destroyed',lambda:val('ints','Upgrade6')==-1)
    assert removed['money']==7084-paid and removed['destroyed']==before_destroyed+1
    time.sleep(.2)
    click(hwnd,*button('ui/gameui/manufacture.xml','BuildButton'));time.sleep(.5)
    click(hwnd,*places[building]);time.sleep(.3);probe.move(hwnd,482,414)
    words,expected=snapshot_aliases()
    for flags in [2,4]:
     focus();event=INPUT();event.type=0;event.mouse=MOUSE(0,0,0,flags,0,0)
     assert u.SendInput(1,C.byref(event),C.sizeof(event))==1;time.sleep(.08)
    actual={key:val('ints',key) for key in expected};assert actual==expected,(words,expected,actual)
    session['all_purchase_alias_words'].append(dict(kind=building,before=words,expected=expected,actual=actual))
    placed=wait_for(f'kind-{building}-built',lambda:val('ints','Upgrade6')==0 and val('uints','ProdType6')==building)
    paid+=costs[building];assert placed['money']==7084-paid and placed['history']==[True]*(building+1)+[False]*(10-building)
    assert placed['loaded']==[0]*7 and placed['levels']==[0]*7+[-1]*13
   wait_for('natural-all-factory-kind-history-award',lambda:val('booleans','Award08BuildAllManufactureDepots') and val('booleans','awardShow8'))
   wait_for('original-time-cutoff1150',lambda:val('floats','GameTime')>=1150,timeout=90)
   assert val('floats','GameTime')<1155
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
 report['status']='explored_actual_all_factory_kinds_one_profile_and_natural_award8_only'
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
