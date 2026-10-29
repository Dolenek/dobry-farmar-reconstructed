#!/usr/bin/env python3
"""Compare actual listener callbacks and unsigned period semantics, without mocks."""
import argparse,datetime,hashlib,json,os,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/squall-worker-probe/Release/squall-listener-worker-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-listener-worker-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-listener-worker'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/n for n in ['tools/squall_listener_worker_reference_test.cpp','tools/validate_squall_listener_worker.py','tools/squall_worker_probe/CMakeLists.txt','tools/squall_exports.inc']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted(paths)},precision_contexts=[])
    env=os.environ.copy();env.pop('FARMAR_LISTENER_WORKER_DIAGNOSE',None)
    try:
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=30,env=env)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout);assert row['result_comparisons']==76 and row['negative_period_observations']==6 and row['export_lookups']==194 and row['actual_directsound']
            assert row['native_callbacks']>=2 and row['source_callbacks']>=2
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_listener_worker_callback_userdata_and_unsigned_period_scope',result_comparisons=152,high_bit_unsigned_period_observations=12,
            actual_callbacks=sum(row['native_callbacks']+row['source_callbacks'] for row in report['precision_contexts']),original_entry_point_executed=True,original_program_code_used_in_implementation=False,reference_files_unchanged=17,
            original_functions=['10004cf0'],implementation_changed=False,
            coverage=['Actual public API before Init, with real DirectSound worker thread and after Free, PC53/PC24',
                'Null worker clears regardless of userdata/interval; invalid code pointer, zero,1,10,100,INT_MAX and high-bit unsigned patterns',
                'Actual successful callbacks pass the original userdata and continue using it after both bad-code and zero-period setter failures',
                'Clearing waits through the real context mutex and no further callback occurs during35ms observation',
                'DWORD intervalsFFFFFFFF/FFFFFFFE/80000000 are accepted and do not fire during35ms observations',
                'Original TEST EAX,EAX/JA requires unsigned nonzero. Existing public source UpdateTime is unsigned, so its<=0 predicate already matches; no speculative repair'],
            pending=['Complete timer wraparound and long unsigned intervals, all callback reentrancy/concurrency and destruction races',
                'Arbitrary executable pointer ranges, listener state/device loss and remaining full97-export/full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={p.name:digest(p) for p in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
