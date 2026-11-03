#!/usr/bin/env python3
"""Compare independently encoded owned MPEG inputs through actual Squall DLLs."""
import argparse,datetime,json,shutil,subprocess,traceback
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write
from squall_mpeg_fixtures import generate


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-mpeg-formats-probe-vs/Release/squall-mpeg-formats-reference-test.exe')
    p.add_argument('--encoder',type=Path,default=ROOT/'.local/audio-fixture-encoder/ffmpeg.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-mpeg-formats-validation.json')
    args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True);encoder=args.encoder.resolve(strict=True)
    provenance=read(ROOT/'.local/audio-fixture-encoder/provenance.json');assert digest(encoder)==provenance['executable_sha256']
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159';stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-mpeg-formats'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_mpeg_formats.py','tools/squall_mpeg_fixtures.py','tools/squall_mpeg_formats_reference_test.cpp','tools/squall_mpeg_formats_probe/CMakeLists.txt','tools/fetch_audio_fixture_encoder.py','tools/squall_exports.inc','tools/cmake/SquallCandidate.cmake']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        candidate_path=candidate.relative_to(ROOT).as_posix(),rebuilt_sha256=digest(candidate),oracle_path=oracle.relative_to(ROOT).as_posix(),oracle_sha256=digest(oracle),
        reference_sha256=digest(original/'Squall.dll'),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},encoder_provenance=provenance,cases=[])
    # Preserve the actual inputs for later diagnosis even after source edits/rebuilds.
    for path in paths:
        target=work/'source-snapshots'/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    for path in [candidate,candidate.with_suffix('.map'),oracle]:
        target=work/'build-artifacts'/path.name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    report['source_snapshot_root']=(work/'source-snapshots').relative_to(ROOT).as_posix()
    report['build_artifact_snapshots']={path.name:digest(path) for path in (work/'build-artifacts').iterdir()}
    runtime_candidate=work/'build-artifacts'/candidate.name
    runtime_oracle=work/'build-artifacts'/oracle.name
    def unchanged():
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/row['path'])==row['sha256']
    try:
        unchanged();fixtures=generate(work,encoder);report['fixtures']=fixtures
        report['fixture_files']={name:digest(work/name) for row in fixtures for name in [row['name'],row['raw_name']]}
        source_map=candidate.with_suffix('.map');lines=source_map.read_text(encoding='latin-1').splitlines()
        def symbol(part):
            rows=[line.split() for line in lines if part in line];assert len(rows)==1
            return int(rows[0][2],16)-0x10000000
        symbols=[symbol('?api_SampleGetFile@@'),symbol('?GetMonoData@CSoundFile@@'),symbol('?GetStereoData@CSoundFile@@'),symbol('?api@@3Usquall_api_s@@A')+12]
        report['source_map']=dict(path=source_map.relative_to(ROOT).as_posix(),sha256=digest(source_map),symbol_rvas=symbols)
        for precision in ['PC53','PC24']:
            for i,fixture in enumerate(fixtures):
                folder=work/precision/str(i);folder.mkdir(parents=True);shutil.copyfile(work/fixture['name'],folder/fixture['name'])
                row=dict(precision=precision,fixture=fixture['name'],directory=folder.relative_to(ROOT).as_posix());report['cases'].append(row)
                command=[str(runtime_oracle),str(original/'Squall.dll'),str(runtime_candidate),*[format(x,'x') for x in symbols],fixture['name'],*(['pc24'] if precision=='PC24' else [])]
                try:
                    result=subprocess.run(command,cwd=folder,capture_output=True,timeout=90)
                    (folder/'stdout.log').write_bytes(result.stdout);(folder/'stderr.log').write_bytes(result.stderr);row['exit_code']=result.returncode
                    if result.returncode:
                        row.update(status='failed',error=result.stderr.decode('utf-8',errors='replace')[-2000:])
                    else:
                        values=json.loads(result.stdout);assert values['guarded_pcm_reads']==40 and values['physical_buffers']==(0 if values['physical_buffers_skipped_native_layer2_overrun'] else 4) and values['actual_directsound'] and values['frequency']==fixture['rate']
                        row.update(status='passed',result=values)
                except subprocess.TimeoutExpired as error:
                    row.update(status='timeout',error=str(error));(folder/'timeout.stdout.log').write_bytes(error.stdout or b'');(folder/'timeout.stderr.log').write_bytes(error.stderr or b'')
                assert digest(runtime_candidate)==report['rebuilt_sha256'] and digest(runtime_oracle)==report['oracle_sha256']
                row['artifacts']={p.name:digest(p) for p in folder.iterdir() if p.is_file()};unchanged();print(json.dumps({k:row.get(k) for k in ['precision','fixture','status','exit_code','error']}),flush=True)
                write(work/'report.json',report)
        failures=[row for row in report['cases'] if row['status']!='passed'];report['failed_cases']=len(failures)
        report.update(status='found_actual_mpeg_decoder_differences_or_failures' if failures else 'passed_for_eight_owned_mpeg_inputs_defined_ranges_and_seven_safe_physical_buffer_formats_only',
            production_implementation_changed=report['rebuilt_sha256']!=read(ROOT/'analysis/squall-mpeg-initial-observations.json')['rebuilt_sha256'],original_program_code_used_in_implementation=False,reference_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,
            pending=['Native MPEG2 Layer2 tail-table overread and unaligned direct-output overrun are separately proved; unsafe physical buffers are not claimed verified','Broader MPEG layer1,metadata/corruption,tracker/WMA and full original goal scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();write(work/'report.json',report);write(args.output,report);print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],failed_cases=report.get('failed_cases'),error=report.get('error'))))
    return int(report['status']!='passed_for_eight_owned_mpeg_inputs_defined_ranges_and_seven_safe_physical_buffer_formats_only')


if __name__=='__main__':raise SystemExit(main())
