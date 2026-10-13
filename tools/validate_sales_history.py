#!/usr/bin/env python3
"""Compare cGame sale history through actual Market sales and native list methods."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/sales-history-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','sales-history'),('PC24','sales-history-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Sales history {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['sales_history_states']==2869 and state['sales_history_records']==1376 and state['sales_history_prunes']==700
        assert state['market_states']==792 and state['market_actions']==561 and state['market_frames']==84 and state['market_pixels']==40320000
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/market_oracle_cases.inc','tools/market_oracle_services.h','tools/monotonic_ui_test_heap.h',
        'tools/game_menu_oracle_cases.inc','tools/game_menu_oracle_services.h','tools/warehouse_oracle_cases.inc',
        'tools/sales_history_oracle_setup.inc','tools/sales_history_oracle_cases.inc','tools/build_sprite_oracle_fixups.py']
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[ROOT/path for path in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_actual_market_sale_history_and_awEarn',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        states=5738,explicit_records=2752,explicit_prunes=1400,connected_market_states=1584,connected_gpu_frames=168,connected_gpu_rgb_pixels=80640000,
        original_functions=['0040c2b0','00410f10','00522f00','00522f20','0040a290'],
        coverage=['Actual Market recordSale calls use real recovered/native history operations; history method is no longer replaced by an observer in this mode',
            'Native linked-list count, next/previous/sentinel order, each timestamp word and unsigned amount, wrap32 total and float cutoff compared after every operation and Market state',
            'Strict time-60 cutoff, exact/equal/nextafter boundaries, original successor-skipping removal including sentinel wrap, out-of-order time sequences',
            'Amount words 0/1/998/999/1000/FFFFFFFF/80000000/7FFFFFFF and 2000 deterministic random records/prunes per precision',
            'Both zeros, subnormals, extreme finite times, infinities, quiet/signaling NaN words and unordered stopping; sticky awEarn profile flag',
            'Complete connected Market stock, sale, sound-request, movement, tutorial and pixel checks retained'],
        test_adapters=['Reference EXE mapped only for diagnosis, entry point never executed',
            'Explicit fake cGame storage hosts the actual native list fields at +9f8/+9fc/+a00/+a04; actual list constructor, insert/count and history instructions execute',
            'Allocator, string/profile/resource and controlled UI clock/input boundaries remain explicit; entire cGame construction is not claimed',
            'Native list storage is explicitly released after comparisons; production SalesHistory uses private editable std::list storage',
            'FlyProductExpence and complete Money effects remain observed world boundaries; physical audible playback remains pending'],
        pending=['Entire cGame and flying text expense/income effects, all Money effects',
            'Original saved-game sales block serialization and bidirectional full-game compatibility',
            'Allocation/length-error exception ABI and normal allocator identities, non-default x87 rounding modes/locales'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Sales history: 5738 states, 2752 records, 1400 prunes, real Market -> history/awEarn chain; 17 originals unchanged')
if __name__=='__main__':main()
