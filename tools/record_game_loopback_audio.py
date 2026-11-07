#!/usr/bin/env python3
"""Bind bounded process-only Windows output captures to actual owned full-game runs."""
import argparse
import array
import datetime
import json
import math
from pathlib import Path
from current_production_regression import current_regression
from record_live_production_verification import ROOT,digest,read,write


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for kind in ['native','source']:
        parser.add_argument('--'+kind+'-report',type=Path,required=True)
        parser.add_argument('--'+kind+'-session-index',type=int,required=True)
        parser.add_argument('--'+kind+'-log',type=Path,required=True)
        parser.add_argument('--'+kind+'-pcm',type=Path,required=True)
    args=parser.parse_args();evidence=[];implementation=None
    for kind in ['native','source']:
        path=getattr(args,kind+'_report').resolve(strict=True);raw=read(path)
        session=raw['sessions'][getattr(args,kind+'_session_index')]
        assert session['kind']==kind and session['exit_code']==0
        # A failed unrelated tutorial readiness assertion does not manufacture
        # a completed scenario; retain its raw failure and actual audio scope.
        assert not raw['original_program_code_used_in_implementation']
        assert raw['reference_files_unchanged']==17
        if kind=='source':
            assert session['status']=='completed';implementation=raw['implementation_files']
            for name,sha in implementation.items():
                actual='Dobrý farmář.exe' if name=='FarmarTutorialWarnings.exe' else name
                assert digest(ROOT/'build/windows/Release'/actual)==sha
        else:
            assert json.loads(session['launcher_completion'])==dict(event='exit',exit_code=0,watchdog_triggered=False)
        log=getattr(args,kind+'_log').resolve(strict=True);pcm=getattr(args,kind+'_pcm').resolve(strict=True)
        rows=[json.loads(line) for line in log.read_text(encoding='utf-8-sig').splitlines() if line.strip()]
        assert len(rows)==2 and [r['event'] for r in rows]==['capturing','completed']
        assert rows[0]['pid']==rows[1]['pid']==session['pid']
        assert rows[0]['process_tree_only'] and not rows[0]['microphone']
        assert [rows[0][key] for key in ['rate','channels','bits']]==[44100,2,16]
        finished=rows[1];samples=array.array('h');samples.frombytes(pcm.read_bytes())
        assert len(samples)==finished['frames']*2 and finished['frames']>=44100*3
        nonzero=sum(value!=0 for value in samples);peak=max(abs(value) for value in samples)
        rms=math.sqrt(sum(value*value for value in samples)/len(samples))
        assert nonzero==finished['nonzero_samples'] and peak==finished['peak']
        assert abs(rms-finished['rms'])<.1
        assert peak>100 and nonzero>44100 and rms>10
        evidence.append(dict(kind=kind,runtime_report=dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path)),
            runtime_driver_status=raw['status'],session_index=getattr(args,kind+'_session_index'),pid=session['pid'],
            actual_game_normal_exit=0,loaded_game_dlls=session['loaded_game_dlls'],
            capture_log=dict(path=log.relative_to(ROOT).as_posix(),sha256=digest(log)),
            captured_pcm=dict(path=pcm.relative_to(ROOT).as_posix(),sha256=digest(pcm),bytes=pcm.stat().st_size),
            nonzero_samples=nonzero,peak=peak,rms=rms,actual_capture_statistics=finished))
    name,_=current_regression()
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    dependencies=['analysis/squall-loader-regression.json','analysis/squall-loader-game-audio-validation.json',
        'analysis/squall-loader-person-sounds-validation.json','analysis/squall-loader-play-validation.json']
    result=dict(status='passed_nonzero_Windows_process_loopback_from_actual_native_and_source_game_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        capture_program=dict(path='build/process-audio-capture/Release/process-audio-capture.exe',
            sha256=digest(ROOT/'build/process-audio-capture/Release/process-audio-capture.exe')),
        source_files={path:digest(ROOT/path) for path in ['tools/process_audio_capture.cpp','tools/process_audio_capture/CMakeLists.txt']},
        API_provenance='https://learn.microsoft.com/en-us/samples/microsoft/windows-classic-samples/applicationloopbackaudio-sample/',
        captures=evidence,implementation_files=implementation,
        related_exact_PCM_and_API_reports={path:digest(ROOT/path) for path in dependencies},
        process_tree_only=True,microphone_or_other_application_audio_captured=False,
        independently_timed_Windows_mix_waveforms_claimed_bit_identical=False,
        physical_speaker_DAC_or_subjective_listening_compared=False,
        original_files_unchanged=17,production_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report=name,new_full_ctest_run=False,whole_game_complete=False)
    path=ROOT/'analysis/current-game-loopback-audio-validation.json';write(path,result)
    verification=read(ROOT/'analysis/verification.json');entry=path.relative_to(ROOT).as_posix()
    if entry not in verification['reports']:verification['reports'].append(entry)
    verification['current_game_audio_output_gate']=dict(report=entry,complete=True)
    write(ROOT/'analysis/verification.json',verification)
    print('Verified actual native/source game Windows output:',[(r['nonzero_samples'],r['peak']) for r in evidence])


if __name__=='__main__':main()
