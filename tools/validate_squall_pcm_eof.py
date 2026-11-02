#!/usr/bin/env python3
"""Compare native address/argument-dependent PCM EOF bytes at identical decoder addresses."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-pcm-eof-probe-vs/Release/squall-pcm-eof-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-pcm-eof-validation.json')
    p.add_argument('--diagnose',action='store_true');args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-pcm-eof'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_pcm_eof.py','tools/squall_pcm_eof_reference_test.cpp','tools/squall_pcm_eof_probe/CMakeLists.txt','tools/squall_exports.inc','tools/squall_wave_fixtures.py']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=[])
    try:
        from squall_wave_fixtures import generate
        fixtures=generate(work);report['fixtures']=fixtures
        report['fixture_files']={row['name']:digest(work/row['name']) for row in fixtures};report['fixture_files']['files.txt']=digest(work/'files.txt')
        source_map=candidate.with_suffix('.map');lines=source_map.read_text(encoding='latin-1').splitlines()
        def symbol(part):
            matches=[line.split() for line in lines if part in line];assert len(matches)==1
            return int(matches[0][2],16)-0x10000000
        symbols=[symbol('?api_SampleGetFile@@')]
        report['source_map']=dict(path=source_map.relative_to(ROOT).as_posix(),sha256=digest(source_map),symbol_rvas=symbols)
        import os
        environment=os.environ.copy();environment.pop('FARMAR_PCM_EOF_DIAGNOSE',None)
        if args.diagnose:environment['FARMAR_PCM_EOF_DIAGNOSE']='1'
        report['diagnose']=args.diagnose
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*[format(x,'x') for x in symbols],*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=90,cwd=work,env=environment)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout)
            assert len(row['controlled_addresses'])>=2 and row['cases']==len(row['controlled_addresses'])*36 and row['native_expected_model_verified'] and row['export_lookups']==194
            if args.diagnose:assert row['source_mismatches']>0
            else:assert row['source_mismatches']==0
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='observed_old_source_pcm_eof_mismatches_at_same_addresses' if args.diagnose else 'passed_native_pcm_eof_saved_this_and_start_argument_bytes_at_same_addresses_only',
            same_address_cases=sum(row['cases'] for row in report['precision_contexts']),
            source_mismatches=sum(row['source_mismatches'] for row in report['precision_contexts']),
            used_inputs=['pcm8-1.wav','pcm16-1.wav','pcm16-2.wav'],reference_files_unchanged=17,
            original_entry_point_executed=True,original_program_code_used_in_implementation=False,
            coverage=['Native PCM8 mono-to-stereo initial byte is high byte of saved this via PUSH ECX',
                'Native PCM16 mono-to-stereo initial word is high word of saved this via PUSH ECX',
                'Native PCM16 stereo-to-mono reuses start argument DWORD as initial two signed words',
                'At least two identical controlled decoder addresses,Memory0/1,EOF/EOF+1/UINT_MAX andloop0/1; complete guarded raw output and native model'],
            pending=['PCM8 stereo-to-mono genuinely uninitialized stack words at EOF','Remaining WAV and full original goal scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={path.name:digest(path) for path in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
