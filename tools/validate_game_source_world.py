#!/usr/bin/env python3
"""Exercise the real source cGame; explicitly does not assert native parity."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_source_game_factory_input_tutorial_and_binary_roundtrip_only'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--test',type=Path,default=ROOT/'build/windows/Release/game-source-runtime-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/game-source-world-validation.json')
    args=parser.parse_args();test=args.test.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,extra,flags in [('PC53',['pc53'],34),('PC24',[],32)]:
        command=[str(test),str(original/'Data'),str(ROOT/'.local/game-source-world-profiles')]+extra
        result=subprocess.run(command,cwd=ROOT,capture_output=True,timeout=120)
        assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
        state=json.loads(result.stdout);assert state['source_game_lifetimes']==2 and state['device_flags']==flags
        assert state['input_clicks']==8 and state['frames']>24 and state['submitted_controls']>0 and state['triangles']>0
        assert state['save_bytes']>100 and state['native_comparison'] is False
        contexts.append(dict(precision=precision,result=state));unchanged()
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/p for p in [
        'CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/game_source_runtime_test.cpp','tools/validate_game_source_world.py']])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},test_path=test.relative_to(ROOT).as_posix(),test_sha256=digest(test),
        dll_files={p.relative_to(ROOT).as_posix():digest(p) for p in [test.parent/'JNGLoad.dll',test.parent/'Squall.dll']},
        contexts=contexts,reference_files_unchanged=len(inventory['files']),original_program_code_used_in_implementation=False,
        source_game_lifetimes=4,mouse_clicks=16,source_save_restore_save_bytes_equal=False,source_save_restore_save_compared_with_native_seed_x_quirk=True,whole_game_complete=False,
        coverage=['Actual cGame factory and all concrete world bindings with sixty Fields, actual Person, tools, Market, Manufacture, HUD, Tutorial, dialogs, Help, Journal and automatic award controller',
                  'Actual masked tool mouse click, Person queue/walk/tool pickup and tutorial20 completion; actual field input and tutorial21 completion',
                  'Actual HUD mouse Journal open/read/return, pause/Help/return and whole Game resume',
                  'Actual .save creation, source Game destruction/recreation, seed metadata/world coordinates, UTF16 messages, replayed SalesHistory and Gameplay coefficients; save/restore/save bytes with only the instruction-derived +1 seed X drift'],
        pending=['Native whole-cGame construction, event dispatch, teardown, frames and input comparison',
                 'Original to reconstructed and reconstructed to original save compatibility',
                 'Remaining tutorial/worker/system paths and native award predicates7/9/18',
                 'Complete interactive engine/main EXE startup and first-user UI',
                 'Complete Squall97-export behavioral parity and isolated uninstall effects',
                 'Native corrupt/truncated save and exception behavior; full engine cleanup of independent Tutorial tips'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:report[k] for k in ['status','source_game_lifetimes','mouse_clicks','source_save_restore_save_compared_with_native_seed_x_quirk','whole_game_complete']}))
if __name__=='__main__':main()
