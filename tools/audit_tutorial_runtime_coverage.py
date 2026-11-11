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
    native_callers = ROOT/'analysis/tutorial-native-callers-audit.json'
    caller_audit = read(native_callers)
    assert caller_audit['independent_text_section_relative_call_scan_exact']
    expected = set(caller_audit['compiled_caller_indices'])
    assert expected == set(range(50)) - {0, 10}
    by_kind = {kind: sorted(index for index, observations in steps.items()
                            if any(row['kind'] == kind for row in observations))
               for kind in ('native', 'source')}
    remaining = {kind: sorted(expected - set(indices)) for kind, indices in by_kind.items()}
    complete = not any(remaining.values()) and not unreadable
    reports[native_callers.relative_to(ROOT).as_posix()] = digest(native_callers)
    result=dict(status=('passed_all_48_compiled_tutorial_steps_in_both_complete_games' if complete else
                        'audited_existing_actual_tutorial_stage_coverage_with_remaining_steps'),
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reports=reports,actual_observed_indices=sorted(steps),actual_observations=steps,
        missing_indices=sorted(set(range(50))-set(steps)),non_json_registry_entries=non_json,unreadable_registry_entries=unreadable,
        actual_observed_indices_by_program=by_kind, remaining_compiled_steps_by_program=remaining,
        no_ordinary_native_compiled_callers_for_indices=[0,10],
        source_hexadecimal_step42_call='ContractProvide::enabledStep tutorialCanShow(0x2a) and Game::World::showTutorial',
        original_repeated_capacity_entry='00451420',original_repeated_mixed_entry='00451490',
        original_person_arrive_entry='0045a730',source_initial_valueC14=100,
        source_repeated_mixed_comparison_uses_C14_not_C1c=True,
        source_code_or_game_properties_changed=False,whole_game_complete=False)
    write(ROOT/'analysis/current-tutorial-runtime-coverage.json',result)
    print(json.dumps(dict(observed=sorted(steps),missing=result['missing_indices'],unreadable=unreadable)))


if __name__=='__main__':main()
