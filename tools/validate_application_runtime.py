#!/usr/bin/env python3
"""Opt-in physical desktop smoke; run separately from unattended CTest."""
import ctypes as C,subprocess,time,json,xml.etree.ElementTree as ET
from ctypes import wintypes as W
from pathlib import Path
import argparse,hashlib,datetime,shutil,zipfile,sys
r=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description='Opt-in real source application smoke: foreground physical input, isolated profile, restart and real menu/help/journal. Requires Windows desktop and Pillow.')
parser.add_argument('--build',type=Path,default=r/'build/windows/Release')
parser.add_argument('--startup-only',action='store_true',help='Check actual fresh/restarted farm without racing tutorial-gated HUD menus')
parser.add_argument('--output',type=Path,default=r/'.local/application-runtime-validation.json')
args=parser.parse_args();build=args.build.resolve(strict=True);data=r.parent/'farmář/Dobrý farmář/Data';dataRoot=data
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=r/'.local/application-runtime'/stamp;work.mkdir(parents=True)
shutil.copyfile(__file__,work/'executed-driver.py')
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
inventory=json.loads((r/'analysis/inventory.json').read_text(encoding='utf-8'))
def originals_unchanged():
 for row in inventory['files']:assert digest(data.parent/row['path'])==row['sha256'],row['path']
originals_unchanged()
for name,target in [('Dobrý farmář.exe','FarmarSourceRuntime.exe'),('JNGLoad.dll','JNGLoad.dll'),('Squall.dll','Squall.dll')]:shutil.copy2(build/name,work/target)
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
# Only graphics window mode is changed; all game data is read in place.
settings=resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0')
common=work/'common';(common/'sys').mkdir(parents=True);ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
report=dict(startup_only=args.startup_only,status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(r).as_posix(),original_program_code_used=False,original_comparison=False,source_files={p.relative_to(r).as_posix():digest(p) for p in sorted((r/'src/game').rglob('*')) if p.is_file()},implementation_files={n:digest(work/n) for n in ['FarmarSourceRuntime.exe','JNGLoad.dll','Squall.dll']},sessions=[])
for name in ['tools/validate_application_runtime.py','tools/windows_game_gui.py','CMakeLists.txt','tools/cmake/SquallCandidate.cmake']:
 report['source_files'][name]=digest(r/name)
try:
 expected_hud=None
 for cycle in range(2):
  session=dict(existing_profile=bool(cycle),actions=[]);report['sessions'].append(session)
  p=subprocess.Popen([str(work/'FarmarSourceRuntime.exe'),'--data',str(data),'--common-dir',str(common)],cwd=work,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  probe.process=p
  try:
   deadline=time.monotonic()+20;hwnd=None
   while time.monotonic()<deadline:
    check_process();found=[row for row in windows(p.pid) if row[2].startswith('melesta_')]
    if found:hwnd=W.HWND(found[0][0]);break
    time.sleep(.1)
   assert hwnd,'No source game window';probe.hwnd=hwnd;focus();time.sleep(1)
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
   if not args.startup_only:
    # Journal is checked on the fresh farm. A restored fresh profile can
    # activate tutorial1 after six seconds; it intentionally gates HUD input.
    if not cycle:
     click(hwnd,*button('ui/gameui/gamemenu.xml','JournalButton'));journal=settle(f'{cycle}-09-journal.png');journal_roi=(180,90,620,435)
     assert roi(journal,journal_roi)!=roi(frame,journal_roi),'Journal did not open'
     click(hwnd,*button('ui/gameui/journal.xml','OkButton'));returned=settle(f'{cycle}-10-journal-return.png');assert roi(returned,journal_roi)!=roi(journal,journal_roi) and roi(returned,(10,8,55,28))==expected_hud,'Journal did not return to game'
     session['actions'].append('Journal opened and returned through actual mouse input; exact game HUD pixels matched')
    else:session['actions'].append('Restored-profile journal deferred to tutorial runtime probe; no animation-difference assertion substitutes for an open panel')
    click(hwnd,*button('ui/gameui/gamemenu.xml','MainMenuButton'));pause=settle(f'{cycle}-05-pause.png');pause_roi=(285,150,515,455)
    assert roi(pause,pause_roi)!=roi(frame,pause_roi),'Pause screen did not open'
    click(hwnd,*button('ui/menu.xml','HelpButton'));help=settle(f'{cycle}-06-help.png');assert roi(help,pause_roi)!=roi(pause,pause_roi),'Help screen did not open'
    click(hwnd,*button('ui/helpview.xml','OkButton'));wait_match(f'{cycle}-07-help-return.png',pause_roi,roi(pause,pause_roi));session['actions'].append('Help actually returned to identical pause-menu pixels')
    click(hwnd,*button('ui/menu.xml','ContinueButton'));continued=wait_match(f'{cycle}-08-continued.png',(10,8,55,28),expected_hud)
    assert roi(continued,pause_roi)!=roi(pause,pause_roi),'Continue retained pause screen'
   u.PostMessageW(hwnd,0x10,0,0);p.wait(timeout=20);assert p.returncode==0,('Application exit',p.returncode)
   session['status']='passed';session['save_files']={x.relative_to(work).as_posix():dict(bytes=x.stat().st_size,sha256=digest(x)) for x in common.rglob('*.save')}
   saved_props={node.get('Name'):node.get('Val') for node in ET.parse(common/'Profiles/00000001.xml').getroot()}
   session['saved_game_time']=float(saved_props['GameTime'])
  finally:
   if p.poll() is None:
    for window,_,_ in windows(p.pid):u.PostMessageW(W.HWND(window),0x10,0,0)
    try:p.wait(timeout=10)
    except subprocess.TimeoutExpired:p.terminate();p.wait(timeout=5);session['forced_termination']=True
   out,err=p.communicate();session['exit_code']=p.returncode;(work/f'{cycle}-stdout.log').write_bytes(out);(work/f'{cycle}-stderr.log').write_bytes(err)
  originals_unchanged()
 report['status']=('passed_for_source_application_fresh_profile_restart_and_actual_farm_only' if args.startup_only else 'passed_for_source_application_fresh_profile_restart_save_restore_pause_help_and_journal')
except Exception as error:report['status']='failed';report['error']=str(error)
finally:
 originals_unchanged();report['reference_files_unchanged']=len(inventory['files']);report['screenshots']={p.name:digest(p) for p in work.glob('*.png')};report['pending']=['Full original-process comparison and bidirectional whole-game saves','Complete tutorial/all gameplay systems/audio behavior and frame/reset fidelity','All97 Squall behaviors and isolated source uninstaller']
 encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8');print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error')),ensure_ascii=True),flush=True)
if report['status']=='failed':sys.exit(1)
