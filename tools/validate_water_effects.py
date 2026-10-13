#!/usr/bin/env python3
"""Compare original water particles or complete Hose/Sprinkle emitters."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--emitters',action='store_true');parser.add_argument('--output',type=Path)
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[];base='water-emitters' if args.emitters else 'water-particles'
    for precision,suffix in [('PC53',''),('PC24','-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),base+suffix],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'{base} {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        expected=dict(states=538,objects=2,creations=256,frames=176,pixels=84480000) if args.emitters else dict(states=354,objects=6,copies=6,frames=144,pixels=69120000)
        prefix='water_emitter_' if args.emitters else 'water_particle_'
        for key,value in expected.items():assert state[prefix+key]==value,(key,state[prefix+key])
        if args.emitters:assert state['water_particle_states']==4119 and state['water_particle_frames']==0
        assert state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        contexts.append(dict(precision=precision,result=state));unchanged()
    extra=['CMakeLists.txt','tools/text_buttons_reference_test.cpp','tools/text_buttons_oracle_fixups.inc',
        'tools/water_particles_oracle_cases.inc','tools/water_emitters_oracle_cases.inc','tools/build_sprite_oracle_fixups.py',
        'tools/monotonic_ui_test_heap.h','tools/gameplay_runtime_oracle_services.h','tools/loading_oracle_services.h','tools/main_menu_oracle_services.h']
    paths=sorted([p for p in (ROOT/'src/game').rglob('*') if p.is_file()]+[ROOT/n for n in extra]+[Path(__file__).resolve()])
    report=dict(status='passed_for_water_emitters_creation_modes_frames_and_disposal' if args.emitters else 'passed_for_water_particles_copy_motion_and_frames',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Dobrý farmář.exe'),
        reference_files_unchanged=len(inventory['files']),original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths},precision_contexts=contexts,
        original_functions=['004416c0','00441770','00441790','004418f0','00441a60','00441e20','00460a90','00460b20','00460bc0','00460e30','00460a70'],
        coverage=['Actual HoseEffect.xml/SprinkleEffect.xml and WaterParticle.dds, original MaxScale decrement and original-size capture',
            'Constructor/copy resets, velocity, strict lifetime finished flag, Picture normal step, animation time, x87 asymmetric X/Y spills, size and alpha interpolation',
            'Actual original CRT rand shared sequence; random lifetime, shared gravity globals originally{0,10}, shader/draw calls and RGB pixels',
            'Native scalar destructor and source owned cleanup with exact texture reference deltas'],
        test_adapters=['Original PE only maps in diagnostic and entry point never executes; implementation contains source code only',
            'Typed XML/resource/CRT/allocator adapters, controlled UI clock/input and native rand thread data',
            'Monotonic diagnostic heap preserves equal-depth allocation ranks'],
        pending=['Actual Farm/Sprinkle/Person/cGame callers, whole game EXE and playthrough',
            'Bidirectional saves, complete audible/audio equivalence',
            'Additional locales/rounding modes, malformed/missing attributes, allocator/exception ABI and extended NaN/inf physics'])
    if args.emitters:
        report.update(states=1076,objects=4,direct_creations=512,connected_particle_states=8238,gpu_frames=352,gpu_rgb_pixels=168960000)
        report['original_functions']+=['00445420','00445540','004446c0','00442370','00442820','00441810','004440c0','00442ed0','0042ccc0','00442fa0','004433f0','00443830','00464270','00464380','004643a0','00463510','00461370','00461820','00462f10','00461d20','00461df0','00462240','00462680']
        report['coverage']+=['Actual effect XML constructors, source/native prototype ownership and prototype hide/disable/absoluteDisable while emitter remains active',
            '512 direct clone creations with exact RNG, angle, lifetime, velocity, position and Hose gravity; original x87 fcos/fsin rotation and trajectory equations',
            'Timed0.3/count4/infinite emission, stop, accumulator/created/time counters, original loop behavior, actual owned particle order/parent links and predicate cleanup',
            'Complete D3D8 frame traversal over both running effects; actual deferred ViewRegistry deletion and native scalar deletion/refcounts, original HoseEffect.cpp206/SprinkleEffect.cpp190/inl56 origins']
        report['test_adapters']+=['Original cGame particle discard service is observed; actual source and native particle deletion/refcounts are explicitly compared',
            'Diagnostic CRT decimal/concatenation adapters preserve original disposal origin strings',
            'Connected mode retains direct particle prefix but skips its separately proven GPU frames to stay within the Win32 monotonic heap limit']
        report['pending']+=['Missing Frequency preserves an unspecified native stack input; source currently reports unsupported input explicitly',
            'Empty-target count mode can retry indefinitely in original; zero/negative/NaN periods and allocation-failure retry behavior remain unverified',
            'Additional real-world start/target/angle sequences, reload and interrupted owner lifecycle']
    else:report.update(states=708,objects=12,copies=12,gpu_frames=288,gpu_rgb_pixels=138240000)
    output=args.output or ROOT/'.local'/f'{base}-validation.json';output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Water emitters: 1076 states, 512 direct creations, 352 frames / 168960000 identical RGB pixels; 17 originals unchanged' if args.emitters else 'Water particles: 708 states, 288 frames / 138240000 identical RGB pixels; 17 originals unchanged')
if __name__=='__main__':main()
