#!/usr/bin/env python3
"""Real native/source requested awards, automatic collect-all and panel/return lifecycle; diagnostic only."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description='Opt-in21 actual native/source award effect/panel/OK/return cycles and automatic collect-all; requires foreground desktop.')
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--validate-prefix-only',action='store_true',help='Check immutable native panels and saved ownership without launching a game')
parser.add_argument('--native-prefix-report',type=Path,help='Reuse normally closed native prefix with stable panels and persisted owned flags')
parser.add_argument('--output',type=Path,default=r/'.local/game-awards-runtime-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/game-awards-runtime'/stamp;work.mkdir(parents=True)
shutil.copyfile(__file__,work/'executed-driver.py')
from windows_game_gui import GameGui,u,k,INPUT,KEY
from windows_game_memory import TutorialMemory
probe=GameGui(work,data)
button=probe.button
capture=probe.capture
check_process=probe.check_process
click=probe.click
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
for n in ['tools/validate_game_awards_runtime.py','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
report['source_file_snapshots']={'tools/validate_game_awards_runtime.py':(work/'executed-driver.py').relative_to(r).as_posix()}
report['observer_layout_executable']=dict(path='build/windows/Release/game-runtime-layout.exe',sha256=digest(r/'build/windows/Release/game-runtime-layout.exe'))
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'.local/native-application-interop-validation.json').read_text(encoding='utf-8'))
baseline=r/fixture['work_directory']/'0-persisted/Profiles'
from PIL import Image
prior=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
expected_hud=roi(Image.open(r/prior['work_directory']/'0-04-game.png'),(10,8,55,28))
award_indices=list(range(21))
commands='a01..a20; final collect-all award automatically after20 committed conditions'
award_regions={};native_start=0;native_baseline=baseline
award_keys=sorted(node.get('Name') for node in ET.parse(baseline/'00000001.xml').getroot() if node.get('Name','').startswith('Award'))
assert len(award_keys)==21
if args.native_prefix_report:
 prefix_path=args.native_prefix_report.resolve(strict=True);prefix=json.loads(prefix_path.read_text(encoding='utf8'));prefix_work=r/prefix['work_directory']
 assert prefix['status']=='failed' and len(prefix['sessions'])==1
 previous=prefix['sessions'][0];assert previous['kind']=='native' and json.loads(previous['launcher_exit'])==dict(event='exit',exit_code=0,watchdog_triggered=False)
 native_baseline=r/previous['common_directory']/'Profiles'
 values={node.get('Name'):node.get('Val') for node in ET.parse(native_baseline/'00000001.xml').getroot()}
 while native_start<20 and values[award_keys[native_start]]==values['awardShow'+str(native_start)]=='1':native_start+=1
 assert 0<native_start<20 and values[award_keys[native_start]]==values['awardShow'+str(native_start)]=='0'
 for i in range(native_start):
  state=next(row for row in previous['fully_appeared_panels'] if row['index']==i);assert state['alpha']>=.999 and state['transition'][2]==0
  name=f'0-award{i:02d}-panel.png';assert digest(prefix_work/name)==prefix['screenshots'][name]
  award_regions[i]=roi(Image.open(prefix_work/name),(250,370,550,465))
 report['native_prefix']=dict(report=prefix_path.relative_to(r).as_posix(),sha256=digest(prefix_path),completed_indices=list(range(native_start)),profile_files={q.name:digest(q) for q in native_baseline.iterdir() if q.is_file()},normal_exit=True)

if args.validate_prefix_only:
 assert args.native_prefix_report
 report.update(status='passed_native_prefix_preflight_only',source_or_native_processes_started=0)
 encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_bytes(encoded.encode('utf8'));args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(encoded.encode('utf8'))
 print(json.dumps(dict(status=report['status'],native_completed_indices=list(range(native_start)),source_or_native_processes_started=0)));sys.exit(0)
u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]
def key(code,down):
 focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
try:
 for cycle,(kind,variant) in enumerate([('native','awards'),('source','awards')]):
  name=None;shared=work/f'{cycle}-shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
  seed=native_baseline if kind=='native' else baseline
  shutil.copytree(seed,common/'Profiles')
  settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
  session=dict(kind=kind,variant=variant,initial_money=int(next(node.get('Val') for node in ET.parse(seed/'00000001.xml').getroot() if node.get('Name')=='Money')),common_directory=common.relative_to(r).as_posix(),actions=[]);report['sessions'].append(session);launcher=None
  session['seed_profile_directory']=seed.relative_to(r).as_posix();session['input_profile_files']={q.name:digest(q) for q in seed.iterdir() if q.is_file()}
  if kind=='native':
   launcher=subprocess.Popen([str(launcher_path),str(data.parent/'Dobrý farmář.exe'),str(runtime),str(shared),'1800'],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
   line=launcher.stdout.readline();assert line,('Native launcher failed',launcher.communicate());event=json.loads(line);assert event['event']=='ready',event;session['launcher']=event;p=Child(event['pid'])
  else:p=subprocess.Popen([str(runtime/'FarmarSourceInterop.exe'),'--data',str(data),'--common-dir',str(common)],cwd=runtime,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  probe.process=p;observer=TutorialMemory(p.pid,kind,r)
  def wait_panel(filename):
   deadline=time.monotonic()+45
   while time.monotonic()<deadline:
    focus();check_process();scene=observer.visible_scene(hwnd)
    panels=[row for row in scene if (row['texture'] or '').lower()=='ui/gameui/avard_new.jng']
    if panels:
     panel=panels[0];alpha=observer.read(observer.read(panel['address']+observer.offsets['alpha'],'<I')[0],'<f')[0]
     transition=observer.read(panel['address']+observer.offsets['control_state'],'<ffI')
     lines=[row for row in scene if (row['texture'] or '').lower()=='ui/menu_font.dds' and 6400<=row['deep']<6401 and 340<=row['geometry'][1]<=480]
     if alpha>=.999 and transition[2]==0 and lines:
      signature=[(row['geometry'],row['sprite']) for row in lines];time.sleep(.25)
      late=[row for row in observer.visible_scene(hwnd) if (row['texture'] or '').lower()=='ui/menu_font.dds' and 6400<=row['deep']<6401 and 340<=row['geometry'][1]<=480]
      if signature==[(row['geometry'],row['sprite']) for row in late]:
       session.setdefault('fully_appeared_panels',[]).append(dict(index=index,alpha=alpha,transition=transition,visible_text_lines=len(lines)))
       return capture(filename)
    time.sleep(.1)
   raise RuntimeError(('Award panel did not fully appear',index))
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
   if name:
    focus()
    for ch in name:assert u.PostMessageW(hwnd,0x102,ord(ch),0);time.sleep(.035)
    scan(0x1c);time.sleep(1);header=common/'Profiles/profiles.xml';assert header.exists() and name in header.read_bytes().decode('utf-16'),'Native/source profile not persisted'
   settle(f'{cycle}-03-menu.png');click(hwnd,*button('ui/mainmenu.xml','StartButton'));start=time.monotonic()
   if expected_hud is None:
    deadline=start+60
    while time.monotonic()<deadline and not list(common.rglob('*.save')):check_process();time.sleep(.2)
    assert list(common.rglob('*.save')),'Native game did not create save';time.sleep(1);game=capture(f'{cycle}-04-game.png');expected_hud=roi(game,(10,8,55,28));assert len(set(expected_hud))>10
   else:game=wait_match(f'{cycle}-04-game.png',(10,8,55,28),expected_hud)
   session['loading_seconds']=time.monotonic()-start;session['actions'].append('Created/restored full game and matched native Menu HUD pixels')
   for index in (range(native_start,21) if kind=='native' else award_indices):
    command=f'a{index+1:02d}'
    if index<20:
     for ch in command:focus();assert u.PostMessageW(hwnd,0x102,ord(ch),0);time.sleep(.05)
    name=f'{cycle}-award{index:02d}-panel.png';panel=wait_panel(name)
    region=(250,370,550,465)
    if kind=='native':award_regions[index]=roi(panel,region)
    else:wait_match(name,region,award_regions[index],timeout=15)
    click(hwnd,*button('ui/awardpanel.xml','OkButton'));deadline=time.monotonic()+45;retry_at=time.monotonic()+1
    while not observer.property('booleans','awardShow'+str(index)):
     focus();check_process();assert time.monotonic()<deadline,('Award effect did not finish',index)
     if time.monotonic()>=retry_at and not observer.property('booleans',award_keys[index]):
      scene=observer.visible_scene(hwnd)
      if any((row['texture'] or '').lower()=='ui/gameui/avard_new.jng' for row in scene):
       session.setdefault('confirmation_input_retries',[]).append(dict(index=index,owned=False,shown=False,reason='Actual panel remains open and owned flag is still false'))
       click(hwnd,*button('ui/awardpanel.xml','OkButton'))
      retry_at=time.monotonic()+1
     time.sleep(.1)
    capture(f'{cycle}-award{index:02d}-accepted.png')
    if index!=19:
     deadline=time.monotonic()+30
     while True:
      returned=capture(f'{cycle}-award{index:02d}-returned.png')
      if roi(returned,(10,8,55,28))==expected_hud:break
      assert time.monotonic()<deadline,('Award screen did not return',index)
      click(hwnd,*button('ui/awards.xml','ReturnButton'));time.sleep(.5)
    session['actions'].append((command if index<20 else 'automatic collect-all predicate')+' -> actual effect, Awards panel and OK'+(' -> Return to Game' if index!=19 else ' -> automatic last award'))
   u.PostMessageW(hwnd,0x10,0,0);code=p.wait(timeout=20) if kind=='native' else (p.wait(timeout=20) or p.returncode);assert code==0,code
   session['exit_code']=code;session['status']='passed';session['persisted_files']={x.relative_to(common).as_posix():dict(bytes=x.stat().st_size,sha256=digest(x)) for x in common.rglob('*') if x.is_file()};shutil.copytree(common,work/f'{cycle}-persisted')
   if launcher:
    launcher.wait(timeout=10);out,err=launcher.communicate();session['launcher_exit']=out.strip();assert launcher.returncode==0,(launcher.returncode,err)
   else:
    out,err=p.communicate();(work/f'{cycle}-stdout.log').write_bytes(out);(work/f'{cycle}-stderr.log').write_bytes(err)
  finally:
   observer.close()
   if p.poll() is None:
    for window,_,_ in windows(p.pid):u.PostMessageW(W.HWND(window),0x10,0,0)
    try:p.wait(timeout=15)
    except Exception:session['shutdown_failed']=True
   if launcher and 'launcher_exit' not in session:
    launcher.wait(timeout=20);out,err=launcher.communicate();session['launcher_exit']=out.strip();session['launcher_return_code']=launcher.returncode;session['launcher_stderr']=err
   if kind=='native':k.CloseHandle(p.handle)
  unchanged();(work/'report-progress.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 comparisons=[]
 left=work/'0-persisted/Profiles';right=work/'1-persisted/Profiles'
 def props(folder):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(folder/'00000001.xml').getroot()}
 l=props(left);rr=props(right);assert set(l)==set(rr)
 changed={n:[l[n],rr[n]] for n in l if l[n]!=rr[n]};assert set(changed)<=set(['GameTime','RunTime']),('Award lifecycle profile effects differ',changed)
 award_keys=sorted(n for n in l if n.startswith('Award'));assert len(award_keys)==21
 for values in [l,rr]:
  for index,key_name in enumerate(award_keys):assert values[key_name]==('Bool','1') and values['awardShow'+str(index)]==('Bool','1'),('Actual award not committed',index,key_name,values[key_name],values['awardShow'+str(index)])
 a=(left/'00000001.save').read_bytes();b=(right/'00000001.save').read_bytes();assert a==b,'Award-world binary save differs'
 comparisons.append(dict(profile_properties=len(l),changed_properties=changed,awards=len(award_indices),binary_save_identical_bytes=len(a)))
 report['comparisons']=comparisons;report['award_indices']=award_indices;report['forced_game_input_awards']=20;report['automatic_collect_all_award']=True;report['panel_exact_roi']=[250,370,550,465];report['exact_panel_roi_rgb_pixels']=598500;report['whole_timed_frames_equal']=False
 report['status']='passed_for_game_input_requested_awards_full_process_panel_return'
except Exception as error:report['status']='failed';report['error']=repr(error);import traceback;report['traceback']=traceback.format_exc()
finally:
 unchanged();report['reference_files_unchanged']=17;report['screenshots']={x.name:digest(x) for x in work.glob('*.png')};report['pending']=['Full timed frame equality and natural attainment of all award conditions','Long-running cheat effect animations, all gameplay/tutorial/system scopes','Remaining engine/Squall97 and isolated source uninstaller'];encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8');print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
