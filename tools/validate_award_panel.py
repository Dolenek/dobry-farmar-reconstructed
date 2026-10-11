#!/usr/bin/env python3
"""Compare the actual AwardPanel, all 21 borrowed effect labels and its input/render tree."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/award-panel-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','panel'),('PC24','panel-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'AwardPanel {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['panel_states']==441 and state['panel_objects']==21 and state['panel_reloads']==42
        assert state['panel_frames']==84 and state['panel_pixels']==40320000
        assert state['panel_callbacks']==21 and state['panel_letters']==22794 and state['panel_sound_residues']==252
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/award_panel_oracle_cases.inc',ROOT/'tools/awards_oracle_services.h',ROOT/'tools/loading_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_award_panel_owned_tree_and_shipped_2d_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        control_states=882,owned_panels=42,bulk_xml_reloads=84,accepted_input_callbacks=42,
        gpu_frames=168,gpu_rgb_pixels=80640000,letter_comparisons=45588,retained_button_sound_references=504,
        original_functions=['004761c0','00475300','00475ae0','00475700','00475b00'],
        coverage=['Actual UI/AwardPanel.xml, all 21 borrowed AwardEffect labels, owned OK TextAnimationButton and both MultilineText controls',
            'Native/recovered constructor/XML/destructor, stored AwardPicPosition, borrowed effect survives panel destruction, full root sprite bytes and ownership deltas',
            'Repeated show/hide/enable/disable; root Sound shown callback, identical last-reference destruction and confirmed earlier reload retention',
            'All 21 shipped congratulation strings, actual Text0 ignored by native Text section lookup; explicit post-frame Text XML exercises the otherwise absent font/text branch',
            'Actual whole native/recovered frame traversal, ordered draws and exact RGB for hover, press and release of OK; 42 accepted callbacks',
            'Repeated button XML reference retention, Sound replacement and empty Sound reload preserving prior resource'],
        test_adapters=['Original PE mapped only in diagnostics, entry point disabled; production never calls original code',
            'Existing native allocation, XML/font/mask extraction, texture and Button sound fixture boundaries',
            'Root Sound resource uses the recovered real cache/cSound; disabled SoundManager callback recorder calls its actual disabled path. Shipped root Sound is empty; nonempty branch is an explicit fixture',
            'Controlled recovered WinMM timer/cursor/input services, 800x600 D3D8 device, monotonic 512 MiB diagnostic heap',
            'Separate Awards validation now compares actual original/recovered AwardPanel factories, pending effect, confirmation condition commit and queued owned deletion'],
        pending=['Gameplay award condition detection and world award manager; complete game/UI lifecycle and MainMenu navigation',
            'Audible root Sound playback, fullscreen/reset, RTL and other screen dimensions',
            'Malformed XML, missing/null borrowed effect and original C++ exception/object ABI',
            'Full game EXE and bidirectional saved-game compatibility'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('AwardPanel: 882 states, 42 input callbacks, 168 GPU frames / 80640000 identical RGB pixels')
if __name__=='__main__':main()
