#!/usr/bin/env python3
"""Prove ten numeric channel-handle interfaces from the original instruction graph."""
import re
from pathlib import Path
from pe import PE
from audit_squall_native_arguments import instructions, Audit, REGS, number
from record_live_production_verification import ROOT, digest
from squall_signature_evidence import publish

METHODS = [('Start', 0x10005310), ('Stop', 0x10005430), ('Status', 0x100054b0),
           ('GetVolume', 0x10005620), ('GetFrequency', 0x100057a0),
           ('GetPlayPosition', 0x100058d0), ('GetPlayPositionMs', 0x10005a30),
           ('GetPriority', 0x10006210), ('GetLoop', 0x10006350), ('GetPan', 0x100064a0)]
OUTPUT_CALLS = {0x10001fb0, 0x10002010, 0x10002180, 0x10002080}
INTEGER_HELPERS = {0x1003d254, 0x1003d330, 0x1003d280}


class Proof:
    def __init__(self, audit):
        self.audit, self.ins = audit, audit.ins

    def run(self, start):
        # I=integer word, H=unmodified public handle, P=address, L:n=local
        # address, U=unknown. Unknown DWORD loads never establish a return type.
        todo = [(start, 0, ('U',) * len(REGS), ((4, 'H'),))]
        seen, returns, handles = set(), set(), set()
        while todo:
            address, sp, tags, memory = todo.pop()
            state = address, sp, tags, memory
            if state in seen:
                continue
            seen.add(state)
            assert len(seen) < 12000
            row = self.ins[address]
            assert row['file_verified']
            op, args = row['op'], row['args']
            parts = args.split(',')
            regs, stack = dict(zip(REGS, tags)), dict(memory)

            def local(operand):
                match = re.fullmatch(r'(?:dword ptr )?\[esp(?:\+([0-9A-F]+h|[0-9]+))?\]', operand)
                return sp + (number(match[1]) if match[1] else 0) if match else None

            def value(operand):
                if operand in regs:
                    return regs[operand]
                offset = local(operand)
                if offset is not None:
                    return stack.get(offset, 'U')
                if re.fullmatch(r'[0-9A-F]+h|[0-9]+', operand):
                    return 'P' if 0x10000000 <= number(operand) < 0x10100000 else 'I'
                if operand == 'dword ptr ds:[10075134h]':
                    return 'P'  # actual channel-array global, not a return value
                if address in [0x1000626b, 0x10005ad1]:
                    return 'I'  # independently witnessed priority/rate fields
                return 'U'

            def release(count):
                nonlocal sp
                assert count >= 0 and count % 4 == 0
                for offset in range(sp, sp + count, 4):
                    stack.pop(offset, None)
                sp += count

            if op == 'push':
                tag = value(args)
                sp -= 4
                stack[sp] = tag
            elif op == 'pop':
                assert args in regs
                regs[args] = stack.get(sp, 'U')
                release(4)
            elif op in ['add', 'sub'] and parts[0] == 'esp':
                amount = number(parts[1])
                if op == 'add':
                    release(amount)
                else:
                    sp -= amount
            elif op == 'mov':
                tag = value(parts[1])
                if parts[0] in regs:
                    regs[parts[0]] = tag
                else:
                    offset = local(parts[0])
                    assert offset is not None, ('Unenumerated store', hex(address), args)
                    stack[offset] = tag
            elif op == 'lea':
                offset = local(parts[1])
                if offset is not None:
                    tag = 'L:' + str(offset)
                else:
                    inputs = [regs[r] for r in re.findall(r'\b(?:eax|ebx|ecx|edx|esi|edi|ebp)\b', parts[1])]
                    tag = 'I' if inputs and all(t in ['I', 'H'] for t in inputs) else 'P' if 'P' in inputs else 'U'
                regs[parts[0]] = tag
            elif op == 'call':
                target = int(args, 16)
                result = 'U'
                if target == 0x10002480:
                    assert stack[sp] == 'H', ('Handle changed before resolution', hex(address), state)
                    handles.add(f'{address:08x}')
                    result = 'I'
                elif target == 0x10007e40:
                    result = 'I'  # independently proved GetNumDevice tail
                elif target in INTEGER_HELPERS:
                    if target != 0x1003d254:
                        assert all(stack[sp + n] in ['I', 'H'] for n in range(0, 16, 4))
                    result = 'I'
                elif target in OUTPUT_CALLS:
                    pointer = stack[sp]
                    assert pointer.startswith('L:')
                    offset = int(pointer[2:])
                    assert stack[offset] == 'I'  # caller initializes even failure paths
                    stack[offset] = 'I'
                    result = 'I'
                for reg in ['eax', 'ecx', 'edx']:
                    regs[reg] = 'U'
                regs['eax'] = result
                if target in [0x1003d330, 0x1003d280]:
                    regs['edx'] = 'I'
                release(self.audit.call_pop(address, {r: None for r in REGS}))
            elif op in ['xor', 'sbb'] and parts[0] == parts[1]:
                regs[parts[0]] = 'I'
            elif op == 'and':
                # Complete DWORD bit-field extraction or partial-AL masking of
                # an already established integer; neither is a pointer return.
                destination = 'eax' if parts[0] == 'al' else parts[0]
                assert destination in regs and re.fullmatch(r'[0-9A-F]+h|[0-9]+', parts[1])
                if parts[0] == 'al':
                    assert regs['eax'] == 'I'
                regs[destination] = 'I'
            elif op == 'or' and parts[1] == '0FFFFFFFFh':
                regs[parts[0]] = 'I'
            elif op == 'shr' and regs[parts[0]] == 'U':
                # Shifting an unknown full word keeps it unknown. The later
                # explicit one-bit mask establishes the public loop value.
                pass
            elif op in ['add', 'sub', 'shl', 'shr', 'sar', 'neg']:
                assert regs[parts[0]] in ['I', 'H'], (hex(address), op, args, regs)
                regs[parts[0]] = 'I'
            elif op == 'cdq':
                assert regs['eax'] == 'I'
                regs['edx'] = 'I'
            elif op == 'ret':
                assert sp == 0 and not args and row['raw'] == b'\xc3' and regs['eax'] == 'I', (hex(start), hex(address), state)
                returns.add(f'{address:08x}')
                continue
            else:
                assert op in ['test', 'cmp', 'fld', 'fild', 'fmul', 'fsub'] or op.startswith('j'), (hex(address), row)
            todo += [(n, sp, tuple(regs[r] for r in REGS), tuple(sorted(stack.items()))) for n in self.audit.successors(address)]
        return dict(all_native_return_sites=sorted(returns), actual_handle_resolver_calls=sorted(handles), numeric_dataflow_states=len(seen))


