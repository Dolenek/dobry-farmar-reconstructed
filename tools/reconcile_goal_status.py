#!/usr/bin/env python3
"""Separate proved source build availability from still-unproved whole-goal completion."""
import datetime
from record_live_production_verification import ROOT,digest,read,write
from current_production_regression import current_regression
from pe import PE


def main():
    name='analysis/production-source-only-build-v4.json';build=read(ROOT/name)
    assert len(build['artifacts'])==4 and build['actual_source_process_exits']==[0,0]
    for artifact in build['artifacts'].values():
        path=ROOT/artifact['path'];assert digest(path)==artifact['sha256'] and PE(path).bits==32
    for path,sha in build['staged_source_manifest']['copied_source_files'].items():
        if path.startswith('src/') or path=='CMakeLists.txt':assert digest(ROOT/path)==sha,path
    current_regression()
    v=read(ROOT/'analysis/verification.json')
    v.update(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        overall_status='active_integrated_source_reconstruction_remaining_original_goal_checks',
        main_game_source_build_available=True,full_game_build_available=True,original_goal_complete=False,
        full_game_build_available_scope='Actual Win32 main source EXE is buildable and runs;this is not a whole-goal completion flag',
        completion_audit_document='docs/COMPLETION_AUDIT.md',build_availability_report=name,
        full_reconstruction_pending=[
            'Finish actual40-contract award in both complete executable processes from genuine retained checkpoints',
            'Finish current final collect-all award lifecycle comparison,keeping requested-panel diagnostics distinct from natural attainment',
            'Verify four complete checkpoint40 cross-restores with both current source DLLs',
            'Resolve explicit native tracker allocation fidelity decision;retain material codec/device/uninitialized-input verification limits',
            'Final requirement-by-requirement current-state audit and documentation'])
    v['goal_status_reconciliation']=dict(recorder_sha256=digest(__file__),source_build_proved=True,whole_goal_claimed_complete=False)
    write(ROOT/'analysis/verification.json',v)
    print('Confirmed4 current source targets and real startup;whole original goal remains explicitly incomplete.')


if __name__=='__main__':main()
