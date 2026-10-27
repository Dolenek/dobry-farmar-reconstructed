#!/usr/bin/env python3
"""Physical original/source runtime modifier comparison; diagnostic only."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description='Opt-in physical original/source modifier and graphics-reset comparison, with owned window DPI coordinates.')
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/runtime-shortcut-gui-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/runtime-shortcuts'/stamp;work.mkdir(parents=True)
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
for n in ['tools/validate_runtime_shortcuts.py','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt','CMakeLists.txt','tools/cmake/SquallCandidate.cmake']:report['source_files'][n]=digest(r/n)
def key(scan,down):
 focus();item=INPUT();item.type=1;item.keyboard=KEY(0,scan&0x7f,8|(1 if scan&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
def modified(scan):
 key(0x38,True);key(scan,True);key(scan,False);key(0x38,False)
u.SetThreadDpiAwarenessContext.argtypes=[C.c_void_p];u.SetThreadDpiAwarenessContext.restype=C.c_void_p
u.GetWindowDpiAwarenessContext.argtypes=[W.HWND];u.GetWindowDpiAwarenessContext.restype=C.c_void_p
u.GetAwarenessFromDpiAwarenessContext.argtypes=[C.c_void_p];u.GetAwarenessFromDpiAwarenessContext.restype=C.c_int
u.GetDpiForWindow.argtypes=[W.HWND];u.GetDpiForWindow.restype=W.UINT
u.GetWindowLongW.argtypes=[W.HWND,C.c_int];u.GetWindowLongW.restype=W.LONG
u.IsIconic.argtypes=[W.HWND]
def dimensions():
 rect=W.RECT();assert u.GetClientRect(hwnd,C.byref(rect));return [rect.right,rect.bottom]
def observe(name):
 logical=dimensions();old=u.SetThreadDpiAwarenessContext(C.c_void_p(-4))
 try:
  physical=dimensions();desktop=[u.GetSystemMetrics(0),u.GetSystemMetrics(1)];capture(name)
 finally:u.SetThreadDpiAwarenessContext(old)
 row=dict(name=name,logical_client=logical,physical_client=physical,desktop_physical=desktop,window_dpi=u.GetDpiForWindow(hwnd),awareness=u.GetAwarenessFromDpiAwarenessContext(u.GetWindowDpiAwarenessContext(hwnd)),style=int(u.GetWindowLongW(hwnd,-16))&0xffffffff)
 session.setdefault('states',[]).append(row);return row
def observe_reset(name, reference=None):
 # Window styles change before the D3D front buffer and compositor agree.
 # Compare the visible reset frame, retaining transition samples and requiring
 # three consecutive matches, rather than treating a fixed1s delay as readiness.
 from PIL import Image
 expected=Image.open(work/reference).convert('RGB').tobytes() if reference else None
 deadline=time.monotonic()+20;attempt=0;consecutive=0;previous=None
 while True:
  check_process();focus();attempt+=1
  sampled=name.replace('.png',f'-sample{attempt:02d}.png')
  row=observe(sampled);im=Image.open(work/sampled).convert('RGB');pixels=im.tobytes()
  colors=len(set(im.get_flattened_data()))
  matches=pixels==expected if expected is not None else colors in (2,3) and pixels==previous
  consecutive=consecutive+1 if matches else 0;previous=pixels
  session.setdefault('reset_transition_samples',[]).append(dict(name=sampled,colors=colors,matches_reference=matches if expected is not None else None,stable_matches=consecutive))
  if consecutive>=3:
   shutil.copy2(work/sampled,work/name);row['name']=name;return
  if time.monotonic()>=deadline:raise AssertionError(('Reset frame failed to settle or match reference',name,attempt))
  time.sleep(.3)

def observe_textured(name):
 # A style change precedes the first visible Present. Preserve every sample,
 # including black transition frames; require a textured frame within20s.
 from PIL import Image
 deadline=time.monotonic()+20;attempt=0
 while True:
  check_process();focus();attempt+=1
  sampled=name.replace('.png',f'-sample{attempt:02d}.png')
  row=observe(sampled);im=Image.open(work/sampled).convert('RGB')
  colors=len(set(im.get_flattened_data()))
  session.setdefault('textured_transition_samples',[]).append(dict(name=sampled,colors=colors,foreground=int(u.GetForegroundWindow()),owned_window=hwnd.value))
  if colors>1024:
   shutil.copy2(work/sampled,work/name);row['name']=name;return
  if time.monotonic()>=deadline:raise AssertionError(('No textured frame after mode change',name,attempt))
  time.sleep(.5)

def wait_mode(full):
 deadline=time.monotonic()+20
 while time.monotonic()<deadline:
  check_process();style=int(u.GetWindowLongW(hwnd,-16))&0xffffffff
  if bool(style&0x00ca0000)!=full and not u.IsIconic(hwnd):return
  time.sleep(.1)
 raise RuntimeError('Window style did not change mode '+str(full))
try:
 for cycle,kind in enumerate(['native','source']):
  shared=work/f'{kind}-shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
  # Existing selected profile avoids first-user typing; the full game is never entered here.
  fixture=json.loads((r/'.local/native-application-interop-validation.json').read_text(encoding='utf-8'))
  shutil.copytree(r/fixture['work_directory']/'source-shared/VirtualFarm/Profiles',common/'Profiles')
  settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
  session=dict(kind=kind,actions=[]);report['sessions'].append(session);launcher=None
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
   loaded=modules(p.pid);session['loaded_game_dlls']={}
   for n in ['jngload.dll','squall.dll']:
    path=Path(loaded[n]).resolve(strict=True);assert path.parent==(data.parent if kind=='native' else runtime).resolve(),(n,str(path));session['loaded_game_dlls'][n]=dict(path=path.relative_to(r.parent).as_posix(),sha256=digest(path))
   for _ in range(4):scan(0x3c)
   time.sleep(3);observe(f'{cycle}-01-windowed.png')
   modified(0x14);time.sleep(.8);observe(f'{cycle}-02-wireframe.png');session['actions'].append('Actual Alt+T release reached wireframe')
   modified(0x1c);wait_mode(True);time.sleep(1);observe_reset(f'{cycle}-03-fullscreen.png','0-03-fullscreen.png' if cycle else None);session['actions'].append('Actual Alt+Enter changed to full desktop client')
   modified(0x1c);wait_mode(False);time.sleep(1);observe_reset(f'{cycle}-04-windowed-after-reset.png','0-04-windowed-after-reset.png' if cycle else None);session['actions'].append('Actual Alt+Enter restored framed window; physical/logical sizes recorded')
   modified(0x14);time.sleep(.8);observe(f'{cycle}-05-toggle-off.png')
   modified(0x14);time.sleep(.8);observe(f'{cycle}-06-wireframe-again.png')
   modified(0x14);time.sleep(.8);observe(f'{cycle}-07-solid-again.png')
   modified(0x3f);time.sleep(.8);observe(f'{cycle}-08-textures-refreshed.png');session['actions'].append('Actual Alt+F5 refresh returned without errors')
   modified(0x1c);wait_mode(True);time.sleep(1);observe_textured(f'{cycle}-09-fullscreen-textured.png')
   modified(0x1c);wait_mode(False);time.sleep(1);observe_textured(f'{cycle}-10-windowed-textured.png');session['actions'].append('Ordinary textured fullscreen and framed return completed')
   u.PostMessageW(hwnd,0x10,0,0);code=p.wait(timeout=20) if kind=='native' else (p.wait(timeout=20) or p.returncode);assert code==0,code;session['status']='passed_for_actions_pending_visual_audit';session['exit_code']=code
   if launcher:
    launcher.wait(timeout=10);out,err=launcher.communicate();session['launcher_exit']=out.strip();assert launcher.returncode==0,(launcher.returncode,err)
   else:
    out,err=p.communicate();(work/f'{cycle}-stdout.log').write_bytes(out);(work/f'{cycle}-stderr.log').write_bytes(err)
  finally:
   if p.poll() is None:
    for window,_,_ in windows(p.pid):u.PostMessageW(W.HWND(window),0x10,0,0)
    try:p.wait(timeout=10)
    except Exception:
     session['shutdown_failed']=True
     if kind=='source':p.terminate();p.wait(timeout=10)
   if kind=='native':k.CloseHandle(p.handle)
  unchanged()
 from PIL import Image,ImageChops
 exact=[];other=[]
 for name in ['01-windowed','02-wireframe','03-fullscreen','04-windowed-after-reset','05-toggle-off','06-wireframe-again','07-solid-again','08-textures-refreshed','09-fullscreen-textured','10-windowed-textured']:
  a=Image.open(work/f'0-{name}.png').convert('RGB');b=Image.open(work/f'1-{name}.png').convert('RGB');assert a.size==b.size,('Game-context screenshot dimensions differ',name,a.size,b.size)
  if name in ['09-fullscreen-textured','10-windowed-textured']:assert len(set(a.get_flattened_data()))>1024 and len(set(b.get_flattened_data()))>1024,('Normal reset did not restore textured rendering',name)
  difference=ImageChops.difference(a,b);different=sum(pixel!=(0,0,0) for pixel in difference.get_flattened_data());row=dict(state=name,width=a.width,height=a.height,different_rgb_pixels=different)
  if name in ['03-fullscreen','04-windowed-after-reset']:
   assert different==0,('Native reset texture-suppression quirk differs',row);exact.append(row)
  else:other.append(row)
 for session in report['sessions']:
  assert all(s['physical_client']==s['desktop_physical'] for s in session['states'] if s['name'].endswith('03-fullscreen.png'))
  assert all(s['physical_client']==[800,600] for s in session['states'] if 'windowed-after-reset' in s['name'])
  session['status']='passed_for_modifier_actions_modes_and_reset'
 report['exact_reset_frames']=exact;report['exact_reset_rgb_pixels']=sum(row['width']*row['height'] for row in exact)
 report['uncontrolled_animation_frames']=other;report['normal_timed_frames_compared']=False
 report['known_original_quirk']='Device reset restores solid fill but keeps the original texture-suppression byte until Alt+T disables it'
 report['status']='passed_for_physical_modifiers_modes_texture_refresh_and_two_exact_reset_frames' 
except Exception as error:report['status']='failed';report['error']=repr(error);import traceback;report['traceback']=traceback.format_exc()
finally:
 unchanged();report['reference_files_unchanged']=17;report['screenshots']={x.name:digest(x) for x in work.glob('*.png')};report['pending']=['Controlled normal-animation whole-runtime frames, remaining Logic processing/debug text and GameInput cheats','Device loss/failure/reset branches and all window lifecycle sequences','All game systems, expanded saves, remaining Squall97 behavior and isolated uninstaller'];encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.write_text(encoded,encoding='utf-8');print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