def main():
    native = ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Squall.dll'
    path = ROOT / '.local/squall-complete-native-disassembly.txt'
    pe = PE(native)
    ins = instructions(path, pe)
    audit = Audit(pe, ins)
    evidence = {}

    def witness(address, op, args):
        row = ins[address]
        assert row['file_verified'] and row['op'] == op and row['args'] == args, (hex(address), row)
        evidence[address] = dict(address=f'{address:08x}', bytes=row['raw'].hex(), op=op, operands=args)

    for row in [
        (0x10002480, 'mov', 'edx,dword ptr [esp+4]'), (0x1000248b, 'mov', 'eax,edx'),
        (0x1000248d, 'and', 'eax,0FFh'), (0x10002492, 'cmp', 'eax,esi'),
        (0x10002494, 'jge', '100024B9'), (0x1000249f, 'sar', 'edx,8'),
        (0x100024af, 'cmp', 'ecx,edx'), (0x100024b3, 'test', 'byte ptr [esi+48h],7'),
        (0x100024b9, 'or', 'eax,0FFFFFFFFh'), (0x100024bd, 'ret', '4'),
        # Signed priority setter -> exact same full DWORD getter field.
        (0x10006195, 'mov', 'esi,dword ptr [esp+10h]'), (0x10006199, 'test', 'esi,esi'),
        (0x1000619b, 'jge', '100061B4'), (0x100061f2, 'mov', 'dword ptr [edx+ecx*4+44h],esi'),
        (0x1000626b, 'mov', 'esi,dword ptr [edx+ecx*4+44h]'),
        # Millisecond conversion consumes a signed numeric sample rate.
        (0x10005ad1, 'mov', 'eax,dword ptr [esi+eax+1Ch]'), (0x10005ad5, 'cdq', ''),
        (0x10005ae1, 'call', '1003D330'), (0x10005aea, 'call', '1003D280'),
        # Volume LONG* is forwarded unchanged to DirectSound slot6.
        (0x10001fb7, 'mov', 'edx,dword ptr [esp+4]'), (0x10001fbd, 'push', 'edx'),
        (0x10001fbf, 'call', 'dword ptr [ecx+18h]'),
        # Frequency DWORD from DirectSound slot8 -> caller output DWORD.
        (0x1000201a, 'lea', 'edx,[esp]'), (0x1000201e, 'push', 'edx'),
        (0x10002020, 'call', 'dword ptr [ecx+20h]'),
        (0x10002027, 'mov', 'eax,dword ptr [esp+8]'), (0x1000202b, 'mov', 'ecx,dword ptr [esp]'),
        (0x1000202f, 'mov', 'dword ptr [eax],ecx'),
        # Pan slot7 -> actual integer arithmetic -> caller output DWORD.
        (0x10002098, 'call', 'dword ptr [ecx+1Ch]'), (0x100020ae, 'imul', 'ecx'),
        (0x100020b0, 'mov', 'eax,dword ptr [esp+8]'), (0x100020be, 'mov', 'dword ptr [eax],edx'),
        # Position slot4 -> actual sample/frame integer divisions and outputs.
        (0x100021a5, 'call', 'dword ptr [ecx+10h]'),
        (0x100021d1, 'idiv', 'eax,ecx'), (0x100021da, 'div', 'eax,ebx'),
        (0x100021e8, 'mov', 'ecx,dword ptr [esp+10h]'), (0x100021ee, 'mov', 'dword ptr [ecx],eax'),
        (0x10002201, 'mov', 'dword ptr [ecx],edx'), (0x1000221a, 'mov', 'dword ptr [edx],eax'),
        # Actual x87-to-integer helper, not a floating-point return.
        (0x1003d26d, 'fistp', 'qword ptr [ebp-0Ch]'),
        (0x1003d273, 'mov', 'eax,dword ptr [ebp-0Ch]'),
        (0x1003d276, 'mov', 'edx,dword ptr [ebp-8]'), (0x1003d27a, 'ret', '')]:
        witness(*row)
    sdk = Path('C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/um/dsound.h')
    text = sdk.read_text(encoding='utf-8-sig')
    interface = text.split('DECLARE_INTERFACE_(IDirectSoundBuffer, IUnknown)', 1)[1].split('};', 1)[0]
    methods = re.findall(r'STDMETHOD(?:_\([^,]+,\s*|\()([A-Za-z0-9]+)\)', interface)
    assert [(methods[n], n * 4) for n in [4, 6, 7, 8]] == [('GetCurrentPosition', 16), ('GetVolume', 24), ('GetPan', 28), ('GetFrequency', 32)]
    for declaration in ['_Out_opt_ LPDWORD pdwCurrentPlayCursor', '_Out_ LPLONG plVolume', '_Out_ LPLONG plPan', '_Out_ LPDWORD pdwFrequency']:
        assert declaration in interface
    # Verify complete private getter return graphs; these are BOOL32 statuses.
    private_returns = {}
    for start in OUTPUT_CALLS:
        todo, seen, returns = [(start, False)], set(), set()
        while todo:
            address, numeric = todo.pop()
            if (address, numeric) in seen:
                continue
            seen.add((address, numeric))
            row = ins[address]
            assert row['file_verified']
            op, args = row['op'], row['args']
            if op == 'call':
                numeric = False
            elif op == 'mov' and args.startswith('eax,'):
                numeric = args == 'eax,1'
            elif op == 'xor' and args == 'eax,eax':
                numeric = True
            elif op == 'ret':
                assert numeric and args == '4'
                returns.add(f'{address:08x}')
            elif args.split(',')[0] == 'eax' and op not in ['test', 'cmp', 'push', 'div', 'idiv', 'imul', 'add']:
                raise AssertionError(('Unknown private EAX write', hex(address), row))
            if op != 'ret':
                todo += [(n, numeric) for n in audit.successors(address)]
        private_returns[f'{start:08x}'] = sorted(returns)
    # Arithmetic helpers contain only integer operations on stack words and
    # registers, with no calls, FP operations or non-stack memory accesses.
    integer_helpers = {}
    allowed = {'push', 'pop', 'mov', 'xor', 'or', 'neg', 'sbb', 'inc', 'dec', 'shr', 'rcr', 'mul', 'div', 'add', 'cmp', 'ret'}
    for start in [0x1003d330, 0x1003d280]:
        todo, seen = [start], set()
        while todo:
            address = todo.pop()
            if address in seen:
                continue
            seen.add(address)
            row = ins[address]
            assert row['file_verified'] and (row['op'] in allowed or row['op'].startswith('j'))
            assert all(re.match(r'\[esp(?:\+|\])', m) for m in re.findall(r'\[[^\]]+\]', row['args']))
            todo += audit.successors(address)
        integer_helpers[f'{start:08x}'] = dict(instructions=len(seen), callee_pop_bytes=audit.cleanup(start))
        assert audit.cleanup(start) == 16
    header = re.sub(r'\s+', '', (ROOT / 'src/squall/runtime/Squall.h').read_text(encoding='utf-8'))
    rows = []
    for suffix, start in METHODS:
        name = 'SQUALL_Channel_' + suffix
        assert 'DLL_APIint' + name + '(intChannelID);' in header
        stack = audit.function(start)
        assert stack['explicit_argument_slots'] == [1]
        result = Proof(audit).run(start)
        assert result['all_native_return_sites'] == stack['balanced_return_addresses']
        assert len(result['actual_handle_resolver_calls']) == 1
        # Preserve all reachable public instruction bytes for independent review.
        todo, seen = [start], set()
        while todo:
            address = todo.pop()
            if address in seen:
                continue
            seen.add(address)
            witness(address, ins[address]['op'], ins[address]['args'])
            todo += audit.successors(address)
        rows.append(dict(name=name, original_address=f'{start:08x}', calling_convention='cdecl', argument_slots=1,
            argument_role=dict(argument='Unchanged32-bit integer channel handle;actual resolver masks low8 index bits and arithmetic-shifts generation bits', pointer=False, bytes=4),
            return_role='Complete32-bit numeric status/value word in EAX on every balanced C3 return;negative errors share that same integer word',
            native_signature_fully_verified=True, whole_behavior_fully_verified=False, **result))
    assert sum(len(row['all_native_return_sites']) for row in rows) == 25
    count = publish('squall-scalar-handle-signatures.json', rows, __file__, dict(
        disassembly_sha256=digest(path), instruction_evidence=[evidence[a] for a in sorted(evidence)],
        private_getter_status_returns=private_returns, integer_helper_graphs=integer_helpers,
        sdk_header_path=str(sdk), sdk_header_sha256=digest(sdk), directsound_vtable_word_bytes=4,
        source_header_matches=True, new_runtime_audio_test_claimed=False,
        original_payload_initialized_for_every_possible_internal_state_claimed=False,
        signed_and_unsigned_32bit_payloads_share_physical_return_word=True,
        dataflow_assumptions=['Standard Win32 x86 nonvolatile register preservation', 'DirectSound documented DWORD/LONG output contracts'],
        behavior_evidence_separate_from_abi_semantics=True))
    print(f'Proved10 integer channel-handle signatures and25 EAX/C3 return sites;{count}/97 ABI semantics verified.')


if __name__ == '__main__':
    main()
