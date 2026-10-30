#!/usr/bin/env python3
"""Record all33 paid factory upgrades and actual production at level3."""
import datetime
from record_live_production_verification import ROOT,digest,read,write,checked
def main():
    reports={};kinds=[];snapshots=0;aliases=0;sizes=[]
    for group in range(3):
        name=f'factory-levels-group{group}-state-validation.json';report=read(ROOT/'.local'/name);checked(report)
        assert report['status']=='passed_for_actual_paid_level3_factory_recipes_and_observed_alias_words_only' and report['auditor_sha256']==digest(ROOT/'tools/validate_factory_levels_state.py')
        c=report['comparison'];assert c['non_alias_discretes_equal'] and not c['normalizations']
        assert not c['recorded_allocation_derived_alias_differences']
        kinds+=report['catalog_kinds'];snapshots+=c['matching_business_snapshots'];sizes.append(c['complete_identical_save_bytes'])
        for session in report['sessions']:aliases+=sum(len(rows) for rows in session['upgrade_alias_words'].values())
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    assert kinds==list(range(11)) and snapshots==95 and aliases==66 and sizes==[3730]*3
    full=read(ROOT/'analysis/live-production-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_all33_paid_factory_upgrades_and_all11_level3_recipes',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,factory_kinds=kinds,actual_processes=6,paid_upgrades_per_executable=33,business_snapshots_per_executable=95,
        exact_save_bytes_per_pair=sizes,validated_alias_word_snapshots=aliases,alias_padding_claimed_equal=False,actual_final_alias_values_equal_in_this_run=True,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/live-production-regression.json',
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Maximum loaded capacities, simultaneous factories and natural manufacturing awards','Remaining whole-game, Squall, JNG and uninstaller scopes'])
    write(ROOT/'analysis/factory-levels-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['factory-levels-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in reports:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/factory-levels-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification);print('Recorded33 paid upgrades,11 actual level3 recipes and95 matching business snapshots per executable')
if __name__=='__main__':main()
