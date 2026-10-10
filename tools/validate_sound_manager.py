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
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Sound files: {report['opens']} opens, {report['reads']} reads; volume: {report['volume_checks']} native comparisons")

if __name__=='__main__':main()
