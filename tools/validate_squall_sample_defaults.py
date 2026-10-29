#!/usr/bin/env python3
"""Actual loaded sample default normalization with original before/after NaNs."""
import argparse,datetime,hashlib,json,os,subprocess,sys,traceback,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/squall-sample-probe/Release/squall-sample-defaults-reference-test.exe')
    parser.add_argument('--before-candidate',type=Path,default=ROOT/'.local/cucumber-sowing/20261029T180721717655Z/runtime/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-sample-defaults-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True);before=args.before_candidate.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();baseline=json.loads((ROOT/'analysis/squall-sample-defaults-before-validation.json').read_text(encoding='utf-8'));assert digest(before)==baseline['candidate_sha256']
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-sample-defaults'/stamp;work.mkdir(parents=True)
    with zipfile.ZipFile(original/'Data/Data.pack') as archive:(work/'sample.ogg').write_bytes(archive.read('Sounds/Menu_music.ogg'))
    assert digest(work/'sample.ogg')==baseline['sample_sha256']
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/n for n in ['tools/squall_sample_defaults_reference_test.cpp','tools/validate_squall_sample_defaults.py','tools/squall_sample_probe/CMakeLists.txt','tools/squall_exports.inc']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),before_candidate_sha256=digest(before),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),before_candidate_path=before.relative_to(ROOT).as_posix(),
        sample_member='Sounds/Menu_music.ogg',sample_sha256=digest(work/'sample.ogg'),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted(paths)},precision_contexts=[],before_contexts=[])
    env=os.environ.copy();env.pop('FARMAR_SAMPLE_DEFAULTS_DIAGNOSE',None)
    try:
        for precision in ['PC53','PC24']:
            extra=['pc24'] if precision=='PC24' else [];before_env=env.copy();before_env['FARMAR_SAMPLE_DEFAULTS_DIAGNOSE']='1'
            old=subprocess.run([str(oracle),str(original/'Squall.dll'),str(before),*extra],cwd=work,capture_output=True,timeout=30,env=before_env)
            (work/f'{precision}-before.stdout.log').write_bytes(old.stdout);(work/f'{precision}-before.stderr.log').write_bytes(old.stderr)
            old_row=json.loads(old.stdout);assert old.returncode==1 and old_row['cases']==2130 and old_row['normalization_mismatches']==2048
            old_row['precision']=precision;report['before_contexts'].append(old_row)
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*extra],cwd=work,capture_output=True,timeout=30,env=env)
            (work/f'{precision}-after.stdout.log').write_bytes(result.stdout);(work/f'{precision}-after.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout)
            for name,value in dict(cases=2130,result_comparisons=4355,pointer_cases=50,explicit_load_default_cases=22,normalization_mismatches=0,initialized_directsound=False,export_lookups=194).items():assert row[name]==value,(name,row[name])
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_sample_default_normalization_and_guarded_pointers_only',result_comparisons=8710,normalization_cases=4260,pointer_cases=100,explicit_load_default_cases=44,before_nan_mismatches=4096,
            original_entry_point_executed=True,original_program_code_used_in_implementation=False,reference_files_unchanged=17,original_functions=['100074a0','100074e0','100083c0','10008410','10007f20'],
            coverage=['Actual pre-Init loading of the original game menu Ogg, SetDefault/GetDefault through independently loaded DLLs',
                'All seven32-bit fields, signed integer clamp boundaries, default frequency0, complete28-byte cached record plus input/output guards',
                '1040 binary32 patterns for each distance field, including1024 signed quiet/signaling NaNs; original unordered/nonpositive values become1 and1000000000',
                'Valid and selected invalid sample IDs across valid/null/address1/read-only/cross-page pointers; original setter checks writeability even though it reads input',
                'Same current harness reproduces4096 NaN discrepancies in the preserved own DLL of commit8599644'],
            pending=['All sample lifecycle/group/playback paths and arbitrary internal overlaps',
                'All2^32 floats, FP status/exception masks, other codecs and remaining full97-export/full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={p.name:digest(p) for p in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
