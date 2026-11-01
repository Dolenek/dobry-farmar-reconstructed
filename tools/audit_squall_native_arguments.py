#!/usr/bin/env python3
"""Instruction-backed original stack-slot references; unknown operations fail closed."""
import argparse,re,struct,traceback,datetime
from pathlib import Path
from pe import PE
from record_live_production_verification import ROOT,digest,read,write

TABLES={0x100038fe:(0x10003954,5),0x100039de:(0x10003a58,5)}
# Argument pushes immediately before these original COM calls, including this.
COM_POPS={0x10003934:8,0x100039b1:8,0x100043a3:20,0x10004538:20,0x100046a3:12,0x100047f3:12,0x10004943:12,0x10004a1f:4}
IMPORT_POPS=dict(IsBadReadPtr=8,IsBadWritePtr=8,IsBadCodePtr=4,CloseHandle=4,DestroyWindow=4,Sleep=4,timeGetTime=0,GetCurrentThreadId=0,WaitForSingleObject=8,ReleaseMutex=4,wsprintfA=0,CreateMutexA=12)
REGS=['eax','ebx','ecx','edx','esi','edi','ebp']


def number(text):
    text=text.strip();return int(text[:-1],16) if text.endswith('h') else int(text,10)


def instructions(path,pe):
    result={};previous=None
    for line in path.read_text(encoding='utf-8').splitlines():
        match=re.match(r'^\s*([0-9A-F]{8}):((?:\s+[0-9A-F]{2})+)\s+([a-z][a-z0-9]*)\s*(.*?)\s*$',line)
        if match:
            address,raw,op,args=match.groups();previous=int(address,16);result[previous]=dict(raw=bytes.fromhex(raw),op=op,args=args)
        elif previous is not None and re.fullmatch(r'\s+[0-9A-F]{2}(?:\s+[0-9A-F]{2})*\s*',line):result[previous]['raw']+=bytes.fromhex(line)
    for address,row in result.items():
        # dumpbin also decodes non-file-backed tail bytes. Retain only actual
        # original instruction bytes, never accept a fabricated zero tail.
        try:raw=pe.data[pe.offset(address-pe.image_base,len(row['raw'])):][:len(row['raw'])]
        except Exception:raw=None
        row['file_verified']=raw==row['raw']
    return result


