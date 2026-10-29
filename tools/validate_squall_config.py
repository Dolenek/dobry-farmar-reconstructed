#!/usr/bin/env python3
"""Compare actual global Squall configuration and guarded hardware outputs."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-config-probe/Release/squall-config-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-config-validation.json')
    args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-config'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_config.py','tools/squall_config_reference_test.cpp','tools/squall_config_probe/CMakeLists.txt','tools/squall_exports.inc']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=[])
    try:
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=90)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout);assert row['result_comparisons']==2317 and row['guarded_output_snapshots']==660 and row['export_lookups']==194 and row['actual_directsound']
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_global_configuration_and_hardware_output_scope',result_comparisons=4634,guarded_output_snapshots=1320,reference_files_unchanged=17,
            original_entry_point_executed=True,original_program_code_used_in_implementation=False,implementation_changed=False,
            coverage=['Actual pre-Init, initialized DirectSound and post-Free lifecycle in PC53/PC24',
                'Buffer size signed boundaries,1..5000 acceptance and getter cache;3D algorithm support-table mutation and repeated0/3',
                'Speaker mode signed bounds, hardware acceleration36 raw flag pairs and actual device recreation with no channels',
                'Actual device enumeration; negative/valid/out-of-range device changes and all four scalar configuration getters after every setter',
                'Hardware getter optional null/bad/alias/cross-page output pointers and exact canary-protected output words'],
            pending=['Configuration with prepared/playing 2D and 3D channels, device loss and recreation failures',
                'All device/algorithm capabilities and primary-buffer formats, no-audio-device host, concurrent mutation and memory failures',
                'Remaining complete97-export and full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={path.name:digest(path) for path in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
