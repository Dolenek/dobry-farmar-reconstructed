#!/usr/bin/env python3
"""Record real sample/group playback and explicitly retain untested branches."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    reports={}
    for suffix in ['', '-fresh']:
        name=f'squall-sample-play{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_for_actual_sample_group_play2d3d_and_sample_pause_stop_scope'
        assert report['result_comparisons']==12240 and report['guarded_channel_snapshots']==836
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256'] and digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        work=ROOT/report['work_directory']
        for path,value in {**report['logs'],**report['sample_files']}.items():assert digest(work/path)==value,path
        for row in report['precision_contexts']:
            assert row['result_comparisons']==6120 and row['actual_play_cases']==387 and row['guarded_channel_snapshots']==418
            assert row['sample_pause_cases']==11 and row['empty_lifecycle_cases']==56 and row['actual_directsound']
            assert not row['multiple_members_per_group_verified'] and not row['audible_output_compared']
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    methods=['Sample_Play','Sample_PlayEx','Sample_Play3D','Sample_Play3DEx','SampleGroup_Play','SampleGroup_PlayEx','SampleGroup_Play3D','SampleGroup_Play3DEx','Sample_Pause','Sample_Stop']
    audit=read(ROOT/'analysis/squall-export-audit.json');addresses={}
    for name in methods:
        row=next(row for row in audit['exports'] if row['name']=='SQUALL_'+name);addresses[row['name']]=row['original_address'];row['status']='partially_compared'
        row['loaded_DLL_sample_play_verified_scope']='Actual DirectSound2D/3D; singleton groups17/-7/0; all16 raw loop/start0/1/-1/2 pairs; default/explicit parameters and guarded cached getters; prepared/playing same-sample pause/stop preserves different ID sharing its file; two builds and PC53/PC24. Multi-member selection, invalid3D pointer faults and device failures pending.'
    audit['sample_play_reports']=['analysis/squall-sample-play-validation.json','analysis/squall-sample-play-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_eight_sample_group_play_exports_and_sample_pause_stop_both_builds',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        original_export_addresses=addresses,api_results_per_build=12240,guarded_channel_snapshots_per_build=836,
        actual_created_channels_per_build=774,sample_pause_cases_per_build=22,empty_lifecycle_cases_per_build=112,
        multiple_members_per_group_verified=False,audible_output_compared=False,
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-sample-play-config.log','squall-sample-play-build.log']},
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',
        original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Multiple sample-group members and selection limit','Invalid3D position faults,exhaustion/preemption and concurrent teardown','Remaining complete97-export ABI/behavior and full-game scopes'])
    write(ROOT/'analysis/squall-sample-play-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-sample-play-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-sample-play-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded ten sample/group exports,774 actual channels and12240 API results per build; full Squall scope remains pending.')


if __name__=='__main__':main()
