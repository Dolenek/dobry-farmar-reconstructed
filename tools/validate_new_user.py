#!/usr/bin/env python3
"""Compare actual first-profile NewUser and its transition to the real MainMenu."""
import argparse,datetime,hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_new_user_font_filter_profile_creation_menu_transition_and_frames'
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/new-user-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),'new-user'+suffix],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if run.returncode:raise RuntimeError(f'NewUser {precision}: {run.returncode} {run.stderr}')
        state=json.loads(run.stdout)
        for key,value in dict(states=44,actions=14,frames=28,pixels=13440000,objects=2,sound_residues=8).items():assert state['new_user_'+key]==value,(precision,key,state['new_user_'+key])
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/validate_new_user.py','tools/text_buttons_reference_test.cpp','tools/build_sprite_oracle_fixups.py',
           'tools/text_buttons_oracle_fixups.inc','tools/monotonic_ui_test_heap.h','tools/new_user_oracle_cases.inc',
           'tools/main_menu_oracle_services.h','tools/select_profile_oracle_services.h','tools/loading_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        contexts=contexts,states=88,input_actions=28,gpu_frames=56,gpu_rgb_pixels=26880000,new_user_lifetimes=4,
        retained_xml_sound_references=16,
        original_functions=['00506780','00506920','005069b0','00505bf0','005053a0','00504fc0','005062d0','005066f0',
                            '00506720','00506750','00481990','0048a680'],
        coverage=['Actual first-profile singleton, owned animated OK/Edit/text labels and XML layout; exact view/control/caret/transition/input state',
            'Empty Enter rejection, typed/delete/Unicode/unsupported characters, shipped font rejects space, empty/nonempty OK hide/show/disable/enable',
            'Complete XML reload appends retained owned decoration labels; old source/native button sounds each retain one reference as original',
            'Actual acceptance by Enter and mouse through normal input dispatch, source ProfileSession persists two names, actual native completion constructs/restores MainMenu',
            'Four lifetimes, including reuse of the same menu, deferred discard origins and cleanup; 56 exact RGB frames across PC53/PC24'],
        test_adapters=['Original mapped without entry point and used only in diagnostic; production NewUser calls reconstructed source only',
            'Native Profile uses existing explicit header/name service adapter; source uses real isolated ProfileSession persistence/defaults. Native Profile construction is not executed here',
            'Native XML/font/texture/sound factories and old CRT use existing checked diagnostic boundaries',
            'SoundManager disabled in this UI comparison; actual click/hover requests match, physical audio has separate tests',
            'Controlled clock/cursor/input and original heap ordering; diagnostic releases XML-overwritten sound references only after proving identical native/source retention'],
        pending=['Actual startup/logo/engine/main EXE and full native cGame comparisons','Bidirectional whole-game saves',
                 'Complete Squall97 behavior and isolated source uninstaller','Native exception ABI and complete first-user startup from original executable'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('NewUser: 4 lifetimes / 88 states / 28 actions; persisted source profiles and actual menu; 56 frames / 26880000 identical RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
