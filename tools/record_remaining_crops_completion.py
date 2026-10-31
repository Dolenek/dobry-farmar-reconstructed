#!/usr/bin/env python3
"""Record actual crop growth, natural awards, five harvests and whole-process saves."""
import datetime,json,xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write,checked


def main():
    growth=read(ROOT/'.local/remaining-crops-growth-state-validation.json');checked(growth)
    harvest=read(ROOT/'.local/remaining-crops-harvest-state-validation.json');checked(harvest)
    interop=read(ROOT/'.local/remaining-crops-interop-validation.json');work=checked(interop)
    assert growth['status']=='passed_for_cross_program_five_crop_growth_and_natural_awards11_13_only'
    assert harvest['status']=='passed_for_cross_program_five_crop_harvests_and_storage_only'
    assert interop['status']=='passed_for_stocked_five_crop_kinds_and_packages_bidirectional_saves_only'
    for name,state in [('growth',growth),('harvest',harvest)]:assert state['auditor_sha256']==digest(ROOT/f'tools/validate_remaining_crops_{name}_state.py')
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/remaining-crops-harvest-state-validation.json')
    assert growth['comparison']['eight_cultures_actually_ready_in_both'] and len(growth['comparison']['actual_award_pixels'])==2
    assert all(r['whole_frame_different_pixels']==0 and r['congratulation_exact'] for r in growth['comparison']['actual_award_pixels'])
    c=harvest['comparison'];assert c['all_profile_discretes_equal'] and c['business_snapshots_per_executable']==12 and c['complete_save_bytes']==5439
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['status']=='completed' and row['exit_code']==0
    for row in interop['comparison']['transfer_comparisons']:
        assert row['complete_save_bytes']==5439 and len(row['actual_seed_restore_x_changes'])==5
        assert [n['kind'] for n in row['normalizations']]==['original_seed_restore_X_plus1','original_uninitialized_seed_padding_only']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(r['event']=='access_violation' for r in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    early=read(ROOT/'.local/remaining-crops-harvest/20261031T150426132599Z/report.json')
    assert early['status']=='failed' and "'take-mower'" in early['error'] and early['sessions'][0]['initial_state']['tutorial']==49
    edge=read(ROOT/'.local/remaining-crops-harvest/20261031T150643626919Z/report.json')
    assert edge['status']=='explored_cross_program_harvest_and_storage_of_five_remaining_crops_only'
    edge_values=[]
    for i in [0,1]:
        values={r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(ROOT/edge['work_directory']/f'{i}-persisted/Profiles/00000001.xml').getroot()}
        edge_values.append({k:values[k] for k in ['GameTime','letter1','letTime1']})
    assert edge_values[0]['letter1']==('Int','-1') and edge_values[1]['letter1']==('Int','49') and 0<float(edge_values[1]['letTime1'][1])<1
    initial=dict(status='preserved_diagnostic_and_time_boundary_observations',
        startup_tutorial49=dict(work_directory=early['work_directory'],report_sha256=digest(ROOT/early['work_directory']/'report.json'),error=early['error'],resolution='Process pending tutorial49 before farm/mower input; no production change',recorded_as_game_failure=False),
        original_cutoff1590=dict(work_directory=edge['work_directory'],report_sha256=digest(ROOT/edge['work_directory']/'report.json'),actual_letter_states=edge_values,resolution='Repeat actual harvesting, then wait normally to1598 so both finish the real letter expiry; no XML or save writes'),
        whole_game_complete=False)
    reports={}
    for name,value in [('remaining-crops-growth-state-validation.json',growth),('remaining-crops-harvest-state-validation.json',harvest),('remaining-crops-interop-validation.json',interop),('remaining-crops-completion-initial-observations.json',initial)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_five_new_crop_growths_eight_ready_cultures_natural_awards_and_harvests_with_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        crop_kinds=[2,4,5,6,7],actual_cross_program_growth_processes=2,actual_cross_program_harvest_processes=2,full_process_bidirectional_final_transfers=4,
        natural_awards=[13,11],actual_whole_award_exact_rgb_pixels=960000,eight_cultures_actually_ready_in_both=True,harvest_business_snapshots_per_executable=12,
        growth_time_dependent_discrete_differences_preserved=growth['comparison']['actual_timed_discrete_differences'],final_all_profile_discretes_equal=True,complete_final_save_bytes=5439,
        final_save_comparison='All bytes equal except15 original seed padding bytes; each subsequent restore separately requires original X+1 for all five seeds',source_restore_debugger_traces=traces,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Actual irrigation, remaining contract paths, weather and natural awards','Remaining Squall97, JNG and uninstaller scopes'])
    write(ROOT/'analysis/remaining-crops-completion-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['remaining-crops-completion-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['remaining-crops-growth-state-validation.json','remaining-crops-harvest-state-validation.json','remaining-crops-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/remaining-crops-completion-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded five new crop completions, eight actually ready kinds, natural11/13,960000 exact RGB pixels and eight complete cross-program processes')


if __name__=='__main__':main()
