#!/usr/bin/env python3
"""Read-only debug traces for the exact owned tracker test host."""
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

from validate_squall_tracker_stages import ROOT, digest, symbol
from squall_tracker_fixtures import generate


def wait_for(predicate, timeout=25):
    deadline = time.monotonic()+timeout
    while not predicate():
        if time.monotonic() >= deadline:
            raise TimeoutError('Diagnostic rendezvous deadline')
        time.sleep(0.02)


def main():
    work = ROOT/'.local/squall-tracker-crash'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    binaries = {'native.dll': ROOT.parent/'farmář/Dobrý farmář/Squall.dll',
                'source.dll': ROOT/'build/windows/squall-candidate/Release/Squall.dll',
                'source.map': ROOT/'build/windows/squall-candidate/Release/Squall.map',
                'squall-tracker-probe.exe': ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-probe.exe',
                'trace.exe': ROOT/'build/squall-tracker-crash-trace-vs/Release/squall-tracker-crash-trace.exe'}
    for name, path in binaries.items():
        shutil.copyfile(path, work/name)
    sources = ['tools/trace_squall_tracker_failures.py','tools/squall_tracker_probe.cpp',
               'tools/squall_tracker_crash_trace/trace.cpp','tools/squall_tracker_crash_trace/CMakeLists.txt']
    sources += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'src/squall').rglob('*')) if p.suffix in ['.h','.cpp']]
    for name in sources:
        target = work/'snapshots'/name
        target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copyfile(ROOT/name,target)
    fixtures = work/'fixtures'
    fixtures.mkdir()
    generate(fixtures)
    fixture = fixtures/'blank-title.mod'
    fixture.write_bytes(bytes(20)+(fixtures/'owned-protracker-0.mod').read_bytes()[20:])
    file_rva = symbol(work/'source.map','?api_SampleGetFile@@')
    report = dict(status='read_only_original_exceptions_not_suppressed',work_directory=work.relative_to(ROOT).as_posix(),
                  binary_hashes={name:digest(work/name) for name in binaries},
                  sources={name:digest(work/'snapshots'/name) for name in sources},cases=[])
    for index, backend in enumerate(['native']+['source']*20):
        folder = work/f'{index:02}-{backend}'
        folder.mkdir()
        state_file = folder/'ready.json'
        env = dict(os.environ,TRACKER_DEBUG_READY_FILE=str(state_file))
        command = [str(work/'squall-tracker-probe.exe'),str(work/f'{backend}.dll'),
                   '8300' if backend=='native' else file_rva,str(fixture),'mono','0','4096',str(index%2)]
        if index%4>=2:
            command.append('pc24')
        with (folder/'stdout.json').open('wb') as out, (folder/'stderr.log').open('wb') as err:
            host = subprocess.Popen(command,cwd=folder,env=env,stdout=out,stderr=err)
            trace = None
            try:
                wait_for(lambda:state_file.exists() and state_file.stat().st_size>0)
                state = json.loads(state_file.read_text(encoding='utf-8'))
                with (folder/'trace.jsonl').open('wb') as trace_out,(folder/'trace-error.log').open('wb') as trace_err:
                    trace = subprocess.Popen([str(work/'trace.exe'),str(host.pid),str(work/'squall-tracker-probe.exe')],cwd=folder,stdout=trace_out,stderr=trace_err)
                    wait_for(lambda:b'"event":"ready"' in (folder/'trace.jsonl').read_bytes())
                    Path(str(state_file)+'.go').write_bytes(b'continue\n')
                    code = host.wait(timeout=30)
                    trace_code = trace.wait(timeout=30)
            finally:
                if host.poll() is None:
                    host.kill();host.wait()
                if trace is not None and trace.poll() is None:
                    trace.kill();trace.wait()
        events = [json.loads(line) for line in (folder/'trace.jsonl').read_text(encoding='utf-8').splitlines()]
        row = dict(backend=backend,index=index,state=state,exit_code=code,trace_exit_code=trace_code,
                   events=events,directory=folder.relative_to(ROOT).as_posix(),
                   artifacts={p.name:digest(p) for p in folder.iterdir()})
        report['cases'].append(row)
        (work/'report.json').write_bytes((json.dumps(report,indent=2)+'\n').encode('utf-8'))
        print(index,backend,hex(code),flush=True)
    for name,value in report['sources'].items():
        assert digest(ROOT/name)==value,name
    (ROOT/'.local/squall-tracker-crash-validation.json').write_bytes((json.dumps(report,indent=2)+'\n').encode('utf-8'))
    print(work.relative_to(ROOT).as_posix())


if __name__=='__main__':
    main()
