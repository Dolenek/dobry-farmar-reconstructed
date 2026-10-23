#!/usr/bin/env python3
"""Compare actual Person channel logic on rebuilt and original physical Squall DLLs."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
from pe import PE
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/person-sounds-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/person-sounds-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True);candidate=oracle.parent/'Squall.dll';original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();candidate_hash=digest(candidate);oracle_hash=digest(oracle)
    assert candidate_hash!=digest(original/'Squall.dll')
    imports=next(row['symbols'] for row in PE(oracle).metadata()['imports'] if row['library'].lower()=='squall.dll')
    assert len(imports)==15
    contexts=[]
    for backend in ['source','original']:
        for precision,suffix in [('PC53',[]),('PC24',['device'])]:
            extra=suffix+(['reference-backend'] if backend=='original' else [])
            run=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),*extra],cwd=ROOT,capture_output=True,text=True,timeout=120)
            if run.returncode:raise RuntimeError(f'Person sounds {backend}/{precision}: {run.returncode} {run.stderr}')
            state=json.loads(run.stdout)
            for key,value in dict(states=83,actions=82,api_events=154,playback_progress_checks=10,paused_cursor_checks=2).items():assert state[key]==value,(key,state[key])
            assert state['physical_source_dll']==(backend=='source') and not state['audible_output_verified']
            assert state['x87_precision']==(0x10000 if precision=='PC53' else 0x20000)
            contexts.append(dict(backend=backend,precision=precision,result=state));unchanged()
            assert digest(candidate)==candidate_hash and digest(oracle)==oracle_hash
    extra=['CMakeLists.txt','tools/cmake/SquallCandidate.cmake','tools/validate_person_sounds.py','tools/person_sounds_reference_test.cpp','tools/person_sounds_oracle_fixups.inc','tools/build_sprite_oracle_fixups.py']
    paths=sorted([p for folder in ['src/game','src/squall'] for p in (ROOT/folder).rglob('*') if p.is_file()]+[ROOT/n for n in extra])
    report=dict(status='passed_for_person_sound_selection_channel_ownership_and_physical_backends',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        original_game_entry_point_executed=False,original_squall_entry_point_executed=True,original_program_code_used_in_implementation=False,reference_files_unchanged=len(inventory['files']),
        reference_game_sha256=digest(original/'Dobrý farmář.exe'),reference_dll_sha256=digest(original/'Squall.dll'),
        oracle_path=oracle.relative_to(ROOT).as_posix(),oracle_sha256=oracle_hash,candidate_path=candidate.relative_to(ROOT).as_posix(),candidate_sha256=candidate_hash,
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},contexts=contexts,states=332,actions=328,api_events=616,playback_progress_checks=40,paused_cursor_checks=8,
        original_functions=['0044a820','0044ad50','00449820','00449840','00449860','00449880'],
        coverage=[
            'Actual eight Person XML sound paths load cSound resources through recovered FileIndex/ResourceManager/SoundManager and physical DLL file callbacks',
            'Five independently owned work/hose channels per side; kinds0/1/2/4 replace and stop old wrappers before new play, with exact resource reference deltas',
            'One-shot kinds3/5/7, unsupported unsigned kinds, exact loop/start/group/volume/frequency/pan flags and API return/order per controller comparison',
            'Hose creates once paused and starts/resumes; paused cached wrapper remains identical even after the current sound resource slot becomes null',
            'Live resource slot replacement on work replay, optional null resources, repeated stops, only-kind6 generic stop and four dedicated work stops',
            'Owned helper destruction stops hose then watering/scythe/tractor/shovel; every native/source reference and callback handle is released',
            '40 actual loop playback cursor advances and eight frozen paused cursor checks; source and original physical backends each execute PC53/PC24'],
        test_adapters=[
            'Native Person data adapter supplies sound/channel fields; full actor constructor, UI, actions and original entry point do not execute',
            'Actual original Person sound/stop instructions execute from a separately mapped reference image; production PersonSounds contains no original code calls',
            'Native SoundManager vtable boundary uses actual recovered SoundManager and cSound/cChannel ABI; cChannel.start uses its separately native-compared source implementation',
            'Each backend compares source controller with original controller against that same physical DLL; numerical channel handles are paired, not assumed equal',
            'This does not establish full Squall equivalence: existing cross-DLL game-audio/PCM/API scopes remain separately recorded',
            'Both controllers together own ten loops. Waiting for specific stopped handles to become NONE avoids intentional voice stealing at the16-channel limit; counts alone omit stopped-but-not-retired handles',
            'One-shot diagnostic voices are stopped only after observing both actual requests; silent volume avoids audible output and does not verify waveform/refill quality',
            'Caller precision is selected after DLL load/CRT startup, before SoundManager construction. JSON is flushed before old DLL unload',
            'Eight resource references are owned by the future actor/visual layer, not this channel helper; complete Person destructor interleaving remains pending'],
        pending=['Connect sounds to complete idle/work/carry and arrival action processor','Full Person constructor/lifecycle and Farm/cGame EXE',
            'Bidirectional complete save files','Full97-export Squall behavior, audible/refill comparison and isolated source uninstaller'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Person sounds: 332 states / 328 actions / 616 API events, 40 live cursor and 8 pause checks; two physical DLLs and PC53/PC24; 17 originals unchanged')
if __name__=='__main__':main()
