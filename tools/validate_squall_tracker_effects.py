#!/usr/bin/env python3
"""Compare actual original tracker effect routines and complete initialized objects."""
import argparse,datetime,json,shutil,subprocess
from pathlib import Path
from validate_squall_tracker_stages import ROOT,digest,symbol

METHODS=[('PortamentoUp',0x28640,0),('PortamentoDown',0x286d0,0),('FinePortamentoUp',0x28760,0),('FinePortamentoDown',0x28800,0),
 ('ExtraFinePortamentoUp',0x288a0,0),('ExtraFinePortamentoDown',0x28940,0),('TonePortamento',0x289e0,0),
 ('VolumeSlide',0x28bd0,0),('PanningSlide',0x28cd0,0),('FineVolumeUp',0x28db0,0),('FineVolumeDown',0x28e10,0)]
# Optimized source inlines several effects. Exercise their actual production
# dispatcher instead of changing production flags or substituting a test copy.
METHODS += [(f'ProcessEffects_{command}',0x27870,256+command) for command in range(32)]

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-tracker-effects-validation.json');args=parser.parse_args()
    work=ROOT/'.local/squall-tracker-effects'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
    binaries={'native.dll':ROOT.parent/'farmář/Dobrý farmář/Squall.dll','source.dll':args.candidate.resolve(),'source.map':args.candidate.resolve().with_suffix('.map'),
        'probe.exe':ROOT/'build/squall-tracker-effects-probe-vs/Release/squall-tracker-effects-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    sources=['tools/validate_squall_tracker_effects.py','tools/squall_tracker_effects_probe.cpp','tools/squall_tracker_effects_probe/CMakeLists.txt','tools/validate_squall_tracker_stages.py']
    sources += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.cpp','.h']]
    for name in sources:
        dest=work/'snapshots'/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,dest)
    rows=[f'{name} {rva:x} {symbol(work/"source.map","?"+("ProcessEffects" if kind>=256 else name)+"@CDecompressTrack@@")} {kind}\n' for name,rva,kind in METHODS]
    (work/'methods.txt').write_bytes(''.join(rows).encode('ascii'))
    run=subprocess.run([str(work/'probe.exe'),str(work/'native.dll'),str(work/'source.dll'),str(work/'methods.txt')],cwd=work,capture_output=True,timeout=240)
    (work/'stdout.jsonl').write_bytes(run.stdout);(work/'stderr.log').write_bytes(run.stderr)
    observations=[json.loads(line) for line in run.stdout.decode('utf-8','replace').splitlines()];summary=observations[-1] if observations else {}
    passed=run.returncode==0 and summary.get('total_pairs')==len(METHODS)*32768 and summary.get('different_pairs')==summary.get('control_differences')==summary.get('guard_differences')==0
    for name in sources:assert digest(ROOT/name)==digest(work/'snapshots'/name),name
    report=dict(status='passed_actual_effect_methods_only' if passed else 'actual_effect_differences_or_failure',work_directory=work.relative_to(ROOT).as_posix(),
        binaries={name:digest(work/name) for name in binaries},source_files={name:digest(work/'snapshots'/name) for name in sources},
        exit_code=run.returncode,methods=observations[:-1],summary=summary,
        artifacts={p.name:digest(p) for p in work.iterdir() if p.is_file()},
        precision_contexts=['PC53','PC24'],parameter_values=list(range(256)),module_types=['MOD','S3M','IT','XM','MT2','STM','AMF','MOD|IT'],
        variants='Eight FIRSTTICK/LINEARSLIDES/FASTVOLSLIDES combinations with zero/boundary/active period,volume,pan,global volume and tempo',
        all_initialized_object_bytes_compared_without_pointer_exclusions=True,production_changed=False,public_tracker_playback_claimed=False)
    encoded=(json.dumps(report,indent=2)+'\n').encode('utf-8');(work/'report.json').write_bytes(encoded);args.output.write_bytes(encoded)
    print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],exit_code=run.returncode,summary=summary)))

if __name__=='__main__':main()
