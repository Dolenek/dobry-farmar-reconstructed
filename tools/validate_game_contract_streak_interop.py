#!/usr/bin/env python3
"""Restore real contract streak checkpoints through both complete EXEs;never edit input profiles."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET,hashlib,datetime,shutil,zipfile,sys
from ctypes import wintypes as W
from pathlib import Path
import argparse
r=Path(__file__).resolve().parents[1];data=r.parent/'farmář/Dobrý farmář/Data'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--launcher',type=Path,default=r/'build/native-process/Release/native-process-launcher.exe')
parser.add_argument('--output',type=Path)
parser.add_argument('--crash-trace',type=Path,help='Attach source-only read-only exception diagnostic before Start')
parser.add_argument('--fixture-report',type=Path,default=r/'analysis/contract-streak-twenty-state-validation.json')
parser.add_argument('--suite',choices=['contract-streak-interop'],default='contract-streak-interop')
parser.add_argument('--expected-streak',type=int,default=20)
args=parser.parse_args();assert 0<args.expected_streak<=40;args.fixture_report=args.fixture_report.resolve(strict=True);launcher_path=args.launcher.resolve(strict=True)
if args.crash_trace:args.crash_trace=args.crash_trace.resolve(strict=True)
if args.output is None:args.output=r/'.local'/f'{args.suite}-validation.json'
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local'/args.suite/stamp;work.mkdir(parents=True)
shutil.copyfile(__file__,work/'driver.py')
from windows_game_gui import GameGui,u,k,INPUT,KEY
from windows_game_memory import TutorialMemory
from stable_live_scene import read_stable_scene
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
for n in ['tools/validate_game_contract_streak_interop.py','tools/windows_game_memory.py','tools/game_runtime_layout.cpp','CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/windows_game_gui.py','tools/native_process/launcher.cpp','tools/native_process/CMakeLists.txt']:report['source_files'][n]=digest(r/n)
# Restore only unmodified checkpoint profiles produced by actual contract gameplay.
fixture=json.loads((args.fixture_report).read_text(encoding='utf-8'));assert fixture['status'] in ['explored_actual_contract_streak_with_original_supply_command_only','passed_actual_paired_contract_receipts_and_complete_ledgers_only','verified_actual_contract_streak_checkpoint_profiles']
if fixture.get('profile_sets'):
 assert len(fixture['profile_sets'])==2 and all(row['streak']==args.expected_streak for row in fixture['profile_sets'])
else:assert fixture['requested_target']==args.expected_streak
report['fixture_report']=dict(path=args.fixture_report.relative_to(r).as_posix(),sha256=digest(args.fixture_report));report['expected_streak']=args.expected_streak
fixture_work=r/fixture['work_directory'];active_fields=list(range(60))
def location(index):
 if index<2 and fixture.get('profile_sets'):
  row=next(s for s in fixture['profile_sets'] if s['kind']==['native','source'][index])
  assert digest(r/row['source_report'])==row['source_report_sha256']
  directory=r/row['profile_directory'];assert {p.name:digest(p) for p in directory.iterdir() if p.is_file()}==row['files']
  return directory
 return (fixture_work if index<2 else work)/f'{index}-persisted/Profiles'
layout_path=r/'analysis/no-dry-award-state-validation.json';layout_source=json.loads(layout_path.read_text(encoding='utf-8'))
report['layout_metadata_report']=dict(path=layout_path.relative_to(r).as_posix(),sha256=digest(layout_path))
from live_farm_fields import read_live_farm_fields
report['source_files']['tools/live_farm_fields.py']=digest(r/'tools/live_farm_fields.py')
report['source_files']['tools/stable_live_scene.py']=digest(r/'tools/stable_live_scene.py')
for name in ['tools/game_save_state.py','tools/validate_factory_history_state.py']:report['source_files'][name]=digest(r/name)
report['diagnostic_tree_retries']=[]
from game_save_state import load_save
import struct
from PIL import Image
prior=json.loads((r/'.local/application-runtime-validation.json').read_text(encoding='utf-8'))
expected_hud=roi(Image.open(r/prior['work_directory']/'0-04-game.png'),(10,8,55,28))
u.GetWindowRect.argtypes=[W.HWND,C.POINTER(W.RECT)]

try:
 for cycle,(kind,variant,seed_from) in enumerate([('source','restore',0),('native','restore',1),('native','restore',2),('source','restore',4)],start=2):
  name=None;shared=work/f'{cycle}-shared';common=shared/'VirtualFarm';common.mkdir(parents=True)
  seed_profiles=location(seed_from)
  shutil.copytree(seed_profiles,common/'Profiles')
  settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
  session=dict(kind=kind,variant=variant,seed_from=seed_from,common_directory=common.relative_to(r).as_posix(),actions=[],input_profile_files={p.name:digest(p) for p in seed_profiles.iterdir() if p.is_file()});report['sessions'].append(session);launcher=None
  if kind=='native':
   launcher=subprocess.Popen([str(launcher_path),str(data.parent/'Dobrý farmář.exe'),str(runtime),str(shared)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
   line=launcher.stdout.readline();assert line,('Native launcher failed',launcher.communicate());event=json.loads(line);assert event['event']=='ready',event;session['launcher']=event;p=Child(event['pid'])
  else:p=subprocess.Popen([str(runtime/'FarmarSourceInterop.exe'),'--data',str(data),'--common-dir',str(common)],cwd=runtime,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  probe.process=p;observer=TutorialMemory(p.pid,kind,r);trace=None
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
   settle(f'{cycle}-03-menu.png');attach_start=time.monotonic()
   if kind=='source' and args.crash_trace:
    trace_file=(work/f'{cycle}-crash-trace.jsonl').open('w',encoding='utf-8')
    trace=subprocess.Popen([str(args.crash_trace.resolve(strict=True)),str(p.pid),str(runtime/'FarmarSourceInterop.exe')],cwd=r,stdout=subprocess.PIPE,stderr=trace_file,text=True)
    while True:
     trace_ready=trace.stdout.readline();trace_file.write(trace_ready);trace_file.flush();assert trace_ready,'Debugger exited before ready'
     if json.loads(trace_ready)['event']=='ready':break
    # Pipe remaining output directly to the diagnostic artifact without blocking UI.
    import threading
    def copy_trace():
     for line in trace.stdout:trace_file.write(line);trace_file.flush()
    trace_reader=threading.Thread(target=copy_trace,daemon=True);trace_reader.start()
    session['debugger_attach_seconds']=time.monotonic()-attach_start
   start=time.monotonic()
   # Native menu can already be visible while its input transition finishes.
   # Retry only until the actual game manager is constructed; never click a
   # farm coordinate merely because a previous fixed delay elapsed.
   while not observer.manager():
    check_process();assert time.monotonic()-start<60,'Main menu Start did not construct game within60s after debugger readiness'
    click(hwnd,*button('ui/mainmenu.xml','StartButton'));time.sleep(.5)
   if expected_hud is None:
    deadline=start+60
    while time.monotonic()<deadline and not list(common.rglob('*.save')):check_process();time.sleep(.2)
    assert list(common.rglob('*.save')),'Native game did not create save';time.sleep(1);game=capture(f'{cycle}-04-game.png');expected_hud=roi(game,(10,8,55,28));assert len(set(expected_hud))>10
   else:game=wait_match(f'{cycle}-04-game.png',(10,8,55,28),expected_hud)
   session['loading_seconds']=time.monotonic()-start;session['actions'].append('Created/restored full game and matched native Menu HUD pixels')
   def key(code,down):
    focus();item=INPUT();item.type=1;item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0);assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.1)
   key(0x38,True);key(0xc5,True);key(0xc5,False);key(0x38,False)
   time.sleep(.2);capture(f'{cycle}-10-restored-late-world-outcome.png')
   session['restored_expanded_farm']=dict(task20_owned=observer.property('booleans','Award04Make20Task'),task40_owned=observer.property('booleans','Award05Make40Task'),market=[observer.property('uints',f'MGoodCount{n}') for n in range(19)],prices=[observer.property('uints',f'MGoodPrice{n}') for n in range(19)],high_sold=observer.property('booleans','awHighSold'),processed=observer.property('uints','awQProcess'),market_milk=observer.property('uints','MGoodCount10'),milk_price=observer.property('uints','MGoodPrice10'),loaded=observer.property('uints','ProdCount0'),building_level=observer.property('ints','Upgrade0'),collected=observer.property('uints','awQCollect'),carried_grass=observer.property('uints','PCC0'),stock=[observer.property('uints',f'WGoodCount{n}') for n in range(19)],main_game=observer.property('booleans','MainGame'),no_dry=observer.property('booleans','awNoDry'),no_dry_time=observer.property('floats','awNoDryTime'),current_upgrade=observer.property('ints','fcu'),money=observer.property('ints','Money'),game_time=observer.property('floats','GameTime'),run_time=observer.property('floats','RunTime'),roses=observer.property('uints','WGoodCount4'),streak=observer.property('uints','awQTask'),awards=[observer.property('booleans',f'awardShow{n}') for n in range(21)],held=observer.property('uints','PCarry'),tasks=[dict(id=observer.property('ints',f'task{n}'),remaining=observer.property('floats',f'taskTime{n}'),pass_cancel=observer.property('booleans',f'taskPassCancel{n}')) for n in range(3)],sprinklers=[dict(exists=observer.property('booleans',f'SPExist{n}'),status=observer.property('uints',f'SPStatus{n}'),x=observer.property('floats',f'SPXPos{n}'),y=observer.property('floats',f'SPYPos{n}')) for n in range(3)],fields=[dict(id=n,status=observer.property('uints',f'FFS{n}'),water=observer.property('floats',f'FFW{n}'),progress=observer.property('floats',f'FFP{n}')) for n in active_fields])
   session['restored_scene']=read_stable_scene(observer,hwnd,report['diagnostic_tree_retries'])
   session['live_fields']=read_live_farm_fields(observer,r,layout_source['field_layout']['metadata'],session['restored_scene'])
   assert all(row['active'] for row in session['live_fields'])
   session['tutorial_step']=observer.step()
   session['actions'].append('Restored actual contract streak,all19 stock/market values,earned threshold flags,journal,383 coefficients and60 fields;actual pause and close')
   u.PostMessageW(hwnd,0x10,0,0);assert p.wait(timeout=20)==0
   shutil.copytree(common,work/f'{cycle}-persisted')
   session['exit_code']=p.poll();session['status']='completed'
   print(json.dumps(dict(kind=kind,cycle=cycle,event='actual_contract_checkpoint_restored',streak=session['restored_expanded_farm']['streak'],money=session['restored_expanded_farm']['money'])),flush=True)
   if launcher:
    launcher.wait(timeout=10);out,err=launcher.communicate();session['launcher_completion']=out.strip();assert launcher.returncode==0,(launcher.returncode,err)
   else:
    out,err=p.communicate();(work/f'{cycle}-stdout.log').write_bytes(out);(work/f'{cycle}-stderr.log').write_bytes(err)
  finally:
   observer.close()
   if p.poll() is None:
    u.PostMessageW(hwnd,0x10,0,0);p.wait(timeout=15)
   if kind=='native':k.CloseHandle(p.handle)
   if trace is not None:
    assert trace.wait(timeout=10)==0
    trace_reader.join(timeout=5);assert not trace_reader.is_alive();trace_file.close()
 def props(index):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(location(index)/'00000001.xml').getroot()}
 varying={'awNoDryTime','GameTime','RunTime','letTime0','letTime1','letTime2'}|{f'{prefix}{n}' for prefix in ['FFP','FFF','FFW'] for n in range(60)}
 from counter_save_audit import exact_restored_binary
 report['source_files']['tools/counter_save_audit.py']=digest(r/'tools/counter_save_audit.py')
 parser_path=r/'analysis/profile-floating-PC24-validation.json';parser_proof=json.loads(parser_path.read_text(encoding='utf-8'))
 assert next(c for c in parser_proof['cases'] if c.get('mode')=='PF' and c.get('payload')=='1.#INF')['native_parsed_bits']=='0000803f'
 report['original_float_parser_evidence']=dict(path=parser_path.relative_to(r).as_posix(),sha256=digest(parser_path),literal='1.#INF',actual_original_parsed_float=1)
 transfers=[]
 for case,(before,after) in enumerate([(0,2),(1,3),(2,4),(4,5)]):
  session=report['sessions'][case];observed=session['restored_expanded_farm']
  left,right=props(before),props(after)
  assert session['input_profile_files']=={p.name:digest(p) for p in location(before).iterdir() if p.is_file()}
  assert observed['streak']==int(left['awQTask'][1])==args.expected_streak
  assert observed['money']==int(left['Money'][1])
  assert observed['current_upgrade']==3 and not observed['main_game']
  assert 11190<=observed['game_time']<14400
  assert observed['stock']==[int(left[f'WGoodCount{n}'][1]) for n in range(19)]
  assert observed['market']==[int(left[f'MGoodCount{n}'][1]) for n in range(19)]
  assert observed['prices']==[99]*19 and all(observed['market'])
  assert observed['awards']==[left[f'awardShow{n}'][1]=='1' for n in range(21)]
  assert observed['task20_owned']==(left['Award04Make20Task'][1]=='1')
  assert observed['task40_owned']==(left['Award05Make40Task'][1]=='1')
  if args.expected_streak>=20:assert observed['task20_owned'] and observed['awards'][4]
  if args.expected_streak>=40:assert observed['task40_owned'] and observed['awards'][5]
  assert all(t['id']==-1 for t in observed['tasks'])
  assert observed['sprinklers']==[dict(exists=True,status=0,x=float(left[f'SPXPos{n}'][1]),y=float(left[f'SPYPos{n}'][1])) for n in range(3)]
  assert len(session['live_fields'])==60 and all(f['active'] and f['culture']==0 and f['water']>=0 for f in session['live_fields'])
  assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
  changed={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
  allowed=set(varying);readiness=[]
  for n in range(60):
   name=f'FFS{n}'
   if name in changed:
    transition=(left[name],right[name])
    assert (transition==(('UInt','0'),('UInt','1')) and right[f'FFP{n}']==('Float','0')) or (transition==(('UInt','1'),('UInt','0')) and right[f'FFW{n}']==('Float','0'))
    readiness.append(n);allowed.add(name)
  dt=float(right['GameTime'][1])-float(left['GameTime'][1]);caches=[]
  for product in range(19):
   name=f'MSellTime{product}'
   if name not in changed:continue
   a,b=left[name][1],right[name][1]
   if a=='1.#INF':
    if b=='1.#INF':assert dt>=.8
    else:assert 0<float(b)<=1 and abs((1-float(b))-dt)<.2
   elif b=='1.#INF':assert 0<=float(a)<=1 and dt>=float(a)-.2
   else:assert 0<float(b)<=float(a)<=1 and abs((float(a)-float(b))-dt)<.2
   caches.append(dict(product=product,before=a,after=b,profile_game_seconds_advanced=dt,original_parser_reads_infinity_spelling_as=1,no_sale_during_cache_transition=True));allowed.add(name)
  assert set(changed)<=allowed,(before,after,changed)
  for name in ['GameTime','RunTime']:assert 0<=float(right[name][1])-float(left[name][1])<15,(name,left[name],right[name])
  old_path,new_path=location(before)/'00000001.save',location(after)/'00000001.save'
  binary=exact_restored_binary(old_path,new_path)
  assert binary['whole_binary_unchanged'] and binary['coefficient_words']==383
  transfers.append(dict(before=before,after=after,profile_properties=len(left),changed_properties=changed,
   actual_grass_readiness_transitions=readiness,original_sale_cache_transitions=caches,binary=binary,input_sha256=digest(old_path),output_sha256=digest(new_path),normalizations=[]))
 report['comparison']=dict(transfer_comparisons=transfers,original_profile_fields_modified_by_probe=False,independent_random_outcomes_preserved=True)
 report['fixture_report_sha256']=digest(args.fixture_report)
 report['status']='passed_actual_contract_streak_and_four_whole_bidirectional_saves_only'
 report['pending']=['Continue actual40 contract threshold and final all-awards condition','Remaining gameplay, full Squall, JNG and uninstaller scopes']

except Exception as e:
 report['status']='failed';report['error']=repr(e)
 import traceback;report['traceback']=traceback.format_exc()
finally:
 unchanged();report['reference_files_unchanged']=17
 report['whole_game_complete']=False
 report['screenshots']={p.name:digest(p) for p in work.glob('*.png')}
 encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_bytes(encoded.encode('utf-8'));args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(encoded.encode('utf-8'))
 print(json.dumps(dict(status=report['status'],work_directory=str(work),error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
