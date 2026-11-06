"""Strict original integer-status proof shared by further ABI auditors."""
import re
from audit_squall_native_arguments import number


def prove_status(audit, start, register=None):
    """Prove all EAX/C3 returns from constants in a register or entryESP-4.

    Calls may preserve the chosen Win32 nonvolatile register/caller local;
    volatile EAX, including its partial-register writes, is always invalidated.
    The caller must first run Audit.function to establish all call cleanups.
    """
    ins = audit.ins
    seen, todo, returns, evidence = set(), [(start, 0, False, False)], set(), {}
    while todo:
        address, sp, defined, eax_status = todo.pop()
        state = address, sp, defined, eax_status
        if state in seen:continue
        seen.add(state)
        assert len(seen) < 15000
        row = ins[address]
        assert row['file_verified']
        op, args = row['op'], row['args']
        parts = args.split(',')
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)
        if op == 'call':
            sp += audit.call_evidence[f'{address:08x}']['callee_pop_bytes'];eax_status = False
        elif op == 'push':sp -= 4
        elif op == 'pop':
            sp += 4
            if register is not None and args == register:defined = False
            if args == 'eax':eax_status = False
        elif op == 'add' and parts[0] == 'esp':sp += number(parts[1])
        elif register is not None and op == 'mov' and parts[0] == register:
            assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]), (hex(address), row)
            defined = True
        elif register is not None and op == 'or' and parts == [register, '0FFFFFFFFh']:defined = True
        elif op == 'mov':
            if register is None:
                match = re.fullmatch(r'dword ptr \[esp(?:\+([0-9A-F]+h|[0-9]+))?\]', parts[0])
                if match and sp + (number(match[1]) if match[1] else 0) == -4:
                    assert re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1]);defined = True
            if parts[0] == 'eax':
                match = re.fullmatch(r'dword ptr \[esp(?:\+([0-9A-F]+h|[0-9]+))?\]', parts[1])
                eax_status = defined and (parts[1] == register if register is not None else
                    bool(match and sp + (number(match[1]) if match[1] else 0) == -4))
            elif parts[0] in ['ax', 'al', 'ah']:
                eax_status = False
        elif register is not None and parts[0] == register and op not in ['test', 'cmp', 'push']:
            raise AssertionError(('Unenumerated native status write', hex(address), row))
        elif (parts[0] in ['eax', 'ax', 'al', 'ah'] or op == 'fnstsw') and op not in ['test', 'cmp', 'push']:
            eax_status = False
        if op == 'ret':
            assert sp == 0 and eax_status and not args and row['raw'] == b'\xc3', (hex(address), state)
            returns.add(f'{address:08x}')
        else:todo += [(n, sp, defined, eax_status) for n in audit.successors(address)]
    return dict(all_native_return_sites=sorted(returns), definite_assignment_states=len(seen)), evidence
