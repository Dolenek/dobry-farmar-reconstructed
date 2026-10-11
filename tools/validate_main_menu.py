#!/usr/bin/env python3
"""Compare complete MainMenu navigation and real Loading with original methods."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/main-menu-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','main-menu'),('PC24','main-menu-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'MainMenu {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['main_menu_states']==22 and state['main_menu_letters']==1447 and state['main_menu_actions']==13
        assert state['main_menu_frames']==117 and state['main_menu_pixels']==56160000 and state['main_menu_quit_checks']==1
        assert state['main_menu_loading_steps']==18 and state['menu_sound_requests']==26
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/main_menu_oracle_cases.inc',ROOT/'tools/main_menu_oracle_services.h',ROOT/'tools/select_profile_oracle_services.h',
        ROOT/'tools/loading_oracle_services.h',ROOT/'tools/awards_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_main_menu_navigation_and_loading_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        control_states=44,letters=2894,input_actions=26,gpu_frames=234,gpu_rgb_pixels=112320000,loading_steps=36,
        original_functions=['00484da0','00483e90','00481e10','00481990','00481770','004836d0','00483ab0',
            '004817d0','004816d0','004817c0','005306c0','004818f0','00481860','00485490',
            '004826c0','00482ef0','00482ae0','004817a0','00484700'],
        coverage=['Actual MainMenu/FixedBack ownership and singleton creation/destruction; all seven owned buttons and both text lines, partial XML reload reuses background',
            'Original reversed View hooks, Escape bind/unbind, empty profile-name step and real persisted source profiles',
            'Full input routes to Options/Credits/Awards/profile selector/Exit, actual return buttons and accepted/cancelled profiles, singleton guards and absent-menu reconstruction',
            'Native/recovered complete scene frame, controlled cursor/clock and thirteen click actions per precision; exact ordered sprite submissions, draw calls and 234 GPU frames',
            'Actual source/native quit callbacks post WM_QUIT with code 0; diagnostic drains its own window message queue',
            'Actual Start click writes isolated MenuResLog, exact MainMenu.cpp153 discard origin, real Loading processes all 539 game entries in 18 batches and invokes completion once'],
        test_adapters=['Original EXE is a diagnostic reference mapped without entry point. Production code never loads original program code',
            'Modern MainMenuContext expresses original engine singleton dependencies and must outlive screens; actual cGame construction is required from the engine and has no default stub',
            'The diagnostic game factory only counts callbacks; original 004832f0 is redirected at that boundary and 00413210 is not executed. Full game/world is unimplemented',
            'Native profile service uses existing explicit 88-byte header/name adapter; source uses actual ProfileSession persistence/defaults',
            'Native XML/texture/font/button-sound factories and original-CRT String/typed/origin formatting use existing checked service adapters',
            'Native resource-log virtual call counts exact filename; source writes actual file and writer byte-equivalence is separately reported in resource-log validation',
            'SoundManager is disabled in this navigation diagnostic, click sound requests are matched; physical audio is compared separately',
            'Generic/MainMenu discard insertion is recorded in diagnostic maps; Loading retains original discard tree instructions, origin and native deletion are compared',
            'Allocation ranks use diagnostic monotonic heap for original equal-depth pointer ordering; clock/cursor/input events controlled, real graphics reset is not exercised'],
        pending=['Required actual cGame construction and engine services, all world systems/tutorial/gameplay/save compatibility',
            'Complete physical window/focus/DirectInput lifecycle and combined audible navigation/audio',
            'Full original object/exception ABI, exceptional construction, full repeated MainMenu XML sound overwrite ownership',
            'LogResources false and missing GameResLog Start branches; initial profile selector before first menu and null-profile exceptional return'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('MainMenu: 26 real input actions, 36 Loading steps, 234 GPU frames / 112320000 identical RGB pixels')
if __name__=='__main__':main()
