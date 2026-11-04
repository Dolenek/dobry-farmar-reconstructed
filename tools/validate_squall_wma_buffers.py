#!/usr/bin/env python3
"""Compare real prepared WMA buffers and independently observe playback/pause."""
import argparse,datetime,json,shutil,subprocess
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll');p.add_argument('--buffer-ms',type=int,default=10);p.add_argument('--fixture-index',type=int);p.add_argument('--output',type=Path,default=ROOT/'.local/squall-wma-buffers-validation.json');args=p.parse_args()
    candidate=args.candidate.resolve(strict=True);oracle=ROOT/'build/squall-wma-buffers-probe-vs/Release/squall-wma-buffers-probe.exe';native=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll';work=ROOT/'.local/squall-wma-buffers'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True);artifact=work/'build-artifacts';artifact.mkdir()
    for path in [candidate,candidate.with_suffix('.map'),oracle]:shutil.copyfile(path,artifact/path.name)
    lines=(artifact/'Squall.map').read_text(encoding='latin-1').splitlines();matches=[line.split() for line in lines if '?api@@3Usquall_api_s@@A' in line];assert len(matches)==1;own=int(matches[0][2],16)-0x10000000+12
    fixtures=read(ROOT/'.local/squall-wma-owned-fixtures/report.json');paths=[path for path in (ROOT/'src/squall').rglob('*') if path.is_file()]+[ROOT/name for name in ['tools/validate_squall_wma_buffers.py','tools/squall_wma_fixtures.py','tools/squall_wma_buffers_probe.cpp','tools/squall_wma_buffers_probe/CMakeLists.txt']]
    for path in paths:
        target=work/'source-snapshots'/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),candidate_path=candidate.relative_to(ROOT).as_posix(),rebuilt_sha256=digest(candidate),reference_sha256=digest(native),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},build_artifacts={path.name:digest(path) for path in artifact.iterdir()},fixture_manifest=fixtures,cases=[])
    def unchanged():
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
    try:
        failures=[];physical=0;playback_progress=0;nonprogress=[];selected=[(index,fixture) for index,fixture in enumerate(fixtures['fixtures']) if args.fixture_index is None or args.fixture_index==index];assert selected and 0<args.buffer_ms<=5000
        for index,fixture in selected:
            filename=fixture['name']
            for pc in ['PC53','PC24']:
                for memory in [0,1]:
                    for spatial in [0,1]:
                        row=dict(fixture=filename,precision=pc,memory=memory,spatial=spatial,buffer_milliseconds=args.buffer_ms,runs=[]);report['cases'].append(row)
                        for backend,dll,rva in [('native',native,0x75034),('source',artifact/'Squall.dll',own)]:
                            folder=work/str(index)/pc/str(memory)/str(spatial)/backend;folder.mkdir(parents=True);source=ROOT/'.local/squall-wma-owned-fixtures'/filename;assert digest(source)==fixtures['files'][filename];shutil.copyfile(source,folder/filename)
                            command=[str(artifact/oracle.name),str(dll),format(rva,'x'),filename,str(spatial),str(memory),str(args.buffer_ms),*(['pc24'] if pc=='PC24' else [])]
                            run=dict(backend=backend,directory=folder.relative_to(ROOT).as_posix(),command=command);row['runs'].append(run)
                            try:
                                result=subprocess.run(command,cwd=folder,capture_output=True,timeout=30,creationflags=0x08000000);(folder/'stdout.json').write_bytes(result.stdout);(folder/'stderr.log').write_bytes(result.stderr);run['exit_code']=result.returncode
                                if result.returncode==0:
                                    try:run['result']=json.loads(result.stdout)
                                    except json.JSONDecodeError as error:run['error']='Incomplete diagnostic stdout: '+str(error)
                                else:run['error']=result.stderr.decode('utf-8',errors='replace')
                            except subprocess.TimeoutExpired as error:run.update(timeout=True,error=str(error));(folder/'timeout.stdout.log').write_bytes(error.stdout or b'');(folder/'timeout.stderr.log').write_bytes(error.stderr or b'')
                            run['artifacts']={path.name:digest(path) for path in folder.iterdir() if path.is_file()}
                        runs=row['runs'];passed=all(value.get('exit_code')==0 and 'result' in value for value in runs)
                        if passed:
                            a,b=[value['result'] for value in runs];passed={k:v for k,v in a.items() if k not in ['raw_cursors','raw_logical_positions','actual_playback_progress','directsound_playing_flag','raw_channel_status']}=={k:v for k,v in b.items() if k not in ['raw_cursors','raw_logical_positions','actual_playback_progress','directsound_playing_flag','raw_channel_status']} and a['channel']>=0 and a['frequency']==fixture['rate'] and a['file_exclusive_after_unload'] and a['actual_pause_stable']
                            if passed:
                                x=(ROOT/runs[0]['directory']/'physical-pcm.bin').read_bytes();y=(ROOT/runs[1]['directory']/'physical-pcm.bin').read_bytes();passed=x==y and len(x)==a['physical_bytes'] and a['nonzero_bytes']>0
                                if passed:
                                    physical+=len(x);row['exact_physical_bytes']=len(x)
                                    for run in runs:
                                        playback_progress+=bool(run['result']['actual_playback_progress'])
                                        if not run['result']['actual_playback_progress']:nonprogress.append(dict(fixture=filename,precision=pc,memory=memory,spatial=spatial,backend=run['backend'],result=run['result']))
                        row['status']='passed' if passed else 'failed'
                        if not passed:failures.append(row)
                        unchanged();write(work/'report.json',report);print(json.dumps(dict(fixture=filename,precision=pc,memory=memory,spatial=spatial,status=row['status'])),flush=True)
        for filename,value in report['source_files'].items():assert digest(ROOT/filename)==value,filename
        report.update(status='found_actual_wma_buffer_or_playback_differences' if failures else 'matched_prepared_wma_buffer_pairs_playback_nonprogress_separately_recorded',failed_cases=len(failures),exact_physical_pcm_bytes=physical,actual_buffer_pairs=len(selected)*8,actual_playback_processes=len(selected)*16,buffer_milliseconds=args.buffer_ms,actual_playback_progress_observations=playback_progress,nonprogress_observations=nonprogress,actual_pause_observations=len(selected)*16,raw_cursors_compared_or_normalized=False,production_implementation_changed=False,original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,pending=['Investigate every short-buffer nonprogress observation; it is not treated as verified playback','Default200ms and other fragment sizes,long playback/EOF/refill failures','General WMA seek,loop,COM errors and lifetime','Whole original game/DLL/uninstaller requirements'])
    except Exception as error:report.update(status='failed',error=repr(error))
    finally:
        unchanged();write(work/'report.json',report);write(args.output,report);print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],failed_cases=report.get('failed_cases'),error=report.get('error'))))
    return int(report['status']!='matched_prepared_wma_buffer_pairs_playback_nonprogress_separately_recorded')
if __name__=='__main__':raise SystemExit(main())
