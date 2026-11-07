#!/usr/bin/env python3
"""Preserve native diagnostic failures and the verified reusable owned-award prefix."""
import datetime
import xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write


def main():
    failures=[]
    for stamp in ['20261107T092743166864Z','20261107T134104987319Z','20261107T135023948573Z']:
        path=ROOT/'.local/game-awards-runtime'/stamp/'report.json';value=read(path);work=path.parent
        assert value['status']=='failed' and len(value['sessions'])==1 and value['sessions'][0]['kind']=='native'
        assert digest(work/'executed-driver.py')==value['source_files']['tools/validate_game_awards_runtime.py']
        for name,sha in value['screenshots'].items():assert digest(work/name)==sha
        session=value['sessions'][0]
        row=dict(report=path.relative_to(ROOT).as_posix(),sha256=digest(path),error=value['error'],source_processes_started=0)
        if 'launcher_exit' in session:
            event=__import__('json').loads(session['launcher_exit']);assert event==dict(event='exit',exit_code=0,watchdog_triggered=False)
            row['normal_native_cleanup']=event
        else:
            assert session['launcher']['timeout_ms']==180000
            row['configured_native_watchdog_ms']=180000
            row['watchdog_cause_is_inference_not_captured_completion_event']=True
        failures.append(row)
    prefix=read(ROOT/'.local/current-game-awards-prefix-preflight.json')
    assert prefix['status']=='passed_native_prefix_preflight_only' and prefix['source_or_native_processes_started']==0
    assert prefix['native_prefix']['completed_indices']==list(range(18))
    actual=ROOT/'.local/game-awards-runtime/20261107T135023948573Z/0-shared/VirtualFarm/Profiles/00000001.xml'
    props={n.get('Name'):n.get('Val') for n in ET.parse(actual).getroot()}
    keys=sorted(k for k in props if k.startswith('Award'));assert len(keys)==21
    assert all(props[keys[n]]==props['awardShow'+str(n)]=='1' for n in range(18))
    assert props[keys[18]]==props['awardShow18']=='0'
    name='analysis/current-awards-native-prefix-preflight.json';write(ROOT/name,prefix)
    result=dict(status='verified_native_input_failures_and18_reusable_committed_panels_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        failures=failures,preflight_report=name,preflight_sha256=digest(ROOT/name),
        reusable_native_indices=list(range(18)),last_unaccepted_index=18,
        classification='Old fixed delays could click disabled Return;last OK did not set ownership. Current driver waits actual stable panels/ownership and retries only while unowned panel remains open.',
        current_full_native_source21_comparison_complete=False,production_implementation_changed=False,whole_game_complete=False)
    out='analysis/current-awards-input-observations.json';write(ROOT/out,result)
    v=read(ROOT/'analysis/verification.json')
    for n in [name,out]:
        if n not in v['reports']:v['reports'].append(n)
    v['current_award_runtime_gate']=dict(report=out,complete=False,reusable_native_panels=18)
    write(ROOT/'analysis/verification.json',v)
    print('Preserved3 native diagnostic failures and18 actually committed stable native panels;full new comparison remains pending.')


if __name__=='__main__':main()
