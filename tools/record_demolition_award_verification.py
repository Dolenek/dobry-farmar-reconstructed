#!/usr/bin/env python3
"""Record actual20-demolition award,paid rebuilds and complete cross-program saves."""
import datetime,hashlib,json
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    state=read(ROOT/'.local/demolition-award-state-validation.json');checked(state)
    assert state['status']=='passed_for_actual_paid_seven_demolitions_and_natural_twenty_threshold_award10_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_demolition_award_state.py')
    c=state['comparison'];assert c['business_snapshots_per_executable']==17 and c['actual_alias_word_snapshots']==98 and c['paid_cost']==700
    assert c['complete_identical_save_bytes']==4252 and c['whole_save_exactly_unchanged_from_unedited_native_baseline'] and not c['normalizations']
    assert c['actual_award_panel_pixels']==dict(compared_pixels=480000,different_pixels=0,difference_bounds=None,exact=True)
    interop=read(ROOT/'.local/demolition-award-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_natural_twenty_demolition_award10_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/demolition-award-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['exit_code']==0 and row['status']=='completed'
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4252 and not row['normalizations']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    initial=[]
    for folder in ['20261105T131015064323Z','20261105T131204450281Z']:
        path=ROOT/'.local/demolition-award'/folder/'report.json';value=read(path)
        assert value['status']=='failed' and len(value['sessions'])==1
        assert value['sessions'][0]['catalog_steps'][-1]['state']['destroyed']==20
        for name,sha in value['source_files'].items():
            if name!='tools/validate_game_demolition_award.py':assert digest(ROOT/name)==sha,name
        initial.append(dict(report=path.relative_to(ROOT).as_posix(),sha256=digest(path),error=value['error'],last_actual_demolition_count=20))
    late=read(ROOT/'.local/demolition-award/20261105T131204450281Z/report.json')['sessions'][0]['late_award_scene']
    glyphs=[row for row in late if (row['texture'] or '').lower()=='ui/menu_font.dds' and 6400<=row['deep']<6401 and 340<=row['geometry'][1]<=480]
    assert len(glyphs)==36 and len({row['geometry'][1] for row in glyphs})==2
    observations=dict(status='two_driver_assumptions_rejected_then_actual_modal_and36_glyph_award_verified',initial_reports=initial,
        first_resolution='Handle actual20-demolition modal before attempting the seventh rebuild;the purchase ghost cannot exist while award blocks input',
        second_resolution='Use actual36 glyphs on two rows for award10;award8 fixture79 glyphs on four rows were a wrong diagnostic assumption',
        production_changed=False,recorded_as_production_failure=False,whole_game_complete=False)
    reports={}
    for name,value in [('demolition-award-state-validation.json',state),('demolition-award-interop-validation.json',interop),('demolition-award-initial-observations.json',observations)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    for name,sha in full['production_source_files'].items():assert digest(ROOT/name)==sha,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_paid_demolitions_natural20_threshold_award10_and_bidirectional_saves_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        original_predicate='004c8650: awQDestroy>19;00499630 increments on actual Manufacture removal',
        original_display_description='100 depots',actual_original_threshold=20,demolition_count_before=13,demolition_count_after=20,
        paid_rebuild_type=7,paid_rebuild_count=7,paid_cost=700,money_before=5234,money_after=4534,
        business_snapshots_per_executable=17,actual_alias_word_snapshots=98,alias_padding_claimed_equal=False,
        actual_award_panel_exact_rgb_pixels=480000,actual_panel_glyphs=36,actual_panel_rows=2,
        complete_identical_save_bytes=4252,entire_save_unchanged_from_native_baseline=True,
        full_process_bidirectional_transfers=4,source_restore_debugger_traces=traces,
        original_profile_fields_modified_by_probe=False,production_implementation_changed=False,
        current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Natural100/1000 harvesting and production counters,20/40 contract streaks and remaining awards','Whole Squall97/JNG/uninstaller and remaining original goal scopes'])
    name='demolition-award-regression.json';write(ROOT/'analysis'/name,result)
    verification=read(ROOT/'analysis/verification.json')
    for entry in list(reports)+[name]:
        if 'analysis/'+entry not in verification['reports']:verification['reports'].append('analysis/'+entry)
    for entry in ['demolition-award-state-validation.json','demolition-award-interop-validation.json']:
        if 'analysis/'+entry not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+entry)
    verification['latest_scoped_regression']=dict(report='analysis/'+name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded real20-demolition award,700 paid cost,480000 exact pixels and four complete4252-byte restores.')

if __name__=='__main__':main()
