#!/usr/bin/env python3
"""Compare cGameInput lifetime/rolling UTF16 recognition against original bodies."""
import argparse,datetime,hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_game_input_utf16_suffix_actions_awards_and_lifetimes'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/game-input-reference-test.exe');p.add_argument('--output',type=Path,default=ROOT/'.local/game-input-validation.json');a=p.parse_args();exe=a.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
 inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
 def unchanged():
  for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
 unchanged();run=subprocess.run([str(exe),str(original/'Dobrý farmář.exe')],capture_output=True,text=True,cwd=ROOT,timeout=30)
 if run.returncode:raise RuntimeError(f'GameInput:{run.returncode} {run.stderr}')
 d=json.loads(run.stdout)
 for key,value in dict(lifetimes=6,states=33345,characters=33333,game_service_events=1038,clear_all_destructions=6,utf16_c_locale_units=65536,reference_relocated=True,game_action_boundaries=True).items():assert d[key]==value,(key,d[key])
 unchanged();extra=['CMakeLists.txt','tools/validate_game_input.py','tools/game_input_reference_test.cpp','tools/game_input_oracle_fixups.inc','tools/build_sprite_oracle_fixups.py'];paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
 report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=17,original_entry_point_executed=False,original_program_code_used_in_implementation=False,oracle_path=exe.relative_to(ROOT).as_posix(),oracle_sha256=digest(exe),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},result=d,original_functions=['004030e0','00401de0','00402400','00402240','00401140','00580415'],coverage=['Six singleton lifetimes and exact keyboard registration; destructor clears ALL character handlers, not only own binding','33333 actual dispatched UTF16 units, rolling20-unit buffer and exact retained allocation capacity after every character','All5 game command suffixes, mixed/upper case, no-game suppression and all21 independent requested awards','Beep1000Hz200ms only for cheatall, then exact money/upgrades/seeds/products service order','All65536 units of actual original C-locale towlower; temporary suffix fold leaves rolling buffer unchanged'],test_adapters=['Original mapped away from preferred address without entry point; all native callback/string/constructor/destructor bodies execute','Native game action entry points are explicit service trace boundaries; real production default calls actual Game methods','Original Input virtual bind/clear backed by actual source InputBindings; Award request is a224-byte native fixture','Only native string global initialized explicitly; native C-locale TLS getptd boundary and allocation/free/memmove adapted'],pending=['Whole native/source process command effects and saved profile comparison','Award request to actual effect/panel sequences via whole process input','All connected gameplay/tutorial/save branches, complete engine/Squall97/uninstaller'])
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print('GameInput:33333 UTF16 units/33345 states/1038 action events;6 lifetimes;21 award suffixes;65536 native case folds;17 originals unchanged')
if __name__=='__main__':main()
