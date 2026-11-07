#!/usr/bin/env python3
"""Index measured stages in passed complete-process reports;not a whole-tutorial completion claim."""
import datetime
import json
from record_live_production_verification import ROOT,digest,read,write


def main():
    verification=read(ROOT/'analysis/verification.json')
    steps={};reports={};non_json=[];unreadable=[]
    names=set(verification['reports'])|set(verification['original_game_runtime']['scope_reports'])|{
        'analysis/seed-transition-state-validation.json','analysis/tomato-harvest-state-validation.json'}
    for name in sorted(names):
        path=ROOT/name
        if path.suffix!='.json':non_json.append(name);continue
        try:report=read(path)
        except (OSError,json.JSONDecodeError) as error:
            unreadable.append(dict(path=name,error=repr(error)));continue
        if not isinstance(report,dict) or not str(report.get('status','')).startswith('passed'):continue
        for session in report.get('sessions',[]):
            if session.get('kind') not in ['native','source'] or session.get('exit_code')!=0:continue
            for row in session.get('stages',[]):
                index=row.get('step')
                if isinstance(index,int):
                    steps.setdefault(index,[]).append(dict(report=name,kind=session['kind'],screenshot=row.get('screenshot')))
                    reports[name]=digest(path)
    result=dict(status='audited_existing_actual_tutorial_stage_coverage_remaining_warning_paths_not_yet_verified',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reports=reports,actual_observed_indices=sorted(steps),actual_observations=steps,
        missing_indices=sorted(set(range(50))-set(steps)),non_json_registry_entries=non_json,unreadable_registry_entries=unreadable,
        source_warning_paths_to_verify_in_complete_game=[40,41,42,44],
        source_indices_without_direct_literal_show_call_not_claimed_native_unreachable=[0,10],
        source_hexadecimal_step42_call='ContractProvide::enabledStep tutorialCanShow(0x2a) and Game::World::showTutorial',
        original_repeated_capacity_entry='00451420',original_repeated_mixed_entry='00451490',
        original_person_arrive_entry='0045a730',source_initial_valueC14=100,
        source_repeated_mixed_comparison_uses_C14_not_C1c=True,
        source_code_or_game_properties_changed=False,whole_game_complete=False)
    write(ROOT/'analysis/current-tutorial-runtime-coverage.json',result)
    print(json.dumps(dict(observed=sorted(steps),missing=result['missing_indices'],unreadable=unreadable)))


if __name__=='__main__':main()
