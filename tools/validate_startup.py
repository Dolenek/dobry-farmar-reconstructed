#!/usr/bin/env python3
"""Compare the original Startup logo sequence and both profile/menu branches."""
import argparse,datetime,hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_startup_logos_timing_input_and_actual_profile_menu_transitions'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/startup-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'startup'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if run.returncode:raise RuntimeError(f'Startup {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        for key,value in dict(states=397,actions=6,frames=393,pixels=188640000,objects=2).items():assert state['startup_'+key]==value,(precision,key,state['startup_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_startup.py','tools/text_buttons_reference_test.cpp','tools/build_sprite_oracle_fixups.py',
           'tools/text_buttons_oracle_fixups.inc','tools/monotonic_ui_test_heap.h','tools/startup_oracle_cases.inc',
           'tools/main_menu_oracle_services.h','tools/select_profile_oracle_services.h','tools/loading_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        contexts=contexts,states=794,input_actions=12,gpu_frames=786,gpu_rgb_pixels=377280000,startup_lifetimes=4,
        original_functions=['0048b4a0','0048b920','0048b030','00489ef0','0048ab10','0048aa90','00489e80','0048bd10'],
        coverage=['All three original JPEG logos, automatic four-second changes, alpha progression and actual input skipping',
            'Initial process input toggle0065d290 is1; key-down/up dispatch and original deferred deletion are compared',
            'Actual LoadingFromLog and native completion bodies create first-profile NewUser or existing-profile MainMenu',
            'Source ProfileSession persists StartupFarm and reopens it; actual owned UI state and all786 RGB frames match'],
        test_adapters=['Original mapped without entry point only as diagnostic; production runs reconstructed code',
            'Native Profile headers/name and constructor use the existing explicit service adapter; source ProfileSession uses isolated real files',
            'Controlled frame clock and input; shared XML/font/texture/sound factories and CRT use existing documented boundaries',
            'Physical audio disabled in this native UI comparison and independently audited',
            'Reusable screenshot buffers avoid monotonic diagnostic heap growth; UI allocation order remains monotonic'],
        pending=['Full original process/runtime comparison and main application frame/reset fidelity',
                 'Bidirectional whole-game saves','All Squall97 behavior and isolated uninstaller','Native exception ABI'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Startup:4 lifetimes /794 states /12 actions;786 frames /377280000 identical RGB pixels;17 originals unchanged')
if __name__=='__main__':main()
