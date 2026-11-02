#!/usr/bin/env python3
"""Compare actual nonzero PCM/IMA/MS ADPCM WAV decoding and physical buffers."""
import argparse,datetime,hashlib,json,subprocess,sys,traceback
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    p.add_argument('--oracle',type=Path,default=ROOT/'build/squall-wave-formats-probe-vs/Release/squall-wave-formats-reference-test.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/squall-wave-formats-validation.json')
    args=p.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-wave-formats'/stamp;work.mkdir(parents=True)
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/name for name in ['tools/validate_squall_wave_formats.py','tools/squall_wave_formats_reference_test.cpp','tools/squall_wave_formats_probe/CMakeLists.txt','tools/squall_exports.inc','tools/squall_wave_fixtures.py']]
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
        symbols=[symbol('?api_SampleGetFile@@'),symbol('?GetMonoData@CSoundFile@@'),symbol('?GetStereoData@CSoundFile@@'),symbol('?api@@3Usquall_api_s@@A')+12]
        report['source_map']=dict(path=source_map.relative_to(ROOT).as_posix(),sha256=digest(source_map),symbol_rvas=symbols)
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(original/'Squall.dll'),str(candidate),*[format(x,'x') for x in symbols],*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=180,cwd=work)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'))
            row=json.loads(result.stdout)
            assert row['loaded_samples_per_dll']==48 and row['physical_buffers']==96 and row['guarded_pcm_reads']==976 and row['actual_directsound'] and row['export_lookups']==194
            row['precision']=precision;report['precision_contexts'].append(row);unchanged();assert digest(candidate)==report['rebuilt_sha256']
        report.update(status='passed_for_actual_nonzero_pcm_ima_msadpcm_wav_decode_seek_loop_and_physical_buffers_only',
            result_comparisons=sum(row['result_comparisons'] for row in report['precision_contexts']),
            guarded_pcm_reads=sum(row['guarded_pcm_reads'] for row in report['precision_contexts']),
            physical_buffers=sum(row['physical_buffers'] for row in report['precision_contexts']),
            all_raw_pcm_outputs_claimed_equal=False,actual_address_dependent_eof_models_verified=True,genuinely_uninitialized_pcm8_eof_claimed_equal=False,
            reference_files_unchanged=17,original_entry_point_executed=True,original_program_code_used_in_implementation=False,implementation_changed=True,
            coverage=['24 owned generated mono/stereo WAVs: PCM8/16,IMA ADPCM and all seven MS ADPCM predictors',
                'Actual Sample_LoadFile memory0/1,exact frequency/frame/time metadata and complete guarded mono/stereo decoder outputs',
                'Nonzero/clipping/sign/low-byte cases,seek and EOF/loop/cross-packet/zero-length reads,PCM UINT_MAX seek clamped to EOF',
                'Actual prepared2D/3D DirectSound buffers exact; stop/retire/unload and exclusive-file reopen proves no retained file handle'],
            pending=['PCM8 stereo-to-mono genuinely uninitialized EOF stack input; raw pairs recorded without normalization',
                'Additional WAV layouts/corrupt inputs,ADPCM invalid states and general callback failures',
                'MPEG/WMA/tracker and other remaining complete97-export/full-game scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={path.name:digest(path) for path in work.glob('*.log')};encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')
if __name__=='__main__':sys.exit(main())
