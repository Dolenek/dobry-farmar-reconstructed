#!/usr/bin/env python3
"""Record actual accepted-contract expiry and complete failure-state transfers."""
import datetime
from record_live_production_verification import ROOT, digest, read, write, checked


def main():
    state=read(ROOT/'.local/contract-expiry-state-validation.json');checked(state)
    assert state['status']=='passed_for_opposite_program_contract37_restore_natural_expiry_and_failure_save_only'
    c=state['comparison'];assert c['opposite_program_active_restores']==2 and c['complete_identical_save_bytes']==4563 and not c['normalizations']
    state['audit_files']={'tools/validate_contract_expiry_state.py':digest(ROOT/'tools/validate_contract_expiry_state.py')}
    interop=read(ROOT/'.local/contract-expiry-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_confirmed_contract37_failure_and_retained_state_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/contract-expiry-state-validation.json')
    transfers=interop['comparison']['transfer_comparisons'];assert len(transfers)==len(interop['sessions'])==4
    for row in transfers:assert row['identical_complete_save_bytes']==4563 and not row['normalizations']
    for s in interop['sessions']:assert s['exit_code']==0 and s['status']=='completed' and s['tutorial_step']==-1
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[readline for line in path.read_text(encoding='utf-8').splitlines() if (readline:=__import__('json').loads(line))]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    observation=dict(status='diagnostic_alpha_compression_assumption_corrected_only',
        original_offer_alpha_compression='JPEG JDAA',original_failure_alpha_compression='PNG IDAT',
        initial_auditor_error='PIL.UnidentifiedImageError while treating the failure panel as JPEG alpha',
        resolution='Read original JHDR and reconstruct its grayscale PNG alpha stream; original pixels are unchanged.',
        failure_audit_alpha_values=[254,254],production_implementation_changed=False,recorded_as_game_failure=False,whole_game_complete=False)
    reports={}
    for name,value in [('contract-expiry-state-validation.json',state),('contract-expiry-interop-validation.json',interop),('contract-expiry-initial-observation.json',observation)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    write(ROOT/'analysis/contract-expiry-regression.json',dict(status='passed_actual_cross_program_active_contract37_expiry_failure_and_four_complete_restores_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        business_snapshots_per_executable=4,opposite_program_active_restores=2,natural_deadline_failures=2,additional_charge_at_expiry=0,roses_retained=2,
        equal_actual_failure_modal_sprites=29,exact_caption_rgb_pixels=c['exact_caption_rgb_pixels'],backdrop_pixel_comparison=c['backdrop_max_channel_difference'],original_backdrop_alpha=254,
        added_original_event_messages=2,identical_complete_save_bytes=4563,further_bidirectional_restores=4,source_restore_debugger_traces=traces,
        nonzero_streak_reset_verified=False,controlled_whole_frame_background_animation_replay_verified=False,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Natural failure after a nonzero completed streak','Remaining natural world events, awards and upgrades','Remaining complete Squall/JNG/uninstaller scopes']))
    print('Recorded two real cross-restored contract failures and four exact4563-byte transfers; production unchanged.')


if __name__=='__main__':main()
