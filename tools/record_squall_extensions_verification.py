#!/usr/bin/env python3
"""Record seven actual extension exports on the explicitly unsupported host."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    reports={}
    for suffix in ['', '-fresh']:
        name=f'squall-extensions{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_for_actual_seven_eax_zoomfx_exports_unsupported_host_scope_only'
        assert report['result_comparisons']==6692 and report['guarded_property_snapshots']==6360
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256'] and digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        work=ROOT/report['work_directory']
        for path,value in {**report['logs'],**report['sample_files']}.items():assert digest(work/path)==value,path
        for row in report['precision_contexts']:
            assert row['actual_eax_version']==0 and not row['successful_hardware_extension_paths_verified']
            assert (row['listener_bytes'],row['channel_bytes'],row['zoomfx_bytes'])==(112,72,52)
            assert row['result_histograms']==[[[-17,240]],[[-17,240]],[[-18,200],[-4,1000]],[[-18,100],[-4,1100]],[[-21,50],[-4,100]],[[-21,25],[-4,125]],[[-17,160]]]
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    for name in ['listener','channel']:
        path=ROOT/f'analysis/squall-extensions-native-{name}-instructions.txt';path.write_bytes((ROOT/f'.local/squall-extensions-native-{name}-disassembly.txt').read_bytes());reports[path.name]=digest(path)
    audit=read(ROOT/'analysis/squall-export-audit.json');addresses={}
    methods=['Listener_EAX_SetProperties','Listener_EAX_GetProperties','Listener_EAX_SetPreset','Channel_EAX_SetProperties','Channel_EAX_GetProperties','Channel_ZOOMFX_SetProperties','Channel_ZOOMFX_GetProperties']
    for name in methods:
        row=next(row for row in audit['exports'] if row['name']=='SQUALL_'+name);addresses[row['name']]=row['original_address'];row['status']='partially_compared'
        row['loaded_DLL_extensions_verified_scope']='Actual EAX0 unsupported host only; pre-Init/empty/real2D/real3D/post-Free, eight version boundaries,32 presets,five channel IDs,six guarded/null/bad/read-only/cross-page/noaccess pointers; exact errors and unchanged complete caller buffers; two builds and PC53/PC24. Supported property paths explicitly pending.'
    assert len(audit['exports'])==97 and all(row['status']=='partially_compared' for row in audit['exports'])
    audit['extensions_reports']=['analysis/squall-extensions-validation.json','analysis/squall-extensions-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_seven_actual_extensions_unsupported_host_only_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,original_export_addresses=addresses,api_results_per_build=6692,guarded_property_snapshots_per_build=6360,
        host_eax_version=0,successful_hardware_extension_paths_verified=False,all_97_exports_have_some_comparison=True,whole_export_signatures_verified=False,
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-extensions-config.log','squall-extensions-build.log']},
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',
        original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Successful EAX/ZOOMFX property paths,presets and version cache behavior','Diagnostic COM boundary comparison for inaccessible hardware branches','Remaining complete97-export ABI/behavior and full-game scopes'])
    write(ROOT/'analysis/squall-extensions-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-extensions-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-extensions-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded all seven EAX/ZOOMFX error paths; all97 exports now have partial comparisons, full Squall remains incomplete.')


if __name__=='__main__':main()
