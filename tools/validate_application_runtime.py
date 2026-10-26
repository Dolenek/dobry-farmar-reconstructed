#!/usr/bin/env python3
"""Opt-in physical desktop smoke; run separately from unattended CTest."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET
from ctypes import wintypes as W
from pathlib import Path
import argparse,hashlib,datetime,shutil,zipfile,sys
r=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description='Opt-in real source application smoke: foreground physical input, isolated profile, restart and real menu/help/journal. Requires Windows desktop and Pillow.')
parser.add_argument('--build',type=Path,default=r/'build/windows/Release')
parser.add_argument('--output',type=Path,default=r/'.local/application-runtime-validation.json')
args=parser.parse_args();build=args.build.resolve(strict=True);data=r.parent/'farmář/Dobrý farmář/Data';dataRoot=data
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/application-runtime'/stamp;work.mkdir(parents=True)
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
inventory=json.loads((r/'analysis/inventory.json').read_text(encoding='utf-8'))
def originals_unchanged():
 for row in inventory['files']:assert digest(data.parent/row['path'])==row['sha256'],row['path']
originals_unchanged()
for name,target in [('Dobrý farmář.exe','FarmarSourceRuntime.exe'),('JNGLoad.dll','JNGLoad.dll'),('Squall.dll','Squall.dll')]:shutil.copy2(build/name,work/target)
u=C.WinDLL('user32',use_last_error=True);k=C.WinDLL('kernel32',use_last_error=True);g=C.WinDLL('gdi32',use_last_error=True)
k.SetErrorMode(3)
CALLBACK=C.WINFUNCTYPE(W.BOOL,W.HWND,W.LPARAM)
u.GetWindowThreadProcessId.argtypes=[W.HWND,C.POINTER(W.DWORD)];u.GetClassNameW.argtypes=[W.HWND,W.LPWSTR,C.c_int];u.GetWindowTextW.argtypes=[W.HWND,W.LPWSTR,C.c_int]
u.PostMessageW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM];u.SendMessageW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM];u.SendMessageW.restype=W.LPARAM
u.SetForegroundWindow.argtypes=[W.HWND];u.GetClientRect.argtypes=[W.HWND,C.POINTER(W.RECT)];u.ClientToScreen.argtypes=[W.HWND,C.POINTER(W.POINT)]
u.GetDC.argtypes=[W.HWND];u.GetDC.restype=W.HDC;u.ReleaseDC.argtypes=[W.HWND,W.HDC]
g.CreateCompatibleDC.argtypes=[W.HDC];g.CreateCompatibleDC.restype=W.HDC;g.CreateCompatibleBitmap.argtypes=[W.HDC,C.c_int,C.c_int];g.CreateCompatibleBitmap.restype=W.HBITMAP
g.SelectObject.argtypes=[W.HDC,W.HGDIOBJ];g.SelectObject.restype=W.HGDIOBJ;g.BitBlt.argtypes=[W.HDC,C.c_int,C.c_int,C.c_int,C.c_int,W.HDC,C.c_int,C.c_int,W.DWORD]
g.DeleteObject.argtypes=[W.HGDIOBJ];g.DeleteDC.argtypes=[W.HDC];g.GetDIBits.argtypes=[W.HDC,W.HBITMAP,W.UINT,W.UINT,C.c_void_p,C.c_void_p,W.UINT]
class KEY(C.Structure):_fields_=[('vk',W.WORD),('scan',W.WORD),('flags',W.DWORD),('time',W.DWORD),('extra',C.c_size_t)]
class MOUSE(C.Structure):_fields_=[('dx',W.LONG),('dy',W.LONG),('mouseData',W.DWORD),('flags',W.DWORD),('time',W.DWORD),('extra',C.c_size_t)]
class UNION(C.Union):_fields_=[('keyboard',KEY),('mouse',MOUSE)]
class INPUT(C.Structure):_anonymous_=['data'];_fields_=[('type',W.DWORD),('data',UNION)]
u.SendInput.argtypes=[W.UINT,C.POINTER(INPUT),C.c_int]
def scan(code):
 focus()
 for flags in [8,10]:
  item=INPUT();item.type=1;item.keyboard=KEY(0,code,flags,0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.08)
def click(hwnd,x,y):
 focus()
 pt=W.POINT(x,y);assert u.ClientToScreen(hwnd,C.byref(pt));assert u.SetCursorPos(pt.x,pt.y);time.sleep(.1)
 for flags in [2,4]:
  item=INPUT();item.type=0;item.mouse=MOUSE(0,0,0,flags,0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
def windows(pid):
 found=[]
 @CALLBACK
 def each(hwnd,_):
  owner=W.DWORD();u.GetWindowThreadProcessId(hwnd,C.byref(owner))
  if owner.value==pid:
   title=C.create_unicode_buffer(512);name=C.create_unicode_buffer(256);u.GetWindowTextW(hwnd,title,512);u.GetClassNameW(hwnd,name,256);found.append((int(hwnd),title.value,name.value))
  return True
 u.EnumWindows(each,0);return found
def errors(pid):
 output=[]
 for hwnd,title,name in windows(pid):
  if name!='#32770':continue
  texts=[]
  @CALLBACK
  def child(window,_):
   value=C.create_unicode_buffer(2048);u.GetWindowTextW(window,value,2048)
   if value.value:texts.append(value.value)
   return True
  u.EnumChildWindows(W.HWND(hwnd),child,0);output.append(dict(title=title,text=texts));u.PostMessageW(W.HWND(hwnd),0x10,0,0)
 return output
def screenshot(hwnd,name):
 from PIL import Image
 rect=W.RECT();assert u.GetClientRect(hwnd,C.byref(rect));width,height=rect.right,rect.bottom
 dc=u.GetDC(hwnd);copy=g.CreateCompatibleDC(dc);bitmap=g.CreateCompatibleBitmap(dc,width,height);old=g.SelectObject(copy,bitmap)
 assert g.BitBlt(copy,0,0,width,height,dc,0,0,0x00cc0020);g.SelectObject(copy,old)
 class HEADER(C.Structure):_fields_=[('size',W.DWORD),('width',W.LONG),('height',W.LONG),('planes',W.WORD),('bits',W.WORD),('compression',W.DWORD),('imageSize',W.DWORD),('x',W.LONG),('y',W.LONG),('used',W.DWORD),('important',W.DWORD)]
 header=HEADER(40,width,-height,1,32,0,width*height*4,0,0,0,0);pixels=C.create_string_buffer(width*height*4)
 assert g.GetDIBits(dc,bitmap,0,height,pixels,C.byref(header),0)==height
 image=Image.frombytes('RGB',(width,height),pixels.raw,'raw','BGRX');image.save(work/name);g.DeleteObject(bitmap);g.DeleteDC(copy);u.ReleaseDC(hwnd,dc)
 return image

u.GetForegroundWindow.restype=W.HWND
u.SetWindowPos.argtypes=[W.HWND,W.HWND,C.c_int,C.c_int,C.c_int,C.c_int,W.UINT]
def focus():
 if u.GetForegroundWindow()!=hwnd.value:u.SetForegroundWindow(hwnd);time.sleep(.1)
 assert u.GetForegroundWindow()==hwnd.value,'Source application lost foreground; no global input was injected'
def resource(name):
 import zipfile
 data=None
 for packed in ['Data.pack','cz.pack']:
  with zipfile.ZipFile(dataRoot/packed) as z:
   for item in z.infolist():
    if item.filename.lower()==name.lower():data=z.read(item)
 assert data is not None,name
 return ET.fromstring(data)
def button(file,name):
 row=resource(file).find(name);return [int(value) for value in row.get('position').split()]

def modules(pid):
 ps=C.WinDLL('psapi',use_last_error=True)
 k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE;k.CloseHandle.argtypes=[W.HANDLE]
 ps.EnumProcessModulesEx.argtypes=[W.HANDLE,C.c_void_p,W.DWORD,C.POINTER(W.DWORD),W.DWORD]
 ps.GetModuleFileNameExW.argtypes=[W.HANDLE,W.HMODULE,W.LPWSTR,W.DWORD]
 handle=k.OpenProcess(0x410,False,pid);assert handle
 try:
  handles=(W.HMODULE*2048)();size=W.DWORD();assert ps.EnumProcessModulesEx(handle,handles,C.sizeof(handles),C.byref(size),3)
  result={}
  for module in handles[:size.value//C.sizeof(W.HMODULE)]:
   path=C.create_unicode_buffer(32768)
   if ps.GetModuleFileNameExW(handle,module,path,len(path)):result[Path(path.value).name.lower()]=path.value
  return result
 finally:k.CloseHandle(handle)
def check_process():
 problem=errors(p.pid)
 if problem:raise RuntimeError(str(problem))
 if p.poll() is not None:raise RuntimeError('Application exited unexpectedly: '+str(p.returncode))
def capture(name):check_process();return screenshot(hwnd,name)
def settle(name):time.sleep(.8);return capture(name)
def roi(image,rect):return image.crop(rect).tobytes()
def wait_match(name,rect,expected,timeout=60):
 deadline=time.monotonic()+timeout
 while time.monotonic()<deadline:
  frame=capture(name)
  if roi(frame,rect)==expected:return frame
  time.sleep(.2)
 raise RuntimeError('Expected screen region did not return: '+name)
# Only graphics window mode is changed; all game data is read in place.
settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0')
common=work/'common';(common/'sys').mkdir(parents=True);ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(r).as_posix(),original_program_code_used=False,original_comparison=False,source_files={p.relative_to(r).as_posix():digest(p) for p in sorted((r/'src/game').rglob('*')) if p.is_file()},implementation_files={n:digest(work/n) for n in ['FarmarSourceRuntime.exe','JNGLoad.dll','Squall.dll']},sessions=[])
for name in ['tools/validate_application_runtime.py','CMakeLists.txt','tools/cmake/SquallCandidate.cmake']:
 report['source_files'][name]=digest(r/name)
try:
 expected_hud=None
 for cycle in range(2):
  session=dict(existing_profile=bool(cycle),actions=[]);report['sessions'].append(session)
  p=subprocess.Popen([str(work/'FarmarSourceRuntime.exe'),'--data',str(data),'--common-dir',str(common)],cwd=work,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  try:
   deadline=time.monotonic()+20;hwnd=None
   while time.monotonic()<deadline:
    check_process();found=[row for row in windows(p.pid) if row[2].startswith('melesta_')]
    if found:hwnd=W.HWND(found[0][0]);break
    time.sleep(.1)
   assert hwnd,'No source game window';focus();time.sleep(1)
   origin=W.POINT(0,0);assert u.ClientToScreen(hwnd,C.byref(origin));bounds=W.RECT();assert u.GetWindowRect(hwnd,C.byref(bounds));client=W.RECT();assert u.GetClientRect(hwnd,C.byref(client))
   session['desktop']=[u.GetSystemMetrics(0),u.GetSystemMetrics(1)];session['initial_client_origin']=[origin.x,origin.y]
   assert session['desktop'][0]>=client.right and session['desktop'][1]>=client.bottom,'Desktop smaller than client; physical input cannot reach all buttons'
   if origin.x<0 or origin.y<0 or origin.x+client.right>session['desktop'][0] or origin.y+client.bottom>session['desktop'][1]:
    assert u.SetWindowPos(hwnd,None,bounds.left-origin.x,bounds.top-origin.y,0,0,0x15)
   loaded=modules(p.pid);session['loaded_game_dlls']={}
   for name in ['jngload.dll','squall.dll']:
    path=Path(loaded[name]).resolve(strict=True);assert path.parent==work.resolve(),(name,str(path));session['loaded_game_dlls'][name]=dict(path=path.relative_to(r).as_posix(),sha256=digest(path))
   capture(f'{cycle}-01-logo.png')
   for _ in range(4):scan(0x3c)
   time.sleep(2);capture(f'{cycle}-02-startup.png')
   if not cycle:
    focus()
    for ch in 'SourceAudit':assert u.PostMessageW(hwnd,0x102,ord(ch),0);time.sleep(.035)
    scan(0x1c);time.sleep(1)
    header=common/'Profiles/profiles.xml';assert header.exists() and 'SourceAudit' in header.read_bytes().decode('utf-16'),'First profile was not persisted'
    session['actions'].append('Actual NewUser accepted SourceAudit through WM_CHAR and real Enter scan')
   menu=settle(f'{cycle}-03-menu.png');click(hwnd,*button('ui/mainmenu.xml','StartButton'))
   start=time.monotonic();deadline=start+60
   if not cycle:
    while time.monotonic()<deadline and not list(common.rglob('*.save')):check_process();time.sleep(.2)
    assert list(common.rglob('*.save')),'Game did not create binary save';time.sleep(1)
    frame=capture(f'{cycle}-04-game.png');expected_hud=roi(frame,(10,8,55,28))
    assert len(set(expected_hud))>10,'Blank game HUD'
   else:frame=wait_match(f'{cycle}-04-game.png',(10,8,55,28),expected_hud)
   session['loading_seconds']=time.monotonic()-start;session['actions'].append('Actual Start created/restored cGame and binary save')
   click(hwnd,*button('ui/gameui/gamemenu.xml','MainMenuButton'));pause=settle(f'{cycle}-05-pause.png');pause_roi=(285,150,515,455)
   assert roi(pause,pause_roi)!=roi(frame,pause_roi),'Pause screen did not open'
   click(hwnd,*button('ui/menu.xml','HelpButton'));help=settle(f'{cycle}-06-help.png');assert roi(help,pause_roi)!=roi(pause,pause_roi),'Help screen did not open'
   click(hwnd,*button('ui/helpview.xml','OkButton'));wait_match(f'{cycle}-07-help-return.png',pause_roi,roi(pause,pause_roi));session['actions'].append('Help actually returned to identical pause-menu pixels')
   click(hwnd,*button('ui/menu.xml','ContinueButton'));continued=wait_match(f'{cycle}-08-continued.png',(10,8,55,28),expected_hud)
   assert roi(continued,pause_roi)!=roi(pause,pause_roi),'Continue retained pause screen'
   click(hwnd,*button('ui/gameui/gamemenu.xml','JournalButton'));journal=settle(f'{cycle}-09-journal.png');journal_roi=(180,90,620,435)
   assert roi(journal,journal_roi)!=roi(continued,journal_roi),'Journal did not open'
   click(hwnd,*button('ui/gameui/journal.xml','OkButton'));returned=settle(f'{cycle}-10-journal-return.png');assert roi(returned,journal_roi)!=roi(journal,journal_roi) and roi(returned,(10,8,55,28))==expected_hud,'Journal did not return to game'
   session['actions'].append('Journal opened and returned through actual mouse input; exact game HUD pixels matched')
   u.PostMessageW(hwnd,0x10,0,0);p.wait(timeout=20);assert p.returncode==0,('Application exit',p.returncode)
   session['status']='passed';session['save_files']={x.relative_to(work).as_posix():dict(bytes=x.stat().st_size,sha256=digest(x)) for x in common.rglob('*.save')}
  finally:
   if p.poll() is None:
    for window,_,_ in windows(p.pid):u.PostMessageW(W.HWND(window),0x10,0,0)
    try:p.wait(timeout=10)
    except subprocess.TimeoutExpired:p.terminate();p.wait(timeout=5);session['forced_termination']=True
   out,err=p.communicate();session['exit_code']=p.returncode;(work/f'{cycle}-stdout.log').write_bytes(out);(work/f'{cycle}-stderr.log').write_bytes(err)
  originals_unchanged()
 report['status']='passed_for_source_application_fresh_profile_restart_save_restore_pause_help_and_journal'
except Exception as error:report['status']='failed';report['error']=str(error)
finally:
 originals_unchanged();report['reference_files_unchanged']=len(inventory['files']);report['screenshots']={p.name:digest(p) for p in work.glob('*.png')};report['pending']=['Full original-process comparison and bidirectional whole-game saves','Complete tutorial/all gameplay systems/audio behavior and frame/reset fidelity','All97 Squall behaviors and isolated source uninstaller']
 encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8');print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
