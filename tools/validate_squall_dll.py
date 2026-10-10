#!/usr/bin/env python3
"""Compare loaded reconstructed DLL samples and physical prepared DirectSound buffers."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/squall-dll-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-dll-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Squall.dll'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert digest(original)==expected
    candidate=args.candidate.resolve(strict=True);candidate_hash=digest(candidate)
    oracle=args.oracle.resolve(strict=True);link_map=candidate.with_suffix('.map')
    contexts=[]
    for precision,output in run_precision_cases(oracle,[original,candidate,original.parent/'Data'],timeout=60):
        row=json.loads(output)
        assert row['export_lookups']==194 and row['samples']==45 and row['api_comparisons']==299
        assert row['init_result']==1 and row['directsound_initialized'] and row['prepared_channels']==1
        assert row['physical_snapshots']==4 and row['physical_pcm_bytes']==282240
        assert row['dll_pcm_calls']==5034 and row['dll_pcm_bytes']==61263876 and row['dll_pcm_frames']==20421292
        assert not row['playback_tested']
        row['precision']=precision;contexts.append(row)
        assert digest(original)==expected and digest(candidate)==candidate_hash
    report=dict(status='passed_for_loaded_DLL_samples_and_prepared_DirectSound_scope',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        rebuilt_sha256=candidate_hash,oracle_sha256=digest(oracle),linker_map_sha256=digest(link_map),
        candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        linker_map_path=link_map.relative_to(ROOT).as_posix(),
        source_files=[dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path)) for path in sorted(
            [path for directory in ('src/squall','tools/cmake') for path in (ROOT/directory).rglob('*') if path.is_file()]
            +[ROOT/'tools/squall_dll_reference_test.cpp',ROOT/'tools/squall_exports.inc',Path(__file__).resolve()])],
        precision_contexts=contexts,original_entry_point_executed=True,reference_files_modified=False,
        original_program_code_used_in_implementation=False,
        coverage=['Both actual DLLs resolve all 97 original names through GetProcAddress; C calling convention declarations used',
            'Real device count, all device name buffer bytes and device capability fields on this host',
            'All 45 memory-mode game samples: IDs, complete defaults, frame/millisecond lengths and frequencies',
            '122527752 full mono/stereo PCM bytes through actual decoder methods in loaded DLLs, buffer guards and sequential sentinel',
            'Actual successful DirectSound initialization with the game parameter values and a real hidden STATIC window',
            'Device/algorithm/buffer/speaker/EAX getters and initially empty channel counts',
            'Real prepared ambient music channel; length, volume/pan/frequency boundaries, loop and priority getters',
            '564480 physical PCM bytes from eight complete DirectSound Lock snapshots, including seek and loop wrap near sample end',
            'Stop return and worker retirement to status 0; native transient STOP status -24 is allowed only until retirement',
            'Loaded source symbols resolve from its own linker map; map timestamp must match the loaded DLL'],
        test_adapters=['Source private sample/decoder/context symbols are read from its own linker map, never exported or forwarded',
            'Reference private wrappers and context are accessed at hash-verified original addresses for comparison only',
            'Actual Windows DirectSound buffers and worker threads; no synthetic COM buffer in this test',
            'Prepared buffers do not play; audible output and time-correlated playback remain unverified'],
        pending=['All 97 signatures and full behavior: name resolution and selected API calls are partial coverage',
            'Playback, pause/resume, group operations, sample defaults and allocation/worker/device lifecycle boundaries',
            '3D parameters, listener workers, EAX/ZoomFX behavior on supporting hardware',
            'Other decoders, streaming mode 0, malformed inputs and historical codec release identity',
            'Exact timing of asynchronous stop/worker transitions and full game audio integration'])
    report['export_scopes']={
        'SQUALL_GetNumDevice':'Actual device count on this host',
        'SQUALL_GetDeviceName':'All device name output bytes in 512-byte guarded buffers on this host',
        'SQUALL_GetDeviceCaps':'All device capability fields on this host',
        'SQUALL_Init':'Successful actual DirectSound initialization with game parameter values and hidden HWND',
        'SQUALL_Free':'Sample-only and initialized prepared-channel cleanup',
        'SQUALL_SetFileCallbacks':'Recovered game read-only callbacks used before and after Init',
        'SQUALL_Sample_LoadFile':'All 45 game samples in memory mode 1, null defaults, and initialized menu music',
        'SQUALL_Sample_Unload':'All 45 valid sample lifetimes and menu music after channel retirement',
        'SQUALL_Sample_GetFileLength':'All 45 game samples',
        'SQUALL_Sample_GetFileLengthMs':'All 45 game samples',
        'SQUALL_Sample_GetFileFrequency':'All 45 game samples',
        'SQUALL_Sample_GetDefault':'All 28 default bytes for 45 valid samples',
        'SQUALL_Sample_PlayEx':'Actual prepared ambient music buffer, loop=1, group=17, start=0, priority=7, volume=43, frequency=0, pan=50',
        'SQUALL_GetChannelsInfo':'Complete empty initialized channel counts',
        'SQUALL_Channel_Status':'Prepared channel and terminal stopped state; asynchronous intermediate stop duration not compared',
        'SQUALL_Channel_GetLength':'Prepared menu music channel',
        'SQUALL_Channel_GetLengthMs':'Prepared menu music channel',
        'SQUALL_Channel_GetLoop':'Prepared looping menu music channel',
        'SQUALL_Channel_GetPriority':'Prepared priority 7 menu music channel',
        'SQUALL_Channel_SetPlayPosition':'Three frame positions including near-end loop wrap; full physical buffers compared',
        'SQUALL_Channel_Stop':'Actual buffer stop return and bounded worker retirement'}
    for operation in ('Device','3DAlgorithm','BufferSize','SpeakerMode','EAXVersion'):
        report['export_scopes']['SQUALL_Get'+operation]='Actual initialized backend getter on this host'
    for operation,values in (('Volume','0,1,43,99,100,-1,101'),('Pan','0,1,49,50,51,99,100,-1,101'),
                             ('Frequency','0,100,22050,44100,88200,-1,100001')):
        for action in ('Set','Get'):
            report['export_scopes']['SQUALL_Channel_'+action+operation]='Prepared ambient channel; setter/getter boundaries '+values
    # Eight 70560-byte buffers, including both precision contexts.
    report['physical_pcm_bytes']=sum(row['physical_pcm_bytes'] for row in contexts)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('Actual Squall DLLs: complete game PCM and 8 physical DirectSound buffers match; playback remains unverified')

if __name__=='__main__':main()
