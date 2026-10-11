#!/usr/bin/env python3
"""Compare recovered resource-log files with isolated original writer instructions."""
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
    parser.add_argument('--output',type=Path,default=ROOT/'.local/resource-log-validation.json')
    args=parser.parse_args();oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();contexts=[]
    for precision,mode in [('PC53','resource-log'),('PC24','resource-log-device')]:
        result=subprocess.run([str(oracle),str(original/'Dobrý farmář.exe'),str(original/'Data'),mode],cwd=ROOT,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Resource log {precision}: {result.returncode} {result.stderr}')
        state=json.loads(result.stdout)
        assert state['resource_log_states']==31 and state['resource_log_entries']==4476 and state['resource_log_units']==250891
        assert state['resource_log_shaders']==1 and state['device_flags']==(0x22 if precision=='PC53' else 0x20)
        state['precision']=precision;contexts.append(state);unchanged()
    paths=sorted([path for path in (ROOT/'src/game').rglob('*') if path.is_file()]+[
        ROOT/'CMakeLists.txt',ROOT/'tools/text_buttons_reference_test.cpp',ROOT/'tools/text_buttons_oracle_fixups.inc',
        ROOT/'tools/resource_log_oracle_cases.inc',ROOT/'tools/resource_log_oracle_services.h',Path(__file__).resolve()])
    report=dict(status='passed_for_resource_log_writer_and_isolated_file_scope',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=digest(original/'Dobrý farmář.exe'),reference_files_unchanged=len(inventory['files']),
        original_entry_point_executed=False,original_program_code_used_in_implementation=False,
        oracle_sha256=digest(oracle),source_files={path.relative_to(ROOT).as_posix():digest(path) for path in paths},precision_contexts=contexts,
        file_comparisons=62,resource_entries=8952,utf16_units=501782,
        original_functions=['0053fdf0','0053f2d0'],
        coverage=['Exact raw files, UTF-16 without BOM, declaration/quotes/spacing/newlines and no trailing newline; empty cache and both shipped log populations in 30-entry batches',
            'Original tree iteration over balanced 48-byte node fixtures; actual source cache with textures/fonts/XML/sound, all reference counts including zero',
            'All five source resource implementations, real D3D8 textures and shader creation; diagnostic VS1.1 bytecode fixture for type 3 absent from shipped logs',
            'Output filename already cached: entry included in snapshot before source FileIndex create invalidates it; detached XML explicitly destroyed afterwards',
            'Unescaped ampersand in an actual custom XML resource filename; source/native create/write/deleting file lifetime and source clearUseless'],
        test_adapters=['Original EXE mapped without entry point; original writer and iterator execute only in diagnostic',
            'Original old-CRT stream construction/insertion/contents use explicitly checked modern wostringstream boundaries; full original CRT locale/stream ABI not asserted',
            'Original manager cache nodes are seeded from separately tracked real source resource factory calls; original native cache allocations/factory internals not asserted',
            'Native file-manager create returns explicit file-vtable adapter backed by recovered FileIndex/cRealFile in a separate isolated directory',
            'SoundManager initialization deliberately fails in this writer-only test; audio playback covered separately by game-audio comparisons'],
        pending=['Original narrow stream widening for non-ASCII resource filenames under other locales',
            'I/O failures and original exception wrapping; full native manager ABI',
            'MainMenu Start/engine/game completion and full playable runtime'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('Resource log: 62 exact files, 8952 entries, 501782 identical UTF-16 units')
if __name__=='__main__':main()
