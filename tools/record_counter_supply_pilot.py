#!/usr/bin/env python3
"""Retain the native supply pilot without claiming a source-game comparison."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write

def main():
    path=ROOT/'.local/counter-supply-cycle/20261105T135229432937Z/report.json';report=read(path);work=ROOT/report['work_directory']
    assert report['status']=='explored_actual_counter_supply_cycle_only' and len(report['sessions'])==1
    for name,sha in report['source_files'].items():
        origin=ROOT/report['source_file_snapshots'][name] if name in report['source_file_snapshots'] else ROOT/name
        assert digest(origin)==sha,name
    for name,sha in report['screenshots'].items():assert digest(work/name)==sha,name
    for name,sha in report['implementation_files'].items():assert digest(work/'runtime'/name)==sha,name
    session=report['sessions'][0];assert session['kind']=='native' and session['status']=='completed' and session['exit_code']==0
    assert [json.loads(line) for line in session['launcher_completion'].splitlines()]==[dict(event='exit',exit_code=0,watchdog_triggered=False)]
    initial=session['counter_cycle_initial'];assert initial==dict(collected=100,processed=6,grass=61,carried=8,milk=5,market_milk=0,money=2504)
    steps=session['conservation_steps'];assert len(steps)==119
    for row in steps:
        assert row['harvested']==row['collected']-100 and row['produced']==row['processed']-6
        assert 69+row['harvested']==row['grass']+row['carried']+row['loaded']+row['produced']
        assert 5+row['produced']==row['milk']+row['market_milk']+row['milk_sold'] and row['carried']<=16 and row['loaded']<=12
        # Paid setup is recorded separately;the manufacturing/supply loop is after all1300 costs.
        if row['label'].startswith('cycle-') or row['label'].startswith('final'):
            assert row['money']==2504-1300+2*row['milk_sold']
    final=session['counter_cycle_final'];assert final['collected']==136 and final['processed']==108 and final['harvested']==36 and final['produced']==102
    assert final['milk_sold']==94 and final['money']==1392 and session['paid_cost']==1300 and len(session['awards'])==1
    assert len(session['award_visible_congratulation_lines'][0])==39
    wraps=[dict(before=a['game_time'],after=b['game_time']) for a,b in zip(steps,steps[1:]) if b['game_time']<a['game_time']]
    assert len(wraps)==1
    failures=[]
    for folder,reason in [('20261105T135004841275Z','Diagnostic mouse helper received float coordinates'),('20261105T135052312671Z','Diagnostic copied Pause0xC5 instead of Insert0xD2;game clock and enabled slider stopped')]:
        p=ROOT/'.local/counter-supply-cycle'/folder/'report.json';v=read(p);assert v['status']=='failed'
        failures.append(dict(report=p.relative_to(ROOT).as_posix(),sha256=digest(p),error=v['error'],diagnostic_cause=reason,production_failure_claimed=False))
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    value=dict(status='passed_native_only_paid_grass_milk_sale_supply_pilot_source_comparison_in_progress',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),runtime_report=path.relative_to(ROOT).as_posix(),runtime_report_sha256=digest(path),
        source_snapshot_hashes=report['source_files'],snapshot_paths=report['source_file_snapshots'],failures=failures,
        paid_cowshed_cost=1300,harvested_grass=36,produced_milk=102,sold_milk=94,milk_price=2,final=final,
        conservation_snapshots=119,natural_new_award=1,original_award_panel_glyphs=39,actual_observed_clock_wraps=wraps,
        native_normal_exit=0,native_watchdog_triggered=False,source_game_comparison_claimed=False,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Longer actual native/source supply cycle to natural1000 harvest/production awards is still running','Independent final-state audit,bidirectional saves and remaining original goal scopes'])
    write(ROOT/'analysis/counter-supply-pilot-observations.json',value)
    print('Recorded native-only pilot:36 harvests,102 productions,94 milk sales and119 conservation snapshots.')

if __name__=='__main__':main()
