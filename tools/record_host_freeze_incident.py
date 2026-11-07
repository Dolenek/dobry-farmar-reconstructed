#!/usr/bin/env python3
"""Record the interrupted host run and CPU-only observer mitigation; never runs a game."""
import ast
import datetime
import json
from pathlib import Path
from current_production_regression import current_regression
from record_live_production_verification import ROOT,digest,read,write


def main():
    hold=ROOT/'.local/desktop-runtime-hold.json';assert read(hold)['active']
    interrupted=ROOT/'.local/tutorial-warning-runtime/20261107T211631362554Z'
    assert interrupted.is_dir() and not (interrupted/'report.json').exists()
    tests=ROOT/'.local/host-freeze-observer-safety-tests.log';text=tests.read_text(encoding='utf-8-sig')
    assert 'Ran 10 tests' in text and '\nOK' in text
    observer=ROOT/'tools/windows_game_memory.py';tree=ast.parse(observer.read_text(encoding='utf-8'))
    calls={n.func.attr for n in ast.walk(tree) if isinstance(n,ast.Call) and isinstance(n.func,ast.Attribute)}
    assert not calls & {'SuspendThread','ResumeThread','Wow64SuspendThread','OpenThread'}
    system=ROOT/'.local/host-freeze-20261107-system.json'
    events=json.loads(system.read_text(encoding='utf-8-sig'))
    assert any(row['ProviderName']=='Microsoft-Windows-Kernel-Power' and row['Id']==41 for row in events)
    regression,_=current_regression()
    original=ROOT.parent/'farmář/Dobrý farmář'
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/row['path'])==row['sha256']
    sources=['tools/windows_game_memory.py','tools/read_only_scene_tree.py','tools/windows_game_gui.py',
        'tools/desktop_runtime_hold.py','tests/test_read_only_scene_tree.py','AGENTS.md','docs/HOST_FREEZE_INCIDENT.md']
    result=dict(status='recorded_host_freeze_restart_interrupted_run_and_CPU_only_mitigation_not_runtime_resolution',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        user_report='Full Windows/display freeze requiring restart;session may continue behind frozen image',
        system_events=dict(path=system.relative_to(ROOT).as_posix(),sha256=digest(system),events=events),
        incident_hold=dict(path=hold.relative_to(ROOT).as_posix(),sha256=digest(hold),active=True),
        interrupted_run=dict(path=interrupted.relative_to(ROOT).as_posix(),completed_report_present=False,
            screenshots={p.name:digest(p) for p in interrupted.glob('*.png')},claimed_pass=False),
        root_cause_proven=False,render_UI_thread_suspension_removed=True,
        foreground_input_queue_attachment_removed=True,
        optimistic_snapshot_claimed_atomic=False,
        actual_GameGui_constructor_refused_before_launch=True,
        CPU_only_tests=dict(count=10,failed=0,log=tests.relative_to(ROOT).as_posix(),sha256=digest(tests)),
        mitigation_source_files={name:digest(ROOT/name) for name in sources},
        host_GPU_input_runtime_tests_restarted=False,revised_observer_actual_game_runtime_validated=False,
        original_files_unchanged=17,production_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report=regression,new_full_ctest_run=False,whole_game_complete=False)
    write(ROOT/'analysis/host-freeze-incident.json',result)
    verification=read(ROOT/'analysis/verification.json')
    verification['host_runtime_incident']=dict(report='analysis/host-freeze-incident.json',resolved=False,
        host_interactive_runtime_tests_held=True,completed_runtime_report_missing=True)
    if 'analysis/host-freeze-incident.json' not in verification['reports']:
        verification['reports'].append('analysis/host-freeze-incident.json')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded unresolved host freeze;10 CPU-only checks;no new host GPU/input test.')


if __name__=='__main__':main()
