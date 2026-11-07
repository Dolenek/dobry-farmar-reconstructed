#!/usr/bin/env python3
"""Compare load-time FP state and all existing JNG pixel/API fixtures in both builds."""
import datetime
import subprocess
import sys
from record_live_production_verification import ROOT,digest,write


def main():
    commands=[('fpu',[sys.executable,'tools/validate_jng_load_fpu.py','--output','.local/jng-loader-fpu-validation.json'])]
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/JNGLoad.dll'
    artifacts={}
    for suffix,build in [('', 'build/windows'),('-fresh','build/squall-source-repro-vs20261010')]:
        dll=build+'/Release/JNGLoad.dll';decoder=build+'/Release/jng-decode.exe'
        for name in [dll,decoder,build+'/Release/jng-error-probe.exe']:
            artifacts[name]=digest(ROOT/name)
        commands += [
            ('abi'+suffix,[sys.executable,'tools/validate_exports.py','--binary',dll,'--original-name','JNGLoad.dll','--output',f'.local/jng-loader-abi{suffix}-validation.json']),
            ('pixels'+suffix,[sys.executable,'tools/validate_jng.py','--decoder',decoder,'--reference-decoder','build/windows/Release/jng-reference-decode.exe','--reference-dll',str(original),'--output',f'.local/jng-loader-pixels{suffix}-validation.json']),
            ('errors'+suffix,[sys.executable,'tools/validate_jng_error_abi.py','--probe',build+'/Release/jng-error-probe.exe','--source-dll',dll,'--output',f'.local/jng-loader-errors{suffix}-validation.json'])]
    report=dict(status='running',started_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        driver_sha256=digest(__file__),artifacts=artifacts,steps=[],new_full_ctest_run=False,whole_game_complete=False)
    try:
        for name,command in commands:
            log=ROOT/f'.local/jng-loader-regression-{name}.log';print('START',name,flush=True)
            with log.open('wb') as out:p=subprocess.run(command,cwd=ROOT,stdout=out,stderr=subprocess.STDOUT)
            report['steps'].append(dict(name=name,command=command,return_code=p.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=digest(log)))
            print('FINISH',name,p.returncode,flush=True);assert p.returncode==0,(name,p.returncode)
        for name,sha in artifacts.items():assert digest(ROOT/name)==sha,name
        report['status']='passed_JNG_loader72_all416_pixels_two_builds_and40_API_pairs_per_build_only'
    except Exception as e:report.update(status='failed',error=repr(e))
    finally:
        report['finished_at_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
        write(ROOT/'.local/jng-loader-regression-pipeline.json',report)
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
