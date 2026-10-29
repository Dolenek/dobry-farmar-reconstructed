#!/usr/bin/env python3
"""Natural first contract continuation in complete native/source processes; diagnostic only."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/contract-runtime-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/contract-runtime'/stamp;work.mkdir(parents=True)
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
for n in ['tools/validate_game_contract_runtime.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'.local/tutorial-runtime-validation.json').read_text(encoding='utf-8'));assert fixture['status'].startswith('passed_for_opening_natural_tutorial')
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
 for cycle,(kind,variant,seed_from) in enumerate([('native','accept',None),('source','accept',None),('source','restore',0),('native','restore',1),('native','restore',2)]):
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
   if variant=='accept':
    # A letter becomes tutorial17 naturally after its120s lifetime falls
    # below90. No profile clocks, availability or task flags are injected.
    observe(17,'10-first-letter');click(hwnd,600,570)
    time.sleep(1.5);offer=capture(f'{cycle}-11-offer.png')
    session['offer_region_sha256']=hashlib.sha256(roi(offer,(210,170,590,405))).hexdigest()
    click(hwnd,*button('gameplay/contractprovide.xml','YesButton'))
    observe(18,'12-contract-goal');click(hwnd,704,18)
    time.sleep(1.5);aim=capture(f'{cycle}-13-requirements.png')
    session['aim_region_sha256']=hashlib.sha256(roi(aim,(210,170,590,405))).hexdigest()
    # Keep the actual requirements dialog open while normally closing the
    # application. Actual Game persistence records the accepted contract.
   else:
    # Real cLogic shortcut freezes stepping while normal WM_CLOSE still owns
    # persistence. No save/profile field or original instruction is patched.
    def key(code,down):
     focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
    key(0x38,True);key(0xc5,True);key(0xc5,False);key(0x38,False)
    time.sleep(.2);capture(f'{cycle}-10-restored-contract.png')
    session['actions'].append('Restored naturally accepted contract, actual Alt+Pause and normal close')
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
 left,right=props(0),props(1);assert len(left)==791 and set(left)==set(right)
 changed={name:[left[name],right[name]] for name in left if left[name]!=right[name]}
 assert set(changed)<={'FFP24','FFW24','GameTime','RunTime','letTime0'},changed
 for index in range(5):
  values=props(index)
  for name,value in [('Money',('Int','2')),('fua',('Int','1')),('task0',('Int','3')),('letter0',('Int','-1')),('tip17',('Bool','1')),('tip18',('Bool','1')),('taskPassCancel0',('Bool','1'))]:
   assert values[name]==value,(index,name,values[name],value)
  assert values['taskTime0'][0]=='Float' and 118<=float(values['taskTime0'][1])<=120,(index,values['taskTime0'])
 # Independent naturally timed runs have one identical grass sale (amount2)
 # with slightly different sale/cutoff clocks. Only these exact float fields
 # are excluded from the initial pair's raw comparison; all remaining bytes,
 # UTF16 messages and all383 coefficient bits must match.
 import struct
 a,b=bytearray(saved(0)),bytearray(saved(1));assert len(a)==len(b)==2186
 for data_bytes in [a,b]:
  assert struct.unpack_from('<II',data_bytes)==(0,1)
  assert struct.unpack_from('<I',data_bytes,16)[0]==2
 report['independent_sale_clocks']=[dict(cutoff=struct.unpack_from('<f',x,8)[0],sale_time=struct.unpack_from('<f',x,12)[0]) for x in [a,b]]
 a[8:16]=b'\0'*8;b[8:16]=b'\0'*8;assert a==b,'Natural contract meaningful binary state differs'
 comparisons=[]
 for before,after in [(0,2),(1,3),(2,4)]:
  a,b=saved(before),saved(after);assert a==b,('Full contract save transfer differs',before,after)
  l,rr=props(before),props(after);assert set(l)==set(rr) and all(l[n][0]==rr[n][0] for n in l)
  delta={name:[l[name],rr[name]] for name in l if l[name]!=rr[name]}
  assert set(delta)<={'FFP24','FFW24','GameTime','RunTime','taskTime0'},(before,after,delta)
  comparisons.append(dict(before=before,after=after,profile_properties=791,changed_properties=delta,identical_binary_save_bytes=len(a)))
 assert report['sessions'][0]['offer_region_sha256']==report['sessions'][1]['offer_region_sha256'],'Contract offer pixels differ'
 assert report['sessions'][0]['aim_region_sha256']==report['sessions'][1]['aim_region_sha256'],'Contract requirements pixels differ'
 report['comparison']=dict(initial_profile_properties=791,initial_changed_properties=changed,initial_meaningful_save_bytes_equal=2186,excluded_independent_clock_bytes=[8,16],transfer_comparisons=comparisons,natural_task_id=3,remaining_task_seconds=120,money_after_actual_deposit=2,original_profile_fields_modified_by_probe=False)
 report['status']='passed_for_natural_first_contract_acceptance_and_bidirectional_save_interchange_only'
 report['scope']=['Natural letter countdown, tutorial17/18, actual offer acceptance and deposit','Exact static offer/requirements pixel regions and both tutorial text regions','Native→source→native and source→native full2186-byte saves without normalization in transfers']
 report['pending']=['Natural completion/expiry of contracts, expanded and long states','All other tutorial/farming/production/engine/audio scopes']
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
