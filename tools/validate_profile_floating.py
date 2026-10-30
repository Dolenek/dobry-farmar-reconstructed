#!/usr/bin/env python3
"""Observe actual old-CRT profile parsing/formatting, using bounded diagnostic-only raw words."""
import argparse,ctypes as C,datetime,hashlib,json,math,random,re,shutil,struct,subprocess,time,traceback,xml.etree.ElementTree as ET
from ctypes import wintypes as W
from pathlib import Path
from windows_game_gui import GameGui,k,u
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/profile-floating-probe.exe')
    p.add_argument('--output',type=Path,default=ROOT/'.local/profile-floating-validation.json')
    p.add_argument('--require-match',action='store_true')
    p.add_argument('--trace',type=Path,help='Read-only original parser entry hardware trace')
    p.add_argument('--extended',action='store_true',help='Also compare200 finite values in6/17-digit decimal spelling')
    p.add_argument('--random-values',type=int,default=200,help='Number of finite values for the extended corpus')
    p.add_argument('--native-precision',choices=['PC24','PC53'],default='PC24',help='Actual game precision, or explicit parser-only FP diagnostic fixture')
    args=p.parse_args();oracle=args.oracle.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář';data=original/'Data'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'.local/profile-floating'/stamp;work.mkdir(parents=True)
    shutil.copyfile(__file__,work/'driver.py')
    runtime=work/'runtime';runtime.mkdir();shutil.copytree(data,runtime/'Data')
    common=work/'shared/VirtualFarm';common.mkdir(parents=True)
    fixture=json.loads((ROOT/'.local/paid-production-state-validation.json').read_text(encoding='utf-8'))
    shutil.copytree(ROOT/fixture['work_directory']/'0-persisted/Profiles',common/'Profiles')
    rng=random.Random(0x31415926)
    float_bits=[0,0x80000000,1,0x80000001,0x007fffff,0x00800000,0x3f800000,0xbf800000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7f800001,0xff800001,0x7fc00000,0xffc00000,0x7fffffff,0xffffffff]+[rng.getrandbits(32) for _ in range(80)]
    double_bits=[0,0x8000000000000000,1,0x8000000000000001,0x000fffffffffffff,0x0010000000000000,0x3ff0000000000000,0xbff0000000000000,0x7fefffffffffffff,0xffefffffffffffff,0x7ff0000000000000,0xfff0000000000000,0x7ff0000000000001,0xfff0000000000001,0x7ff8000000000000,0xfff8000000000000,0x7fffffffffffffff,0xffffffffffffffff]+[rng.getrandbits(64) for _ in range(80)]
    literals=['','0','-0','-0.0','  -0.0','inf','-inf','infinity','1.#INF','-1.#INF','nan','nan(123)','1.#QNAN','-1.#QNAN','1.#SNAN','-1.#IND','1e9999','-1e9999','1e-9999','-1e-9999','1.25junk','+','-','1.23456789','3.4028234663852886e38','1.7976931348623157e308','5e-324','1e-45','1e+','1e-','1e','.','-.','1,25','0x1p+1','0x1.8p+2','1e-308','-1e-308','2.2250738585072014e-308','2.225073858507201e-308','1e-307','1e-46','-1e-46','3.4028236e38','-3.4028236e38','1.7976931348623159e308']
    literals+=['2.2250738585072011e-308','2.2250738585072012e-308','2.2250738585072013e-308','2.2250738585072015e-308']
    literals+=['.00123','0.000123','-000.000123','00000.00000','1.23456789012345678901234567890123456789012345',
        '123456789012345678901234567890123456789012345','123456789012345678901234567890123456789012345e-10',
        '.00000000000000000000000000000000000001234567890123456789','1e00000000000000000000000002',
        '1e00000000000000000000000000','1e123456789012','1e-123456789012','0.0001e-304','0.0001e-305']
    if args.extended:
        values=[]
        while len(values)<args.random_values:
            value=struct.unpack('<d',rng.getrandbits(64).to_bytes(8,'little'))[0]
            if math.isfinite(value):values.append(value)
        literals += [format(value,spec) for value in values for spec in ['.6g','.17g']]
    cases=[]
    for mode,words,width in [('F',float_bits,8),('D',double_bits,16)]:
        for i,word in enumerate(words):cases.append(dict(mode=mode,key=f'FormatProbe_{mode}{i:03}',payload=f'{word:0{width}x}'))
    for mode in ['PF','PD']:
        for i,literal in enumerate(literals):cases.append(dict(mode=mode,key=f'FormatProbe_{mode}{i:03}',payload=literal))
    profile=common/'Profiles/00000001.xml';tree=ET.parse(profile)
    for case in cases:ET.SubElement(tree.getroot(),'prop',Name=case['key'],Type='Float' if case['mode'] in ['F','PF'] else 'Double',Val=case['payload'] if case['mode'].startswith('P') else '0')
    tree.write(profile,encoding='utf-16',xml_declaration=True)
    source_input=work/'fixture.tsv';source_input.write_text(''.join(f"{case['mode']}\t{case['key']}\t{case['payload']}\n" for case in cases),encoding='ascii')
    probe=GameGui(work,data);settings=probe.resource('sys/settings.xml');settings.find('graphics').set('fullscreen','0');(common/'sys').mkdir();ET.ElementTree(settings).write(common/'sys/settings.xml',encoding='utf-16',xml_declaration=True)
    launcher_path=ROOT/'build/native-process/Release/native-process-launcher.exe'
    report=dict(status='running',work_directory=work.relative_to(ROOT).as_posix(),recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        source_files={path.relative_to(ROOT).as_posix():digest(path) for path in list((ROOT/'src/game/profiles').glob('*'))+[ROOT/'tools/profile_floating_probe.cpp',ROOT/'tools/validate_profile_floating.py',ROOT/'CMakeLists.txt'] if path.is_file()},
        oracle_sha256=digest(oracle),reference_sha256=digest(original/'Dobrý farmář.exe'),cases=cases,diagnostic_profile_keys_only=True,
        original_program_code_used_in_implementation=False,reference_streams_unmodified=True,raw_words_written_only_in_owned_reference_process=True,
        windows_numeric_runtime=dict(path=str(Path('C:/Windows/SysWOW64/msvcrt.dll')),sha256=digest(Path('C:/Windows/SysWOW64/msvcrt.dll'))),
        extended_random_values=args.random_values if args.extended else 0,
        native_precision=args.native_precision,diagnostic_native_fp_context_changed=args.native_precision=='PC53')
    assert args.native_precision=='PC24' or args.trace,'PC53 requires the explicit parser FP fixture tracer'
    process=None;handle=None;hwnd=None;launcher=None;trace=None;trace_reader=None;trace_file=None;trace_errors=None
    k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE
    k.GetExitCodeProcess.argtypes=[W.HANDLE,C.POINTER(W.DWORD)];k.CloseHandle.argtypes=[W.HANDLE]
    k.ReadProcessMemory.argtypes=[W.HANDLE,C.c_void_p,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t)]
    k.WriteProcessMemory.argtypes=[W.HANDLE,C.c_void_p,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t)]
    class Child:
        def __init__(self,pid):self.pid=pid
        def poll(self):
            code=W.DWORD();assert k.GetExitCodeProcess(handle,C.byref(code));return None if code.value==259 else code.value
        def wait(self,timeout=20):
            end=time.monotonic()+timeout
            while time.monotonic()<end:
                code=self.poll()
                if code is not None:return code
                time.sleep(.05)
            raise RuntimeError('Owned reference did not exit')
    def read(address,size):
        buffer=C.create_string_buffer(size);done=C.c_size_t();assert k.ReadProcessMemory(handle,address,buffer,size,C.byref(done)) and done.value==size;return buffer.raw
    def word(address):return struct.unpack('<I',read(address,4))[0]
    def value_address(key,single):
        head=word((0x65ff38 if single else 0x660464)+4);node=word(head+4);visited=set();wanted=key.encode('ascii');nil=45 if single else 57;key_offset=16 if single else 20
        while not read(node+nil,1)[0]:
            assert node not in visited and len(visited)<64;visited.add(node)
            size,capacity=struct.unpack('<II',read(node+key_offset+16,8));assert size<=1024 and size<=capacity
            raw=read(word(node+key_offset) if capacity>=16 else node+key_offset,size)
            if raw==wanted:return node+(40 if single else 48)
            node=word(node+(0 if wanted<raw else 8))
        raise KeyError(key)
    try:
        launcher=subprocess.Popen([str(launcher_path),str(original/'Dobrý farmář.exe'),str(runtime),str(work/'shared')],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        ready=launcher.stdout.readline();assert ready,launcher.communicate();event=json.loads(ready);assert event['event']=='ready';report['launcher']=event
        handle=k.OpenProcess(0x100000|0x1000|0x38,False,event['pid']);assert handle;process=Child(event['pid']);probe.process=process
        k.QueryFullProcessImageNameW.argtypes=[W.HANDLE,W.DWORD,W.LPWSTR,C.POINTER(W.DWORD)]
        path=C.create_unicode_buffer(32768);length=W.DWORD(32768);assert k.QueryFullProcessImageNameW(handle,0,path,C.byref(length)) and Path(path.value).resolve()==(original/'Dobrý farmář.exe').resolve()
        if args.trace:
            import threading
            trace_file=(work/'parser-trace.jsonl').open('w',encoding='utf-8');trace_errors=(work/'parser-trace.stderr.log').open('w',encoding='utf-8')
            trace=subprocess.Popen([str(args.trace.resolve(strict=True)),str(process.pid),str(original/'Dobrý farmář.exe'),*(['pc53'] if args.native_precision=='PC53' else [])],cwd=ROOT,stdout=subprocess.PIPE,stderr=trace_errors,text=True)
            trace_ready=trace.stdout.readline();trace_file.write(trace_ready);trace_file.flush();assert trace_ready and json.loads(trace_ready)['event']=='ready',trace_ready
            def copy_trace():
                for line in trace.stdout:trace_file.write(line);trace_file.flush()
            trace_reader=threading.Thread(target=copy_trace,daemon=True);trace_reader.start();report['parser_tracer_sha256']=digest(args.trace)
        deadline=time.monotonic()+20
        while time.monotonic()<deadline:
            probe.check_process();found=[row for row in probe.windows(process.pid) if row[2].lower().startswith('melesta_')]
            if found:hwnd=W.HWND(found[0][0]);break
            time.sleep(.1)
        assert hwnd;probe.hwnd=hwnd;probe.focus()
        for _ in range(4):probe.scan(0x3c)
        deadline=time.monotonic()+45;last_error=None
        while True:
            probe.focus();probe.check_process()
            try:value_address(cases[-1]['key'],False);break
            except (AssertionError,KeyError,OSError) as error:last_error=repr(error)
            assert time.monotonic()<deadline,('Diagnostic profile not loaded',last_error)
            probe.scan(0x3c);time.sleep(.25)
        probe.settle('native-menu.png')
        # Only the injected diagnostic keys below are writable; no game property
        # or instruction/stream implementation is patched by this probe.
        for case in cases:
            single=case['mode'] in ['F','PF'];size=4 if single else 8;address=value_address(case['key'],single)
            before=read(address,size);case['native_parsed_bits']=before.hex()
            if not case['mode'].startswith('P'):
                assert before==b'\0'*size and case['key'].startswith('FormatProbe_')
                raw=int(case['payload'],16).to_bytes(size,'little');buffer=C.create_string_buffer(raw);done=C.c_size_t()
                assert k.WriteProcessMemory(handle,address,buffer,size,C.byref(done)) and done.value==size and read(address,size)==raw
                case['native_written_bits']=raw.hex()
        u.PostMessageW(hwnd,0x10,0,0);assert process.wait()==0;report['native_exit_code']=0
        launcher.wait(timeout=10);out,err=launcher.communicate();assert launcher.returncode==0,(out,err)
        if trace:
            assert trace.wait(timeout=10)==0;trace_reader.join(timeout=5);assert not trace_reader.is_alive();trace_file.close();trace_errors.close()
            trace_rows=[json.loads(line) for line in (work/'parser-trace.jsonl').read_text(encoding='utf-8').splitlines()]
            assert trace_rows[-1]['event']=='exit' and trace_rows[-1]['exit_code']==0
            entries={row['key']:row for row in trace_rows if row['event']=='parse'};assert set(entries)=={case['key'] for case in cases},(len(entries),len(cases))
            assert {row['fp_control']&0x300 for row in entries.values()}=={0x200 if args.native_precision=='PC53' else 0}
            results={row['key']:row for row in trace_rows if row['event']=='result'};assert set(results)==set(entries),(len(results),len(entries))
            report['parser_entries']=entries;report['parser_results']=results;report['parser_trace_sha256']=digest(work/'parser-trace.jsonl')
            source_input.write_text(''.join(f"{case['mode']}\t{case['key']}\t{case['payload']}\t{entries[case['key']]['seed']}\n" for case in cases),encoding='ascii')
        native_text=profile.read_bytes().decode('utf-16');native_values={name:value for name,value in re.findall(r'<prop\s+Name="([^"]*)"\s+Val="([^"]*)"',native_text)}
        sources=[]
        for precision in [args.native_precision]:
            prefix=work/precision;result=subprocess.run([str(oracle),str(source_input),str(prefix),*(['pc24'] if precision=='PC24' else [])],capture_output=True,timeout=15)
            assert result.returncode==0,(result.returncode,result.stderr)
            actual_text=(prefix.with_suffix('.xml')).read_bytes().decode('utf-16');actual_values={name:value for name,value in re.findall(r'<prop\s+Name="([^"]*)"\s+Val="([^"]*)"',actual_text)}
            seeded_text=Path(str(prefix)+'.seeded.xml').read_bytes().decode('utf-16');seeded_values={name:value for name,value in re.findall(r'<prop\s+Name="([^"]*)"\s+Val="([^"]*)"',seeded_text)}
            own={row[0]:(row[1],row[2],row[3]) for row in (line.split('\t') for line in prefix.with_suffix('.tsv').read_text(encoding='ascii').splitlines())}
            rows=[]
            for case in cases:
                key=case['key'];bits,legacy,seeded_bits=own[key];native_bits=case.get('native_written_bits',case['native_parsed_bits']);native_bits_display=bytes.fromhex(native_bits)[::-1].hex()
                failed=bool(args.trace and (report['parser_results'][key].get('empty_skipped') or (report['parser_results'][key]['state'] or 0)&6))
                rows.append(dict(key=key,mode=case['mode'],native_bits=native_bits_display,source_bits=bits,same_seed_source_bits=seeded_bits,native_text=native_values[key],source_text=actual_values[key],same_seed_source_text=seeded_values[key],legacy_text=legacy,native_failed_or_empty=failed))
            sources.append(dict(precision=precision,rows=rows,parse_differences=sum(row['native_bits']!=row['source_bits'] for row in rows),format_differences=sum(row['native_text']!=row['source_text'] for row in rows),raw_format_differences=sum(row['native_text']!=row['source_text'] for row in rows if row['mode'] in ['F','D']),same_seed_parse_differences=sum(row['native_bits']!=row['same_seed_source_bits'] for row in rows),same_seed_format_differences=sum(row['native_text']!=row['same_seed_source_text'] for row in rows),assigned_parse_differences=sum(row['native_bits']!=row['source_bits'] for row in rows if not row['native_failed_or_empty']),legacy_format_differences=sum(row['native_text']!=row['legacy_text'] for row in rows if row['mode'] in ['F','D'])))
        report['precision_contexts']=sources;report['status']='observed_actual_native_profile_floating_formats'
        report['native_profile_sha256']=digest(profile);report['fixture_sha256']=digest(source_input)
        if args.require_match:
            assert args.trace,'Matching parser destination words requires the native entry trace'
            assert all(row['same_seed_parse_differences']==row['same_seed_format_differences']==row['assigned_parse_differences']==row['raw_format_differences']==0 for row in sources),[(row['precision'],row['same_seed_parse_differences'],row['assigned_parse_differences']) for row in sources]
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    finally:
        if process and process.poll() is None:
            if hwnd:u.PostMessageW(hwnd,0x10,0,0)
            process.wait()
        if handle:k.CloseHandle(handle)
        unchanged();report['reference_files_unchanged']=17;report['whole_game_complete']=False
        encoded=json.dumps(report,ensure_ascii=False,indent=2)+'\n';(work/'report.json').write_text(encoded,encoding='utf-8');args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(encoded,encoding='utf-8')
        print(json.dumps(dict(status=report['status'],error=report.get('error'),work_directory=str(work),summary=[{key:row[key] for key in ['precision','parse_differences','format_differences','raw_format_differences','same_seed_parse_differences','same_seed_format_differences','assigned_parse_differences','legacy_format_differences']} for row in report.get('precision_contexts',[])])))
    return int(report['status']=='failed')
if __name__=='__main__':raise SystemExit(main())
