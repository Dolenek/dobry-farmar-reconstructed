#!/usr/bin/env python3
"""Compare all original Gameplay definitions and x87 time conversion."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/gameplay-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','gameplay'),('PC24','gameplay-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=60)
        if result.returncode:raise RuntimeError(f'Gameplay {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['gameplay_time_states']==208643 and state['gameplay_timeline_states']==10
        assert state['gameplay_event_states']==2386 and state['gameplay_contract_states']==794
        assert state['gameplay_popularity_states']==938 and state['gameplay_objects']==2 and state['gameplay_reloads']==8
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/gameplay_oracle_cases.inc',ROOT/'tools/gameplay_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_gameplay_definitions_and_time_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        time_states=417286,timeline_states=20,event_states=4772,contract_states=1588,popularity_states=1876,objects=4,xml_reloads=16,
        shipped_event_count=383,shipped_event_type_population=[22,7,43,7,4,131,155,3,10,1],
        original_functions=['004adac0','004b09e0','004b0a40','004b0a60','004b0a80','004b0b10','004b0b30','004b0b50',
            '004b1240','004b1710','004b1bd0','004b2040','004b2500','004b29c0','004b3300','004b3a00','004b3e70','004b4340','004b4a70','004b4b60','004b50d0','004aeb10'],
        coverage=['Actual Gameplay/Gameplay.xml: all 383 events, ten types, full common and per-type payload comparison after repeated reload',
            'Stable equal-time insertion, unsigned lexicographic time, ignored unknown types, retained list on reload and absent Events',
            'Both contract requirements and initial copies, both popularity vectors, 2/19 entry limits, LosNote and LetterTime at native +a0',
            'Original native x87 time conversion versus source implementation for 200000 arbitrary float bit patterns and 8643 time-boundary inputs per precision',
            'Negative values, infinities, NaNs and extreme floats; three uint32 components and preserved x87 control word',
            'Original derived constructors, per-type XML loaders and sorted timeline loader execute; timeline storage clears on native destruction'],
        test_adapters=['Original EXE maps for diagnosis without entry point; production components never load original code',
            '64-byte native fixture supplies empty vectors and explicit end-index sentinel; full Gameplay constructor and profile restoration do not execute',
            'Native XML tree and typed numeric parsing are old-CRT service boundaries; 004b0b70 string-time parser uses independent diagnostic adapter',
            'Native monotonic diagnostic allocation is zeroed: unspecified record scalars are compared as representations without claiming normal original malloc defaults',
            'Native destructor releases common Note only; source RAII also frees derived contract strings/popularity vectors, differing from original retained allocations'],
        pending=['Gameplay constructor 004b59c0, randomization 004ae570 and step 004b5c60; actual world dispatch and full cGame integration',
            'Native string-time parsing, invalid input/exception and allocation ABI, non-default locale and other x87 rounding modes',
            'Whole game, tutorial, all world systems, save/load compatibility and playable source-built EXE'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Gameplay definitions: 383 original events, 4772 payload states, 417286 time conversions; 17 originals unchanged')
if __name__=='__main__':main()
