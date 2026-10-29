#!/usr/bin/env python3
"""Record paid tool upgrades and a naturally earned all-tools award."""
import datetime
from record_live_production_verification import ROOT,digest,read,write,checked
def main():
    report=read(ROOT/'.local/tool-upgrades-state-validation.json');checked(report)
    assert report['status']=='passed_for_actual_paid_all_farm_tool_upgrades_and_natural_award7_only' and report['auditor_sha256']==digest(ROOT/'tools/validate_tool_upgrades_state.py')
    assert report['comparison']['actual_total_cost']==3000 and report['comparison']['complete_identical_save_bytes']==4089
    pixels=[row['pixel_comparison'] for row in report['sessions'][1]['stages']]
    assert [row['different_pixels'] for row in pixels]==[0,0,170]
    write(ROOT/'analysis/tool-upgrades-state-validation.json',report)
    failed=ROOT/'.local/tool-upgrades/20261029T213929051243Z';probe=read(failed/'report.json');assert probe['status']=='failed'
    write(ROOT/'analysis/tool-upgrades-initial-probe.json',dict(status='diagnostic_unhandled_natural_arrow47_not_a_game_failure',work_directory=failed.relative_to(ROOT).as_posix(),
        runtime_report_sha256=digest(failed/'report.json'),driver_sha256=digest(failed/'driver.py'),error=probe['error'],recorded_as_pass=False,
        resolution='Read original MowBuilding.enabledStep and confirm new Arrow47 before next upgrade; also service Water arrows46/29'))
    full=read(ROOT/'analysis/live-production-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_paid_tool_upgrades_and_natural_all_tools_award7',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),report_sha256=digest(ROOT/'analysis/tool-upgrades-state-validation.json'),actual_paid_upgrades=8,total_cost=3000,
        natural_award_condition_index=7,award_cheat_requested=False,exact_tutorial_pixels=57200,nonexact_tutorial29=pixels[2],
        complete_identical_save_bytes=4089,implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/live-production-regression.json',
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Actual work and maximum capacities with upgraded tools; natural conditions9/18 and remaining scopes','Timed tutorial29 frame equality and all remaining full-goal scopes'])
    write(ROOT/'analysis/tool-upgrades-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in ['tool-upgrades-state-validation.json','tool-upgrades-initial-probe.json','tool-upgrades-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    if 'analysis/tool-upgrades-state-validation.json' not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/tool-upgrades-state-validation.json')
    verification['latest_scoped_regression']=dict(report='analysis/tool-upgrades-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification);print('Recorded actual paid upgrades and natural award7; tutorial29 pixels remain explicitly nonexact')
if __name__=='__main__':main()
