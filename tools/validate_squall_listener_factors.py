#!/usr/bin/env python3
"""Verify original ordered factor predicates, actual listener state and before/after NaNs."""
import argparse,datetime,hashlib,json,os,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/squall-listener-factors-reference-test.exe')
    parser.add_argument('--before-candidate',type=Path,default=ROOT/'.local/empty-seed-interop/20261029T131211877257Z/runtime/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-listener-factors-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True);before=args.before_candidate.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();baseline=json.loads((ROOT/'analysis/squall-listener-before-validation.json').read_text(encoding='utf-8'));assert digest(before)==baseline['candidate_sha256']
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-listener-factors'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/n for n in ['tools/squall_listener_factors_reference_test.cpp','tools/validate_squall_listener_factors.py','tools/squall_exports.inc','tools/cmake/SquallCandidate.cmake','tools/squall_listener_probe/CMakeLists.txt']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),before_candidate_sha256=digest(before),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),before_candidate_path=before.relative_to(ROOT).as_posix(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted(paths)},precision_contexts=[],before_contexts=[])
    env=os.environ.copy();env.pop('FARMAR_LISTENER_DIAGNOSE_PREINIT',None)
    try:
        for precision in ['PC53','PC24']:
            extra=['pc24'] if precision=='PC24' else []
            before_env=env.copy();before_env['FARMAR_LISTENER_DIAGNOSE_PREINIT']='1'
            old=subprocess.run([str(oracle),str(original/'Squall.dll'),str(before),*extra],capture_output=True,timeout=15,env=before_env)
            (work/f'{precision}-before.stdout.log').write_bytes(old.stdout);(work/f'{precision}-before.stderr.log').write_bytes(old.stderr)
            old_row=json.loads(old.stdout);assert old.returncode==1 and old_row==dict(preinit_inputs=3120,nan_inputs=3072,mismatches=3072,setter_count=3)
            old_row['precision']=precision;report['before_contexts'].append(old_row)
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*extra],capture_output=True,timeout=30,env=env)
            (work/f'{precision}-after.stdout.log').write_bytes(result.stdout);(work/f'{precision}-after.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout);assert row['result_comparisons']==12481 and row['factor_inputs']==9360 and row['nan_inputs']==9216
            assert row['initialized_getter_checks']==3120 and row['factor_setters']==3 and row['actual_directsound'] and row['export_lookups']==194
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_ordered_listener_factor_predicates_and_actual_cached_values_only',
            result_comparisons=24962,factor_inputs=18720,nan_inputs=18432,initialized_getter_checks=6240,before_nan_mismatches=6144,
            original_entry_point_executed=True,original_program_code_used_in_implementation=False,reference_files_unchanged=17,
            original_functions=['10004600','10004750','100048a0','100046c0','10004810','10004960'],
            coverage=['Distance, rolloff and doppler setters before Init, with an actual DirectSound listener and after Free',
                '1040 binary32 patterns per setter/lifecycle: signed zero/subnormal/normal, near10 boundary, large finite and infinities,1024 signed quiet/signaling NaN payloads',
                'All three original predicates require ordered0<value<=10; NaNs return-4 before any cached listener mutation',
                '6240 successful actual initialized getters with exact cached factor bits and output guards',
                'The previous source DLL from commit38bedf7 reproduces6144 NaN discrepancies with this same current harness; it is used only for diagnosis'],
            pending=['All2^32 float bit patterns, FP status/exception masks and arbitrary output pointers',
                'Other listener3D vectors, callback/concurrency/device paths and remaining97-export/full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={p.name:digest(p) for p in work.glob('*.log')}
        encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
