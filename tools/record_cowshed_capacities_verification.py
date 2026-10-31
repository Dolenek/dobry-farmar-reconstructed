#!/usr/bin/env python3
"""Record live capacity controls, actual padding words, paused input and full-store saves."""
import datetime,json,xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    capacity=read(ROOT/'.local/cowshed-capacities-state-validation.json');checked(capacity)
    assert capacity['status']=='passed_for_actual_cowshed_capacities_return_slider_and_one_space_production_only'
    assert capacity['auditor_sha256']==digest(ROOT/'tools/validate_cowshed_capacities_state.py')
    c=capacity['comparison'];assert c['capacities']==[2,4,6,12] and c['business_snapshots_per_executable']==52
    assert c['partial_slider_remaining']==[0,2,4,10] and c['non_alias_profile_discretes_equal'] and c['complete_identical_save_bytes']==4089
    assert c['validated_actual_alias_word_snapshots']==8 and not c['alias_padding_claimed_equal'] and not c['normalizations']
    paused=read(ROOT/'.local/paused-production-input-state-validation.json');checked(paused)
    assert paused['status']=='passed_for_actual_prepositioned_mouse_during_logic_pause_and_resume_only'
    assert paused['auditor_sha256']==digest(ROOT/'tools/validate_paused_production_input_state.py')
    assert paused['comparison']['complete_identical_save_bytes']==4089 and paused['comparison']['all_profile_discretes_equal']
    interop=read(ROOT/'.local/cowshed-capacities-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_maximum_cowshed_capacity_and_full_warehouse_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/cowshed-capacities-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['status']=='completed' and row['exit_code']==0
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4089 and not row['normalizations']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        events=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert events[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in events)
        traces[path.name]=dict(sha256=digest(path),events=events)
    assert len(traces)==2
    reports={}
    for name,report in [('cowshed-capacities-state-validation.json',capacity),('paused-production-input-state-validation.json',paused),('cowshed-capacities-interop-validation.json',interop)]:
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    probes=[]
    for folder,stamp,reason in [
        ('cowshed-capacities','20261030T175931115912Z','Paused before preparing the target; first feed did not occur. Prepositioned target is tested separately, no general mouse-disable conclusion.'),
        ('cowshed-capacities','20261030T180443498006Z','Native completed; source naturally produced one unit while slow individual moves accumulated time. Strict next-count expectation failed; not a proved implementation difference.'),
        ('cowshed-capacities','20261031T093719425955Z','Both workflows completed, but the unobserved alias padding produced different extra profile values. Not accepted as complete profile equality.'),
        ('paused-production-input','20261031T093005091610Z','Native accepted one paused input; driver incorrectly expected total1 after an additional active input. Original then completed both units during the faulty wait.'),
        ('paused-production-input','20261031T093418335336Z','Foreground guard refused all input before the window was ready; desktop was Default and unlocked. Current driver waits for window visibility.')]:
        path=ROOT/'.local'/folder/stamp/'report.json';probe=read(path)
        entry=dict(work_directory=probe['work_directory'],runtime_status=probe['status'],error=probe.get('error'),sha256=digest(path),reason=reason,recorded_as_full_pass=False)
        if stamp=='20261031T093719425955Z':
            values=[{r.get('Name'):r.get('Val') for r in ET.parse(path.parent/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
            entry['unequal_alias_values']=[v['Upgrade16843009'] for v in values]
            entry['exact_old_padding_words_observed']=False
        probes.append(entry)
    write(ROOT/'analysis/cowshed-capacities-initial-probes.json',dict(status='unaccepted_initial_driver_and_timing_probes_preserved',probes=probes,whole_game_complete=False))
    reports['cowshed-capacities-initial-probes.json']=digest(ROOT/'analysis/cowshed-capacities-initial-probes.json')
    full=read(ROOT/'analysis/profile-floating-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_cowshed_all_four_capacities_paused_input_and_full_store_restore',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,actual_factory_kind=2,capacities=[2,4,6,12],business_snapshots_per_executable=52,
        source_and_native_padding_words_claimed_equal=False,actual_alias_word_snapshots=8,current_non_alias_discretes_equal=True,current_all_profile_discretes_equal=c['all_profile_discretes_equal'],
        actual_paused_input=1,actual_produced_after_one_space=1,complete_identical_save_bytes=4089,full_process_bidirectional_transfers=4,
        source_restore_debugger_traces=traces,production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/profile-floating-regression.json',
        original_files_unchanged=17,whole_game_complete=False,pending=['Other factory maximum-input paths, simultaneous buildings and natural award9','Remaining whole-game, Squall97, JNG and uninstaller scopes'])
    write(ROOT/'analysis/cowshed-capacities-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['cowshed-capacities-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['cowshed-capacities-state-validation.json','paused-production-input-state-validation.json','cowshed-capacities-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/cowshed-capacities-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded capacities2/4/6/12,52 paired snapshots,8 actual alias words, paused input and four4089-byte restores; full goal continues')
if __name__=='__main__':main()
