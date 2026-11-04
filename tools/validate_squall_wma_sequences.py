#!/usr/bin/env python3
"""Compare stateful WMA reads across mono/stereo, start/loop and zero lengths."""
import argparse,datetime,json,shutil,subprocess
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll');p.add_argument('--output',type=Path,default=ROOT/'.local/squall-wma-sequences-validation.json');args=p.parse_args()
    candidate=args.candidate.resolve(strict=True);oracle=ROOT/'build/squall-wma-probe-vs/Release/squall-wma-probe.exe';native=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll';work=ROOT/'.local/squall-wma-sequences'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True);artifact=work/'build-artifacts';artifact.mkdir()
    for path in [candidate,candidate.with_suffix('.map'),oracle]:shutil.copyfile(path,artifact/path.name)
    lines=(artifact/'Squall.map').read_text(encoding='latin-1').splitlines();matches=[line.split() for line in lines if '?api_SampleGetFile@@' in line];assert len(matches)==1;own=int(matches[0][2],16)-0x10000000
    fixtures=read(ROOT/'.local/squall-wma-owned-fixtures/report.json');paths=[path for path in (ROOT/'src/squall').rglob('*') if path.is_file()]+[ROOT/name for name in ['tools/validate_squall_wma_sequences.py','tools/squall_wma_probe.cpp','tools/squall_wma_probe/CMakeLists.txt']]
    for path in paths:
        target=work/'source-snapshots'/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),candidate_path=candidate.relative_to(ROOT).as_posix(),rebuilt_sha256=digest(candidate),reference_sha256=digest(native),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},build_artifacts={path.name:digest(path) for path in artifact.iterdir()},fixture_manifest=fixtures,cases=[])
    def unchanged():
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
    try:
        failures=[];total=0
        for index,fixture in enumerate(fixtures['fixtures']):
            for pc in ['PC53','PC24']:
                for memory in [0,1]:
                    filename=fixture['name'];row=dict(fixture=filename,precision=pc,memory=memory,runs=[]);report['cases'].append(row)
                    for backend,dll,rva in [('native',native,0x8300),('source',artifact/'Squall.dll',own)]:
                        folder=work/str(index)/pc/str(memory)/backend;folder.mkdir(parents=True);source=ROOT/'.local/squall-wma-owned-fixtures'/filename;assert digest(source)==fixtures['files'][filename];shutil.copyfile(source,folder/filename)
                        command=[str(artifact/oracle.name),str(dll),format(rva,'x'),filename,'sequence','0','128',str(memory),*(['pc24'] if pc=='PC24' else [])];run=dict(backend=backend,directory=folder.relative_to(ROOT).as_posix(),command=command);row['runs'].append(run)
                        try:
                            result=subprocess.run(command,cwd=folder,capture_output=True,timeout=30,creationflags=0x08000000);(folder/'stdout.json').write_bytes(result.stdout);(folder/'stderr.log').write_bytes(result.stderr);run['exit_code']=result.returncode
                            if result.returncode==0:
                                try:run['result']=json.loads(result.stdout)
                                except json.JSONDecodeError as error:run['error']='Incomplete diagnostic stdout: '+str(error)
                            else:run['error']=result.stderr.decode('utf-8',errors='replace')
                        except subprocess.TimeoutExpired as error:run.update(timeout=True,error=str(error));(folder/'timeout.stdout.log').write_bytes(error.stdout or b'');(folder/'timeout.stderr.log').write_bytes(error.stderr or b'')
                        run['artifacts']={path.name:digest(path) for path in folder.iterdir() if path.is_file()}
                    runs=row['runs'];passed=all(value.get('exit_code')==0 and 'result' in value for value in runs) and runs[0].get('result')==runs[1].get('result')
                    if passed:
                        a=(ROOT/runs[0]['directory']/'sequence-pcm.bin').read_bytes();b=(ROOT/runs[1]['directory']/'sequence-pcm.bin').read_bytes();passed=a==b and runs[0]['result']['guarded_sequence_reads']==14
                        if passed:total+=len(a)-14*32;row['exact_destination_bytes']=len(a)-14*32
                    row['status']='passed' if passed else 'failed'
                    if not passed:failures.append(row)
                    unchanged();write(work/'report.json',report);print(json.dumps(dict(fixture=filename,precision=pc,memory=memory,status=row['status'])),flush=True)
        for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
        report.update(status='found_stateful_wma_differences_or_failures' if failures else 'passed_twenty_stateful_wma_pairs_280_guarded_reads_only',failed_cases=len(failures),sequence_pairs=20,guarded_read_pairs=280,exact_destination_bytes=total,production_implementation_changed=False,whole_squall_complete=False,whole_game_complete=False,pending=['General EOF data exhaustion,malformed WMA and COM error/lifetime branches','Full original game and library requirements'])
    except Exception as error:report.update(status='failed',error=repr(error))
    finally:
        unchanged();write(work/'report.json',report);write(args.output,report);print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],failed_cases=report.get('failed_cases'),error=report.get('error'))))
    return int(report['status']!='passed_twenty_stateful_wma_pairs_280_guarded_reads_only')
if __name__=='__main__':raise SystemExit(main())
