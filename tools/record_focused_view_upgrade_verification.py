#!/usr/bin/env python3
"""Record the focused-View alias with explicit independent-allocation limits."""
import datetime,hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text(encoding='utf-8'))
def write(path,value):Path(path).write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def checked(report):
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
def main():
    reports={}
    for suffix in ['', '-fresh']:
        name=f'focused-upgrade{suffix}-validation.json';report=read(ROOT/'.local'/name);checked(report)
        assert report['status']=='passed_for_abstract_upgrade_and_cost_state_scope' and report['focused_view_alias_states']==6144 and len(report['precision_contexts'])==2
        for row in report['precision_contexts']:assert row['upgrade_alias_states']==3072
        path=ROOT/('build/squall-source-repro-vs20261010' if suffix else 'build/windows')/'Release/text-buttons-reference-test.exe'
        assert digest(path)==report['oracle_sha256'];write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    runtime=read(ROOT/'.local/manufacture-tutorial-state-validation.json');checked(runtime)
    assert runtime['status']=='passed_for_manufacture_tutorial_with_recorded_allocation_derived_property_only'
    assert runtime['comparison']['identical_complete_save_bytes']==3557 and not runtime['comparison']['normalizations']
    assert runtime['comparison']['profile_properties']==795 and not runtime['comparison']['allocation_value_normalized_as_equal']
    for name,value in runtime['audit_files'].items():assert digest(ROOT/name)==value,name
    for name,value in runtime['implementation_files'].items():assert digest(ROOT/'build/windows/Release'/('Dobrý farmář.exe' if name=='FarmarSourceInterop.exe' else name))==value,name
    write(ROOT/'analysis/manufacture-tutorial-state-validation.json',runtime)
    trace=read(ROOT/'analysis/manufacture-focused-upgrade-trace.json')
    for name,value in trace['source_files'].items():assert digest(ROOT/name)==value,name
    assert digest(ROOT/'build/native-upgrade-trace/Release/native-upgrade-trace.exe')==trace['tracer_sha256']
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_for_focused_View_alias_same_words_and_recorded_actual_manufacture_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,runtime_report_sha256=digest(ROOT/'analysis/manufacture-tutorial-state-validation.json'),
        native_cause_report_sha256=digest(ROOT/'analysis/manufacture-focused-upgrade-trace.json'),implementation_files=runtime['implementation_files'],
        build_logs={n:digest(ROOT/'.local'/n) for n in ['focused-upgrade-main-build.log','focused-upgrade-fresh-build.log']},
        current_native_alias_states_per_build=6144,full_ctest_repeated_for_this_change=False,
        previous_full_ctest=dict(report='analysis/squall-listener-regression.json',implementation_commit='2745060',passed=102),
        allocation_padding_identical_between_actual_processes=False,original_files_unchanged=17,whole_game_complete=False,
        pending=['Fresh-process padding/object ABI remains explicit; supplied identical words compare exactly',
            'Full regression after this game change, cucumber completion, real production and remaining full-goal scopes'])
    write(ROOT/'analysis/focused-view-upgrade-regression.json',result)
    verification=read(ROOT/'analysis/verification.json');verification['latest_scoped_regression']=dict(report='analysis/focused-view-upgrade-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests']['full_ctest_repeated_for_latest_game_alias_fix']=False
    for name in list(reports)+['focused-view-upgrade-regression.json','manufacture-tutorial-state-validation.json','manufacture-focused-upgrade-trace.json','manufacture-tutorial-runtime-observation.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    if 'analysis/manufacture-tutorial-state-validation.json' not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/manufacture-tutorial-state-validation.json')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded two-build alias tests and complete manufacture actions; independent padding value and full-regression limits remain explicit')
if __name__=='__main__':main()
