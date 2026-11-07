#!/usr/bin/env python3
"""Retain actual native41/audio and failed premature-readiness evidence, not a paired pass."""
import array
import datetime
import json
import xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write


def main():
    path=ROOT/'.local/tutorial-warning-runtime/20261107T210917905903Z/report.json'
    raw=read(path);assert raw['status']=='failed' and 'has not naturally ripened' in raw['error']
    session=raw['sessions'][0];assert session['kind']=='native' and session['case']=='full_warehouse'
    assert session['status']=='completed' and session['exit_code']==0
    assert json.loads(session['launcher_completion'])==dict(event='exit',exit_code=0,watchdog_triggered=False)
    initial,final=session['initial_state'],session['final_state'];evidence=session['capacity_warning_evidence']
    assert sum(initial['stock'])==evidence['original_capacity']==15
    assert initial['held']==6 and initial['carried']==[0]*8 and initial['counters']['capacity_counter']==100
    assert final['held']==2 and final['carried']==[1]+[0]*7 and final['counters']['capacity_counter']==0
    assert final['collected']==initial['collected']+1
    assert final['stock']==initial['stock'] and final['market']==initial['market'] and final['money']==initial['money']
    assert evidence['attempted_store_count']==2 and 0<=evidence['second_attempt_game_seconds_since_first']<3
    assert not initial['processed']['41'] and final['processed']['41'] and final['tutorial']==-1
    saved={n.get('Name'):n.get('Val') for n in ET.parse(ROOT/session['persisted_profile_directory']/'00000001.xml').getroot()}
    assert saved['tip41']=='1' and saved['PCC0']=='1' and saved['PCarry']=='2'
    for n in range(19):assert int(saved[f'WGoodCount{n}'])==final['stock'][n] and int(saved[f'MGoodCount{n}'])==final['market'][n]
    retained=ROOT/raw['work_directory']/'tutorial_warning_scenarios-retained-after-run.py'
    assert digest(retained)==raw['source_files']['tools/tutorial_warning_scenarios.py']
    result=dict(status='passed_completed_native_full_warehouse_warning41_only_source_counterpart_unverified',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        raw_report=dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path),original_status=raw['status']),
        retained_scenario_source=dict(path=retained.relative_to(ROOT).as_posix(),sha256=digest(retained),retained_after_run=True),
        session=session,original_driver_failure_preserved=raw['error'],
        source_counterpart_verified=False,whole_warning_gate_complete=False,whole_game_complete=False)
    write(ROOT/'analysis/tutorial-warning-native-warehouse-observation.json',result)
    mixed=raw['sessions'][1];assert mixed['kind']=='native' and mixed['exit_code']==0
    at=mixed['failure_live_state'];assert at['carried']==[0,1]+[0]*6 and at['stock'][1]==3 and at['processed']['40']
    write(ROOT/'analysis/tutorial-warning-growth-observations.json',dict(
        status='recorded_premature_readiness_assertion_not_original_game_failure',raw_report=result['raw_report'],
        normal_native_exit=0,actual_four_tomato_harvests=True,stored_tomatoes=3,carried_tomatoes=1,
        observed_warning44=False,correction='Wait for actual FFS cucumber readiness;never infer readiness from duration of tomato UI inputs',
        corrected_retry_completed=False,interrupted_retry='.local/tutorial-warning-runtime/20261107T211631362554Z',
        production_changed=False,whole_game_complete=False))
    log=ROOT/'.local/native-game-live-loopback.log';pcm=ROOT/'.local/native-game-live-loopback.pcm'
    events=[json.loads(line) for line in log.read_text(encoding='utf-8-sig').splitlines() if line.strip()]
    assert [r['event'] for r in events]==['capturing','completed'] and events[0]['pid']==events[1]['pid']==mixed['pid']
    assert events[0]['process_tree_only'] and not events[0]['microphone']
    samples=array.array('h');samples.frombytes(pcm.read_bytes());last=events[1]
    assert len(samples)==last['frames']*2 and sum(v!=0 for v in samples)==last['nonzero_samples']
    assert max(abs(v) for v in samples)==last['peak'] and last['nonzero_samples']>44100
    write(ROOT/'analysis/native-game-loopback-audio-observation.json',dict(
        status='observed_nonzero_Windows_output_from_actual_native_game_only_source_capture_unverified',
        capture_program=dict(path='build/process-audio-capture/Release/process-audio-capture.exe',
            sha256=digest(ROOT/'build/process-audio-capture/Release/process-audio-capture.exe')),
        capture_source_files={name:digest(ROOT/name) for name in
            ['tools/process_audio_capture.cpp','tools/process_audio_capture/CMakeLists.txt']},
        API_provenance='https://learn.microsoft.com/en-us/samples/microsoft/windows-classic-samples/applicationloopbackaudio-sample/',
        raw_runtime_report=result['raw_report'],native_session_index=1,native_exit_code=0,
        capture_log=dict(path=log.relative_to(ROOT).as_posix(),sha256=digest(log)),
        captured_pcm=dict(path=pcm.relative_to(ROOT).as_posix(),sha256=digest(pcm),bytes=pcm.stat().st_size),
        capture_events=events,source_output_verified=False,physical_speaker_or_subjective_listening_compared=False,
        whole_game_complete=False))
    verification=read(ROOT/'analysis/verification.json')
    for name in ['tutorial-warning-native-warehouse-observation.json','tutorial-warning-growth-observations.json',
        'native-game-loopback-audio-observation.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    write(ROOT/'analysis/verification.json',verification)
    print('Retained native41, premature cucumber assertion, and native-only loopback;no paired/final pass.')


if __name__=='__main__':main()
