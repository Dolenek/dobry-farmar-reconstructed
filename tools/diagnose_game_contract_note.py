#!/usr/bin/env python3
"""Read live original/source contract note and letter state without changing implementation."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/contract-note-diagnostic-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/contract-note-diagnostic'/stamp;work.mkdir(parents=True)
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
report=dict(status='running',work_directory=work.relative_to(r).as_posix(),sessions=[],reference_executable_sha256=digest(data.parent/'Dobrý farmář.exe'),source_file_snapshots={'tools/diagnose_game_contract_note.py':(work/'driver.py').relative_to(r).as_posix()},os_boundary_only=True,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),launcher_path=launcher_path.relative_to(r).as_posix(),launcher_sha256=digest(launcher_path),source_files={p.relative_to(r).as_posix():digest(p) for p in sorted((r/'src/game').rglob('*')) if p.is_file()},implementation_files={n:digest(runtime/n) for n in ['FarmarSourceInterop.exe','JNGLoad.dll','Squall.dll']})
for n in ['tools/diagnose_game_contract_note.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'analysis/irrigation-interop-validation.json').read_text(encoding='utf-8'));assert fixture['status']=='passed_for_three_sprinklers_and_live_watering_bidirectional_saves_only'
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
    if step in [11,19,24,25,26,27,28,29,46,47,49]:
     at=observe(step,f'auto-{step}','Arrow');click(hwnd,*at);return True
    if step==39:
     observe(39,'auto-journal');click(hwnd,200,18);time.sleep(1)
     capture(f'{cycle}-journal.png');click(hwnd,*button('ui/gameui/journal.xml','OkButton'));return True
    return False
   def state():
    return dict(tutorial=observer.step(),money=val('ints','Money'),held=val('uints','PCarry'),stock_roses=val('uints','WGoodCount4'),game_time=val('floats','GameTime'),run_time=val('floats','RunTime'),tasks=[dict(id=val('ints',f'task{n}'),remaining=val('floats',f'taskTime{n}'),pass_cancel=val('booleans',f'taskPassCancel{n}')) for n in range(3)],letters=[dict(id=val('ints',f'letter{n}'),remaining=val('floats',f'letTime{n}')) for n in range(3)],sprinklers=[val('booleans',f'SPExist{n}') for n in range(3)])
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
   initial=state();assert initial['money']==4534 and initial['stock_roses']==2 and all(task['id']==-1 for task in initial['tasks']),initial
   session['initial_state']=initial;session['fixture_report_sha256']=digest(r/'analysis/irrigation-interop-validation.json')
   while service_tip():pass
   click(hwnd,400,550);time.sleep(1)
   def key(code,down):
    focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
   def shortcut(code):
    key(0x38,True);key(code,True);key(code,False);key(0x38,False)
   shortcut(0xd2);session['actions'].append('Actual original Alt+Insert15x until natural rose offer1110')
   wait_state('natural-rose-contract37-arrived',lambda:any(val('ints',f'letter{n}')==37 for n in range(3)),timeout=20)
   shortcut(0xd2);session['actions'].append('Actual original Alt+Insert1x for offer UI')
   slot=next(n for n in range(3) if val('ints',f'letter{n}')==37);session['offer_slot']=slot
   places=[tuple(map(float,row.get('Pos').split())) for row in resource('UI/gameUI/farm.xml').find('LetterPlace')]
   from game_save_state import load_save
   original_save=load_save(baseline/'00000001.save');assert original_save['coefficient_bits'][37] in [0,0x80000000]
   events=list(resource('Gameplay/gameplay.xml').find('Events'));events.sort(key=lambda e:sum(int(v)*scale for v,scale in zip(e.get('Time').split(':'),[3600,60,1])))
   event=events[37];assert event.get('Time')=='0:18:30' and event.get('Type')=='5'
   definition=event.find('Data');assert definition.get('Penalty')=='30' and definition.get('WinTime')=='0:04:00'
   requirement=list(definition.find('Requirements'))[0];assert requirement.get('ProductType')=='4' and requirement.get('Count')=='4'
   session['contract_definition']=dict(index=37,penalty=30,bonus=90,win_time_seconds=240,product=4,count=4,initial_stock=2,coefficient_bits=original_save['coefficient_bits'][37])
   click(hwnd,*places[slot]);time.sleep(1.5)
   if kind=='native':provide=observer.read(0x65f760,'<I')[0];note=observer.read(provide+0x38,'<I')[0]
   else:
    map_rows=(r/'build/windows/Release/DobryFarmar.map').read_text().splitlines()
    def symbol(part):
     found=[row.split() for row in map_rows if part in row];assert len(found)==1,(part,len(found));return int(found[0][2],16)
    base=observer.singleton-(symbol('3PAVTutorialManager@')-0x400000)
    provide=observer.read(base+symbol('?currentProvide@')-0x400000,'<I')[0];note=observer.read(provide+68,'<I')[0]
   def wstring(address):
    raw=observer.read(address,'<7I');length,capacity=raw[5:];assert length<10000
    pointer=raw[1] if capacity>=8 else address+4
    return observer.read(pointer,'<'+str(length*2)+'s')[0].decode('utf-16-le') if length else ''
   def control(address):
    alpha=observer.read(address+observer.offsets['alpha'],'<I')[0]
    return dict(address=address,geometry=observer.read(address+observer.offsets['geometry'],'<ffff'),deep=observer.read(address+observer.offsets['deep'],'<f')[0] if kind=='source' else observer.read(address+0x228,'<f')[0],view=observer.read(address+observer.offsets['control_state'],'<ffI'),view_flags=observer.read(address+(0x24 if kind=='native' else observer.offsets['control_state']+12),'<6B'),alpha=observer.read(alpha,'<f')[0],texture=observer.read(address+observer.offsets['texture'],'<I')[0],sprite=observer.read(address+observer.offsets['sprite'],'<19I'))
   session['samples']=[]
   for index in range(4):
    time.sleep(.5);scene=observer.visible_scene(hwnd)
    row=dict(index=index,state=state(),provide_address=provide,note=control(note),glyphs=[row for row in scene if (row['texture'] or '').lower()=='ui/menu_font.dds' and 90<=row['geometry'][1]<=170 and row['deep']>=2000])
    if kind=='source':
     row['note'].update(text=wstring(note+404),flags=observer.read(note+432,'<3B'),lines=[])
     begin,end,capacity=observer.read(note+392,'<III');assert end>=begin and (end-begin)%4==0 and end-begin<400
     for line in observer.read(begin,'<'+str((end-begin)//4)+'I') if end>begin else []:
      current=control(line);current.update(text=wstring(line+392),text_dirty=observer.read(line+488,'<B')[0],dirty=observer.read(line+212,'<B')[0],metrics=observer.read(line+500,'<ff'))
      lb,le,lc=observer.read(line+420,'<III');assert le>=lb and le-lb<40000
      current['letter_count']=(le-lb)//4;current['first_letters']=[control(address) for address in observer.read(lb,'<'+str(min(3,(le-lb)//4))+'I')] if le>lb else []
      row['note']['lines'].append(current)
    for glyph in row['glyphs']:
     texture=observer.read(glyph['address']+observer.offsets['texture'],'<I')[0];glyph['resource']=texture;glyph['gpu_metrics']=observer.read(texture+0x24,'<9I')
    owner=W.DWORD();thread_id=u.GetWindowThreadProcessId(hwnd,C.byref(owner));assert owner.value==p.pid
    thread=k.OpenThread(2,False,thread_id);assert thread
    try:
     assert k.SuspendThread(thread)!=0xffffffff
     if kind=='native':
      graphics=observer.read(0x65ff8c,'<I')[0];head,count=observer.read(graphics+0x180,'<II');node=observer.read(head,'<I')[0];batches=[];visited=set()
      while node!=head:
       assert node not in visited and len(visited)<2048;visited.add(node);batches.append(observer.read(node+8,'<9I'));node=observer.read(node,'<I')[0]
      assert len(batches)==count;row['graphics']=dict(address=graphics,batches=batches,current=observer.read(graphics+0x188,'<9I'))
     else:
      runtime=observer.read(base+symbol('?active@Runtime@GameApplication')-0x400000,'<I')[0];batch=observer.read(runtime+72,'<I')[0]
      begin,end,capacity=observer.read(batch+56,'<III');assert end>=begin and (end-begin)%32==0 and end-begin<100000
      row['graphics']=dict(runtime=runtime,address=batch,active=observer.read(batch+12,'<B')[0],vertex_count=observer.read(batch+16,'<I')[0],index_count=observer.read(batch+20,'<I')[0],batches=[observer.read(at,'<8I') for at in range(begin,end,32)],current=observer.read(batch+24,'<8I'))
      buffers=observer.read(runtime+68,'<I')[0];row['graphics']['buffer_capacities']=observer.read(buffers+16,'<III')
      if not row['graphics']['active'] and row['graphics']['vertex_count']>2100:
       vertices,indices=observer.read(batch+4,'<II')
       try:
        row['graphics']['note_vertex_window']=[observer.read(vertices+at*24,'<fffIff') for at in range(1788,2048)]
        row['graphics']['note_index_window']=observer.read(indices+2690*2,'<400H')
       except Exception as error:row['graphics']['buffer_read_error']=repr(error)

    finally:
     k.ResumeThread(thread);k.CloseHandle(thread)
    session['samples'].append(row);capture(f'{cycle}-offer-sample-{index}.png')
   session['final_scene']=observer.visible_scene(hwnd);session['final_state']=state()
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
 report['status']='observed_actual_contract_note_state_only'
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
