#!/usr/bin/env python3
"""Preserve exact pre-fix public pointer deviations and all tested old inputs."""
import shutil
import hashlib
from record_live_production_verification import ROOT,digest,read,write


def main():
    raw_path=ROOT/'.local/squall-pointer-faults/20261106T195301366191Z/report.json'
    report=read(raw_path);work=ROOT/report['work_directory']
    assert report['status']=='observed_actual_pointer_outcome_deviations'
    assert len(report['cases'])==90 and report['isolated_processes']==270
    assert report['fault_processes']==120 and report['normal_processes']==150
    assert len(report['deviations'])==64
    for row in report['deviations']:
        assert row['method'] in range(4) and row['case'] in range(4)
        assert row['backend'] in ['source','fresh']
        assert row['differences']=={'fault_address':{'native':8,'candidate':4}}
    for path,sha in report['source_files'].items():assert digest(work/'snapshots'/path)==sha,path
    for path,sha in report['logs'].items():assert digest(work/path)==sha,path
    snapshots={}
    for name,row in {**report['binaries'],'probe':report['probe']}.items():
        src=ROOT/row['path'];dst=work/(name+src.suffix)
        if not dst.exists():
            assert digest(src)==row['sha256'];shutil.copyfile(src,dst)
        assert digest(dst)==row['sha256']
        snapshots[name]=dict(path=dst.relative_to(ROOT).as_posix(),sha256=digest(dst))
    report.update(raw_runtime_report=dict(path=raw_path.relative_to(ROOT).as_posix(),sha256=digest(raw_path)),
        source_snapshot_root=(work/'snapshots').relative_to(ROOT).as_posix(),binary_snapshots=snapshots,
        recorder_sha256=digest(__file__),correction_applied=False,
        observed_deviation='Native reads Position[2] first;both candidate builds read Position[1] first after invalid input was nulled')
    write(ROOT/'analysis/squall-pointer-faults-before.json',report)
    name='analysis/squall-pointer-faults-before.json'
    verification=read(ROOT/'analysis/verification.json')
    if name not in verification['reports']:verification['reports'].append(name)
    verification['squall_pointer_faults']=dict(report=name,resolved=False,actual_processes=270,observed_deviations=64,whole_behavior_complete=False)
    verification['latest_scoped_regression']=dict(report=name,recorded_at_utc=report['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    values=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(values).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report']=name;write(ROOT/'analysis/verification.json',verification)
    print('Preserved270 actual isolated processes,64 precise pre-fix read-order deviations and all source/binary snapshots.')


if __name__=='__main__':main()
