#!/usr/bin/env python3
"""Compare actual EAX/ZOOMFX unsupported-host errors, parameter order and guarded output."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-extensions-probe-vs/Release/squall-extensions-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-extensions-validation.json')
    args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-extensions'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_extensions.py','tools/squall_extensions_reference_test.cpp','tools/squall_extensions_probe/CMakeLists.txt','tools/squall_exports.inc']]
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
            assert row['result_comparisons']==3346 and row['guarded_property_snapshots']==3180 and row['export_lookups']==194 and row['actual_directsound'] and row['actual_eax_version']==0 and not row['successful_hardware_extension_paths_verified']
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_seven_eax_zoomfx_exports_unsupported_host_scope_only',
            result_comparisons=sum(row['result_comparisons'] for row in report['precision_contexts']),
            guarded_property_snapshots=sum(row['guarded_property_snapshots'] for row in report['precision_contexts']),
            reference_files_unchanged=17,original_entry_point_executed=True,original_program_code_used_in_implementation=False,implementation_changed=False,
            coverage=['All seven actual EAX/ZOOMFX listener/channel APIs with native actual EAX version0',
                'Five phases: pre-Init,initialized empty,actual prepared2D,actual prepared3D,post-Free',
                'Eight signed version boundaries,32 presets,five channel IDs and six writable/null/bad/read-only/cross-page/noaccess pointers',
                'Exact canary-protected caller memory,full result histograms and actual error-priority order'],
            pending=['Successful EAX/ZOOMFX hardware property paths,presets and supported-version branches',
                'Fake-COM boundary reference audit of unexercised hardware paths',
                'Remaining whole97-export ABI/behavior and full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={path.name:digest(path) for path in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
