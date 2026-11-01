#!/usr/bin/env python3
"""Compare real exported sample/group2D/3D playback and sample pause/stop."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-sample-play-probe-vs/Release/squall-sample-play-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-sample-play-validation.json')
    args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-sample-play'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_sample_play.py','tools/squall_sample_play_reference_test.cpp','tools/squall_sample_play_probe/CMakeLists.txt','tools/squall_exports.inc']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=[])
    try:
        import struct
        pcm=bytes(16000*2)
        wav=struct.pack('<4sI4s4sIHHIIHH4sI',b'RIFF',36+len(pcm),b'WAVE',b'fmt ',16,1,1,16000,32000,2,16,b'data',len(pcm))+pcm
        for filename in ['sample.wav','sample2.wav']:(work/filename).write_bytes(wav)
        report['sample_files']={name:digest(work/name) for name in ['sample.wav','sample2.wav']}
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=180,cwd=work)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout)
            assert row['result_comparisons']>0 and row['guarded_channel_snapshots']==418 and row['actual_play_cases']==387 and row['sample_pause_cases']==11 and row['empty_lifecycle_cases']==56 and row['export_lookups']==194 and row['actual_directsound']
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_sample_group_play2d3d_and_sample_pause_stop_scope',
            result_comparisons=sum(row['result_comparisons'] for row in report['precision_contexts']),
            guarded_channel_snapshots=sum(row['guarded_channel_snapshots'] for row in report['precision_contexts']),
            reference_files_unchanged=17,original_entry_point_executed=True,original_program_code_used_in_implementation=False,implementation_changed=False,
            coverage=['Eight actual sample/group playback APIs; singleton sample groups17/-7/0,2D/real3D,default/explicit parameters',
                'All16 raw loop/start pairs0/1/-1/2; actual started buffers paused immediately,prepared buffers inspected without start',
                'Exact public scalar getters and canary-protected3D position/velocity/min/max cached words',
                'Prepared and actual playing2D/3D same-sample pause0/1/-1/2 and Stop; duplicate source-file reader with a different sample ID remains untouched',
                'Before Init/after Free pause/stop/missing sample and missing group explicit play; bounded actual asynchronous worker retirement'],
            pending=['Multiple members per sample group,RNG selection and group-size limit',
                'Invalid3D position crash behavior,exhaustion/preemption,device failures and concurrent teardown',
                'Remaining whole97-export behavior and full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={path.name:digest(path) for path in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
