#!/usr/bin/env python3
"""Observe actual owned WMA inputs through original and reconstructed system COM paths."""
import argparse,datetime,json,shutil,subprocess
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll');p.add_argument('--output',type=Path,default=ROOT/'.local/squall-wma-validation.json');args=p.parse_args()
    candidate=args.candidate.resolve(strict=True);oracle=ROOT/'build/squall-wma-probe-vs/Release/squall-wma-probe.exe';native=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll';stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-wma'/stamp;work.mkdir(parents=True)
    artifact=work/'build-artifacts';artifact.mkdir()
    for path in [candidate,candidate.with_suffix('.map'),oracle]:shutil.copyfile(path,artifact/path.name)
    lines=(artifact/'Squall.map').read_text(encoding='latin-1').splitlines();matches=[line.split() for line in lines if '?api_SampleGetFile@@' in line];assert len(matches)==1;own=int(matches[0][2],16)-0x10000000
    fixtures=read(ROOT/'.local/squall-wma-owned-fixtures/report.json');paths=[path for path in (ROOT/'src/squall').rglob('*') if path.is_file()]+[ROOT/name for name in ['tools/validate_squall_wma.py','tools/squall_wma_fixtures.py','tools/squall_wma_probe.cpp','tools/squall_wma_probe/CMakeLists.txt']]
    for path in paths:
        target=work/'source-snapshots'/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),candidate_path=candidate.relative_to(ROOT).as_posix(),rebuilt_sha256=digest(candidate),reference_sha256=digest(native),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},build_artifacts={path.name:digest(path) for path in artifact.iterdir()},fixture_manifest=fixtures,cases=[])
    def unchanged():
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
    def run(folder,backend,fixture,mode,start,count,memory,pc):
        folder.mkdir(parents=True);source=ROOT/'.local/squall-wma-owned-fixtures'/fixture;assert digest(source)==fixtures['files'][fixture];shutil.copyfile(source,folder/fixture)
        dll=native if backend=='native' else artifact/'Squall.dll';rva=0x8300 if backend=='native' else own;command=[str(artifact/oracle.name),str(dll),format(rva,'x'),fixture,mode,str(start),str(count),str(memory),*(['pc24'] if pc=='PC24' else [])]
        row=dict(backend=backend,fixture=fixture,mode=mode,start=start,count=count,memory=memory,precision=pc,directory=folder.relative_to(ROOT).as_posix(),command=command)
        try:
            result=subprocess.run(command,cwd=folder,capture_output=True,timeout=30,creationflags=0x08000000);(folder/'stdout.json').write_bytes(result.stdout);(folder/'stderr.log').write_bytes(result.stderr);row['exit_code']=result.returncode
            if result.returncode==0:row['result']=json.loads(result.stdout)
            else:row['error']=result.stderr.decode('utf-8',errors='replace')
        except subprocess.TimeoutExpired as error:row.update(timeout=True,error=str(error));(folder/'timeout.stdout.log').write_bytes(error.stdout or b'');(folder/'timeout.stderr.log').write_bytes(error.stderr or b'')
        row['artifacts']={path.name:digest(path) for path in folder.iterdir() if path.is_file()};return row
    try:
        unchanged();com=subprocess.run([str(artifact/oracle.name),'com'],cwd=work,capture_output=True,timeout=20,creationflags=0x08000000);(work/'com.stdout.json').write_bytes(com.stdout);(work/'com.stderr.log').write_bytes(com.stderr);assert com.returncode==0;report['com']=json.loads(com.stdout);assert report['com']['multimedia_stream_present']
        failures=[];pcm_bytes=0;partial_reads=0;shared_failures=0;guarded_bytes=0
        for index,fixture in enumerate(fixtures['fixtures']):
            filename=fixture['name'];base=[]
            for backend in ['native','source']:base.append(run(work/str(index)/'baseline'/backend,backend,filename,'metadata',0,128,0,'PC53'))
            row=dict(fixture=filename,phase='metadata_baseline',runs=base);report['cases'].append(row)
            if not all(value.get('exit_code')==0 for value in base) or base[0].get('result')!=base[1].get('result'):
                row['status']='failed';failures.append(row);write(work/'report.json',report);continue
            row['status']='passed';frames=base[0]['result']['frames'];assert frames>4097 and base[0]['result']['frequency']==fixture['rate']
            for pc in ['PC53','PC24']:
                for memory in [0,1]:
                    for mode,count in [('mono',128),('stereo',128),('mono',2049),('stereo',4097),('mono',frames),('stereo',frames)]:
                        runs=[];folder=work/str(index)/pc/str(memory)/(mode+'-'+str(count))
                        for backend in ['native','source']:runs.append(run(folder/backend,backend,filename,mode,0,count,memory,pc))
                        row=dict(fixture=filename,phase='pcm',mode=mode,count=count,memory=memory,precision=pc,runs=runs);report['cases'].append(row)
                        passed=all(value.get('exit_code')==0 for value in runs) and runs[0].get('result')==runs[1].get('result')
                        same_unhandled=all(value.get('exit_code')==0xc0000005 for value in runs) and fixture['channels']==2 and mode=='mono' and count>2048
                        if passed:
                            a=(ROOT/runs[0]['directory']/'decoded-pcm.bin').read_bytes();b=(ROOT/runs[1]['directory']/'decoded-pcm.bin').read_bytes();value=runs[0]['result'];declared=count*(2 if mode=='mono' else 4)
                            passed=a==b and value['guards_intact'] and 0<=value['returned_bytes']<=declared and value['changed_bytes']>0
                            if passed:
                                row['exact_equal_returned_pcm_bytes']=value['returned_bytes'];pcm_bytes+=value['returned_bytes'];guarded_bytes+=declared
                                row['matching_partial_output_return']=value['returned_bytes']<declared;partial_reads+=row['matching_partial_output_return']
                        if same_unhandled:row['status']='same_unhandled_failure_recorded_not_pcm_equivalence';shared_failures+=1
                        else:row['status']='passed' if passed else 'failed'
                        if not passed and not same_unhandled:failures.append(row)
                        unchanged();write(work/'report.json',report);print(json.dumps(dict(fixture=filename,precision=pc,memory=memory,mode=mode,count=count,status=row['status'])),flush=True)
        for filename,value in report['source_files'].items():assert digest(ROOT/filename)==value,filename
        report.update(status='found_actual_wma_differences_or_failures' if failures else 'matched_returned_wma_pcm_with_native_unhandled_failures_separately_recorded',failed_cases=len(failures),exact_returned_pcm_bytes=pcm_bytes,matching_partial_output_reads=partial_reads,same_unhandled_failures=shared_failures,exact_guarded_destination_bytes=guarded_bytes,production_implementation_changed=False,original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,pending=['Unhandled failure code matches are not PCM equivalence; representative Temp0 underflow is separately traced','Stateful seek/loop/EOF and actual physical buffer behavior','COM failures and malformed input,lifetime and all remaining original scopes'])
    except Exception as error:report.update(status='failed',error=repr(error))
    finally:
        unchanged();write(work/'report.json',report);write(args.output,report);print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],failed_cases=report.get('failed_cases'),error=report.get('error'))))
    return int(report['status']!='matched_returned_wma_pcm_with_native_unhandled_failures_separately_recorded')
if __name__=='__main__':raise SystemExit(main())
