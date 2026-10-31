#!/usr/bin/env python3
"""Record actual purchase/sowing/watering of the five remaining crop kinds."""
import datetime
from record_live_production_verification import ROOT,digest,read,write,checked


def main():
    state=read(ROOT/'.local/remaining-crops-sowing-state-validation.json');checked(state)
    assert state['status']=='passed_for_five_remaining_crop_kinds_purchase_sowing_and_watering_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_remaining_crops_sowing_state.py')
    c=state['comparison'];assert c['business_snapshots_per_executable']==35 and c['kinds']==[2,4,5,6,7]
    assert c['all_profile_discretes_equal'] and c['paid_cost']==275 and c['save_bytes']==4372
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    initial=read(ROOT/'.local/remaining-crops-sowing/20261031T145325679783Z/report.json')
    assert initial['status']=='failed' and 'float' in initial['error']
    write(ROOT/'analysis/remaining-crops-sowing-state-validation.json',state)
    result=dict(status='passed_actual_five_remaining_crop_kinds_purchase_sowing_watering_and_partial_packages',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reports={'remaining-crops-sowing-state-validation.json':digest(ROOT/'analysis/remaining-crops-sowing-state-validation.json')},
        crop_kinds=c['kinds'],fields=c['fields'],paid_cost=275,business_snapshots_per_executable=35,profile_properties=c['profile_properties'],all_profile_discretes_equal=True,
        complete_save_bytes=4372,complete_save_raw_equal=c['save_bytes_exact'],normalizations=c['normalizations'],remaining_packages=5,remaining_seeds_per_package=3,
        messages_equal=c['messages_equal'],coefficient_bits_equal=c['coefficient_bits_equal'],
        initial_diagnostic_driver_error=dict(work_directory=initial['work_directory'],report_sha256=digest(ROOT/initial['work_directory']/'report.json'),error=initial['error'],resolution='Round XML float coordinates to integer screen pixels before existing Win32 input helper',recorded_as_game_failure=False),
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Cross-program resume and maturation of these crops','Natural award11, actual new-kind harvests and further saves','Remaining full-game, Squall, JNG and uninstaller scopes'])
    write(ROOT/'analysis/remaining-crops-sowing-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in ['remaining-crops-sowing-state-validation.json','remaining-crops-sowing-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    name='analysis/remaining-crops-sowing-state-validation.json'
    if name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append(name)
    verification['latest_scoped_regression']=dict(report='analysis/remaining-crops-sowing-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded five actual new crop kinds,70 combined work snapshots,275 paid and five3-seed packages; growth remains separate')


if __name__=='__main__':main()
