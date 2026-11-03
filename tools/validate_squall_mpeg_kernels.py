#!/usr/bin/env python3
"""Compare exact original MPEG tables and numerical kernels on owned fixtures."""
import argparse,datetime,json,shutil,subprocess
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll');p.add_argument('--output',type=Path,default=ROOT/'.local/squall-mpeg-kernels-validation.json');a=p.parse_args()
    dll=a.candidate.resolve(strict=True);stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-mpeg-kernels'/stamp;work.mkdir(parents=True)
    native=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll';build=ROOT/'build/squall-mpeg-formats-probe-vs/Release'
    files=[dll,dll.with_suffix('.map'),build/'squall-mpeg-math-probe.exe',build/'squall-mpeg-kernel-probe.exe'];artifact=work/'build-artifacts';artifact.mkdir()
    for path in files:shutil.copyfile(path,artifact/path.name)
    source=[path for path in (ROOT/'src/squall').rglob('*') if path.is_file()]+[ROOT/name for name in ['tools/validate_squall_mpeg_kernels.py','tools/squall_mpeg_math_probe.cpp','tools/squall_mpeg_kernel_probe.cpp','tools/squall_mpeg_formats_probe/CMakeLists.txt','tools/cmake/SquallCandidate.cmake']]
    for path in source:
        target=work/'source-snapshots'/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),candidate_path=dll.relative_to(ROOT).as_posix(),rebuilt_sha256=digest(dll),reference_sha256=digest(native),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in source},build_artifacts={path.name:digest(path) for path in artifact.iterdir()},cases=[])
    lines=(artifact/'Squall.map').read_text(encoding='latin-1').splitlines()
    def symbol(name):
        matches=[line.split() for line in lines if '?'+name+'@CDecompressMpeg@@' in line];assert len(matches)==1,name
        return int(matches[0][2],16)-0x10000000
    def run(kind,name,nrva,pc):
        folder=work/(pc+'-'+name);folder.mkdir();rvas=[format(nrva,'x'),format(symbol(name),'x')]
        command=[str(artifact/('squall-mpeg-math-probe.exe' if kind=='tables' else 'squall-mpeg-kernel-probe.exe')),str(native),str(artifact/'Squall.dll')]
        command+=rvas+['0','11936'] if kind=='tables' else [name,*rvas,format(symbol('L3table_init'),'x')]
        command+=['pc24'] if pc=='PC24' else [];result=subprocess.run(command,cwd=folder,capture_output=True,timeout=60);(folder/'stdout.json').write_bytes(result.stdout);(folder/'stderr.log').write_bytes(result.stderr)
        row=dict(kind=kind,name=name,precision=pc,command=command,exit_code=result.returncode,artifacts={path.relative_to(work).as_posix():digest(path) for path in folder.iterdir() if path.is_file()});report['cases'].append(row)
        if result.stdout:row['result']=json.loads(result.stdout)
        write(work/'report.json',report);assert result.returncode==0,(name,pc,result.stderr.decode('utf-8',errors='replace'),row.get('result'))
        if kind=='tables':assert row['result']['words']==11936 and row['result']['different_words']==0
        else:assert row['result']['cases']==4096 and row['result']['failed_cases']==row['result']['different_bytes']==0
    try:
        for pc in ['PC53','PC24']:
            for name,nrva in [('L1table_init',0x8610),('L2table_init',0x8f30),('L3table_init',0xd620),('imdct_init',0xe8f0)]:run('tables',name,nrva,pc)
            for name,nrva in [('fdct32',0xf250),('window',0x10cf0),('window_dual',0x10e60),('imdct18',0xea50),('imdct6_3',0xeee0)]:run('kernel',name,nrva,pc)
        for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
        for path in artifact.iterdir():assert digest(path)==report['build_artifacts'][path.name]
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
        report.update(status='passed_eight_exact_table_initializers_and_40960_guarded_kernel_cases_only',kernel_cases=40960,table_object_bytes_compared=8*47744,x87_control_word_and_stack_balanced=True,whole_squall_complete=False,whole_game_complete=False)
    except Exception as error:report.update(status='failed',error=repr(error))
    write(work/'report.json',report);write(a.output,report);print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))));return int(report['status']=='failed')
if __name__=='__main__':raise SystemExit(main())
