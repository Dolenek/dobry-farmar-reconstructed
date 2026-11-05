#!/usr/bin/env python3
"""Compare native stored pointers and actual file callback reader ownership."""
import argparse,datetime,json,struct,subprocess,traceback,shutil
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/squall-file-callbacks-probe-vs/Release/squall-file-callbacks-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-file-callbacks-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=read(ROOT/'analysis/inventory.json')
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-file-callbacks'/stamp;work.mkdir(parents=True)
    lines=(candidate.parent/'Squall.map').read_text(encoding='latin1').splitlines()
    def symbol(prefix):
        found=[line.split() for line in lines if len(line.split())>=3 and line.split()[1].startswith(prefix)]
        assert len(found)==1,(prefix,len(found))
        preferred=[line for line in lines if 'Preferred load address is' in line];assert len(preferred)==1
        return int(found[0][2],16)-int(preferred[0].split()[-1],16)
    symbols=[symbol('?api@@3Usquall_api_s@@A'),symbol('?api_SampleGetFile@@'),symbol('?GetMonoData@CSoundFile@@')]
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_file_callbacks.py','tools/squall_file_callbacks_reference_test.cpp','tools/squall_file_callbacks_probe/CMakeLists.txt','tools/squall_exports.inc']]
    for name in ['tools/validate_squall_file_callbacks.py','tools/squall_file_callbacks_reference_test.cpp']:
        shutil.copyfile(ROOT/name,work/Path(name).name)
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=[],
        original_addresses=dict(export='10003c10',store='100085a0',callbacks=['10075990','10075994','10075998','1007599c'],sample_file='10008300',mono_read='1002e020'),
        source_symbols_rva=[hex(n) for n in symbols],map_sha256=digest(candidate.parent/'Squall.map'))
    try:
        pcm=b''.join(struct.pack('<h',((n*733)%20001)-10000) for n in range(64))
        wav=struct.pack('<4sI4s4sIHHIIHH4sI',b'RIFF',36+len(pcm),b'WAVE',b'fmt ',16,1,1,8000,16000,2,16,b'data',len(pcm))+pcm
        for name in ['sample.wav','sample2.wav']:(work/name).write_bytes(wav)
        report['sample_files']={name:digest(work/name) for name in ['sample.wav','sample2.wav']}
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*[f'{n:x}' for n in symbols],*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=180,cwd=work)
            stdout=work/f'{precision}.stdout.json';stderr=work/f'{precision}.stderr.log';stdout.write_bytes(result.stdout);stderr.write_bytes(result.stderr)
            report.setdefault('processes',[]).append(dict(precision=precision,exit_code=result.returncode,stdout_sha256=digest(stdout),stderr_sha256=digest(stderr)))
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout);events=row.pop('callback_events');assert len(events)==row['callback_events_per_dll']>0
            assert row['pointer_combinations']==12288 and row['exact_pointer_stores']==49152
            assert row['pointer_fpu_checks']==12288 and row['fpu_observations']
            assert all(stage['precision']==(65536 if precision=='PC53' else 131072) for stage in row['fpu_observations'])
            assert row['actual_pcm_reads']==15360 and row['guarded_pcm_bytes']==841728
            assert row['loaded_samples_per_dll']==591 and row['forced_failures_per_dll']==9 and row['lifecycle_phases']==3
            assert row['result_comparisons']>row['pointer_combinations'] and row['callback_event_snapshots']>row['actual_pcm_reads']
            row.update(precision=precision,raw_callback_event_file=stdout.name,raw_callback_event_file_sha256=digest(stdout));report['precision_contexts'].append(row)
            unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_actual_file_callbacks_pointer_storage_and_reader_ownership_scope',
            pointer_combinations=sum(row['pointer_combinations'] for row in report['precision_contexts']),
            actual_pcm_reads=sum(row['actual_pcm_reads'] for row in report['precision_contexts']),
            guarded_pcm_bytes=sum(row['guarded_pcm_bytes'] for row in report['precision_contexts']),
            original_files_unchanged=17,implementation_changed=False,original_program_code_used_in_implementation=False,
            coverage=['4096 pointer combinations per lifecycle: null,A,B,1,-1,readable,noaccess,executable pages; exact stored pointers even on error; invalid installed callbacks never invoked',
                'Before Init,actual DirectSound initialized,and after Free; both x87 precisions',
                'All16 independent A/B callback generations,streaming and in-memory flags0/1/-1/2,duplicate reader retained after changing global callbacks',
                'Actual callback-backed PCM decoding,complete guarded destinations,handle identity/lifecycle and callback arguments/data',
                'Controlled open/seek/read initialization failures with actual cleanup and successful retry; UnloadAll with two retained callback generations'],
            pending=['Invoked invalid callbacks and concurrent setter mutation remain unverified','Remaining full97-export behavior,codec and whole-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['whole_game_complete']=False;report['whole_squall_complete']=False
        write(work/'report.json',report);write(args.output,report)
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    if report['status']=='failed':raise SystemExit(1)

if __name__=='__main__':main()
