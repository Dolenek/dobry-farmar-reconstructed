#!/usr/bin/env python3
"""Compare complete native LoadingFromLog and real resource-log instruction paths."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/loading-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','loading'),('PC24','loading-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Loading {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['loading_states']==451 and state['loading_objects']==7 and state['loading_completed']==6 and state['loading_missing']==1
        assert state['loading_ranges']==432 and state['loading_decorations']==902
        assert state['loading_frames']==12 and state['loading_pixels']==5760000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/loading_oracle_cases.inc',ROOT/'tools/loading_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_loading_and_shipped_resource_log_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        control_states=902,owned_objects=14,completed_screens=12,missing_log_screens=2,resource_ranges=864,
        gpu_frames=24,gpu_rgb_pixels=11520000,
        original_functions=['00480770','0047fff0','004800f0','00480110','00481130','00480d10','00541700'],
        coverage=['Both actual shipped logs, 539 game entries and 148 menu entries, original constructor/XML/steps/destructor and resource dispatcher instructions',
            'Six completed screens per precision, batches 1/3/7/30/UINT_MAX, zero batch and missing log; first-step clear once, retained shared maximum and old-index progress',
            'Native original discard tree insertion, exact origin String, source discard map and native destructor removal; counted completion callback ordering',
            'Actual Loading background/Progress texture tree, all sprite bytes, texture release deltas; native/recovered frame traversal, 24 exact RGB/draw GPU frames'],
        test_adapters=['Diagnostic maps original EXE with entry point disabled; implementation contains no original-code calls',
            'XML indexed sections/attributes use recovered parsed shipped trees; XML deleting destructor boundary counts calls over independently owned immutable trees',
            'Texture/font/sound factories use existing resource/native-layout font adapters. Native dispatcher executes; modern source cache behavior has separate scoped comparisons',
            'SoundManager initialization deliberately fails in this UI scenario; disabled music return and resource replacement are compared. Actual physical audio/Loading is separately tested by validate_game_audio.py',
            'Allocation and String/WString, typed extraction and decimal/concat origin construction are explicit existing diagnostic CRT boundaries',
            'Frame clock/cursor/input are controlled; Loading disabled temporarily during GPU frames to prevent an extra native tick; completion callback counts calls and never pretends to instantiate cGame'],
        pending=['Actual cGame completion, MainMenu action wiring, engine and full gameplay/save compatibility',
            'Type 3 shader log entry absent from both original logs; malformed/missing Type, unknown resource type and missing resource errors',
            'Unsigned range wrap/large totals, total zero NaN progress and exception/object ABI',
            'Original XML resource-cache identity/destructor side effects under repeated file names; independent trees are explicit test boundaries'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('LoadingFromLog: 902 states, 864 resource ranges, 24 GPU frames / 11520000 identical RGB pixels')
if __name__=='__main__':main()
