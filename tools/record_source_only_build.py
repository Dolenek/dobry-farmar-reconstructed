#!/usr/bin/env python3
"""Record four real production targets built outside the reference tree."""
import datetime
from record_live_production_verification import ROOT,digest,read,write
from pe import PE


def main():
    stage=ROOT/'work/production-source-20261107-v3';build=ROOT/'build/production-source-standalone-v3/Release'
    manifest=read(stage/'source-stage.json')
    assert digest(ROOT/'tools/stage_production_source.py')==manifest['stager_sha256']
    for name,sha in manifest['copied_source_files'].items():
        assert digest(stage/name)==sha,name
        if name.startswith(('src/','tools/cmake/')) or name=='CMakeLists.txt':assert digest(ROOT/name)==sha,name
    artifacts={}
    for name in ['Dobr\u00fd farm\u00e1\u0159.exe','JNGLoad.dll','Squall.dll','Uninstall.exe']:
        path=build/name;pe=PE(path);assert pe.bits==32 and pe.machine==0x14c
        artifacts[name]=dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path),bytes=path.stat().st_size)
    smoke=read(ROOT/'.local/production-source-standalone-v3-runtime-validation.json')
    assert smoke['status']=='passed_for_source_application_fresh_profile_restart_save_restore_pause_help_and_journal'
    assert len(smoke['sessions'])==2 and all(s['status']=='passed' and s['exit_code']==0 and not s.get('forced_termination') for s in smoke['sessions'])
    assert smoke['implementation_files']=={n:artifacts[a]['sha256'] for n,a in [('FarmarSourceRuntime.exe','Dobr\u00fd farm\u00e1\u0159.exe'),('JNGLoad.dll','JNGLoad.dll'),('Squall.dll','Squall.dll')]}
    assert smoke['source_files']['tools/validate_application_runtime.py']==digest(ROOT/'tools/validate_application_runtime.py')
    reports={}
    for stem in ['jng-abi','squall-abi','uninstall-validation','runtime-validation']:
        name=f'production-source-standalone-v3-{stem}.json';value=read(ROOT/'.local'/name)
        if stem.endswith('-abi'):assert value['status']=='matches_export_contract'
        write(ROOT/'analysis'/name,value);reports['analysis/'+name]=digest(ROOT/'analysis'/name)
    uninstaller=artifacts['Uninstall.exe']['sha256'];effect_reports={}
    for name in ['uninstaller-runtime-validation.json','uninstaller-ui-validation.json','uninstaller-bootstrap-validation.json']:
        value=read(ROOT/'analysis'/name)
        # Match the precise executed source artifact, not a similar NSIS version.
        assert uninstaller in str(value),(name,uninstaller)
        effect_reports['analysis/'+name]=digest(ROOT/'analysis'/name)
    failures=[]
    for stamp in ['20261107T083435854054Z','20261107T084609312600Z']:
        path=ROOT/'.local/application-runtime'/stamp/'report.json';value=read(path);assert value['status']=='failed'
        failures.append(dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path),error=value['error']))
    logs={}
    for kind in ['config','build']:
        p=ROOT/f'.local/production-source-standalone-v3-{kind}.log';logs[p.relative_to(ROOT).as_posix()]=digest(p)
    result=dict(status='passed_four_source_only_Win32_production_targets_and_actual_fresh_restart_UI_smoke_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        staged_source_manifest=manifest,staged_source_manifest_sha256=digest(stage/'source-stage.json'),artifacts=artifacts,
        configuration=dict(BUILD_TESTING=False,BUILD_SQUALL_CANDIDATE=True,BUILD_SQUALL_REFERENCE_LAYOUT=False),
        logs=logs,reports=reports,isolated_uninstaller_effects_transferred_by_exact_binary_sha256=effect_reports,
        actual_source_process_exits=[0,0],journal_checked_on_fresh_session_only=True,
        failed_diagnostic_attempts=failures,
        diagnostic_correction='Check journal before menu/help on the fresh farm; do not mistake restored tutorial1 animation for journal. Alt+Pause freezes the whole application and was rejected as a menu-fixture method.',
        original_program_code_used_in_implementation=False,whole_game_complete=False,
        pending=['Finish remaining actual integrated game checks','Known JNGLoad startup flags difference'])
    name='analysis/production-source-only-build.json';write(ROOT/name,result)
    v=read(ROOT/'analysis/verification.json')
    for n in list(reports)+[name]:
        if n not in v['reports']:v['reports'].append(n)
    v['production_source_only_build']=dict(report=name,all_four_targets=True,testing_disabled=True)
    write(ROOT/'analysis/verification.json',v)
    print('Recorded4 source-only PE32 targets,2 normal real game exits,and exact-uninstaller-byte effects transfer.')


if __name__=='__main__':main()
