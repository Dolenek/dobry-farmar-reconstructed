#!/usr/bin/env python3
"""Compare actual positioned Squall channels, C3 branches and raw signaling-NaN copies."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-channel-vectors-probe/Release/squall-channel-vectors-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-channel-vectors-validation.json')
    p.add_argument('--expect-difference',choices=['c3','signaling-copy'])
    p.add_argument('--fixture-only',action='store_true',help='No audio Init: compare private methods on same-address byte fixtures and recorded COM boundary')
    args=p.parse_args();assert not(args.fixture_only and args.expect_difference)
    candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-channel-vectors'/stamp;work.mkdir(parents=True)
    baseline={'c3':'99f7b608031aea175c7b80cfbc3a87123310f060f15fa830464a3b4607ee0fb8','signaling-copy':'ddead022ee1306cb032e7a80236d86ddc5ab2037be30baea16b114dd15b0dac3'}
    if args.expect_difference:assert digest(candidate)==baseline[args.expect_difference],'Expected preserved own DLL, never an arbitrary or original binary'
    paths=[x for x in (ROOT/'src/squall').rglob('*') if x.is_file()]+[ROOT/n for n in ['tools/squall_channel_vectors_reference_test.cpp','tools/validate_squall_channel_vectors.py','tools/squall_exports.inc','tools/squall_channel_vectors_probe/CMakeLists.txt']]
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),source_files={x.relative_to(ROOT).as_posix():digest(x) for x in sorted(paths)},precision_contexts=[],baseline_difference=args.expect_difference,
        source_manifest_describes='current editable repair and diagnostic sources; preserved historical/intermediate candidate has its recorded binary hash' if args.expect_difference else 'current editable candidate and diagnostic sources')
    report['method_fixture_contexts']=[]
    if not args.expect_difference:
        report['linker_map_path']=candidate.with_suffix('.map').relative_to(ROOT).as_posix()
        report['linker_map_sha256']=digest(candidate.with_suffix('.map'))
    try:
        for precision in ['PC53','PC24']:
            command=[str(oracle),str(original/'Squall.dll'),str(candidate)]+(['pc24'] if precision=='PC24' else [])
            if args.fixture_only:
                result=subprocess.run(command+['fixture'],capture_output=True,timeout=90)
                (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
                assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
                row=json.loads(result.stdout);assert row==dict(setter_fixtures=2048,getter_fixtures=16384,same_object_address=True,com_boundary_recorded=True,actual_directsound=False)
                row['precision']=precision;report['method_fixture_contexts'].append(row);continue
            result=subprocess.run(command,capture_output=True,timeout=90)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            if args.expect_difference:
                text=result.stderr.decode('utf-8',errors='replace');assert result.returncode==1,(precision,result.returncode,text)
                expected=['case=6 operation=2 source=-13 native=1','Actual channel 3D result differs'] if args.expect_difference=='c3' else ['word=1 source=7fc00001 native=7f800001','Cached channel 3D words or guards differ']
                assert all(value in text for value in expected),(precision,expected,text[-2000:])
                report['precision_contexts'].append(dict(precision=precision,expected_difference_observed=True,expected_exit_code=1,diagnostic=expected))
            else:
                assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace')[-3000:])
                row=json.loads(result.stdout)
                for key,value in dict(api_comparisons=13275,snapshots=2636,setter_calls=2418,distance_pairs=722,cone_integer_cases=1024,output_pointer_cases=88,actual_directsound=True,prepared_positioned_channels=2,export_lookups=194,waveform_compared=False).items():assert row[key]==value,(key,row[key])
                row['precision']=precision;report['precision_contexts'].append(row)
                fixture=subprocess.run(command+['fixture'],capture_output=True,timeout=90)
                (work/f'{precision}-fixtures.stdout.log').write_bytes(fixture.stdout);(work/f'{precision}-fixtures.stderr.log').write_bytes(fixture.stderr)
                assert fixture.returncode==0,(precision,fixture.returncode,fixture.stderr.decode('utf-8',errors='replace'))
                row=json.loads(fixture.stdout);assert row==dict(setter_fixtures=2048,getter_fixtures=16384,same_object_address=True,com_boundary_recorded=True,actual_directsound=False)
                row['precision']=precision;report['method_fixture_contexts'].append(row)
            unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='confirmed_preserved_own_dll_difference_in_both_precisions' if args.expect_difference else 'passed_for_actual_positioned_channels_c3_raw_copies_and_selected_bounds_only',reference_files_unchanged=17,whole_game_complete=False,
            original_functions=['10001a70','10001c10','10006570','10006660','10006740','10006830','10006910','100069b0','10006aa0','10006ba0'],
            coverage=['Actual mono16kHz PCM callbacks, real DirectSound3D buffers and prepared channels with algorithm1; immediate and deferred listeners',
                '108 binary32 triples per context across Position/Velocity/Cone setters; each-axis repeated signed quiet/signaling NaN payloads and raw getter snapshots',
                '19x19 distance pairs per context,512 cone integer boundary tuples per context and guarded input/output pages',
                'Cone setter requires writable input as in original; cone OutsideVolume kept writable because original success path writes it unconditionally'],
            pending=['Algorithm3 buffers rejected by both DLLs on this host; exact backend HRESULT still unobserved','Full audio spatial waveform, running-channel timing, EAX/ZoomFX, device loss and callbacks',
                'Invalid cone OutsideVolume successful-getter faults, arbitrary internal pointer overlaps, all float patterns and FP status/exception masks','All remaining97-export and whole-game scopes'])
        if args.fixture_only:report['status']='passed_for_equal_address_3d_method_fixtures_only'
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={p.name:digest(p) for p in work.glob('*.log')}
        encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
