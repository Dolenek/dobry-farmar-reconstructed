"""Exact saved-sale replay, without altering any input or output profile."""
import struct
from game_save_state import load_save


def replay_sales(sales):
    rows = []
    for sale in sales:
        cutoff = struct.unpack('<f', struct.pack('<f', sale['time'] - 60))[0]
        index = 0
        while index < len(rows):
            if not rows[index]['time'] < cutoff:
                break
            rows.pop(index)
            index = 0 if index == len(rows) else index + 1
        rows.append(sale)
    return rows


def exact_restored_binary(before, after):
    left, right = load_save(before), load_save(after)
    assert not left['seeds'] and not right['seeds']
    rows = replay_sales(left['sales'])
    assert right['sales'] == rows
    assert left['cutoff'] == right['cutoff']
    assert left['messages'] == right['messages']
    assert left['coefficient_bits'] == right['coefficient_bits']
    # Only the real restore-time linked-list replay may change this binary.
    # Verify the complete output bytes, preserving journal/coefficient tail.
    old = before.read_bytes()
    tail = old[12 + 8 * len(left['sales']):]
    expected = struct.pack('<IIf', 0, len(rows), left['cutoff'])
    expected += b''.join(struct.pack('<fI', s['time'], s['amount']) for s in rows)
    expected += tail
    assert after.read_bytes() == expected
    return dict(input_bytes=len(old), output_bytes=len(expected),
                input_sales=len(left['sales']), output_sales=len(rows),
                original_replay_removed_sales=len(left['sales']) - len(rows),
                whole_binary_unchanged=old == expected,
                complete_expected_output_bytes=len(expected),
                journal_records=len(right['messages']), coefficient_words=len(right['coefficient_bits']),
                normalizations=[])
