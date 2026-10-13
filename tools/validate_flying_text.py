#!/usr/bin/env python3
"""Compare original cFlyText and actual cGame/Market visual-effect creation."""
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
    parser.add_argument('--connected',action='store_true')
    parser.add_argument('--output',type=Path)
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[];base='flying-market' if args.connected else 'flying-text'
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),base+suffix],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'{base} {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        if args.connected:
            assert state['flying_market_effects']==183 and state['flying_market_states']==876
            assert state['market_states']==792 and state['market_actions']==561 and state['market_frames']==84 and state['market_pixels']==40320000
            assert state['sales_history_states']==2869 and state['sales_history_records']==1376 and state['sales_history_prunes']==700
        else:
            for key,value in dict(states=348,objects=6,frames=144,pixels=69120000,reloads=6,callbacks=3).items():assert state['flying_text_'+key]==value,(key,state['flying_text_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/flying_text_oracle_cases.inc','tools/flying_market_oracle_setup.inc','tools/flying_market_oracle_cases.inc',
        'tools/market_oracle_cases.inc','tools/market_oracle_services.h','tools/game_menu_oracle_cases.inc',
        'tools/sales_history_oracle_setup.inc','tools/sales_history_oracle_cases.inc','tools/monotonic_ui_test_heap.h','tools/build_sprite_oracle_fixups.py']
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[ROOT/path for path in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_actual_market_flying_effects_and_sale_history' if args.connected else 'passed_for_flying_text_path_alpha_and_frames',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),
        reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        original_functions=['0047cc50','0047c250','0047d590','0047d4d0','0047b8e0','0047baf0','0047ba00','0040ef00','0040cf20','0040d450'],
        coverage=['Actual FlyProductExpence/FlyTextExpence/FlyTextIncome XML, original path length/velocity and alpha points with duplicate thresholds',
            'Constructor, reload retaining old icon and current indices/distance, scaled timer movement, alpha interpolation, text dirty flags and original deferred discard',
            'Optional completion callback executes once and clears only its function word, original TextLine glyphs and icon resource reference deltas'],
        test_adapters=['Original PE maps for diagnosis only, entry point never executed',
            'Explicit XML/resource/allocator/CRT adapters and controlled UI clock/input',
            'Deferred discard service is observed; actual source ViewRegistry.deleteDiscarded and original destructor cleanup are explicitly compared after frames',
            'Monotonic diagnostic allocator preserves equal-depth allocation ranks'],
        pending=['Entire cGame constructor/world and all setMoney refresh effects, physical audible output and complete game lifecycle',
            'Original saved-game world serialization and bidirectional compatibility',
            'Malformed/missing path points, zero path length, NaN/inf movement and allocation/exception ABI, additional locales/rounding modes'])
    if args.connected:
        report.update(effects=366,states=1752,connected_market_states=1584,connected_sales_history_states=5738,
            gpu_frames=168,gpu_rgb_pixels=80640000)
        report['coverage']+=['Actual Market -> cGame productExpense -> real FlyingText construction, original product icon copy, and actual SalesHistory/awEarn operations; those methods are not replaced by observers',
            '183 owned effects per precision: every real sale plus twelve real money expense/income factory calls with six uint amounts including FFFFFFFF and 80000000',
            'All created effect glyphs, icons, complete point/alpha records, owner links, 72 explicit movement/alpha steps and actual deferred deletion/refcounts',
            'Existing full connected Warehouse/Manufacture/HUD/WareDetail/Gameplay/tutorial/mouse/Market pixel scenario retained']
        report['test_adapters']+=['The connected diagnostic game View remains hidden to preserve its established world fixtures; owned effects are explicitly stepped. Visible rendering is verified independently by the standalone cFlyText full-frame oracle',
            'Native effect allocations are observed only to identify actual constructed objects; original constructors/factories/steps execute without replacement']
    else:
        report.update(states=696,objects=12,xml_reloads=12,completion_callbacks=6,gpu_frames=288,gpu_rgb_pixels=138240000)
        report['coverage']+=['Actual D3D8 full-frame traversal/submission/draw and every RGB pixel across all three visible effects in PC53 and PC24']
    output=args.output or ROOT/'.local'/('flying-market-validation.json' if args.connected else 'flying-text-validation.json')
    output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Flying Market: 366 actual effects, 1752 states, actual expense/history chain; 17 originals unchanged' if args.connected else 'FlyingText: 696 states, 288 frames / 138240000 identical RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
