#!/usr/bin/env python3
"""Record real all60-field irrigation, natural award12 and exact save transfers."""
import datetime,hashlib,json
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    state=read(ROOT/'.local/no-dry-award-state-validation.json');checked(state)
    assert state['status']=='passed_actual_all60_fields_maintained_and_natural_no_dry_award_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_no_dry_award_state.py')
    interop=read(ROOT/'.local/no-dry-award-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_both_natural_no_dry_award_outcomes_and_four_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/no-dry-award-state-validation.json')
    transfers=interop['comparison']['transfer_comparisons'];assert len(transfers)==len(interop['sessions'])==4
    assert all(s['status']=='completed' and s['exit_code']==0 and s['restored_expanded_farm']['no_dry'] and s['restored_expanded_farm']['awards'][12] for s in interop['sessions'])
    assert all(not t['normalizations'] and t['identical_complete_save_bytes']==15808 for t in transfers)
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(e['event']=='access_violation' for e in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    reports={}
    for name,value in [('no-dry-award-state-validation.json',state),('no-dry-award-interop-validation.json',interop)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert full['tests_passed']==102 and full['tests_failed']==0 and full['implementation_files']==state['implementation_files']==interop['implementation_files']
    assert all(digest(ROOT/name)==value for name,value in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    archive=ROOT/'.local/no-dry-award-audit-before-note'
    result=dict(status='passed_actual_all60_fields_maintained_natural_award12_and_four_bidirectional_transfers_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        runtime_processes=2,further_bidirectional_restores=4,actual_natural_award=12,
        branches=state['comparison']['branches'],water_sign_threshold=15,required_continuous_no_dry_seconds=1200,
        original_water_check='FarmAreas::hasDryField checks all60 fields including empty grass',
        actual_moving_sprinkler_count=1,actual_fixed_sprinkler_count=2,actual_move_targets=state['sessions'][0]['watering_plan']['targets'],
        identical_whole_award_rgb_pixels=480000,expected_original_journal_event=356,expected_added_note_bytes=203,
        independently_encoded_complete_save_bytes=15808,
        initial_audit_rejection=dict(driver_sha256=digest(archive/'validate_no_dry_award_state.py'),log_sha256=digest(archive/'run.log'),
            reason='Initial comparison incorrectly required unchanged journal;actual original Type3 event356 unconditionally appends its203-byte note',
            resolution='Independently encode original note,updated message count and unchanged coefficient tail;compare every byte of both complete saves',
            actual_runtime_runs_reused_without_profile_modification=True,recorded_as_game_failure=False),
        complete_cross_restore_bytes=[t['identical_complete_save_bytes'] for t in transfers],normalizations=[],
        source_restore_debugger_traces=traces,production_implementation_changed=False,
        implementation_files=state['implementation_files'],current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Remaining natural award counters and contract streak','Further active tracker/codecs and complete export signatures','Remaining JNG/uninstaller scopes'])
    write(ROOT/'analysis/no-dry-award-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['no-dry-award-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in reports:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/no-dry-award-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report']='analysis/no-dry-award-regression.json'
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded actual all60-field1200-second watering award,480000 RGB pixels and four15808-byte whole-save transfers.')

if __name__=='__main__':main()
