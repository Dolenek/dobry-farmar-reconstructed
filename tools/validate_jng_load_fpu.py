#!/usr/bin/env python3
"""Compare actual DLL load/unload x87 control/status and SSE control environments."""
import argparse
import datetime
import json
import shutil
import subprocess
import traceback
from record_live_production_verification import ROOT,digest,read,write


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--allow-observed-deviations',action='store_true')
    parser.add_argument('--output',default='.local/jng-load-fpu-validation.json')
    args=parser.parse_args();original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    dlls={'native':original/'JNGLoad.dll','source':ROOT/'build/windows/Release/JNGLoad.dll',
        'fresh':ROOT/'build/squall-source-repro-vs20261010/Release/JNGLoad.dll'}
    probe=ROOT/'build/squall-load-fpu-probe-vs/Release/squall-load-fpu-probe.exe'
    work=ROOT/'.local/jng-load-fpu'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    sources=[p for p in (ROOT/'src/jngload').rglob('*') if p.suffix in ['.cpp','.h']]+[
        ROOT/'tools/validate_jng_load_fpu.py',ROOT/'tools/squall_load_fpu_probe.cpp',ROOT/'tools/squall_load_fpu_probe/CMakeLists.txt']
    for p in sources:
        dest=work/'snapshots'/p.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,dest)
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        work_directory=work.relative_to(ROOT).as_posix(),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sources},
        binaries={n:dict(path=str(p),sha256=digest(p)) for n,p in dlls.items()},
        probe=dict(path=probe.relative_to(ROOT).as_posix(),sha256=digest(probe)),cases=[],deviations=[],whole_jngload_complete=False)
    for name,path in {**dlls,'probe':probe}.items():shutil.copyfile(path,work/(name+path.suffix))
    try:
        for pc in [0,0x200,0x300]:
            for rounding in [0,0x400,0x800,0xc00]:
                for sticky in [0,1]:
                    control=0x7f|pc|rounding;row=dict(control=control,sticky=sticky,outcomes={})
                    for name,path in dlls.items():
                        tag=f'{control:04x}-{sticky}-{name}'
                        p=subprocess.run([str(probe),str(path),f'{control:x}',str(sticky)],cwd=work,capture_output=True,timeout=15)
                        (work/(tag+'.stdout.log')).write_bytes(p.stdout);(work/(tag+'.stderr.log')).write_bytes(p.stderr)
                        assert p.returncode==0,(tag,p.returncode)
                        value=json.loads(p.stdout);assert value['requested_control']==control and value['sticky']==sticky and len(value['cycles'])==2
                        for cycle in value['cycles']:
                            assert cycle['before']==dict(control=control,status=4 if sticky else 0,mxcsr=0x1f80|((control&0xc00)<<3))
                            if name=='native':assert cycle['loaded']['control']==control
                        row['outcomes'][name]=value
                    for name in ['source','fresh']:
                        differences=[]
                        for i,(native,value) in enumerate(zip(row['outcomes']['native']['cycles'],row['outcomes'][name]['cycles'])):
                            for phase in ['loaded','unloaded']:
                                for field,expected in native[phase].items():
                                    if value[phase][field]!=expected:differences.append(dict(cycle=i,phase=phase,field=field,native=expected,candidate=value[phase][field]))
                        if differences:report['deviations'].append(dict(control=control,sticky=sticky,backend=name,differences=differences))
                    report['cases'].append(row)
            print(json.dumps(dict(input_precision_bits=pc,completed_cases=len(report['cases']),deviations=len(report['deviations']))),flush=True)
        for name,p in dlls.items():assert digest(p)==report['binaries'][name]['sha256']
        for name,sha in report['source_files'].items():assert digest(ROOT/name)==sha,name
        for item in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/item['path'])==item['sha256']
        if not args.allow_observed_deviations:assert not report['deviations'],len(report['deviations'])
        report.update(status='observed_actual_DLL_loader_fpu_deviations' if report['deviations'] else 'passed_actual_DLL_load_unload_x87_and_sse_environments_only',
            isolated_processes=72,actual_load_unload_cycles=144,original_files_unchanged=17,
            x87_precisions=3,rounding_modes=4,sticky_zero_divide_cases=True,
            original_program_code_used_in_implementation=False,whole_game_complete=False,
            pending=['Unmasked pending FP exceptions and nonempty x87 input stacks','Other original goal scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        report['logs']={p.name:digest(p) for p in work.glob('*.log')};write(work/'report.json',report);write(ROOT/args.output,report)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],deviations=len(report['deviations']),error=report.get('error'))))
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
