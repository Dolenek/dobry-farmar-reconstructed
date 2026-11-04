#!/usr/bin/env python3
"""Run PCM/game audio and save regressions after all tracker-loader and mixer reconstruction."""
import argparse,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--skip-game-restores',action='store_true');args=parser.parse_args()
    commands=[]
    for suffix,build in [('', 'build/windows'),('-fresh','build/squall-source-repro-vs20261010')]:
        candidate=build+'/squall-candidate/Release/Squall.dll'
        commands += [
            ['tools/validate_exports.py','--binary',candidate,'--original-name','Squall.dll','--output',f'.local/squall-tracker-loaders{suffix}-abi.json'],
            ['tools/validate_squall_pcm_eof.py','--candidate',candidate,'--output',f'.local/squall-tracker-loaders-pcm-eof{suffix}-validation.json'],
            ['tools/validate_squall_wave_formats.py','--candidate',candidate,'--output',f'.local/squall-tracker-loaders-wave-formats{suffix}-validation.json'],
            ['tools/validate_squall_dll.py','--candidate',candidate,'--oracle',build+'/Release/squall-dll-reference-test.exe','--output',f'.local/squall-tracker-loaders-dll{suffix}-validation.json'],
            ['tools/validate_game_audio.py','--build-dir',build,'--output',f'.local/squall-tracker-loaders-game-audio{suffix}-validation.json'],
            ['tools/validate_person_sounds.py','--oracle',build+'/Release/person-sounds-reference-test.exe','--output',f'.local/squall-tracker-loaders-person-sounds{suffix}-validation.json']]
    if not args.skip_game_restores:commands += [['tools/validate_game_field_disasters_interop.py','--crash-trace','build/source-crash-trace-vs/Release/source-crash-trace.exe','--output','.local/squall-tracker-loaders-game-interop-validation.json']]
    for i,command in enumerate(commands):
        path=ROOT/'.local'/f'squall-tracker-loaders-regression-{i:02d}.log';print('START',i,' '.join(command),flush=True)
        with path.open('wb') as output:result=subprocess.run([sys.executable,*command],cwd=ROOT,stdout=output,stderr=subprocess.STDOUT)
        print('FINISH',i,result.returncode,path.relative_to(ROOT).as_posix(),flush=True)
        if result.returncode:return result.returncode
    return 0
if __name__=='__main__':raise SystemExit(main())
