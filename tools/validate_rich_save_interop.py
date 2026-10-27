#!/usr/bin/env python3
"""Full native process diagnostic with isolated common files; not a production original-code dependency."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description='Source/native/source/native rich-save diagnostic using the actual existing world fixture generator.')
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path,default=r/'.local/rich-save-interop-validation.json')
args=parser.parse_args();launcher_path=args.launcher.resolve(strict=True)
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/rich-save-interop'/stamp;work.mkdir(parents=True)
from windows_game_gui import GameGui,u,k
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
for n in ['tools/validate_rich_save_interop.py','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt','tools/game_source_runtime_test.cpp','tools/cmake/SquallCandidate.cmake']:report['source_files'][n]=digest(r/n)
# Generate a real serialized world with the current source composition test.
fixture_root=work/'source-fixtures';generator=r/'build/windows/Release/game-source-runtime-test.exe'
generator_process=subprocess.Popen([str(generator),str(data),str(fixture_root)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
generator_out,generator_err=generator_process.communicate(timeout=120);(work/'fixture-stdout.json').write_bytes(generator_out);(work/'fixture-stderr.log').write_bytes(generator_err)
assert generator_process.returncode==0,(generator_process.returncode,generator_err.decode('utf-8',errors='replace'))
generated=json.loads(generator_out);assert generated['device_flags']==32 and generated['source_game_lifetimes']==2 and generated['input_clicks']==8
shared=work/'shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
shutil.copytree(fixture_root/str(generator_process.pid)/'GameWorldProfiles',common/'Profiles')
settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
shutil.copytree(common,work/'0-persisted');report['fixture_generator']=dict(path=generator.relative_to(r).as_posix(),sha256=digest(generator),result=generated);report['hud_template']=dict(report_path='.local/application-runtime-validation.json',report_sha256=digest(r/'.local/application-runtime-validation.json'))
try:
 from PIL import Image
 source_runtime=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
 expected_hud=roi(Image.open(r/source_runtime['work_directory']/'0-04-game.png'),(10,8,55,28))
 for cycle,kind in enumerate(['source','native','source','native'],start=1):
  name=None
  session=dict(kind=kind,created_profile=name,common_directory=common.relative_to(r).as_posix(),actions=[]);report['sessions'].append(session);launcher=None
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
   # Normal process load/close preserves an actual serialized world. This is
   # not a tutorial timing or manual navigation scenario.
   session['actions'].append('Whole rich save loaded through real menu; normal close invokes actual game persistence')
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
 import struct
 def parse_save(path):
  data=path.read_bytes();offset=0
  def read(fmt):
   nonlocal offset
   size=struct.calcsize('<'+fmt);assert offset+size<=len(data);result=struct.unpack_from('<'+fmt,data,offset);offset+=size;return result[0] if len(result)==1 else list(result)
  count=read('I');assert count<10000;seeds=[]
  for _ in range(count):seeds.append(read('ffIIIB3x'))
  count=read('I');cutoff=read('f');assert count<10000;sales=[read('fI') for _ in range(count)]
  count=read('I');assert count<10000;messages=[]
  for _ in range(count):
   flag=read('B');length=read('I');assert offset+length*2<=len(data);text=data[offset:offset+length*2].decode('utf-16-le');offset+=length*2;messages.append(dict(read=flag,text=text))
  count=read('I');assert count<10000;coefficients=[read('f') for _ in range(count)];assert offset==len(data)
  return dict(seeds=seeds,cutoff=cutoff,sales=sales,messages=messages,coefficients=coefficients)
 baseline=parse_save(work/'1-persisted/Profiles/00000001.save');assert baseline['seeds']==[[126.0,245.0,2,7,0,1]] and len(baseline['coefficients'])==383 and sum(x!=0 for x in baseline['coefficients'])==22;assert len(baseline['seeds'])==1 and len(baseline['sales'])==1 and baseline['sales'][0]==[5.0,17] and any(m['text']=='Whole world save/load probe' for m in baseline['messages'])
 previous=baseline
 for after in range(2,5):
  before=after-1;left=work/f'{before}-persisted/Profiles';right=work/f'{after}-persisted/Profiles';saved=parse_save(right/'00000001.save');expected=json.loads(json.dumps(previous));expected['seeds'][0][0]+=1
  assert saved==expected,('Meaningful binary-save fields changed beyond original seed X drift',before,after)
  # Compare every actual file byte too; only original uninitialized seed padding
  # and the separately instruction-proven relative X increment are normalized.
  expected_raw=bytearray((left/'00000001.save').read_bytes());actual_raw=bytearray((right/'00000001.save').read_bytes())
  for i in range(len(previous['seeds'])):
   struct.pack_into('<f',expected_raw,4+i*24,previous['seeds'][i][0]+1)
   expected_raw[4+i*24+21:4+i*24+24]=bytes(3);actual_raw[4+i*24+21:4+i*24+24]=bytes(3)
  assert actual_raw==expected_raw,('Binary bytes differ beyond native X/padding',before,after)
  def props(folder):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(folder/'00000001.xml').getroot()}
  l=props(left);rr=props(right);changed={n:[l.get(n),v] for n,v in rr.items() if l.get(n)!=v}
  assert len(l)==790 and set(l)==set(rr) and set(changed)<=set(['GameTime','RunTime']),('Unexpected profile changes',before,after,changed)
  comparisons.append(dict(before=before,after=after,source_to_native=after in [2,4],native_to_source=after==3,meaningful_save_fields_match=True,normalized_binary_bytes=len(actual_raw),seed_before=previous['seeds'],seed_after=saved['seeds'],sales=saved['sales'],messages=saved['messages'],coefficient_count=len(saved['coefficients']),nonzero_coefficients=sum(x!=0 for x in saved['coefficients']),changed_profile_properties=changed,added_profile_keys=sorted(set(rr)-set(l)),removed_profile_keys=sorted(set(l)-set(rr))))
  previous=saved
 report['comparisons']=comparisons
 report['status']='passed_for_rich_source_native_source_native_save_fields_and_seed_x_drift' 
except Exception as error:report['status']='failed';report['error']=repr(error);import traceback;report['traceback']=traceback.format_exc()
finally:
 unchanged();report['reference_files_unchanged']=17;report['screenshots']={x.name:digest(x) for x in work.glob('*.png')};report['pending']=['Expanded farms, long histories and entire-game save compatibility','Complete tutorial and all gameplay systems/native frame/reset behavior','All97 Squall behavioral scopes and isolated source uninstaller'];encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8');print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
