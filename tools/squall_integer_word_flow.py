"""Conservative numeric-word flow for original public/backend return graphs."""
import re
from audit_squall_native_arguments import REGS, number


def prove_integer_word(audit, start, numeric_calls=(), numeric_memory=()):
    ins = audit.ins
    todo, seen, returns, evidence = [(start, (False,) * len(REGS))], set(), set(), {}
    aliases = dict(ax='eax', al='eax', ah='eax', bx='ebx', bl='ebx', bh='ebx',
                   cx='ecx', cl='ecx', ch='ecx', dx='edx', dl='edx', dh='edx')
    while todo:
        address, tags = todo.pop();state = address, tags
        if state in seen:continue
        seen.add(state);assert len(seen) < 50000
        row = ins[address];assert row['file_verified']
        op, args = row['op'], row['args'];parts = args.split(',')
        regs = dict(zip(REGS, tags))
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

        def value(operand):
            if operand in regs:return regs[operand]
            if address in numeric_memory and operand.startswith('dword ptr '):return True
            if operand == 'dword ptr ds:[10075578h]':return True # actual sample-slot count
            if re.fullmatch(r'[0-9A-F]+h|[0-9]+', operand):
                integer = number(operand)
                return integer < 0x10000000 or integer >= 0xffff0000
            return False

        if op == 'call':
            for reg in ['eax', 'ecx', 'edx']:regs[reg] = False
            regs['eax'] = bool(re.fullmatch(r'[0-9A-F]{8}', args) and int(args, 16) in numeric_calls)
        elif op == 'mov':
            if parts[0] in regs:regs[parts[0]] = value(parts[1])
            elif parts[0] in aliases:
                # An already established integer remains an integer after a
                # byte/word change. Unknown full pointer/float words stay unknown.
                pass
        elif op == 'lea' and parts[0] in regs:
            inputs = re.findall(r'\b(?:eax|ebx|ecx|edx|esi|edi|ebp)\b', parts[1])
            regs[parts[0]] = bool(inputs and all(regs[r] for r in inputs))
        elif op == 'pop' and args in regs:regs[args] = False
        elif op in ['xor', 'sbb'] and parts[0] == parts[1] and parts[0] in regs:regs[parts[0]] = True
        elif op == 'or' and parts[1:] == ['0FFFFFFFFh'] and parts[0] in regs:regs[parts[0]] = True
        elif op in ['add', 'sub', 'and', 'or', 'xor', 'sbb'] and parts[0] in regs:
            regs[parts[0]] = regs[parts[0]] and value(parts[1])
        elif op in ['inc', 'dec', 'neg', 'not', 'shl', 'shr', 'sar'] and parts[0] in regs:
            pass # Integer arithmetic cannot make an unknown word known.
        elif op in ['inc', 'dec', 'neg', 'not', 'shl', 'shr', 'sar', 'and', 'or', 'xor'] and parts[0] in aliases:
            pass # A partial-register change also preserves only existing knowledge.
        elif op == 'cdq':regs['edx'] = regs['eax']
        elif op == 'fnstsw':regs['eax'] = False
        elif parts[0] in regs or parts[0] in aliases:
            assert op in ['test', 'cmp', 'push'], ('Unenumerated register operation', hex(address), row)
        if op == 'ret':
            assert regs['eax'], ('Unknown/noninteger original return', hex(start), hex(address), state)
            returns.add(f'{address:08x}')
        else:todo += [(n, tuple(regs[r] for r in REGS)) for n in audit.successors(address)]
    return dict(all_native_return_sites=sorted(returns), integer_word_states=len(seen)), evidence
