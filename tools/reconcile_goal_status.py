#!/usr/bin/env python3
"""Separate proved source build availability from still-unproved whole-goal completion."""
import datetime
import hashlib
from record_live_production_verification import ROOT,digest,read,write
from current_production_regression import current_regression
from pe import PE


def current_implementation(files):
    for name,sha in files.items():
        actual='Dobr\u00fd farm\u00e1\u0159.exe' if name in ['FarmarSourceInterop.exe','FarmarTutorialWarnings.exe'] else name
        assert digest(ROOT/'build/windows/Release'/actual)==sha,name


def verified_gate(v,key,status):
    gate=v.get(key,{})
    if not gate.get('complete'):return False
    report=read(ROOT/gate['report']);assert report['status']==status
    assert report['current_full_ctest_source_hashes_match'] and not report['production_changed']
    for name,sha in report.get('reports',{}).items():assert digest(ROOT/'analysis'/name)==sha,name
    if key=='current_contract_streak40_gate':
        assert report['final_streak']==40 and report['exact_award_rgb_pixels']==480000
        assert len(report['four_cross_restore_bytes'])==4 and report['source_restore_processes_without_access_violation']==2
        current_implementation(read(ROOT/'analysis/contract-streak-forty-interop-validation.json')['implementation_files'])
    else:
        assert report['exact_panel_roi_rgb_pixels']==598500 and report['saved_owned_and_shown_flags']==42
        raw=report['runtime_report'];assert digest(ROOT/raw['path'])==raw['sha256']
        current_implementation(read(ROOT/raw['path'])['implementation_files'])
    return True


def main():
    name='analysis/production-source-only-build-v4.json';build=read(ROOT/name)
    assert len(build['artifacts'])==4 and build['actual_source_process_exits']==[0,0]
    for artifact in build['artifacts'].values():
        path=ROOT/artifact['path'];assert digest(path)==artifact['sha256'] and PE(path).bits==32
    for path,sha in build['staged_source_manifest']['copied_source_files'].items():
        if path.startswith('src/') or path=='CMakeLists.txt':assert digest(ROOT/path)==sha,path
    current_regression()
    v=read(ROOT/'analysis/verification.json')
    contracts=verified_gate(v,'current_contract_streak40_gate','passed_real40_award5_full_pixels_and_four_whole_cross_restores_only')
    awards=verified_gate(v,'current_award_runtime_gate','passed_current21_requested_panels_effects_returns_saved_flags_and_automatic_final_award_only')
    warnings=False
    warning_gate=v.get('current_tutorial_warning_gate',{})
    if warning_gate.get('complete'):
        warning_report=read(ROOT/warning_gate['report'])
        assert warning_report['status']=='passed_actual_full_carry_storage_capacity_mixed_products_and_contract_warning_paths_only'
        assert set(warning_report['verified_steps'])>={40,41,42,44}
        assert warning_report['original_files_unchanged']==17 and not warning_report['production_changed']
        for path,sha in warning_report['reports'].items():assert digest(ROOT/path)==sha,path
        current_implementation(warning_report['implementation_files']);warnings=True
    audio=False
    audio_gate=v.get('current_game_audio_output_gate',{})
    if audio_gate.get('complete'):
        audio_report=read(ROOT/audio_gate['report'])
        assert audio_report['status']=='passed_nonzero_Windows_process_loopback_from_actual_native_and_source_game_only'
        assert audio_report['original_files_unchanged']==17 and not audio_report['production_changed']
        assert audio_report['current_full_ctest_source_hashes_match']
        current_implementation(audio_report['implementation_files'])
        assert len(audio_report['captures'])==2
        for row in audio_report['captures']:
            for key in ['runtime_report','capture_log','captured_pcm']:
                assert digest(ROOT/row[key]['path'])==row[key]['sha256']
            assert row['nonzero_samples']>44100 and row['actual_game_normal_exit']==0
        audio=True
    decision_path=ROOT/'analysis/native-tracker-fidelity-decision.json'
    accepted=(decision_path.exists() and read(decision_path).get('status')==
        'user_accepted_documented_sufficient_tracker_allocation_exception')
    pending=[]
    host_hold=ROOT/'.local/desktop-runtime-hold.json'
    if host_hold.exists() and read(host_hold).get('active',True):
        pending.append('Resolve reported host Windows/display freeze and validate revised observer/runtime paths under the independently monitored host authorization;unmonitored host interactive tests are held')
    if not contracts:
        pending.extend(['Finish actual40-contract award in both complete executable processes from genuine retained checkpoints',
            'Verify four complete checkpoint40 cross-restores with both current source DLLs'])
    if not awards:pending.append('Finish current final collect-all award lifecycle comparison,keeping requested-panel diagnostics distinct from natural attainment')
    if not warnings:pending.append('Verify complete-game tutorial warning paths40,41,42,44 and document native reachability of unused data indices0/10')
    if not audio:pending.append('Verify nonzero Windows audio output from actual native and source game processes;keep exact decoded PCM and audible-output scopes distinct')
    if not accepted:pending.append('Resolve explicit native tracker allocation fidelity decision;retain material codec/device/uninitialized-input verification limits')
    audit_path=ROOT/'analysis/original-goal-completion-audit.json'
    final_audit=False
    if audit_path.exists():
        audit=read(audit_path)
        if audit.get('status')=='passed_all_six_original_completion_criteria':
            assert contracts and awards and accepted and warnings and audio
            assert len(audit['criteria'])==6 and all(row['completed'] for row in audit['criteria'])
            assert audit['supporting_reports']
            for path,sha in audit['supporting_reports'].items():assert digest(ROOT/path)==sha,path
            final_audit=True
    if not final_audit:pending.append('Final requirement-by-requirement current-state audit and documentation')
    complete=not pending
    v.update(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        overall_status=('completed_source_reconstruction_with_documented_verification_limits' if complete else
            'active_integrated_source_reconstruction_remaining_original_goal_checks'),
        main_game_source_build_available=True,full_game_build_available=True,original_goal_complete=complete,
        full_game_build_available_scope='Actual Win32 main source EXE is buildable and runs;this is not a whole-goal completion flag',
        completion_audit_document='docs/COMPLETION_AUDIT.md',build_availability_report=name,
        full_reconstruction_pending=pending)
    v['goal_status_reconciliation']=dict(recorder_sha256=digest(__file__),source_build_proved=True,
        contract40_and_four_restores_proved=contracts,current_final_awards_proved=awards,
        complete_game_tutorial_warning_paths_proved=warnings,
        actual_game_Windows_audio_output_proved=audio,
        tracker_allocation_exception_explicitly_accepted=bool(accepted),six_criteria_final_audit_proved=final_audit,
        whole_goal_claimed_complete=complete)
    if not warnings:v['current_tutorial_warning_gate']=dict(report='analysis/current-tutorial-runtime-coverage.json',complete=False,required_actual_steps=[40,41,42,44])
    for name in ['analysis/current-tutorial-runtime-coverage.json','analysis/native-tracker-fidelity-decision.json']:
        if (ROOT/name).exists() and name not in v['reports']:v['reports'].append(name)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*')
        if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    v['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf8')).hexdigest()
    v['source_tree_hash_as_of_report']='analysis/original-goal-completion-audit.json' if complete else v['latest_scoped_regression']['report']
    write(ROOT/'analysis/verification.json',v)
    print('Confirmed4 current source targets;whole original goal '+('proved within documented limits.' if complete else 'remains explicitly incomplete.'))


if __name__=='__main__':main()
