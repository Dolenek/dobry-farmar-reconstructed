#!/usr/bin/env python3
"""Real native/source GameInput commands with isolated profiles; diagnostic only."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description='Opt-in full-process original/source GameInput effects, normal and wrap32 money cases; requires foreground desktop.')
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/game-input-runtime-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/game-input-runtime'/stamp;work.mkdir(parents=True)
from windows_game_gui import GameGui,u,k,INPUT,KEY
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
for n in ['tools/validate_game_input_runtime.py','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Use an already native-created first-farm baseline, independently for each run.
fixture=json.loads((r/'.local/native-application-interop-validation.json').read_text(encoding='utf-8'))
baseline=r/fixture['work_directory']/'0-persisted/Profiles'
from PIL import Image
prior=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
expected_hud=roi(Image.open(r/prior['work_directory']/'0-04-game.png'),(10,8,55,28))
commands='CHEATALLcheatmoneyCHEATUPGRADEScHeAtSeEdScheatproduct'
u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]
def key(code,down):
 focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
try:
 for cycle,(kind,variant) in enumerate([('native','normal'),('source','normal'),('native','overflow'),('source','overflow')]):
  name=None;shared=work/f'{cycle}-shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
  shutil.copytree(baseline,common/'Profiles')
  profile_path=common/'Profiles/00000001.xml';tree=ET.parse(profile_path)
  if variant=='overflow':
   for item in tree.getroot():
    if item.get('Name')=='Money':item.set('Val','2147483300')
    if item.get('Name').startswith('Award'):item.set('Val','1')
   tree.write(profile_path,encoding='utf-16',xml_declaration=True)
  settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
  session=dict(kind=kind,variant=variant,initial_money=20 if variant=='normal' else 2147483300,common_directory=common.relative_to(r).as_posix(),actions=[]);report['sessions'].append(session);launcher=None
  if kind=='native':
   launcher=subprocess.Popen([str(launcher_path),str(data.parent/'Dobrý farmář.exe'),str(runtime),str(shared)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
   line=launcher.stdout.readline();assert line,('Native launcher failed',launcher.communicate());event=json.loads(line);assert event['event']=='ready',event;session['launcher']=event;p=Child(event['pid'])
  else:p=subprocess.Popen([str(runtime/'FarmarSourceInterop.exe'),'--data',str(data),'--common-dir',str(common)],cwd=runtime,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  probe.process=p
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
   # Pause only Logic stepping with its actual shortcut. Input continues via
   # WM_CHAR -> real window/InputDevices -> registered cGameInput callback.
   key(0x38,True);key(0xc5,True);key(0xc5,False);key(0x38,False);time.sleep(.3)
   for ch in commands:focus();assert u.PostMessageW(hwnd,0x102,ord(ch),0);time.sleep(.035)
   time.sleep(.4);capture(f'{cycle}-05-commands.png');session['actions'].append('Actual Alt+Pause, then WM_CHAR queue through real InputDevices and GameInput: '+commands)
   u.PostMessageW(hwnd,0x10,0,0);code=p.wait(timeout=20) if kind=='native' else (p.wait(timeout=20) or p.returncode);assert code==0,code
   session['exit_code']=code;session['status']='passed';session['persisted_files']={x.relative_to(common).as_posix():dict(bytes=x.stat().st_size,sha256=digest(x)) for x in common.rglob('*') if x.is_file()};shutil.copytree(common,work/f'{cycle}-persisted')
   if launcher:
    launcher.wait(timeout=10);out,err=launcher.communicate();session['launcher_exit']=out.strip();assert launcher.returncode==0,(launcher.returncode,err)
   else:
    out,err=p.communicate();(work/f'{cycle}-stdout.log').write_bytes(out);(work/f'{cycle}-stderr.log').write_bytes(err)
  finally:
   if p.poll() is None:
    for window,_,_ in windows(p.pid):u.PostMessageW(W.HWND(window),0x10,0,0)
    try:p.wait(timeout=15)
    except Exception:session['shutdown_failed']=True
   if kind=='native':k.CloseHandle(p.handle)
  unchanged();(work/'report-progress.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 comparisons=[]
 for before,after,expected_money in [(0,1,2020),(2,3,1000)]:
  left=work/f'{before}-persisted/Profiles';right=work/f'{after}-persisted/Profiles'
  def props(folder):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(folder/'00000001.xml').getroot()}
  l=props(left);rr=props(right);assert set(l)==set(rr) and all(l[n][0]==rr[n][0] for n in l),'Profile names/types differ'
  for values in [l,rr]:
   assert values['Money']==('Int',str(expected_money)),('Actual money/wrap/clamp differs',before,values['Money'])
   for i in range(15):assert values['UpAvail'+str(i)]==('UInt',str(2 if i==13 else 3)),('Upgrade command',i,values['UpAvail'+str(i)])
   for i in range(8):assert values['SeedAvail'+str(i)]==('UInt','1'),('Seed command',i)
   for i in range(19):assert values['WGoodCount'+str(i)]==('UInt',str(2 if i<7 else 1 if i==7 else 0)),('Actual product command/capacity',i,values['WGoodCount'+str(i)])
  changed={n:[l[n],rr[n]] for n in l if l[n]!=rr[n]};assert set(changed)<=set(['GameTime','RunTime']),('Actual game command effects differ',before,after,changed)
  a=(left/'00000001.save').read_bytes();b=(right/'00000001.save').read_bytes();assert a==b,('Command-world binary save differs',before,after)
  comparisons.append(dict(native_session=before,source_session=after,profile_properties=len(l),expected_money=expected_money,all15_upgrade_availability=True,all8_seed_availability=True,all19_warehouse_goods=True,warehouse_capacity_clamp_total=15,all_profile_values_equal_except_clocks=True,binary_save_identical_bytes=len(a),changed_properties=changed))
 report['commands']=commands;report['comparisons']=comparisons;report['logic_stepping_paused_during_commands']=True;report['input_boundary']='WM_CHAR queued into owned foreground game window; actual native/source InputDevices and registered GameInput callbacks'
 report['status']='passed_for_actual_game_input_commands_profile_effects_and_money_wrap'
except Exception as error:report['status']='failed';report['error']=repr(error);import traceback;report['traceback']=traceback.format_exc()
finally:
 unchanged();report['reference_files_unchanged']=17;report['screenshots']={x.name:digest(x) for x in work.glob('*.png')};report['pending']=['Requested awards through full effect/panel lifecycle','Long-running cheat effect animations, all gameplay/tutorial/system scopes','Remaining engine/Squall97 and isolated source uninstaller'];encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8');print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
