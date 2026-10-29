#!/usr/bin/env python3
"""Compare real DLL channel ranges, guarded outputs and pointer-seeded native position."""
import argparse,datetime,hashlib,json,os,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/squall-channel-ranges-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-channel-ranges-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-channel-ranges'/stamp;work.mkdir(parents=True)
    paths=[p for folder in ['src/squall','src/game'] for p in (ROOT/folder).rglob('*') if p.is_file()]+[ROOT/n for n in ['tools/squall_channel_ranges_reference_test.cpp','tools/validate_squall_channel_ranges.py','tools/squall_exports.inc','tools/cmake/SquallCandidate.cmake']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted(paths)},precision_contexts=[],bounded_nonreturn_experiments=[])
    env=os.environ.copy()
    for name in ['FARMAR_RANGE_PAST_END','FARMAR_RANGE_NEGATIVE_MS','FARMAR_RANGE_SOURCE_FIRST']:env.pop(name,None)
    try:
        for precision in ['PC53','PC24']:
            command=[str(oracle),str(original/'Squall.dll'),str(candidate),str(original/'Data')]+(['pc24'] if precision=='PC24' else [])
            completed=subprocess.run(command,capture_output=True,timeout=30,env=env)
            (work/f'{precision}-stdout.log').write_bytes(completed.stdout);(work/f'{precision}-stderr.log').write_bytes(completed.stderr)
            assert completed.returncode==0,(precision,completed.returncode,completed.stderr.decode('utf-8',errors='replace'))
            row=json.loads(completed.stdout);assert row['api_comparisons']==1271 and row['equal_address_position_cases']==240
            assert row['output_pointer_cases']==638 and row['range_setter_cases']==24 and row['seek_cases']==14
            assert row['export_lookups']==194 and row['real_directsound'] and not row['audible_output_compared']
            row['precision']=precision;report['precision_contexts'].append(row)
            for scenario,flag,marker in [('frame-past-end','FARMAR_RANGE_PAST_END','frame setter'),('negative-ms','FARMAR_RANGE_NEGATIVE_MS','ms setter')]:
                for side in ['Native','Source']:
                    trial_env=env.copy();trial_env[flag]='1'
                    if side=='Source':trial_env['FARMAR_RANGE_SOURCE_FIRST']='1'
                    filename=f'{precision}-{scenario}-{side}.log';timed_out=False
                    with (work/filename).open('wb') as log:
                        try:subprocess.run(command,stdout=log,stderr=log,env=trial_env,timeout=4,check=True)
                        except subprocess.TimeoutExpired:timed_out=True
                    lines=(work/filename).read_text(encoding='utf-8').splitlines()
                    assert timed_out and lines[-1]==f'{side} {marker}',(precision,scenario,side,timed_out,lines[-3:])
                    report['bounded_nonreturn_experiments'].append(dict(precision=precision,scenario=scenario,side=side,deadline_seconds=4,returned_before_deadline=False,last_stage=lines[-1],log=filename))
            unchanged();assert digest(candidate)==report['rebuilt_sha256'] and digest(oracle)==report['oracle_sha256']
        report.update(status='passed_for_channel_ranges_guarded_outputs_and_pointer_position_scope',result_comparisons=2542,public_api_result_comparisons=1582,equal_address_method_result_comparisons=960,
            original_entry_point_executed=True,original_program_code_used_in_implementation=False,reference_files_unchanged=17,
            original_functions=['10002180','100020d0','10002300','10005840','100058d0','10005970','10005a30','10005c30','10005cf0','10005dd0','10005ef0','10006170','10006210','10006280','10006350'],
            coverage=['Actual prepared ambient channel fragments, clamping, swapped bounds, invalid/equal bounds and frame/millisecond conversion',
                '1276 guarded output-pointer cases across precisions: separate/aliased outputs, null, invalid, read-only and cross-page pointers',
                '480 equal-address native/source position method cases across stages0..7, buffering0/1, three pause cursors and five logical cursors',
                'Native10002180 ECX push seeds the cursor with this outside PLAY/PAUSE; actual buffer queries and paused pointer override it',
                'Actual Channel_Start/Pause before defined paused frame/millisecond position setters/getters, exact frame EOF and frame-1 decoder sentinel',
                'Loop low-bit values-1,0,1,2,3,255,256,INT_MAX; priorities including65536/INT_MAX and negative rejection; four invalid channel IDs',
                'Eight separate four-second probes observe both DLLs fail to return at frame-past-end and negative-ms seeks in PC53/PC24'],
            test_adapters=['Actual OS DirectSound buffers and DLL workers; public GetProcAddress calls and recovered file callbacks',
                'Pointer-position replay uses copies of the actual300-byte source channel at the same address for both method bodies; real borrowed buffer, no fake COM',
                'Source private symbols resolved from its timestamp-verified linker map; original private address only in diagnostic test',
                'Nonreturn observations mean no completion within4s; each direct child is killed by subprocess timeout, no game/system files modified'],
            pending=['Longer nonreturn liveness proof, other files/codecs/devices, arbitrary malformed fragments and overflow',
                'Full physical buffer PCM comparison after each range mutation, concurrent playback/workers/callbacks and teardown',
                'All remaining97-export signature/behavior and full-game scopes'])
    except Exception as error:
        report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={p.name:digest(p) for p in work.glob('*.log')}
        encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
