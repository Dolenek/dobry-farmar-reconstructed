#!/usr/bin/env python3
"""Record actual ignored extended effects and keep the rejected fixture separate."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    value=read(ROOT/'.local/squall-tracker-effect-formats-validation.json');work=ROOT/value['work_directory']
    assert value['status']=='passed_actual_MT2_256_fx_commands_and_AMS2_64_effects_in_both_builds_only'
    assert len(value['cases'])==4 and value['isolated_processes']==12
    for name,sha in value['source_files'].items():assert digest(ROOT/name)==sha and digest(work/'snapshots'/name)==sha,name
    for name,sha in value['fixture_hashes'].items():assert digest(work/'fixtures'/name)==sha
    for case in value['cases']:
        buffers=[]
        for backend,outcome in case['outcomes'].items():
            folder=ROOT/outcome['directory'];assert outcome['exit_code']==0 and outcome['state']['create_result']==1 and outcome['state']['guards_intact']
            for name,sha in outcome['artifacts'].items():assert digest(folder/name)==sha,name
            buffers.append((folder/'pattern0.bin').read_bytes())
        assert buffers[0]==buffers[1]==buffers[2] and len(buffers[0])==case['pattern_bytes']
    rejected=ROOT/'.local/squall-tracker-effect-formats/20261107T090444971681Z/report.json';failed=read(rejected)
    assert failed['status']=='failed' and len(failed['cases'])==2 and 'Actual Create rejected fixture' in failed['error']
    name='analysis/squall-tracker-effect-formats-validation.json';write(ROOT/name,value)
    result=dict(status='passed_native_extended_effect_absence_MT2_and_AMS2_both_builds_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        report=name,report_sha256=digest(ROOT/name),native_MT2_fxcmd_values=256,native_AMS2_effect_values=64,
        candidate_builds=2,precision_contexts=['PC53','PC24'],isolated_processes=12,
        exact_allocated_pattern_bytes_per_candidate=sum(c['pattern_bytes'] for c in value['cases']),
        MT2_nonzero_fxcmd_native_behavior='255 values leave command and parameter zero;fxcmd0 actual MOD-effect conversion is exercised as control',
        AMS2_native_behavior='Effect codes16..63 are ignored;0..15 MOD conversion and12 volume-column branch are exercised in the same actual pattern',
        rejected_zero_sample_fixture=dict(report=rejected.relative_to(ROOT).as_posix(),sha256=digest(rejected),error=failed['error']),
        source_TODO_comments_are_not_reconstruction_stubs=True,production_implementation_changed=False,whole_squall_complete=False,
        pending=['Broader compressed sample,envelope,packed pattern and malformed file cases','Current integrated game and original whole-project requirements'])
    out='analysis/squall-tracker-effect-formats-regression.json';write(ROOT/out,result)
    v=read(ROOT/'analysis/verification.json')
    for n in [name,out]:
        if n not in v['reports']:v['reports'].append(n)
    v['squall_tracker_extended_effects']=dict(report=out,native_absence_confirmed=True,MT2_commands=256,AMS2_effects=64)
    write(ROOT/'analysis/verification.json',v)
    print('Recorded native absence of extended MT2/AMS2 effects in both builds;12 actual loader processes.')


if __name__=='__main__':main()
