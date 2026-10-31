#!/usr/bin/env python3
"""Record actual delayed acceptance and the resolved translucent-panel observation."""
import collections
import datetime
from record_live_production_verification import ROOT, digest, read, write, checked
from contract_offer_pixels import modal_sprites, panel_alpha
from PIL import Image, ImageChops


def main():
    state=read(ROOT/'.local/contract-delay-acceptance-state-validation.json');checked(state)
    assert state['status']=='passed_for_natural_rose_contract_delay_acceptance_and_active_save_only'
    assert state['comparison']['complete_identical_save_bytes']==4089
    state['audit_files']={name:digest(ROOT/name) for name in ['tools/validate_contract_delay_acceptance_state.py','tools/contract_offer_pixels.py']}
    diagnostic=read(ROOT/'.local/contract-note-diagnostic/20261031T162919074773Z/report.json');work=checked(diagnostic)
    scenes=[s['final_scene'] for s in diagnostic['sessions']]
    assert modal_sprites(scenes[0])==modal_sprites(scenes[1]) and sum(modal_sprites(scenes[0]).values())==118
    images=[Image.open(work/f'{i}-offer-sample-2.png').convert('RGB') for i in [0,1]]
    assert images[0].crop((140,100,660,170)).tobytes()==images[1].crop((140,100,660,170)).tobytes()
    delta=ImageChops.difference(*(im.crop((210,170,590,405)) for im in images))
    assert max(hi for lo,hi in delta.getextrema())==1
    assert collections.Counter(panel_alpha(ROOT).crop((100,105,480,340)).tobytes())=={254:89300}
    initial_work=ROOT/'.local/contract-delay-acceptance/20261031T154530797910Z';initial=read(initial_work/'report.json')
    assert digest(initial_work/'driver.py')==initial['source_files']['tools/validate_game_contract_delay_acceptance.py']
    failed=read(ROOT/'.local/contract-note-diagnostic/20261031T163743026805Z/report.json')
    assert failed['status']=='failed' and 'Snapshot window does not belong' in failed['error']
    replay=ROOT/'.local/contract-render-trace';font=ROOT/'.local/contract-note-diagnostic/20261031T170552898337Z/runtime/renderer-font.bin'
    assert (replay/'replay-source.bgra').read_bytes()==(replay/'replay-native.bgra').read_bytes()
    assert (replay/'replay-source.bgra').stat().st_size==800*600*4
    assert font.read_bytes()==(replay/'replay-font.bin').read_bytes() and font.stat().st_size==8388608
    observation=dict(status='resolved_original_alpha254_backdrop_difference_without_production_change',
        initial_work_directory=initial['work_directory'],initial_report_sha256=digest(initial_work/'report.json'),preserved_initial_driver_sha256=digest(initial_work/'driver.py'),
        ordinary_build_diagnostic_work_directory=diagnostic['work_directory'],ordinary_build_diagnostic_report_sha256=digest(work/'report.json'),
        corrected_false_visual_claim='The offer note was never missing: all36400 note RGB pixels and all118 textured modal sprites match.',
        original_panel_alpha_in_all89300_audited_pixels=254,ordinary_backdrop_different_pixels=sum(any(p) for p in delta.getdata()),ordinary_backdrop_max_channel_difference=1,
        independent_frozen_world_animation_phases_synchronized=False,files_or_compared_pixels_normalized=False,
        isolated_full_frame_replay=dict(identical_rgb_pixels=480000,direct_source_device_calls=True,dynamic_buffer_replays=200,original_reference_method='00515830',original_entry_point_executed=False,
            production_implementation=False,source_sha256=digest(replay/'Replay.cpp'),cmake_sha256=digest(replay/'CMakeLists.txt'),source_bgra_sha256=digest(replay/'replay-source.bgra'),native_bgra_sha256=digest(replay/'replay-native.bgra'),
            equal_actual_font_texture_bytes=8388608,font_texture_sha256=digest(font)),
        diagnostic_failure=dict(work_directory=failed['work_directory'],report_sha256=digest(ROOT/failed['work_directory']/'report.json'),error=failed['error'],
            scope='Private COM-vtable experiment only; discarded. Not a game failure or production implementation.'),
        discarded_diagnostics='Private renderer variants, post-Unlock pointer reads and encoding-failed CMake attempts were exploratory, not production validation.',
        production_implementation_changed=False,whole_game_complete=False)
    reports={}
    for name,value in [('contract-delay-acceptance-state-validation.json',state),('contract-offer-rendering-observation.json',observation)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    write(ROOT/'analysis/contract-delay-acceptance-regression.json',dict(status='passed_actual_delayed_rose_offer_charged_acceptance_and_saved_active230_seconds_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,business_snapshots_per_executable=4,deposit_paid=30,money_after=4504,roses_retained=2,
        active_task=37,saved_remaining_seconds=230,identical_complete_save_bytes=4089,production_implementation_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Opposite-program active restore and actual expiry','Further failure-state bidirectional saves','Remaining full original goal scopes']))
    print('Recorded actual delayed offer37, paid acceptance and active230-second save; production unchanged.')


if __name__=='__main__':main()
