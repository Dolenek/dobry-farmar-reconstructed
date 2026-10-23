#!/usr/bin/env python3
"""Compare ordinary Person motion with actual Field, SeedPackage and mouse queue."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/person-walk-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']: assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'person-walk'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=120)
        if run.returncode: raise RuntimeError(f'Person walk {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        assert state['person_walk_frames']==48 and state['person_walk_pixels']==23040000
        assert state['person_walk_states']==653 and state['person_walk_actions']==605
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_person_walk.py','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/build_sprite_oracle_fixups.py','tools/monotonic_ui_test_heap.h','tools/person_walk_oracle_cases.inc',
        'tools/person_visuals_oracle_cases.inc','tools/person_tasks_oracle_cases.inc','tools/farm_field_oracle_cases.inc',
        'tools/farm_field_oracle_services.h','tools/seed_package_controller_oracle_cases.inc','tools/gameplay_runtime_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    total=lambda key:sum(c['result']['person_walk_'+key] for c in contexts)
    report=dict(status='passed_for_person_ordinary_motion_field_seed_and_mouse_queue',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,
        original_program_code_used_in_implementation=False,oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        precision_contexts=contexts,states=total('states'),actions=total('actions'),gpu_frames=total('frames'),gpu_rgb_pixels=total('pixels'),
        original_functions=['004520a0','00452630','0044e780','00418800','0044fe20','0044f1a0','0044f250','0044d2f0','0044e300','0044d790','0045e130','0045cf50'],
        coverage=[
            'Actual Field goals and Person hotspot/movement appearance for five carry values, four tool levels, near/exact/beyond-threshold starts and shifted actual Farm movingPicture',
            'Exact velocity, remaining, relative goal, near flag, world pose, animation time and XPersPos/YPersPos including final snap in PC53 and PC24',
            'Unordered X/Y near comparisons, zero/negative/slow/fast speeds, explicit cancel and both branches of actual position/profile setter',
            'Actual moving SeedPackage terminates motion without displacement; actual status setter installs it and motion resumes to its goal',
            'Actual mouse input enqueues an actual Field task/marker; FIFO consumer moves rendered actor through arrival and actual marker completion/discard',
            'Every recorded frame compares exact D3D8 draw calls and all RGB pixels; texture ownership and deferred cleanup are checked'],
        test_adapters=[
            'Explicit FieldObject drawing/input base on both sides; full Person constructor and absoluteProcess are pending',
            'Native 48-byte motion data adapter invokes actual original begin/step; native step boolean ABI is AL only',
            'Actual PersonTasks and Field/SeedPackage controllers execute; test consumes motion and separately completes the marker, without substituting an arrival dispatcher',
            'Mandatory Dig/Mow and carried-seed observations remain explicit data boundaries; shipped configuration is adapted as previously verified data',
            'Controlled GameTimer clock with actual UI elapsed-time behavior in the rendered scene; direct arithmetic cases supply explicit elapsed values',
            'Actual View root activates inherited flags of the Farm scene; parent reassignment alone does not update inherited enabled flags in the original',
            'Original normal CRT math mode and SSE2 floor feature are initialized as in previously verified scenes; exceptional CRT ABI is unverified',
            'Mapped original code is diagnostic reference only and never used by production implementation'],
        pending=['Arrival dispatcher0045a730 and concrete working actions','Vehicle/axis movement00452bc0/00453380',
            'Full Person constructor/lifecycle, idle/working/carry/sound behavior','Complete Farm/cGame EXE and bidirectional complete save files',
            'Complete Squall behavior and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(f"Person walk: {total('states')} states / {total('actions')} actions, {total('frames')} frames / {total('pixels')} identical RGB pixels; actual Field mouse queue and Seed target; 17 originals unchanged")
if __name__=='__main__': main()
