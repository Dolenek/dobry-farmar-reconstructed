#!/usr/bin/env python3
"""Compare multiple-member group selection, original256 cap and real2D/3D group APIs."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-sample-selection-probe-vs/Release/squall-sample-selection-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-sample-selection-validation.json')
    args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-sample-selection'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_sample_selection.py','tools/squall_sample_selection_reference_test.cpp','tools/squall_sample_selection_probe/CMakeLists.txt','tools/squall_exports.inc']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        reference_sha256=digest(original/'Squall.dll'),rebuilt_sha256=digest(candidate),oracle_sha256=digest(oracle),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=[])
    try:
        import struct
        pcm=b''.join(struct.pack('<h',((n*733)%20001)-10000) for n in range(64))
        wav=struct.pack('<4sI4s4sIHHIIHH4sI',b'RIFF',36+len(pcm),b'WAVE',b'fmt ',16,1,1,8000,16000,2,16,b'data',len(pcm))+pcm
        for filename in ['sample.wav','sample2.wav']:(work/filename).write_bytes(wav)
        report['sample_files']={name:digest(work/name) for name in ['sample.wav','sample2.wav']}
        source_map=candidate.with_suffix('.map');lines=source_map.read_text(encoding='latin-1').splitlines()
        def symbol(part):
            matches=[line.split() for line in lines if part in line];assert len(matches)==1
            return int(matches[0][2],16)-0x10000000
        selector_rva=symbol('?api_SampleGetFileGroup@@');context_rva=symbol('?api@@3Usquall_api_s@@A')+12
        report['source_map']=dict(path=source_map.relative_to(ROOT).as_posix(),sha256=digest(source_map),selector_rva=selector_rva,context_rva=context_rva)
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),format(selector_rva,'x'),format(context_rva,'x'),*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=180,cwd=work)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout)
            assert row['selector_calls_compared']==114688 and row['actual_export_channels']==112 and row['loaded_samples_per_dll']==300 and row['actual_directsound'] and not row['rng_state_written'] and row['private_context_read_only']
            assert [g['members'] for g in row['groups']]==[1,2,7,255,256,257,300]
            assert all(g['eligible']==min(g['members'],256) and sum(g['selected_counts'])==16384 and all(g['selected_counts']) for g in row['groups'])
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_multi_member_groups_native_rng_sequence_256_cap_and_real_group_channels_only',
            result_comparisons=sum(row['result_comparisons'] for row in report['precision_contexts']),
            selector_calls_compared=sum(row['selector_calls_compared'] for row in report['precision_contexts']),
            actual_export_channels=sum(row['actual_export_channels'] for row in report['precision_contexts']),
            reference_files_unchanged=17,original_entry_point_executed=True,original_program_code_used_in_implementation=False,implementation_changed=False,
            coverage=['300 actual loaded samples sharing one file,actual public group reassignment and unique per-sample defaults',
                'Actual original/source private selector and untouched default CRT sequence; seven group sizes1/2/7/255/256/257/300,all eligible members observed',
                'Missing groups do not consume RNG and unused selector num boundaries0/1/UINT_MAX/256 do not select a fixed member',
                'All four actual group playback exports consume exactly one expected selection and create real2D/3D buffers; readonly actual SampleID,frequency and priority'],
            pending=['Concurrent group mutation,65535 sample limit and generation wraparound',
                'Other decoder formats and remaining complete97-export/full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={path.name:digest(path) for path in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
