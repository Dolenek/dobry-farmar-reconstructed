#!/usr/bin/env python3
"""Verify both DLLs from the latest standalone source tree against native loader fixtures."""
import datetime
import json
import subprocess
from record_live_production_verification import ROOT,digest,read,write


def main():
    work=ROOT/'.local/production-source-standalone-v4-loader';work.mkdir(exist_ok=False)
    build=ROOT/'build/production-source-standalone-v4/Release'
    probe=ROOT/'build/squall-load-fpu-probe-vs/Release/squall-load-fpu-probe.exe'
    stage=ROOT/'work/production-source-20261107-v4';manifest=read(stage/'source-stage.json')
    for name,sha in manifest['copied_source_files'].items():
        assert digest(stage/name)==sha,name
        if name.startswith(('src/','tools/cmake/')) or name=='CMakeLists.txt':assert digest(ROOT/name)==sha,name
    result=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        driver_sha256=digest(__file__),work_directory=work.relative_to(ROOT).as_posix(),
        stage_manifest_sha256=digest(stage/'source-stage.json'),source_files=manifest['copied_source_files'],
        probe_sha256=digest(probe),dll_sha256={n:digest(build/n) for n in ['Squall.dll','JNGLoad.dll']},cases=[])
    try:
        references={}
        for name,report in [('Squall.dll','analysis/squall-loader-fpu-validation.json'),('JNGLoad.dll','analysis/jng-loader-fpu-validation.json')]:
            value=read(ROOT/report);assert not value['deviations']
            references[report]=digest(ROOT/report)
            for case in value['cases']:
                control,sticky=case['control'],case['sticky'];tag=f'{name}-{control:04x}-{sticky}'
                p=subprocess.run([str(probe),str(build/name),f'{control:x}',str(sticky)],capture_output=True,timeout=15)
                (work/(tag+'.stdout.json')).write_bytes(p.stdout);(work/(tag+'.stderr.log')).write_bytes(p.stderr)
                assert p.returncode==0,(tag,p.returncode)
                actual=json.loads(p.stdout);assert actual==case['outcomes']['native'],tag
                result['cases'].append(dict(dll=name,control=control,sticky=sticky,exit_code=0,outcome=actual))
        result.update(status='passed_current_standalone_source_DLL_loader_state48_processes_only',isolated_processes=48,
            actual_load_unload_cycles=96,native_reference_reports=references,whole_game_complete=False)
    except Exception as error:result.update(status='failed',error=repr(error))
    result['logs']={p.name:digest(p) for p in work.iterdir() if p.is_file()}
    write(work/'report.json',result);write(ROOT/'.local/production-source-standalone-v4-loader-validation.json',result)
    print(json.dumps({k:result.get(k) for k in ['status','isolated_processes','actual_load_unload_cycles','error']}))
    return int(result['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
