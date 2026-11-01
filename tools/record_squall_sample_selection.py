#!/usr/bin/env python3
"""Record actual group membership, untouched RNG and all four real playback paths."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    reports={};sequences=None
    for suffix in ['', '-fresh']:
        name=f'squall-sample-selection{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_for_actual_multi_member_groups_native_rng_sequence_256_cap_and_real_group_channels_only'
        assert report['result_comparisons']==236896 and report['selector_calls_compared']==229376 and report['actual_export_channels']==224
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256'] and digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        assert digest(ROOT/report['source_map']['path'])==report['source_map']['sha256']
        work=ROOT/report['work_directory']
        for path,value in {**report['logs'],**report['sample_files']}.items():assert digest(work/path)==value,path
        for context in report['precision_contexts']:
            assert context['selector_calls_compared']==114688 and context['actual_export_channels']==112 and context['loaded_samples_per_dll']==300
            assert context['private_context_read_only'] and not context['rng_state_written']
            groups=context['groups'];assert [g['members'] for g in groups]==[1,2,7,255,256,257,300]
            for group in groups:assert group['eligible']==min(group['members'],256) and len(group['selected_counts'])==group['eligible'] and all(group['selected_counts']) and sum(group['selected_counts'])==16384
            if sequences is None:sequences=groups
            else:assert sequences==groups
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    lines=(ROOT/'.local/squall-complete-native-disassembly.txt').read_text(encoding='utf-8').splitlines()
    start=next(i for i,line in enumerate(lines) if '10008330:' in line);end=next(i for i,line in enumerate(lines) if '10008410:' in line)
    path=ROOT/'analysis/squall-sample-selection-native-instructions.txt';path.write_text('\n'.join(lines[start:end])+'\n',encoding='utf-8');reports[path.name]=digest(path)
    audit=read(ROOT/'analysis/squall-export-audit.json')
    for name in ['SampleGroup_Play','SampleGroup_PlayEx','SampleGroup_Play3D','SampleGroup_Play3DEx']:
        row=next(row for row in audit['exports'] if row['name']=='SQUALL_'+name)
        row['loaded_DLL_sample_selection_verified_scope']='Actual300 loaded sample IDs sharing one Reader; public membership/default updates; groups1/2/7/255/256/257/300,first256 cap,untouched default CRT sequence,all eligible observed; actual all four2D/3D group play methods and readonly selected SampleID/default-vs-explicit frequency/priority; two builds and PC53/PC24'
    audit['sample_selection_reports']=['analysis/squall-sample-selection-validation.json','analysis/squall-sample-selection-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_multi_member_group_rng_cap_and_four_real_group_play_exports_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        api_results_per_build=236896,selector_results_per_build=229376,actual_group_channels_per_build=224,loaded_samples_per_dll_per_precision=300,
        member_counts=[1,2,7,255,256,257,300],eligible_limit=256,selectors_per_group_per_precision=16384,original_selector_address='10008330',
        readonly_native_sample_id_offset='4c',rng_state_written=False,opaque_ids_normalized=False,
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-sample-selection-config.log','squall-sample-selection-build.log']},
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Concurrent group mutation,65535 sample limit and generation wraparound','Additional decoder formats and remaining complete97-export/full-game scopes'])
    write(ROOT/'analysis/squall-sample-selection-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-sample-selection-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-sample-selection-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both236896-result multi-member selection audits, native256 cap and224 real group playback channels each.')


if __name__=='__main__':main()
