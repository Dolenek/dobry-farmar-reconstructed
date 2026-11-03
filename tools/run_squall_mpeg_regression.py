#!/usr/bin/env python3
"""Run current PCM/game audio regressions serially after the complete CTest run."""
import argparse,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--skip-game-restores',action='store_true');args=parser.parse_args()
    commands=[]
    for suffix,build in [('', 'build/windows'),('-fresh','build/squall-source-repro-vs20261010')]:
        candidate=build+'/squall-candidate/Release/Squall.dll'
        commands += [
            ['tools/validate_exports.py','--binary',candidate,'--original-name','Squall.dll','--output',f'.local/squall-mpeg{suffix}-abi.json'],
            ['tools/validate_squall_pcm_eof.py','--candidate',candidate,'--output',f'.local/squall-mpeg-pcm-eof{suffix}-validation.json'],
            ['tools/validate_squall_wave_formats.py','--candidate',candidate,'--output',f'.local/squall-mpeg-wave-formats{suffix}-validation.json'],
            ['tools/validate_squall_dll.py','--candidate',candidate,'--oracle',build+'/Release/squall-dll-reference-test.exe','--output',f'.local/squall-mpeg-dll{suffix}-validation.json'],
            ['tools/validate_game_audio.py','--build-dir',build,'--output',f'.local/squall-mpeg-game-audio{suffix}-validation.json'],
            ['tools/validate_person_sounds.py','--oracle',build+'/Release/person-sounds-reference-test.exe','--output',f'.local/squall-mpeg-person-sounds{suffix}-validation.json']]
    if not args.skip_game_restores:commands += [['tools/validate_game_field_disasters_interop.py','--crash-trace','build/source-crash-trace-vs/Release/source-crash-trace.exe','--output','.local/squall-mpeg-game-interop-validation.json']]
    for i,command in enumerate(commands):
        path=ROOT/'.local'/f'squall-mpeg-regression-{i:02d}.log';print('START',i,' '.join(command),flush=True)
        with path.open('wb') as output:result=subprocess.run([sys.executable,*command],cwd=ROOT,stdout=output,stderr=subprocess.STDOUT)
        print('FINISH',i,result.returncode,path.relative_to(ROOT).as_posix(),flush=True)
        if result.returncode:return result.returncode
    return 0
if __name__=='__main__':raise SystemExit(main())
