#!/usr/bin/env python3
"""Compare recovered sound file callbacks and group-volume arithmetic with native routines."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/sound-manager-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/sound-manager-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    data=original.parent/'Data'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    oracle=args.oracle.resolve(strict=True);contexts=[]
    for precision,output in run_precision_cases(oracle,[original,data]):
        context=json.loads(output)
        for key,value in dict(lifetimes=32,opens=384,missing=32,reads=3072,seeks=3072,
                              bytes=3972516,volume_checks=200202,group_calls=100102,
                              initializations=32,frees=32,sound_files=45).items():
            assert context[key]==value,(key,context[key])
        context['precision']=precision;contexts.append(context)
    manager_contexts=[]
    for precision,output in run_precision_cases(oracle,[original,data,'manager']):
        context=json.loads(output)
        for key,value in dict(manager_states=20250,manager_lifetimes=24,sample_objects=24,
                              channel_objects=2886,api_events=33330,group_calls=105938,
                              initializations=81,frees=81).items():
            assert context[key]==value,(key,context[key])
        context['precision']=precision;manager_contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key not in ('precision','sound_files')}
    report.update(status='passed',sound_files=45,precision_contexts=contexts,
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=expected,oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),
        original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['00535900','00533ea0','00533a90','005342c0','00535d50','00535280','00532e40','00532f40','00532f90','00580f98'],
        coverage=['64 file-map lifetimes, first success handle 2 and persistent counter, simultaneous handles, randomized tree erase and singleton/allocation cleanup',
            '768 opens from 45 indexed original WAV/OGG files, exact real file read bytes and positions; size-only seek does not reposition',
            '13-word original initialization parameters and native failed-init Free/disabled behavior; no native Squall implementation runs',
            'All usual unsigned volume values 0..100 plus seeded full-uint32 range including 0xffffffff and 0x80000000: 400404 exact native sound/music transformed fields',
            '200204 native group-volume calls: group 1 sound, group 2 music, ignored negative API result and disabled-call suppression'],
        test_adapters=['Native file-manager virtual open calls real recovered FileIndex; iFile virtual read/size/close/delete execute real recovered file implementations',
            'Native original handle tree allocation/lookup/insert/erase and callback instructions execute in a relocated test-only mapping',
            'Native null-file-manager outcome is injected explicitly; actual source missing file propagates FileIndex exception. These are separate observations, not a verified match of missing-file exception ABI',
            'Squall Init, Free and ChannelGroupSetVolume boundaries record original arguments; this verifies no audio device, decoding, mixing or playback',
            'Native String byte assignment uses recovered String and CRT allocation uses diagnostic malloc/free; native ftol2 instructions execute'],
        pending=['Whole SoundManager enabled construction/settings/destruction and resource/channel methods',
            'Native invalid-handle exception content/ABI, singleton duplication exception and old CRT unwinding',
            'Handle counter wrap and overwriting an existing handle',
            'Enabled destructor with files remaining, disabled destructor with artificially opened files',
            'Actual Squall initialization/decoders/device/playback and production UI sound binding',
            'Complete reconstructed game runtime and physical audible validation'])
    report['manager_precision_contexts']=manager_contexts
    for key in ('manager_states','manager_lifetimes','sample_objects','channel_objects','api_events'):
        report[key]=sum(context[key] for context in manager_contexts)
    report['original_functions']+=['00532e00','00535840','00535850','00532de0','00532df0',
        '00533620','005346e0','00534730','00534b70','00534be0','00559570','00559970','00559960',
        '004fb8b0','004fb700','004fbda0','004fc1d0','0050ae80','0050ada0','0050ae20','0050ae30','0050ae50']
    report['coverage']+=[
        '48 whole manager lifetimes: enabled/failed initialization, present/absent Settings, music-before-sound setup, original retained caller pause byte and changed-only pause API calls',
        '48 real-layout cSound source/native constructor/destructor pairs: sample load flags, names, IDs, references, type, disabled sample 0 and conditional unload',
        '5772 cChannel source/native pairs: sound/music PlayEx eight arguments, frequency-before-play, loop/start/group/transformed volume, sound start=false pause, scripted statuses, pause/start call order, unconditional destructor stop/refcount decrement',
        '40500 whole manager/resource state checks and 66660 native/source API events; negative ignored API results do not change controller branches',
        'Enabled manager destruction saves both Settings before Stop/Free; installed real file callbacks remain live during Free close, and leftover files are deleted afterwards',
        'Source integration in both precisions: actual ResourceManager getSound lowercase/reuse/refcounts/wrong-type/forced-delete and cSound unload; Button click and Options volume/getter/reset routing use recovered SoundManager']
    report['test_adapters']+=[
        'Required SoundApi is an abstract boundary; API fixtures exist only in tools. No production decoder, mixer, device or Squall export implementation is supplied by this fixture',
        'Native manager Settings adapters use independently constructed recovered Settings; cSound and cChannel construction/refcount/lifetime run original instructions with relocated native virtual tables',
        'Native getSound manager factory/map remains instruction-analyzed; integrated source cache/UI binding checks are distinguished from native whole-manager comparisons',
        'SoundManager caller provides the initial pause byte left unwritten by original constructor; tested bytes 0/1. The final engine allocation path remains unverified']
    report['pending']=[entry for entry in report['pending'] if not entry.startswith('Whole SoundManager enabled') and not entry.startswith('Enabled destructor with files')]
    report['pending']+=[
        'Native getSound ResourceManager cache trees and whole physical UI-to-Squall integration',
        'Original negative sample/channel and file-callback setup exceptions/logging, allocation failures, manager construction unwinding and full old ABI',
        'cSound destruction after manager and cChannel destruction after manager lifetime; production SoundApi must outlive wrappers',
        'Native Button hover discards returned cChannel and retains a wrapper/reference; source binding preserves this observed leak, but its integrated lifetime is not exercised here',
        'Original initial pause allocation byte, actual MainMenu/engine lifecycle, complete Squall.dll and graphics reset']
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Sound files: {report['opens']} opens, {report['reads']} reads; volume: {report['volume_checks']} native comparisons")

if __name__=='__main__':main()
