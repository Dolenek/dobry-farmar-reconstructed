#!/usr/bin/env python3
"""Measure all export callers and audit every reachable original return instruction."""
import argparse,datetime,json,re,subprocess,struct,traceback
from pathlib import Path
from pe import PE
from record_live_production_verification import ROOT,digest,read,write


def native_returns(path,disassembly):
    pe=PE(path);instructions={};previous=None
    for line in disassembly.read_text(encoding='utf-8').splitlines():
        match=re.match(r'^\s*([0-9A-F]{8}):((?:\s+[0-9A-F]{2})+)\s+([a-z][a-z0-9]*)\s*(.*?)\s*$',line)
        if match:
            address,raw,op,args=match.groups();previous=int(address,16)
            instructions[previous]=dict(raw=bytes.fromhex(raw),op=op,args=args)
        elif previous is not None and re.fullmatch(r'\s+[0-9A-F]{2}(?:\s+[0-9A-F]{2})*\s*',line):
            instructions[previous]['raw']+=bytes.fromhex(line)
    for address,row in instructions.items():
        raw=row['raw'];assert pe.data[pe.offset(address-pe.image_base,len(raw)):][:len(raw)]==raw,hex(address)
    # Both public speaker switches explicitly compare decremented index with4.
    # Other indirect jumps must be reported, never guessed as fallthrough.
    tables={0x100038fe:(0x10003954,5),0x100039de:(0x10003a58,5)}
    results=[]
    for export in pe.exports():
        pending=[pe.image_base+export['rva']];visited=set();returns=[];switches=[]
        while pending:
            address=pending.pop()
            if address in visited:continue
            visited.add(address);assert address in instructions,('Missing reachable instruction',export['names'],hex(address))
            row=instructions[address];op,args,raw=row['op'],row['args'],row['raw']
            if op=='ret':
                assert raw==b'\xc3',('Callee pops arguments',export['names'],hex(address),raw.hex())
                returns.append(format(address,'08x'));continue
            if op.startswith('j') or op.startswith('loop'):
                if re.fullmatch(r'[0-9A-F]{8}',args):pending.append(int(args,16))
                else:
                    assert address in tables,('Unresolved indirect branch',export['names'],hex(address),args)
                    table,count=tables[address];targets=[pe.u32(pe.offset(table-pe.image_base+i*4)) for i in range(count)]
                    pending+=targets;switches.append(dict(address=format(address,'08x'),table=format(table,'08x'),bounded_entries=count,targets=[format(target,'08x') for target in targets]))
                if op=='jmp':continue
            assert op not in ['int','int3','ud2','iret'],('Unexpected terminating instruction',hex(address),op)
            pending.append(address+len(raw))
        assert returns
        results.append(dict(name=export['names'][0],ordinal=export['ordinal'],original_address=format(pe.image_base+export['rva'],'08x'),
            reachable_instructions=len(visited),return_addresses=sorted(returns),all_returns_caller_cleanup=True,resolved_switches=switches))
    assert len(results)==97
    return results


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/squall-export-stack-probe-vs/Release/squall-export-stack-reference-test.exe')
    parser.add_argument('--disassembly',type=Path,default=ROOT/'.local/squall-exports-native-disassembly.txt')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-export-stack-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True);native=ROOT.parent/'farmář/Dobrý farmář/Squall.dll'
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/squall-export-stack'/stamp;work.mkdir(parents=True)
    paths=list((ROOT/'src/squall').rglob('*'))+[ROOT/name for name in ['tools/validate_squall_export_stack.py','tools/squall_export_stack_reference_test.cpp','tools/squall_export_stack_probe/CMakeLists.txt','tools/squall_exports.inc','tools/pe.py']]
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),work_directory=work.relative_to(ROOT).as_posix(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in paths if p.is_file()},candidate_path=candidate.relative_to(ROOT).as_posix(),rebuilt_sha256=digest(candidate),
        oracle_path=oracle.relative_to(ROOT).as_posix(),oracle_sha256=digest(oracle),reference_sha256=digest(native),disassembly_sha256=digest(args.disassembly),precision_contexts=[])
    def unchanged():
        for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
    try:
        unchanged();report['native_return_paths']=native_returns(native,args.disassembly)
        pcm=bytes(128);wav=struct.pack('<4sI4s4sIHHIIHH4sI',b'RIFF',36+len(pcm),b'WAVE',b'fmt ',16,1,1,8000,16000,2,16,b'data',len(pcm))+pcm
        (work/'sample.wav').write_bytes(wav);report['sample_sha256']=digest(work/'sample.wav')
        names={row['name'] for row in report['native_return_paths']}
        for precision in ['PC53','PC24']:
            result=subprocess.run([str(oracle),str(native),str(candidate),*(['pc24'] if precision=='PC24' else [])],capture_output=True,cwd=work,timeout=90)
            (work/f'{precision}.stdout.log').write_bytes(result.stdout);(work/f'{precision}.stderr.log').write_bytes(result.stderr)
            assert result.returncode==0,(precision,result.returncode,result.stderr.decode('utf-8',errors='replace'),result.stdout[-1000:].decode('utf-8',errors='replace'))
            row=json.loads(result.stdout);assert row['export_count']==97 and row['export_lookups']==194 and {e['name'] for e in row['measured_exports']}==names
            for e in row['measured_exports']:assert e['source_stack_delta']==e['native_stack_delta']==0 and e['declared_stack_argument_bytes']==4*e['argument_count'] and e['stack_canaries_per_call']==16
            row['precision']=precision;report['precision_contexts'].append(row);unchanged()
        report.update(status='passed_all97_export_caller_cleanup_paths_and_measured_stack_scope_only',reference_files_unchanged=17,
            measured_calls_per_dll=194,measured_stack_canaries_per_dll=3104,production_implementation_changed=False,whole_squall_complete=False,whole_game_complete=False,
            pending=['Original parameter-count and pointer/return semantics require independent instruction/behavior evidence; declared metadata alone is insufficient','Remaining full Squall and original goal scopes'])
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        unchanged();report['logs']={p.name:digest(p) for p in work.glob('*.log')};write(work/'report.json',report);write(args.output,report)
        print(json.dumps(dict(status=report['status'],work_directory=report['work_directory'],error=report.get('error'))))
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
