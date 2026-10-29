#!/usr/bin/env python3
"""Record all11 actual factory recipes, with initial native/driver failures kept separate."""
import datetime,json
from record_live_production_verification import ROOT,digest,read,write,checked
def main():
    reports={};all_kinds=[];snapshots=0;save_sizes=[]
    for group in range(3):
        name=f'factory-catalog-group{group}-state-validation.json';report=read(ROOT/'.local'/name);checked(report)
        assert report['status']=='passed_for_selected_actual_factory_recipes_purchase_feed_produce_destroy_only'
        assert report['auditor_sha256']==digest(ROOT/'tools/validate_factory_catalog_state.py')
        comparison=report['comparison'];assert comparison['all_profile_names_types_and_discrete_values_equal'] and not comparison['normalizations']
        all_kinds+=report['catalog_kinds'];snapshots+=comparison['matching_business_snapshots'];save_sizes.append(comparison['complete_identical_save_bytes'])
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    assert all_kinds==list(range(11)) and snapshots==62
    probes=[]
    for stamp,meaning in [('20261029T212543648442Z','Native cAwardManager error after early Return between queued natural money awards; source replay remains unverified'),('20261029T212717029093Z','Diagnostic Python float click coordinates rejected before purchase; no game failure')]:
        work=ROOT/'.local/factory-catalog'/stamp;report=read(work/'report.json')
        assert report['status']=='failed'
        probes.append(dict(work_directory=work.relative_to(ROOT).as_posix(),meaning=meaning,error=report['error'],runtime_report_sha256=digest(work/'report.json'),driver_sha256=digest(work/'driver.py'),
            native_only=True,native_actions=report['sessions'][0]['actions'],native_award_panels=report['sessions'][0].get('awards',[]),
            error_log=(work/'runtime/error.log').read_text(encoding='ascii') if (work/'runtime/error.log').exists() else None))
    write(ROOT/'analysis/factory-catalog-initial-probes.json',dict(status='recorded_failed_probes_not_counted_as_passes',probes=probes,whole_game_complete=False))
    full=read(ROOT/'analysis/live-production-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_for_all11_actual_level0_factory_recipes_with_paid_warehouse_and_natural_money_awards',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,factory_kinds=all_kinds,actual_processes=6,business_snapshots_per_executable=snapshots,whole_save_bytes_per_pair=save_sizes,
        original_commands_only=True,original_profile_or_game_memory_modified_by_probe=False,production_implementation_changed=False,
        current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/live-production-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Higher factory levels, simultaneous factories, partial production saves and warehouse-full output blocking',
            'Natural all-factories/seven-live-factories awards and remaining full-goal scopes',
            'Native early-Return queued-award failure is preserved; source behavior in that failing input sequence remains unverified'])
    write(ROOT/'analysis/factory-catalog-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['factory-catalog-initial-probes.json','factory-catalog-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in reports:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/factory-catalog-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded all11 actual recipes and62 matching business snapshots per executable; entire reconstruction incomplete')
if __name__=='__main__':main()
