#!/usr/bin/env python3
"""Compare original Gameplay constructor, scheduling, randomization and save coefficients."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/gameplay-runtime-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','gameplay-runtime'),('PC24','gameplay-runtime-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Gameplay runtime {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        for key,value in dict(states=35184,steps=5052,objects=14,events=1345353,restores=30118).items():
            assert state['gameplay_runtime_'+key]==value,(key,state['gameplay_runtime_'+key])
        assert state['gameplay_random_states']==300000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/gameplay_oracle_services.h',ROOT/'tools/gameplay_runtime_oracle_cases.inc',ROOT/'tools/gameplay_runtime_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_gameplay_constructor_scheduling_randomization_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        states=70368,steps=10104,objects=28,event_states=2690706,coefficient_restores=60236,random_states=600000,
        original_functions=['004b59c0','004b5c60','004ae570','004b4ac0','004b4ad0','004b4920','0042f030','004aeb10','00581021','00580f98'],
        coverage=['Actual 383-event plan and controlled ten-type plan; new game and restored nonzero GameTime; singleton construction/destruction',
            '600000 native rand result/state comparisons across three seeds; native old CRT rand instructions run with explicit thread-data storage adapter',
            'At most one dispatch per frame, contract state 3 skips execution but advances cursor, wrap to beginning and strict restored-time selection',
            'Boundary frames on both sides of every due time, NaN frame, captured GameTime when event callback changes property, repeating end time differs from final timeline time',
            'Every coefficient byte, RNG state and randomized contract/popularity field compared after construction, steps and coefficient reloads',
            '30000 arbitrary float coefficient vectors per precision plus zero, negative, infinities and NaN; wrap32 amounts/unsigned counts and original ten-unit quantization',
            'Native coefficient save-copy and restore-copy/apply instructions; repeated restore accumulates on current values rather than resetting baseline',
            'Empty and zero-time plans neither query GameTime nor schedule events; source actual ResourceManager XML path used for first shipped-plan construction'],
        test_adapters=['Original EXE maps for diagnosis without entry point; implementation never loads original code',
            'Actual native constructor executes on 64-byte storage; XML loading/release use explicit tree/ownership adapter and float property lookup uses actual source ProfileProperties',
            'Original event virtual execution slot is replaced ONLY in diagnostic by trace receiver; required source GameplayEventExecutor has no default/no-op implementation',
            'Contract availability executes original 0042f030 against fixture controller +94; actual contract world controller still pending',
            'Old CRT string/XML numeric and time-string parsing boundaries remain explicit; native x87 conversion and original _ftol2 run',
            'Monotonic test heap zeroes uninitialized scalar representations; normal original allocation defaults and exception ABI are not claimed'],
        pending=['All ten actual world dispatch implementations and game UI, tutorial, full cGame construction and MainMenu completion',
            'Engine startup random seed and shared consumption by all world subsystems, physical gameplay frame loop',
            'Full game save file binary stream and bidirectional original compatibility; coefficient block only is compared here',
            'Native constructor failure when restored time is after final event, invalid coefficient count, allocation/exception ABI',
            'Non-MSVC/non-x86 NaN/inf conversion fallback, non-default locale and other x87 rounding modes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Gameplay runtime: 70368 states, 10104 steps, 2690706 event states, 600000 rand states; 17 originals unchanged')
if __name__=='__main__':main()