class Audit:
    def __init__(self,pe,ins):
        self.pe=pe;self.ins=ins;self.cleanups={};self.call_evidence={};self.cleanup_returns={}
        self.imports={pe.image_base+s['iat_rva']:s.get('name',str(s.get('ordinal'))) for dll in pe.imports() for s in dll['symbols']}

    def successors(self,address):
        row=self.ins[address];op,args=row['op'],row['args'];targets=[]
        if op=='ret':return []
        if op.startswith('j') or op.startswith('loop'):
            if re.fullmatch('[0-9A-F]{8}',args):targets.append(int(args,16))
            else:
                assert address in TABLES,('Unresolved branch',hex(address),args)
                table,count=TABLES[address];targets += [self.pe.u32(self.pe.offset(table-self.pe.image_base+i*4)) for i in range(count)]
            if op=='jmp':return targets
        return targets+[address+len(row['raw'])]

    def cleanup(self,start):
        if start in self.cleanups:return self.cleanups[start]
        seen=set();todo=[start];pops=set();returns=[]
        while todo:
            address=todo.pop()
            if address in seen:continue
            seen.add(address);row=self.ins[address];assert row['file_verified'],hex(address)
            if row['op']=='ret':
                pops.add(number(row['args']) if row['args'] else 0)
                returns.append(dict(address=format(address,'08x'),instruction_bytes=row['raw'].hex(),pop_bytes=number(row['args']) if row['args'] else 0))
            else:todo+=self.successors(address)
        assert len(pops)==1,('Inconsistent callee cleanup',hex(start),pops)
        value=pops.pop();self.cleanups[start]=value;self.cleanup_returns[format(start,'08x')]=sorted(returns,key=lambda row:row['address']);return value

    def call_pop(self,address,registers):
        args=self.ins[address]['args']
        if re.fullmatch('[0-9A-F]{8}',args):
            target=int(args,16);pop=self.cleanup(target);kind='direct_ret_instruction';description=format(target,'08x')
        elif address in COM_POPS:pop=COM_POPS[address];kind='native_com_argument_pushes';description=args
        else:
            match=re.search(r'\[([0-9A-F]{8})h\]',args)
            name=self.imports[int(match[1],16)] if match else registers.get(args)
            assert name in IMPORT_POPS,('Unresolved call ABI',hex(address),args,name)
            pop=IMPORT_POPS[name];kind='win32_import_contract';description=name
        self.call_evidence[format(address,'08x')]=dict(kind=kind,target=description,callee_pop_bytes=pop)
        return pop

    def function(self,start):
        initial=(start,0,None,(None,)*len(REGS));todo=[initial];seen=set();references={};returns=[]
        while todo:
            address,sp,bp,tags=todo.pop();state=(address,sp,bp,tags)
            if state in seen:continue
            seen.add(state);assert len(seen)<20000,('State explosion',hex(start))
            row=self.ins[address];assert row['file_verified'],hex(address)
            op,args=row['op'],row['args'];registers=dict(zip(REGS,tags))
            for match in re.finditer(r'\[(esp|ebp)(?:([+-])([0-9A-F]+h|[0-9]+))?\]',args):
                reg,sign,disp=match.groups();base=sp if reg=='esp' else bp
                if base is None:continue # EBP is also an ordinary nonvolatile pointer register.
                offset=base+(number(disp)*(1 if sign=='+' else -1) if disp else 0)
                if offset>=4:
                    size=8 if 'qword ptr' in args else 10 if 'tbyte ptr' in args else 2 if re.search(r'(?<!d)word ptr',args) else 1 if 'byte ptr' in args else 4
                    references.setdefault(format(address,'08x'),set()).add((offset,size,op,args))
            operands=[part.strip() for part in args.split(',')]
            if op=='push':sp-=4
            elif op=='pop':
                sp+=4
                if args=='ebp':bp=None
                if args in registers:registers[args]=None
            elif op in ['sub','add'] and operands[0]=='esp':sp+=(1 if op=='add' else -1)*number(operands[1])
            elif op=='mov' and operands[0]=='ebp':bp=sp if operands[1]=='esp' else None
            elif op=='mov' and operands[0]=='esp':assert operands[1]=='ebp' and bp is not None,(hex(address),args);sp=bp
            elif op=='leave':assert bp is not None;sp=bp+4;bp=None
            elif op=='call':
                sp+=self.call_pop(address,registers)
                for reg in ['eax','ecx','edx']:registers[reg]=None
            elif op=='ret':
                assert sp==0,('Unbalanced original return',hex(start),hex(address),sp)
                returns.append(format(address,'08x'));continue
            elif re.match(r'esp(,|$)',args) and op not in ['cmp','test','nop']:
                raise AssertionError(('Unhandled stack register change',hex(address),op,args))
            # Track cached Win32 import pointers through nonvolatile registers.
            if op=='mov' and operands[0] in registers:
                source=operands[1];match=re.fullmatch(r'dword ptr ds:\[([0-9A-F]{8})h\]',source)
                registers[operands[0]]=self.imports.get(int(match[1],16)) if match else registers.get(source)
            elif operands[0] in registers and op not in ['push','cmp','test','call','jmp']:
                registers[operands[0]]=None
            for target in self.successors(address):todo.append((target,sp,bp,tuple(registers[reg] for reg in REGS)))
        assert returns
        rows=[dict(address=address,entry_esp_offset=offset,bytes=size,op=op,operands=args) for address,values in sorted(references.items()) for offset,size,op,args in sorted(values)]
        highest=max((r['entry_esp_offset']+r['bytes']-1 for r in rows),default=0)
        slots=sorted({slot for row in rows for slot in range(row['entry_esp_offset']//4,(row['entry_esp_offset']+row['bytes']-1)//4+1)})
        return dict(referenced_argument_slots=highest//4,explicit_argument_slots=slots,stack_references=rows,balanced_return_addresses=sorted(set(returns)),symbolic_states=len(seen))


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-native-arguments-audit.json');args=parser.parse_args()
    native=ROOT.parent/'farmář/Dobrý farmář/Squall.dll';disassembly=ROOT/'.local/squall-complete-native-disassembly.txt';pe=PE(native);ins=instructions(disassembly,pe);audit=Audit(pe,ins)
    declared={row['name']:row for row in read(ROOT/'analysis/squall-export-stack-validation.json')['precision_contexts'][0]['measured_exports']}
    report=dict(status='running',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),auditor_sha256=digest(__file__),original_sha256=digest(native),disassembly_sha256=digest(disassembly),exports=[])
    try:
        for export in pe.exports():
            name=export['names'][0];row=dict(name=name,original_address=format(pe.image_base+export['rva'],'08x'),declared_argument_count=declared[name]['argument_count'])
            report['exports'].append(row);row.update(audit.function(pe.image_base+export['rva']))
            assert row['referenced_argument_slots']==row['declared_argument_count'],('Argument count mismatch',name,row['referenced_argument_slots'],row['declared_argument_count'])
            assert row['explicit_argument_slots']==list(range(1,row['declared_argument_count']+1)),('Missing declared slot reference',name)
        report['status']='passed_all97_native_referenced_argument_slots_only'
    except Exception as error:report.update(status='failed',error=repr(error),traceback=traceback.format_exc())
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
    report.update(call_cleanup_evidence=audit.call_evidence,direct_callee_cleanup_summaries={format(k,'08x'):v for k,v in audit.cleanups.items()},direct_callee_return_evidence=audit.cleanup_returns,
        declared_contract_report='analysis/squall-export-stack-validation.json',declared_contract_report_sha256=digest(ROOT/'analysis/squall-export-stack-validation.json'),
        reference_files_unchanged=17,source_files={name:digest(ROOT/name) for name in ['tools/audit_squall_native_arguments.py','tools/pe.py']},whole_native_signature_semantics_verified=False,whole_game_complete=False)
    write(args.output,report);print(report['status']);print(report.get('error','all97 original argument slot counts agree'))
    return int(report['status']=='failed')


if __name__=='__main__':raise SystemExit(main())
