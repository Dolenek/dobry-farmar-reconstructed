#!/usr/bin/env python3
"""Compare actual BuyBuilding, PictureButton, tooltips and connected HUD/tutorial."""
import argparse
import datetime
import hashlib
import json
import os
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/buy-building-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','buy-building'),('PC24','buy-building-device')]:
        shards=[]
        for shard in range(16):
            env=os.environ.copy();env['FARMAR_BUILDING_TEST_SHARD']=str(shard)
            result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,env=env,capture_output=True,text=True,timeout=60)
            if result.returncode:raise RuntimeError(f'BuyBuilding {precision} shard {shard}: {result.returncode} {result.stderr}')
            state=json.loads(result.stdout)
            assert state['buy_building_states']==(831 if shard==0 else 641),state['buy_building_states']
            assert state['buy_building_starts']==(140 if shard==0 else 128)
            assert state['buy_building_frames']==(91 if shard==0 else 0)
            assert state['hud_frames']==state['buy_building_frames'] and state['hud_pixels']==state['hud_frames']*480000
            assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
            state['shard']=shard;shards.append(state)
        counts={key:sum(s['buy_building_'+key] for s in shards) for key in ['states','starts','frames','choices','letters']}
        assert counts['states']==10446 and counts['starts']==2060 and counts['frames']==91 and counts['choices']==1368,counts
        contexts.append(dict(precision=precision,shards=shards,**counts));unchanged()
    assert {k:contexts[0][k] for k in ['states','starts','frames','choices','letters']}=={k:contexts[1][k] for k in ['states','starts','frames','choices','letters']}
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/game_menu_oracle_cases.inc',ROOT/'tools/game_menu_oracle_services.h',ROOT/'tools/tutorial_manager_oracle_services.h',ROOT/'tools/buy_building_oracle_cases.inc',ROOT/'tools/buy_building_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_buy_building_ui_profile_and_tutorial_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,oracle_sha256=digest(oracle),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        **{key:sum(c[key] for c in contexts) for key in ['states','starts','frames','choices','letters']},pixels=87360000,
        availability_masks_per_precision=2048,cursor_probes=160,stencil_format='D24S8',
        original_functions=['00491690','0048edf0','0048eef0','004928e0','0048dea0','0048e130','0048e550','0048e980','0048e9e0','0048df00','0048df50','0048e030','005070d0','005071b0','00507660','00507af0','0048fa00','00491900','0048fc60'],
        coverage=['Actual shipped BuyBuilding.xml, eleven 68-byte definitions, eleven PictureButton with owned solid-color overlays, ExtendedButton tooltip masks, TextMoney/coins and TextLine labels',
            'Every 2048 UpAvail uints mask in both precision contexts, zero/available and -1/unavailable behavior, actual GameConfiguration upgrade costs and product icons',
            'All eight tutorial enable/processed combinations for 35/36 and ordered two Money queries, unsigned price format, signed availability gating',
            'Actual mouse hover/press/release for first/last building and outside cancellation, stencil writes/tests and 182 full GPU frames with matching ordered sprites, draw API calls and RGB pixels',
            'Real TutorialManager guide 35 disabled-anchor then 36 repeated-action, OK and purchase input, profile updates and original double completion',
            'Cursor rectangles, input binding lifetime, owned resource destruction and source/native HUD/profile/world-query comparisons'],
        corrections=['Native control +1b0 is dirty, not deepProcessing (+26a). Corrected both purchase controllers; direct source/native assertions now cover both fields'],
        test_adapters=['Reference instructions are executed only by diagnostic oracle; original entry point never runs and production sources do not call it',
            'Existing 512 MiB monotonic test heap requires sixteen independent 128-mask shards; validator requires all sixteen per precision, GUI scenario runs in shard zero',
            'Both renderings start with identical stencil/alpha states and separately cleared D24S8 stencil buffers; comparison keeps all native graphics API calls',
            'Native Manufacture 00494390 is an observed diagnostic boundary. Fixture records type/slot and supplies the actual selection predicate signal; actual building factories/world effects remain pending',
            'Earlier HUD navigation tutorial flags are preprocessed only for the isolated 35/36 purchase scenario; the full tutorial sequence remains pending',
            'Silent sound observation uses recovered resources; physical full-game audio remains pending'],
        pending=['Actual Manufacture lifecycle, all eleven building factories, placement, production/upgrade/destroy effects, source cGame and scene integration',
            'XML reload/malformed/extreme configuration branches, other stencil hardware formats, physical mouse/keyboard and full tutorial play sequence',
            'Main game EXE and bidirectional full-world saves, complete Squall and isolated uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(f"BuyBuilding: {report['states']} states, {report['choices']} choices, 182 D24S8 frames/87360000 pixels; 17 originals unchanged")
if __name__=='__main__':main()
