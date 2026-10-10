#!/usr/bin/env python3
"""Audit editable Squall sample decoding and MDCT; not whole DLL equivalence."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]

def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir',type=Path,default=ROOT/'build/windows')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-runtime-validation.json')
    args=parser.parse_args()
    build=args.build_dir.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář/Squall.dll'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    reference=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert digest(original)==reference
    def run(name,arguments):
        contexts=[]
        for precision,output in run_precision_cases(build/'Release'/name,arguments,timeout=60):
            context=json.loads(output);context['precision']=precision;contexts.append(context)
            assert digest(original)==reference
        return contexts
    runtime=run('squall-runtime-reference-test.exe',[original,original.parent/'Data'])
    for context in runtime:
        assert context['files']==45 and context['metadata']==180 and context['pcm_calls']==270
        assert context['pcm_bytes']==825918 and context['sequential_calls']==5034
        assert context['sequential_bytes']==61263876 and context['sequential_frames']==20421292
        assert context['decoders']=={'482a0':45}
    mdct=run('squall-vorbis-reference-test.exe',[original])
    for context in mdct:
        assert context['lookup_values']==20400 and context['butterfly_values']==400000
        assert context['transforms']==1024 and context['transform_values']==2088960 and context['differences']==0
    candidate=build/'squall-candidate/Release/Squall.dll'
    abi_path=ROOT/'.local/squall-candidate-abi.json'
    subprocess.run(['python',str(ROOT/'tools/validate_exports.py'),'--binary',str(candidate),
                    '--original-name','Squall.dll','--output',str(abi_path)],check=True)
    abi=json.loads(abi_path.read_text(encoding='utf-8'))
    cache=(build/'CMakeCache.txt').read_text(encoding='utf-8')
    options={key:next(line.split('=',1)[1] for line in cache.splitlines() if line.startswith(key+':'))
             for key in ('SQUALL_VORBIS_CANDIDATE','SQUALL_VORBIS_FP')}
    source_paths=sorted([path for directory in ('src/squall','tools/cmake') for path in (ROOT/directory).rglob('*') if path.is_file()])
    report=dict(status='passed_for_recorded_sample_and_transform_scope',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=reference,
        reference_files_modified=False,original_entry_point_executed=True,
        original_program_code_used_in_implementation=False,build_options=options,
        export_contract=abi,runtime_precision_contexts=runtime,mdct_precision_contexts=mdct,
        source_files=[dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path)) for path in source_paths],
        oracle_files=[dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path)) for path in
                      (build/'Release/squall-runtime-reference-test.exe',build/'Release/squall-vorbis-reference-test.exe')],
        native_addresses=dict(sample_get_file='10008300',mono_pcm='1002e020',stereo_pcm='1002e060',
            mdct_init='10037110',mdct_clear='100372f0',mdct_backward='10037330',
            butterfly32='10037840',butterfly8='10037d40',generic_stage='10037dd0'),
        coverage=['Actual DirectSound device enumeration count compared on this host',
            'Recovered FileIndex and SoundFiles read-only callbacks load all 45 original sounds in memory mode 1',
            'Public sample IDs, frame and millisecond length, frequency and all 28 default bytes',
            'Initial, middle and final random-access windows in mono and stereo',
            'Complete PCM in both channel conversions through 4096-frame chunks, sequential start sentinel and buffer guards',
            'MDCT lookup, bit reversal and scale for sizes 64..8192; seeded whole transforms and 8/32 butterflies',
            'Both PC53 and PC24 set after loading the oracle; precision remains unchanged after comparisons'],
        test_adapters=['Original DLL is loaded normally as an independent test oracle, including its CRT entry point',
            'PCM accesses the original private sample reader and wrappers; source calls its own editable static runtime',
            'No Squall initialization, DirectSound playback or fake audio device is used by this sample comparison',
            'Candidate DLL export contract is checked separately; its whole runtime is not proved by statically linked PCM tests'],
        pending=['Actual candidate DLL public API runtime and all 97 interfaces/calling conventions',
            'Full initialization, DirectSound buffer playback, channel state, device switching, workers and thread synchronization',
            '3D listener and channels, EAX/ZoomFX and hardware-dependent behavior',
            'PCM WAV, IMA/MS ADPCM, MPEG, WMA and tracker decoders: all original game sound files use Vorbis',
            'Streaming file mode 0 with genuinely seekable callbacks, looping, EOF/error/corrupt inputs and custom allocations',
            'Original Ogg/Vorbis release tag; identical game-file PCM does not identify the historical release',
            'Whole game audio integration, real menu/engine and saves'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    mapping=json.loads((ROOT/'analysis/squall-source-match.json').read_text(encoding='utf-8'))
    verified={'SQUALL_GetNumDevice':'Real device count on this host; device details and switching unverified',
        'SQUALL_SetFileCallbacks':'Four recovered read-only callbacks used for all game samples; partial/null/custom cases unverified',
        'SQUALL_Sample_LoadFile':'All 45 original Vorbis files with memory flag 1, null defaults and real game callbacks',
        'SQUALL_Sample_Unload':'Successful loaded sample IDs and callback lifetime; invalid/busy IDs unverified',
        'SQUALL_Sample_GetFileLength':'All 45 valid loaded sample frame lengths',
        'SQUALL_Sample_GetFileLengthMs':'All 45 valid loaded sample millisecond lengths',
        'SQUALL_Sample_GetFileFrequency':'All 45 valid loaded sample rates',
        'SQUALL_Sample_GetDefault':'All 28 default bytes for 45 valid loaded samples',
        'SQUALL_Free':'Cleanup after sample-only enumeration/loading; initialized backend lifecycle unverified',
        'SQUALL_Init':'Normalizer and all 256 volume table words only; backend initialization unverified',
        'SQUALL_Channel_GetVolume':'All -10000..0 inverse arithmetic values with native lookup and explicit COM getter fixture'}
    audit=dict(status='all_97_export_contracts_present_whole_behavior_audit_incomplete',
        rebuilt_sha256=abi['rebuilt_sha256'],reference_sha256=reference,
        exports=[dict(name=row['name'],ordinal=row['ordinal'],original_address=row['address'],
            status='partially_compared' if row['name'] in verified else 'editable_source_compiles_behavior_unverified',
            verified_scope=verified.get(row['name']),native_signature_fully_verified=False)
            for row in mapping['exports']])
    (ROOT/'.local/squall-export-audit.json').write_text(json.dumps(audit,indent=2)+'\n',encoding='utf-8')
    print('Squall sample audit: 122527752 whole PCM bytes, 4177920 transform values; full runtime remains incomplete')

if __name__=='__main__':main()
