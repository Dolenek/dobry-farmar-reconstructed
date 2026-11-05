#!/usr/bin/env python3
"""Record later actual play, watchdog diagnosis and four untouched save transfers."""
import datetime,hashlib,json
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    state=read(ROOT/'.local/late-timeline-state-validation.json');checked(state)
    assert state['status']=='passed_actual_later_timeline_three_hour_award_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_late_timeline_state.py')
    interop=read(ROOT/'.local/late-timeline-interop-validation.json');checked(interop)
    assert interop['status']=='passed_both_late_world_outcomes_three_hour_award_and_four_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/late-timeline-state-validation.json')
    transfers=interop['comparison']['transfer_comparisons'];assert len(transfers)==len(interop['sessions'])==4
    assert all(s['status']=='completed' and s['exit_code']==0 and s['restored_expanded_farm']['awards'][14] for s in interop['sessions'])
    assert all(not t['normalizations'] and t['identical_complete_save_bytes']>6943 for t in transfers)
    archive=ROOT/'.local/native-launcher-before-late';manifest=read(archive/'manifest.json')
    assert all(digest(archive/name)==value for name,value in manifest.items())
    original_cpp=(archive/'tools/native_process/launcher.cpp').read_text(encoding='utf-8')
    assert 'WaitForSingleObject(process.hProcess,180000)' in original_cpp
    failures=[]
    for stamp in ['20261104T194208073047Z','20261104T195816215800Z']:
        folder=ROOT/'.local/late-timeline'/stamp;failed=read(folder/'report.json')
        assert failed['status']=='failed' and failed['launcher_sha256']==manifest['build/native-process/Release/native-process-launcher.exe']
        failures.append(dict(work_directory=failed['work_directory'],report_sha256=digest(folder/'report.json'),
            driver_sha256=digest(folder/'driver.py'),error=failed['error'],
            recorded_as_game_failure=False,original_process_exit_code=failed['sessions'][0].get('cleanup_exit_code'),
            explanation='Old diagnostic launcher closes its owned reference process after180000ms; first driver masked process exit as lost foreground'))
    invalid=read(ROOT/'.local/native-launcher-late-argument-validation.json')
    assert [c['argument'] for c in invalid]==['0','1801','not-a-number']
    assert all(c['exit_code']==1 and not c['reference_process_created'] for c in invalid)
    rejected_path=ROOT/'.local/late-timeline-interop/20261105T034150065501Z/report.json'
    rejected=read(rejected_path);assert rejected['status']=='failed' and 'MEffPrice10' in rejected['error']
    assert all(s['status']=='completed' and s['exit_code']==0 for s in rejected['sessions'])
    traces={}
    for path in (ROOT/interop['work_directory']).glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(e['event']=='access_violation' for e in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    observation=dict(status='diagnostic_three_minute_watchdog_identified_and_bounded_timeout_made_explicit',
        old_launcher_archive_manifest=manifest,failures=failures,invalid_timeout_cases=invalid,
        default_timeout_ms=180000,explicit_timeout_seconds_range=[30,1800],actual_long_run_timeout_ms=1200000,
        current_launcher_sha256=digest(ROOT/'build/native-process/Release/native-process-launcher.exe'),
        initial_interop_audit=dict(report_path=rejected_path.relative_to(ROOT).as_posix(),report_sha256=digest(rejected_path),
            error=rejected['error'],all_four_processes_exited_normally=True,recorded_as_game_failure=False,
            resolution='Verify actual crossed popularity event303 at GameTime10440,all19 prices and zero saved coefficient; no profile edits'),
        launcher_build_log_sha256=digest(ROOT/'.local/native-launcher-late-build.log'),
        production_implementation_changed=False,whole_game_complete=False)
    reports={}
    for name,value in [('late-timeline-state-validation.json',state),('late-timeline-interop-validation.json',interop),('late-timeline-watchdog-observation.json',observation)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert full['tests_passed']==102 and full['tests_failed']==0 and full['implementation_files']==state['implementation_files']==interop['implementation_files']
    assert all(digest(ROOT/name)==value for name,value in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_late_world_three_hour_award_and_four_bidirectional_transfers_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        runtime_processes=2,further_bidirectional_restores=4,timeline_checkpoints_per_executable=13,
        money_event_ledger=state['comparison']['money_event_ledger'],actual_natural_award=14,
        actual_later_crop_destruction=state['comparison']['actual_later_crop_destruction'],
        identical_award_text_rgb_pixels=state['comparison']['identical_award_text_rgb_pixels'],
        identical_whole_award_rgb_pixels=state['comparison']['identical_whole_award_rgb_pixels'],
        branch_save_bytes=state['comparison']['complete_save_bytes'],
        complete_cross_restore_bytes=[t['identical_complete_save_bytes'] for t in transfers],normalizations=[],
        matched_rng_and_particle_timing_verified=False,production_implementation_changed=False,
        implementation_files=state['implementation_files'],current_full_ctest_source_hashes_match=True,
        source_restore_debugger_traces=traces,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Remaining natural awards and contract streak','MainGame ending, post-campaign loop and remaining gameplay','Remaining complete Squall/JNG/uninstaller scopes'])
    write(ROOT/'analysis/late-timeline-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['late-timeline-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['late-timeline-state-validation.json','late-timeline-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/late-timeline-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report']='analysis/late-timeline-regression.json'
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded real three-hour award,24-event ledgers and four unmodified late-save transfers.')

if __name__=='__main__':main()
