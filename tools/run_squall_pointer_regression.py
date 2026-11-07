#!/usr/bin/env python3
"""Build both current trees and run the regression needed by ordered3D reads."""
import datetime
import argparse
import subprocess
import sys
from record_live_production_verification import ROOT,digest,write,read


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--resume-pipeline',type=str)
    args=parser.parse_args()
    commands=[('build-probe',['cmake','--build','build/squall-pointer-fault-probe-vs','--config','Release'])]
    builds=[('', 'build/windows'),('-fresh','build/squall-source-repro-vs20261010')]
    for suffix,build in builds:
        commands.append(('build'+suffix,['cmake','--build',build,'--config','Release','--parallel','4']))
    commands.append(('ctest',['ctest','--test-dir','build/windows','-C','Release','--output-on-failure',
        '--output-junit',str(ROOT/'.local/squall-pointer-ctest.xml')]))
    commands.append(('faults',[sys.executable,'tools/validate_squall_pointer_faults.py']))
    for suffix,build in builds:
        dll=build+'/squall-candidate/Release/Squall.dll'
        for name,command in [
            ('abi',['tools/validate_exports.py','--binary',dll,'--original-name','Squall.dll']),
            ('play',['tools/validate_squall_sample_play.py','--candidate',dll]),
            ('stack',['tools/validate_squall_export_stack.py','--candidate',dll]),
            ('dll',['tools/validate_squall_dll.py','--candidate',dll,'--oracle',build+'/Release/squall-dll-reference-test.exe']),
            ('game-audio',['tools/validate_game_audio.py','--build-dir',build]),
            ('person-sounds',['tools/validate_person_sounds.py','--oracle',build+'/Release/person-sounds-reference-test.exe'])]:
            commands.append((name+suffix,[sys.executable,*command,'--output',f'.local/squall-pointer-{name}{suffix}-validation.json']))
    report=dict(status='running',started_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        driver_sha256=digest(__file__),steps=[],whole_game_complete=False)
    if args.resume_pipeline:
        prior_path=ROOT/args.resume_pipeline;prior=read(prior_path)
        assert prior['status']=='failed' and prior['steps'][-1]['name']=='faults'
        assert prior['steps'][-1]['return_code']==1
        reused=prior['steps'][:-1]
        assert len(reused)==4
        for step,(name,command) in zip(reused,commands):
            assert step['name']==name and step['command']==command and step['return_code']==0
            assert digest(ROOT/step['log'])==step['log_sha256']
        failed=read(ROOT/'.local/squall-pointer-faults/20261106T201034530041Z/report.json')
        for value in failed['binaries'].values():assert digest(ROOT/value['path'])==value['sha256']
        report.update(resumed_from=dict(path=args.resume_pipeline,sha256=digest(prior_path),
            reason='Python auditor used incorrect CRT precision constants;production binaries unchanged'),steps=reused)
        commands=commands[len(reused):]
    try:
        for name,command in commands:
            log=ROOT/'.local'/f'squall-pointer-regression-{name}.log'
            print('START',name,flush=True)
            with log.open('wb') as out:result=subprocess.run(command,cwd=ROOT,stdout=out,stderr=subprocess.STDOUT)
            report['steps'].append(dict(name=name,command=command,return_code=result.returncode,
                log=log.relative_to(ROOT).as_posix(),log_sha256=digest(log)))
            print('FINISH',name,result.returncode,flush=True)
            assert result.returncode==0,(name,result.returncode)
        report['status']='passed_both_builds_full102_pointer_play_stack_pcm_and_game_audio_pipeline_only'
    except Exception as error:report.update(status='failed',error=repr(error))
    finally:
        report['finished_at_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
        write(ROOT/'.local/squall-pointer-regression-pipeline.json',report)
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
