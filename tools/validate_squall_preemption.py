#!/usr/bin/env python3
"""Compare eight actual public play methods after all four channels are occupied."""
import argparse
import datetime
import json
import shutil
import struct
import subprocess
import traceback
from record_live_production_verification import ROOT,digest,read,write
from pe import PE
from audit_squall_native_arguments import instructions,Audit


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--allow-observed-deviations',action='store_true')
    parser.add_argument('--output',default='.local/squall-preemption-validation.json')
    args=parser.parse_args();original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    dlls={'native':original/'Squall.dll','source':ROOT/'build/windows/squall-candidate/Release/Squall.dll',
        'fresh':ROOT/'build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll'}
    probe=ROOT/'build/squall-preemption-probe-vs/Release/squall-preemption-probe.exe'
    work=ROOT/'.local/squall-preemption'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    sources=sorted(list((ROOT/'src/squall').rglob('*.cpp'))+list((ROOT/'src/squall').rglob('*.h')))+[
        ROOT/'tools/validate_squall_preemption.py',ROOT/'tools/squall_preemption_probe.cpp',ROOT/'tools/squall_preemption_probe/CMakeLists.txt',ROOT/'tools/squall_exports.inc']
    for p in sources:
        dest=work/'snapshots'/p.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,dest)
    pcm=bytes(32000);(work/'sample.wav').write_bytes(struct.pack('<4sI4s4sIHHIIHH4sI',b'RIFF',36+len(pcm),b'WAVE',b'fmt ',16,1,1,16000,32000,2,16,b'data',len(pcm))+pcm)
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        work_directory=work.relative_to(ROOT).as_posix(),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sources},
        binaries={name:dict(path=str(p),sha256=digest(p)) for name,p in dlls.items()},
        probe=dict(path=probe.relative_to(ROOT).as_posix(),sha256=digest(probe)),sample_sha256=digest(work/'sample.wav'),
        cases=[],deviations=[],whole_game_complete=False,whole_squall_complete=False)
    for name,p in {**dlls,'probe':probe}.items():shutil.copyfile(p,work/(name+p.suffix))

    def unchanged():
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/row['path'])==row['sha256']
        for name,p in dlls.items():assert digest(p)==report['binaries'][name]['sha256']
        for name,sha in report['source_files'].items():assert digest(ROOT/name)==sha,name

    try:
        native=PE(dlls['native']);ins=instructions(ROOT/'.local/squall-complete-native-disassembly.txt',native);audit=Audit(native,ins)
        todo,seen=[0x100024c0],set()
        while todo:
            address=todo.pop()
            if address in seen:continue
            seen.add(address);assert ins[address]['file_verified'];todo+=audit.successors(address)
        for a,op,operands in [(0x100024dc,'mov','edi,dword ptr [esp+14h]'),(0x100024ed,'jl','100024F3'),
            (0x100024ef,'mov','ebp,edx'),(0x100024f1,'mov','edi,eax'),(0x10002525,'mov','eax,edi')]:
            assert ins[a]['op']==op and ins[a]['args']==operands
        report['native_selection_instructions']=[dict(address=f'{a:08x}',bytes=ins[a]['raw'].hex(),op=ins[a]['op'],operands=ins[a]['args']) for a in sorted(seen)]
        unchanged()
        scenarios=[(0,p) for p in [0,1,2,3,20,21,49,50,2147483647]]+[(1,20),(2,20)]
        for precision in [53,24]:
            for method in range(8):
                for pattern,priority in scenarios:
                    row=dict(precision=precision,method=method,pattern=pattern,priority=priority,outcomes={})
                    for name,dll in dlls.items():
                        tag=f'{precision}-{method}-{pattern}-{priority}-{name}'
                        p=subprocess.run([str(probe),str(dll),str(method),str(priority),str(precision),str(pattern)],cwd=work,capture_output=True,timeout=25)
                        (work/(tag+'.stdout.log')).write_bytes(p.stdout);(work/(tag+'.stderr.log')).write_bytes(p.stderr)
                        if p.returncode:
                            assert name!='native',(tag,p.returncode)
                            row['outcomes'][name]=dict(unhandled_process_exit=p.returncode);continue
                        out=json.loads(p.stdout)
                        assert out['before_precision_bits']==out['after_precision_bits']==(0x10000 if precision==53 else 0x20000)
                        row['outcomes'][name]=out
                    reference=row['outcomes']['native'];assert reference['exception']==0 and reference['new_status']==3 and reference['new_priority']==priority
                    prior=[[20,30,40,50],[20,20,20,20],[50,20,20,40]][pattern]
                    selected=priority;minimum=priority
                    for index,value in enumerate(prior):
                        if minimum>=value:minimum=value;selected=index
                    assert 0<=selected<4 and reference['result']==512+selected
                    for index,channel in enumerate(reference['channels']):
                        assert channel['id']==256+index and channel['before_status']==3 and channel['before_priority']==prior[index]
                        assert channel['after_status']==(0 if index==selected else 3)
                        assert channel['after_priority']==(-12 if index==selected else prior[index])
                    row['native_selected_index']=selected
                    for name in ['source','fresh']:
                        out=row['outcomes'][name]
                        differences={key:dict(native=value,candidate=out.get(key)) for key,value in reference.items() if key!='instruction_rva' and out.get(key)!=value}
                        if differences:report['deviations'].append(dict(precision=precision,method=method,pattern=pattern,priority=priority,backend=name,differences=differences))
                    report['cases'].append(row)
                print(json.dumps(dict(precision=precision,method=method,completed_cases=len(report['cases']),deviations=len(report['deviations']))),flush=True)
        unchanged()
        if not args.allow_observed_deviations:assert not report['deviations'],len(report['deviations'])
        report.update(status='observed_actual_channel_preemption_deviations' if report['deviations'] else 'passed_actual_full_channel_preemption_and_priority_fallback_only',
            isolated_processes=len(report['cases'])*3,original_files_unchanged=17,
            audible_output_compared=False,prepared_buffers_only=True,original_program_code_used_in_implementation=False,
            pending=['Out-of-range fallback index cases,started/paused channel preemption and concurrent mutation','Remaining full original goal scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        report['logs']={p.name:digest(p) for p in work.glob('*.log')};write(work/'report.json',report);write(ROOT/args.output,report)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],deviations=len(report['deviations']),error=report.get('error'))))
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
