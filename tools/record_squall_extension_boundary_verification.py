#!/usr/bin/env python3
"""Record supported wrapper branches with a diagnostic COM fixture, not hardware claims."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    reports={}
    for suffix in ['', '-fresh']:
        name=f'squall-extension-boundary{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_for_eax_zoomfx_private_context_and_com_boundary_scope_only'
        assert report['result_comparisons']==report['cached_and_caller_snapshots']==7800
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256'] and digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        assert digest(ROOT/report['source_map']['path'])==report['source_map']['sha256']
        for path,value in report['logs'].items():assert digest(ROOT/report['work_directory']/path)==value,path
        for row in report['precision_contexts']:
            assert row['fixture_cases']==row['result_comparisons']==row['cached_and_caller_snapshots']==row['com_event_snapshots']==3900
            assert row['preset_cases']==960 and row['method_cases']==[120,120,1200,1200,150,150,960]
            assert row['com_calls']==[12,12,96,96,32,32,567] and row['context_bytes']==1344
            assert row['private_context_fixture_only'] and not row['actual_hardware_extension_compared']
            assert row['source_context_rva']==report['source_map']['source_context_rva']
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    audit=read(ROOT/'analysis/squall-export-audit.json')
    methods=['Listener_EAX_SetProperties','Listener_EAX_GetProperties','Listener_EAX_SetPreset','Channel_EAX_SetProperties','Channel_EAX_GetProperties','Channel_ZOOMFX_SetProperties','Channel_ZOOMFX_GetProperties']
    for name in methods:
        row=next(row for row in audit['exports'] if row['name']=='SQUALL_'+name)
        row['diagnostic_COM_boundary_verified_scope']='Actual exported wrappers with isolated private context bytes, instruction-backed context/channel offsets and restored bytes before cleanup; supported/negative flags,versions/IDs/ambient+positioned,exact GUID/property/instance/size/payload,COM success/set failure/get failure/short write+reported0/null interface,preset bytes for all27 presets in EAX1/2/3; two builds and PC53/PC24. Actual supported hardware remains unverified.'
    audit['extension_boundary_reports']=['analysis/squall-extension-boundary-validation.json','analysis/squall-extension-boundary-fresh-validation.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_eax_zoomfx_supported_wrapper_and_preset_com_boundary_only_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,fixture_cases_per_build=7800,cached_and_caller_snapshots_per_build=7800,com_event_snapshots_per_build=7800,preset_cases_per_build=1920,
        original_context_rva='00075034',native_context_offsets=dict(useEax=4,eaxSupport=8,eaxListener=28,eaxLP=32,zoomfxSupport=144,channelsArray=256),
        native_channel_size=300,native_channel_offsets=dict(property3d=16,eaxCache=148,zoomfxCache=220),
        native_instruction_reports=['analysis/squall-extensions-native-listener-instructions.txt','analysis/squall-extensions-native-channel-instructions.txt'],
        actual_hardware_extension_compared=False,diagnostic_private_context_fixture_only=True,original_program_code_used_in_implementation=False,
        build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-extension-boundary-config.log','squall-extension-boundary-build.log','squall-extension-boundary-initial-build.log','squall-extension-boundary-second-build.log']},
        diagnostic_build_corrections=['Explicit mmsystem.h supplies WAV types before dsound.h','Explicit string header supplies std::stoul'],
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',
        original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Actual supported hardware and driver-specific constraints','Supported guarded pointers,aliasing and additional COM failure modes','Remaining complete97-export ABI/behavior and full-game scopes'])
    write(ROOT/'analysis/squall-extension-boundary-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-extension-boundary-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-extension-boundary-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both7800-case EAX/ZOOMFX COM-boundary audits, all27 presets/EAX1/2/3; actual supported hardware remains unverified.')


if __name__=='__main__':main()
