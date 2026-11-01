#!/usr/bin/env python3
"""Compare original and source extension wrappers at an isolated diagnostic COM boundary."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-extension-boundary-probe-vs/Release/squall-extension-boundary-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-extension-boundary-validation.json')
    args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-extension-boundary'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_extension_boundary.py','tools/squall_extension_boundary_reference_test.cpp','tools/squall_extension_boundary_probe/CMakeLists.txt','tools/squall_exports.inc']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=[])
    try:
        source_map=candidate.with_suffix('.map');lines=source_map.read_text(encoding='latin-1').splitlines()
        matches=[line.split() for line in lines if '?api@@3Usquall_api_s@@A' in line];assert len(matches)==1
        source_context_rva=int(matches[0][2],16)-0x10000000+12
        report['source_map']=dict(path=source_map.relative_to(ROOT).as_posix(),sha256=digest(source_map),source_context_rva=source_context_rva)
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),format(source_context_rva,'x'),*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=180,cwd=work)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout)
            assert row['fixture_cases']==row['result_comparisons']==row['cached_and_caller_snapshots']==row['com_event_snapshots']==3900
            assert row['preset_cases']==960 and row['method_cases']==[120,120,1200,1200,150,150,960] and row['export_lookups']==194 and row['private_context_fixture_only'] and not row['actual_hardware_extension_compared']
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_eax_zoomfx_private_context_and_com_boundary_scope_only',
            result_comparisons=sum(row['result_comparisons'] for row in report['precision_contexts']),
            cached_and_caller_snapshots=sum(row['cached_and_caller_snapshots'] for row in report['precision_contexts']),
            reference_files_unchanged=17,original_entry_point_executed=True,original_program_code_used_in_implementation=False,implementation_changed=False,
            coverage=['Actual seven exported wrappers,original private context RVA75034 and source map-derived private context',
                'Diagnostic context/channel offsets statically tied to original disassembly; owned context bytes restored before DLL cleanup',
                'Supported/unsupported/negative capability flags,two channel IDs/invalid IDs,positioned/ambient and eight version boundaries',
                'COM GUID/property/instance/size/payload event identity,success/set failure/get failure/short write+reported0/null interface',
                'All32 preset boundaries for useEax0/1/2/3/4/-1,exact static preset payloads and full caller/cached union words'],
            pending=['Actual supported hardware behavior and driver-specific constraints',
                'Guarded pointers on supported fixtures,aliasing,concurrent teardown and additional COM failure modes',
                'Remaining whole97-export ABI/behavior and full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={path.name:digest(path) for path in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
