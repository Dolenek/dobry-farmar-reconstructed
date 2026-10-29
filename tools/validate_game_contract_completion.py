#!/usr/bin/env python3
"""Natural first contract fulfillment through real watering, harvesting and warehouse input."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/contract-completion-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/contract-completion'/stamp;work.mkdir(parents=True)
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
for n in ['tools/validate_game_contract_completion.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'.local/contract-runtime-validation.json').read_text(encoding='utf-8'));assert fixture['status'].startswith('passed_for_natural_first_contract_acceptance')
baseline=r/fixture['work_directory']/'0-persisted/Profiles'
from PIL import Image
prior=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
expected_hud=roi(Image.open(r/prior['work_directory']/'0-04-game.png'),(10,8,55,28))
templates={}
u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]
def observe(step,name,style=None):
 filename=f'{cycle}-{name}.png';deadline=time.monotonic()+60
 while observer.step()!=step:
  check_process()
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
 for cycle,(kind,variant,seed_from) in enumerate([('native','fulfill',None),('source','fulfill',None),('source','restore',0),('native','restore',1),('native','restore',2)]):
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
   assert hwnd;probe.hwnd=hwnd;focus();time.sleep(1)
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
   if variant=='fulfill':
    val=observer.property
    def state():
     return dict(tutorial=observer.step(),money=val('ints','Money'),held=val('uints','PCarry'),carried=val('uints','PCC0'),stock=val('uints','WGoodCount0'),task=val('ints','task0'),remaining=val('floats','taskTime0'),game_time=val('floats','GameTime'))
    def wait_state(label,test,timeout=35):
     deadline=time.monotonic()+timeout
     while True:
      focus();check_process();at=state()
      if test():break
      if at['tutorial'] not in [-1,None]:
       capture(f'{cycle}-unexpected-{label}.png');raise RuntimeError(('Unexpected natural tutorial',label,at))
      if time.monotonic()>=deadline:
       capture(f'{cycle}-timeout-{label}.png');raise RuntimeError(('Work timeout',label,at))
      time.sleep(.1)
     capture(f'{cycle}-work-{label}.png')
     session.setdefault('work_steps',[]).append(dict(label=label,state=at));print(json.dumps(dict(kind=kind,label=label,state=at)),flush=True)
    initial=state();assert initial['money']==2 and initial['held']==6 and initial['task']==3,initial
    session['initial_state']=initial
    click(hwnd,710,90)
    wait_state('filled-bucket',lambda:val('uints','PCarry')==1 and val('floats','WaterLevel')==480)
    for field,x,y in [(25,433,270),(26,485,270),(33,375,320)]:
     click(hwnd,x,y)
     wait_state(f'water-{field}',lambda f=field:val('floats',f'FFW{f}')>118)
    click(hwnd,736,407);wait_state('mower',lambda:val('uints','PCarry')==6)
    for count,(field,x,y) in enumerate([(24,375,270),(25,433,270),(26,485,270)],1):
     wait_state(f'ready-{field}',lambda f=field:val('uints',f'FFS{f}')==1,timeout=45)
     click(hwnd,x,y);wait_state(f'harvest-{field}',lambda n=count:val('uints','PCC0')==n)
    click(hwnd,120,100);wait_state('stored-three',lambda:val('uints','WGoodCount0')==3 and val('uints','PCC0')==0)
    wait_state('ready-33',lambda:val('uints','FFS33')==1,timeout=45)
    click(hwnd,375,320);wait_state('harvest-fourth',lambda:val('uints','PCC0')==1)
    click(hwnd,120,100);wait_state('stored-four',lambda:val('uints','WGoodCount0')==4 and val('uints','PCC0')==0)
    time.sleep(1.5);confirm=capture(f'{cycle}-11-confirm-completion.png')
    session['confirmation_region_sha256']=hashlib.sha256(roi(confirm,(210,170,590,405))).hexdigest()
    click(hwnd,*button('gameplay/contractpassconfirm.xml','OkButton'))
    wait_state('contract-completed',lambda:val('ints','task0')==-1 and val('ints','Money')==62 and val('uints','WGoodCount0')==0)
    session['final_state']=state();capture(f'{cycle}-12-completed.png')
   else:
    def key(code,down):
     focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
    key(0x38,True);key(0xc5,True);key(0xc5,False);key(0x38,False)
    time.sleep(.2);capture(f'{cycle}-10-restored-completed-contract.png')
    session['actions'].append('Restored naturally completed contract and pending next letter; actual Alt+Pause and normal close')
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
 def props(index):
  return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(work/f'{index}-persisted/Profiles/00000001.xml').getroot()}
 def saved(index):return (work/f'{index}-persisted/Profiles/00000001.save').read_bytes()
 left,right=props(0),props(1);assert len(left)==793 and set(left)==set(right)
 varying={'GameTime','RunTime','letTime0','taskTime0'}|{f'{prefix}{n}' for prefix in ['FFP','FFW'] for n in [24,25,26,33]}
 changed={name:[left[name],right[name]] for name in left if left[name]!=right[name]}
 assert set(changed)<=varying,changed
 for index in range(5):
  values=props(index)
  expected={'Money':('Int','62'),'awQTask':('UInt','1'),'awQCollect':('UInt','5'),'task0':('Int','-1'),'WGoodCount0':('UInt','0'),'PCC0':('UInt','0'),'PCarry':('UInt','6'),'req0_0':('Int','0'),'req0_1':('Int','0'),'letter0':('Int','4')}
  for name,value in expected.items():assert values[name]==value,(index,name,values[name],value)
  for field in [24,25,26,33]:assert values[f'FFC{field}']==('UInt','0') and values[f'FFS{field}']==('UInt','0'),(index,field)
 assert saved(0)==saved(1),'Independent completed contract binary saves differ'
 assert report['sessions'][0]['confirmation_region_sha256']==report['sessions'][1]['confirmation_region_sha256'],'Completion dialog pixels differ'
 transfers=[]
 for before,after in [(0,2),(1,3),(2,4)]:
  l,rr=props(before),props(after);assert set(l)==set(rr) and all(l[n][0]==rr[n][0] for n in l)
  delta={name:[l[name],rr[name]] for name in l if l[name]!=rr[name]};assert set(delta)<=varying,(before,after,delta)
  assert saved(before)==saved(after),('Complete save transfer differs',before,after)
  transfers.append(dict(before=before,after=after,profile_properties=len(l),changed_properties=delta,identical_binary_save_bytes=len(saved(after))))
 report['comparison']=dict(initial_profile_properties=793,initial_changed_properties=changed,identical_independent_save_bytes=len(saved(0)),transfer_comparisons=transfers,harvested_for_contract=4,warehouse_capacity_cycle=[3,0,1,0],paid_bonus=60,final_money=62,completed_tasks=1,pending_next_letter_event=4,exact_completion_dialog_rgb_pixels=380*235,original_profile_fields_modified_by_probe=False)
 report['status']='passed_for_natural_first_contract_fulfillment_and_bidirectional_saves_only'
 report['pending']=['Other contracts, expiry, failures, variable requirements, repeated task histories','Next seed/farm/production tutorial and all gameplay/engine/audio scopes']
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
