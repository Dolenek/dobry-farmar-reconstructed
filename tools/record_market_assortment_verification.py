#!/usr/bin/env python3
"""Record actual19-row market behavior, natural award pixels and four whole-process restores."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    market=read(ROOT/'.local/market-assortment-state-validation.json');checked(market)
    assert market['status']=='passed_for_actual19_market_rows_physical_price_acceleration_and_natural_award18_only'
    assert market['auditor_sha256']==digest(ROOT/'tools/validate_market_assortment_state.py')
    comparison=market['comparison']
    assert comparison['business_snapshots_per_executable']==21 and comparison['all_profile_discretes_equal'] and comparison['complete_save_equal']
    assert comparison['save_bytes']==[4089,4089] and not comparison['normalizations']
    assert comparison['all_physical_price_change_sequences_equal'] and comparison['physical_price_clicks_per_executable']==[264,264]
    assert comparison['actual_award_panel_pixels']['exact'] and comparison['actual_award_panel_pixels']['compared_pixels']==480000
    interop=read(ROOT/'.local/market-assortment-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_all19_market_goods_prices_and_infinity_clock_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/market-assortment-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['status']=='completed' and row['exit_code']==0 and row['clock_after_restore']['recomputed']==['inf']*19
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4089 and not row['normalizations']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        events=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert events[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in events)
        traces[path.name]=dict(sha256=digest(path),events=events)
    assert len(traces)==2
    reports={}
    for name,report in [('market-assortment-state-validation.json',market),('market-assortment-interop-validation.json',interop)]:
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    initial=ROOT/'.local/market-assortment/20261030T173656679112Z'
    initial_report=read(initial/'report.json');assert all(row['exit_code']==0 for row in initial_report['sessions'])
    write(ROOT/'analysis/market-assortment-initial-visual-probe.json',dict(status='actual_business_workflow_completed_but_panel_frames_captured_during_different_opacity_transitions',
        report_sha256=digest(initial/'report.json'),work_directory=initial.relative_to(ROOT).as_posix(),recorded_as_static_visual_match=False,
        screenshots={name:digest(initial/name) for name in ['0-award-0.png','1-award-0.png']},
        different_pixels=146431,difference_bounds=[223,60,584,518],
        resolution='Wait for actual AwardPanel transition=0 and full alpha before capture; repeat both workflows and all four transfers',whole_game_complete=False))
    reports['market-assortment-initial-visual-probe.json']=digest(ROOT/'analysis/market-assortment-initial-visual-probe.json')
    full=read(ROOT/'analysis/profile-floating-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual19_market_rows_natural_award18_and_all19_clock_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,business_snapshots_per_executable=21,physical_price_clicks_per_executable=264,
        actual_price_changes_equal=True,original_award_predicate_index=18,award_requested_by_cheat=False,
        all19_market_clock_spellings_equal=True,actual_award_panel_exact_rgb_pixels=480000,complete_identical_save_bytes=4089,
        full_process_bidirectional_transfers=4,source_restore_debugger_traces=traces,production_implementation_changed=False,
        current_full_ctest_report='analysis/profile-floating-regression.json',current_full_ctest_source_hashes_match=True,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Other market inputs and every remaining natural award','Factory maximum capacities, simultaneous buildings and complete goal scopes'])
    write(ROOT/'analysis/market-assortment-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['market-assortment-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['market-assortment-state-validation.json','market-assortment-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/market-assortment-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded all19 market rows, natural award18,480000 exact RGB pixels and four4089-byte transfers; complete goal continues')

if __name__=='__main__':main()
