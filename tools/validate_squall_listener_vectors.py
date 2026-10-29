#!/usr/bin/env python3
"""Compare loaded DLL vector operations on actual immediate/deferred listeners."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/squall-vectors-probe/Release/squall-listener-vectors-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-listener-vectors-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-listener-vectors'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/n for n in ['tools/squall_listener_vectors_reference_test.cpp','tools/validate_squall_listener_vectors.py','tools/squall_exports.inc','tools/squall_vectors_probe/CMakeLists.txt']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted(paths)},precision_contexts=[])
    try:
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=40)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout)
            for name,value in dict(result_comparisons=2278,snapshots=888,setter_calls=840,getter_pointer_cases=504,actual_directsound=True,contexts=4,export_lookups=194).items():assert row[name]==value,(name,row[name])
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_listener_vectors_guarded_pointers_and_commit_only',result_comparisons=4556,snapshots=1776,setter_calls=1680,getter_pointer_cases=1008,
            original_entry_point_executed=True,original_program_code_used_in_implementation=False,reference_files_unchanged=17,
            original_functions=['10003f80','100041a0','100042d0','100043d0','10004470','10004560','100049f0'],
            coverage=['Actual DLL exports before Init, actual immediate listener, actual deferred listener and after Free',
                '11 binary32 triples: signed zeros, finite/repeated values, subnormals, infinities, quiet/signaling NaN and orientation bases',
                'All16 Position/Front/Top/Velocity null masks plus single Position/Velocity setters; cached exact words and guard arrays after every mutation and commit',
                'Null, address1, read-only and no-access-crossing input pages;21 output arrangements include all16 presence masks, unreadable/read-only/guard and full/partial output aliasing',
                'Return codes and complete cached outputs compared, including state retained following rejected DirectSound updates; no COM mocks'],
            pending=['All float patterns and pointer overlaps with internal state, floating-point status/exceptions, audio spatial waveform',
                'Workers/callback races, device loss, hardware EAX/ZoomFX, all remaining97-export and full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={p.name:digest(p) for p in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
