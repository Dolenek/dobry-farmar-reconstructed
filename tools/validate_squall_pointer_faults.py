#!/usr/bin/env python3
"""Compare isolated actual public 3D/cone pointer outcomes in all three DLLs."""
import argparse
import datetime
import json
import shutil
import struct
import subprocess
import traceback
from pathlib import Path
from record_live_production_verification import ROOT, digest, read, write


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--allow-observed-deviations',action='store_true')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-pointer-faults-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    inventory=read(ROOT/'analysis/inventory.json')['files']
    binaries={'native':original/'Squall.dll','source':ROOT/'build/windows/squall-candidate/Release/Squall.dll',
        'fresh':ROOT/'build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll'}
    probe=ROOT/'build/squall-pointer-fault-probe-vs/Release/squall-pointer-fault-probe.exe'
    paths=list((ROOT/'src/squall').rglob('*.cpp'))+list((ROOT/'src/squall').rglob('*.h'))+[
        ROOT/'tools/validate_squall_pointer_faults.py',ROOT/'tools/squall_pointer_fault_probe.cpp',ROOT/'tools/squall_pointer_fault_probe/CMakeLists.txt']
    work=ROOT/'.local/squall-pointer-faults'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    for path in paths:
        dst=work/'snapshots'/path.relative_to(ROOT);dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,dst)
    pcm=bytes(32000)
    wav=struct.pack('<4sI4s4sIHHIIHH4sI',b'RIFF',36+len(pcm),b'WAVE',b'fmt ',16,1,1,16000,32000,2,16,b'data',len(pcm))+pcm
    (work/'sample.wav').write_bytes(wav)
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        work_directory=work.relative_to(ROOT).as_posix(),driver_sha256=digest(__file__),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        binaries={name:dict(path=str(p),sha256=digest(p)) for name,p in binaries.items()},
        probe=dict(path=probe.relative_to(ROOT).as_posix(),sha256=digest(probe)),
        sample_sha256=digest(work/'sample.wav'),cases=[],deviations=[],whole_squall_complete=False,whole_game_complete=False)

    def unchanged():
        for f in inventory:assert digest(original/f['path'])==f['sha256'],f['path']
        for name,p in binaries.items():assert digest(p)==report['binaries'][name]['sha256'],name
        for p,sha in report['source_files'].items():assert digest(ROOT/p)==sha,p

    try:
        unchanged()
        for precision in [53,24]:
            for method in range(5):
                for case in range(5 if method==4 else 10):
                    row=dict(precision=precision,method=method,case=case,outcomes={})
                    for name,dll in binaries.items():
                        tag=f'{precision}-{method}-{case}-{name}'
                        p=subprocess.run([str(probe),str(dll),str(method),str(case),str(precision)],cwd=work,capture_output=True,timeout=25)
                        (work/(tag+'.stdout.log')).write_bytes(p.stdout);(work/(tag+'.stderr.log')).write_bytes(p.stderr)
                        assert p.returncode==0,(tag,p.returncode,p.stderr.decode(errors='replace'))
                        out=json.loads(p.stdout);assert (out['method'],out['case'],out['precision'])==(method,case,precision)
                        assert out['lead_guard']==0x5a5a5a5a and out['tail_guard']==0xa5a5a5a5
                        assert out['initialized']==1 and out['sample']>0
                        fault=(method<4 and case<4) or (method==4 and case<4)
                        assert bool(out['exception'])==fault and out['explicit_fault_process_termination']==fault,(tag,out)
                        if fault:
                            assert out['exception']==0xc0000005 and out['access']==int(method==4)
                            assert out['fault_address']<12
                        else:assert out['result']>=0
                        row['outcomes'][name]=out
                    # Instruction RVA is retained independently: different
                    # compiler layouts cannot share instruction addresses.
                    reference=row['outcomes']['native']
                    for name in ['source','fresh']:
                        differences={key:dict(native=value,candidate=row['outcomes'][name][key])
                            for key,value in reference.items() if key!='instruction_rva' and row['outcomes'][name][key]!=value}
                        if differences:report['deviations'].append(dict(precision=precision,method=method,case=case,backend=name,differences=differences))
                    report['cases'].append(row)
                print(json.dumps(dict(precision=precision,method=method,completed_cases=len(report['cases']),deviations=len(report['deviations']))),flush=True)
        unchanged()
        if not args.allow_observed_deviations:assert not report['deviations'],('Actual public pointer outcomes differ',len(report['deviations']))
        report.update(status='observed_actual_pointer_outcome_deviations' if report['deviations'] else 'passed_actual_isolated_3d_cone_pointer_outcomes_only',
            original_files_unchanged=17,isolated_processes=len(report['cases'])*3,
            fault_processes=sum(bool(o['exception']) for c in report['cases'] for o in c['outcomes'].values()),
            normal_processes=sum(not o['exception'] for c in report['cases'] for o in c['outcomes'].values()),
            original_program_code_used_in_implementation=False,ordinary_gameplay_crash_claimed=False,
            coverage=['four real public3D play methods,null/bad/guard/truncated/readonly/edge Position and optional Velocity',
                'actual prepared3D channel cone getter,null/bad/guard/readonly OutsideVolume and writable control',
                'both source builds and PC53/PC24,exact exception/access/fault byte and remaining output words'],
            pending=['other malformed arguments,device faults,concurrent teardown and whole original goal'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        report['logs']={p.name:digest(p) for p in work.glob('*.log')}
        write(work/'report.json',report);write(args.output,report)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],deviations=len(report['deviations']),error=report.get('error'))))
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
