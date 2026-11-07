#!/usr/bin/env python3
"""Record exact pre-fix DLL FP effects and independently verified channel selection."""
from record_live_production_verification import ROOT,digest,read,write
from pe import PE
from audit_squall_native_arguments import instructions,Audit


def main():
    paths=['.local/squall-preemption/20261107T074541047816Z/report.json',
        '.local/squall-load-fpu/20261107T080016278987Z/report.json']
    reports=[read(ROOT/p) for p in paths]
    preempt,fpu=reports
    assert len(preempt['cases'])==176 and preempt['isolated_processes']==528 and len(preempt['deviations'])==176
    assert all(r['precision']==24 and r['differences']=={'loaded_precision_bits':{'native':0x10000,'candidate':0x20000}} for r in preempt['deviations'])
    assert len(fpu['cases'])==24 and fpu['isolated_processes']==72 and fpu['actual_load_unload_cycles']==144 and len(fpu['deviations'])==42
    for path,value,name in zip(paths,reports,['squall-preemption-before.json','squall-load-fpu-before.json']):
        work=ROOT/value['work_directory']
        for n,sha in value['source_files'].items():assert digest(work/'snapshots'/n)==sha,n
        for n,sha in value['logs'].items():assert digest(work/n)==sha,n
        for n,item in {**value['binaries'],'probe':value['probe']}.items():
            assert digest(work/(n+__import__('pathlib').Path(item['path']).suffix))==item['sha256']
        value.update(raw_runtime_report=dict(path=path,sha256=digest(ROOT/path)),recorder_sha256=digest(__file__))
        write(ROOT/'analysis'/name,value)
    native=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    pe=PE(native);ins=instructions(ROOT/'.local/squall-complete-native-disassembly.txt',pe);audit=Audit(pe,ins);evidence={}
    for start in [0x1003e047,0x1003df6e,0x1003e814,0x1003cf10,0x1003eacb,0x10042ec3,0x100423b5]:
        todo,seen=[start],set()
        while todo:
            a=todo.pop()
            if a in seen:continue
            seen.add(a);r=ins[a];assert r['file_verified']
            evidence[a]=dict(address=f'{a:08x}',bytes=r['raw'].hex(),op=r['op'],operands=r['args']);todo+=audit.successors(a)
    assert pe.u32(pe.offset(0x72fe4))==0x1003cf10
    for a,op,args in [(0x1003df72,'cmp','eax,1'),(0x1003e819,'test','eax,eax'),
        (0x1003e81d,'call','eax'),(0x1003cf1f,'call','1003EACB'),(0x1003cf24,'fnclex',''),
        (0x1003eacb,'push','30000h'),(0x1003ead0,'push','10000h')]:
        assert ins[a]['op']==op and ins[a]['args']==args
    result=dict(status='observed_actual_loader_FP_differences_before_CPP_static_initializers',recorder_sha256=digest(__file__),
        instruction_evidence=[evidence[a] for a in sorted(evidence)],native_entry=f'{pe.image_base+pe.entry_rva:08x}',
        fp_initializer_pointer='10072fe4',fp_initializer='1003cf10',process_attach_only=True,
        pc_mask_crt=0x30000,pc53_crt=0x10000,clears_x87_exception_flags=True,fninit_used_by_original=False,
        sse_original_environment_preserved_on_host=True,preemption_state_deviations_excluding_loader_precision=0,
        original_FP_init_precedes_CPP_initializers=True,correction_applied=False,
        reports={name:digest(ROOT/'analysis'/name) for name in ['squall-preemption-before.json','squall-load-fpu-before.json']},
        original_program_code_used_in_implementation=False,whole_game_complete=False)
    write(ROOT/'analysis/squall-loader-fpu-observations.json',result)
    print('Recorded528 exact preemption processes,176 load precision differences and72 full environment processes/42 differing pairs.')


if __name__=='__main__':main()
