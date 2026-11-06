#!/usr/bin/env python3
"""Independently account for real contract receipts, fixture stock and saved bytes."""
import argparse
import datetime
import hashlib
import json
import math
import struct
from pathlib import Path
from record_live_production_verification import ROOT, digest, read, write
from validate_counter_supply_state import props, seconds, audit_timeline
from validate_factory_history_state import resource
from gameplay_coefficient_audit import audit_coefficients, f32
from game_save_state import load_save
from counter_save_audit import replay_sales


def model_for(event, epochs):
    data = event.find('Data')
    original = {key.lower(): int(data.get(key)) for key in ['Bonus', 'Penalty']}
    values = original.copy()
    requirements = [dict(product=int(row.get('ProductType')), initial_count=int(row.get('Count')),
                         count=int(row.get('Count')))
                    for row in list(data.find('Requirements'))[:2] if row.get('ReqType', '1') == '0']
    for word in epochs:
        coefficient = struct.unpack('<f', struct.pack('<I', word))[0]
        for key in values:
            values[key] += math.trunc(f32(original[key] * coefficient))
            if values[key] > 100:
                values[key] = math.trunc(f32(math.trunc(f32(values[key] * f32(.1))) * 10))
        for row in requirements:
            row['count'] = (row['count'] + math.trunc(f32(row['initial_count'] * coefficient))) & 0xffffffff
    return dict(initial_bonus=original['bonus'], initial_penalty=original['penalty'],
                requirements=requirements, **values)


def binary_expected(before, notes, coefficients):
    assert before['seeds'] == []
    sales = replay_sales(before['sales'])
    data = struct.pack('<IIf', 0, len(sales), before['cutoff'])
    data += b''.join(struct.pack('<fI', row['time'], row['amount']) for row in sales)
    messages = before['messages'] + notes
    data += struct.pack('<I', len(messages))
    for row in messages:
        text = row['text'].encode('utf-16-le')
        data += struct.pack('<BI', row['read'], len(text) // 2) + text
    data += struct.pack('<I', len(coefficients)) + struct.pack('<' + 'I' * len(coefficients), *coefficients)
    return data


def timeline_with_contract_notes(timeline, successful_notes, baseline, events):
    """Keep timed notes and the XML success strings in actual receipt order.

    WinNote is a literal string, not the coefficient-adjusted reward. It is
    delivered read=true by native ContractStatus::fail (the success path).
    """
    cursor = dict(baseline)
    prices, notes, wraps, rounding = {}, [], [], []
    begin = 0
    for position, text in successful_notes + [(len(timeline) - 1, None)]:
        values = timeline[begin:position + 1]
        if values:
            changes, timed, loops, rounded = audit_timeline(values, cursor, events)
            prices.update(changes);notes.extend(timed);wraps.extend(loops);rounding.extend(rounded)
            cursor.update(changes)
            cursor['GameTime'] = ('Float', repr(values[-1]))
        if text is not None:
            notes.append(dict(read=1, text=text))
        begin = position + 1
    return prices, notes, wraps, rounding


def audit(report_path, allow_retained_failure=False):
    report = read(report_path)
    failed = report['status'] == 'failed'
    if failed:
        assert allow_retained_failure and 'Injected pointer missed owned window target' in report['error']
    else:
        assert report['status'] == 'explored_actual_contract_streak_with_original_supply_command_only'
    work = ROOT / report['work_directory']
    for name, sha in report['source_files'].items():
        path = ROOT / report.get('source_file_snapshots', {}).get(name, name)
        assert digest(path) == sha, name
    for name, sha in report['screenshots'].items():
        assert digest(work / name) == sha, name
    for name, sha in report['implementation_files'].items():
        assert digest(ROOT / 'build/windows/Release' / ('Dobr\u00fd farm\u00e1\u0159.exe' if name == 'FarmarSourceInterop.exe' else name)) == sha
    baseline_path = ROOT / report['baseline_report']['path']
    assert digest(baseline_path) == report['baseline_report']['sha256']
    baseline = read(baseline_path)
    events = list(resource('Gameplay/gameplay.xml').find('Events'))
    events.sort(key=seconds)
    branches = []
    assert len(report['sessions']) in [1, 2] and len({s['kind'] for s in report['sessions']}) == len(report['sessions'])
    for index, session in enumerate(report['sessions']):
        retained = session.get('status') != 'completed'
        if retained:
            assert failed and session['failure_cleanup_exit_code'] == 0
        else:
            assert session['exit_code'] == 0
        suffix = 'failure-persisted' if retained else 'persisted'
        saved = work / f'{index}-{suffix}/Profiles/00000001.save'
        values = props(work / f'{index}-{suffix}/Profiles/00000001.xml')
        if session['kind'] == 'native' and not retained:
            assert json.loads(session['launcher_completion']) == dict(event='exit', exit_code=0, watchdog_triggered=False)
        base = ROOT / session['seed_profile_directory'] if 'seed_profile_directory' in session else ROOT / baseline['work_directory'] / f"{session['seed_from']}-persisted/Profiles"
        for name, sha in session['input_profile_files'].items():
            assert digest(base / name) == sha
        before_values = props(base / '00000001.xml')
        before_save = load_save(base / '00000001.save')
        epochs = session['coefficient_epochs']
        assert epochs[0]['words'] == before_save['coefficient_bits']
        coefficient_audits = [audit_coefficients(row['words'], events) for row in epochs]
        assert (load_save(saved)['coefficient_bits'] if retained else session['final_coefficient_words']) == epochs[-1]['words']
        initial = int(before_values['awQTask'][1])
        assert session['initial_streak'] == initial
        start_stock = [int(before_values[f'WGoodCount{n}'][1]) for n in range(19)]
        start_market = [int(before_values[f'MGoodCount{n}'][1]) for n in range(19)]
        additions, transfers, consumed = [0] * 19, [0] * 19, [0] * 19
        for row in session['supply_commands']:
            assert row['command'] == 'cheatproduct'
            left, right = row['before'], row['after']
            expected = left['stock'].copy()
            free = 100 - sum(expected)
            assert free >= 0 and initial <= left['streak'] < report['requested_target']
            for n in range(19):
                amount = min(2, free)
                free -= amount
                expected[n] += amount
                additions[n] += amount
            assert right['stock'] == expected
            assert all(left[key] == right[key] for key in ['market', 'money', 'streak', 'tasks'])
        for row in session['market_transfers']:
            n = row['product']
            left, right = row['before'], row['after']
            quantity = row['quantity']
            assert quantity == left['stock'][n] > 0
            expected_stock, expected_market = left['stock'].copy(), left['market'].copy()
            expected_stock[n] = 0
            expected_market[n] += quantity
            assert right['stock'] == expected_stock and right['market'] == expected_market
            assert all(left[key] == right[key] for key in ['money', 'streak', 'tasks'])
            transfers[n] += quantity
        money = int(before_values['Money'][1])
        timeline = []
        successful_notes = []
        last_epoch = 0
        receipt_rows = []
        for counter, receipt in enumerate(session['contract_receipts'], initial):
            offer, epoch = receipt['offer'], receipt['epoch']
            assert events[offer].get('Type') == '5'
            expected = model_for(events[offer], [row['words'][offer] for row in epochs[:epoch + 1]])
            assert receipt['model'] == dict(index=offer, **expected)
            restored = receipt['before_acceptance'] is None
            accepted = receipt['restored_active_state'] if restored else receipt['before_acceptance']
            ready, done = [receipt[k] for k in ['before_completion', 'after_completion']]
            if restored:
                assert counter == initial and int(before_values['task0'][1]) == offer
                assert receipt['acceptance_and_deposit_occurred_before_this_process']
                assert accepted['tasks'] == [offer, -1, -1]
            else:
                assert accepted['tasks'] == [-1] * 3
            assert ready['tasks'] == [offer, -1, -1] and done['tasks'] == [-1] * 3
            assert accepted['streak'] == ready['streak'] == counter and done['streak'] == counter + 1
            deposit = 0 if restored else expected['penalty']
            assert accepted['money'] == money and ready['money'] == money - deposit
            money += expected['bonus'] - deposit
            assert done['money'] == money and ready['market'] == done['market']
            remaining = ready['stock'].copy()
            for req in expected['requirements']:
                n = req['product']
                assert remaining[n] >= req['count']
                remaining[n] -= req['count']
                consumed[n] += req['count']
            assert done['stock'] == remaining
            while last_epoch < epoch:
                last_epoch += 1
                timeline += [epochs[last_epoch]['before'], epochs[last_epoch]['game_time']]
            timeline += [accepted['game_time'], ready['game_time'], done['game_time']]
            win_note = events[offer].find('Data').findtext('WinNote')
            if win_note:
                successful_notes.append((len(timeline) - 1, win_note))
            receipt_rows.append(dict(offer=offer, epoch=epoch, counter_before=counter, counter_after=counter + 1,
                                     deposit_charged_this_process=deposit, model_penalty=expected['penalty'],
                                     acceptance_occurred_before_this_process=restored,
                                     bonus=expected['bonus'], requirements=expected['requirements']))
        retained_active = None
        if retained:
            # The failed operation asserted before sending its mouse button.
            # The immediately preceding successful transfer is fully recorded;
            # compare its complete state with the actual normal-close profile.
            final = session['market_transfers'][-1]['after']
            active = final['tasks'][0]
            assert active >= 0 and final['tasks'][1:] == [-1, -1]
            model = model_for(events[active], [epoch['words'][active] for epoch in epochs])
            money -= model['penalty']
            retained_active = dict(offer=active, model=model, paid_deposit=model['penalty'],
                                   acceptance_snapshot_captured=False, completion_claimed=False)
            assert int(values['task0'][1]) == active and int(values['task1'][1]) == int(values['task2'][1]) == -1
            assert float(values['taskTime0'][1]) == final['remaining'][0]
            assert session['failure_live_state']['task'] == active
            assert session['failure_live_state']['game_time'] == final['game_time']
        else:
            final = session['final_streak_state']
            assert final['streak'] == report['requested_target'] and final['tasks'] == [-1] * 3
        assert initial + len(receipt_rows) == final['streak'] and final['money'] == money
        assert final['stock'] == [start_stock[n] + additions[n] - transfers[n] - consumed[n] for n in range(19)]
        assert final['market'] == [start_market[n] + transfers[n] for n in range(19)]
        pauses = session.get('actual_supply_pause_intervals', [])
        for pindex, pause in enumerate(pauses):
            assert pause['start']['remaining'] > 0
            if 'end' in pause:
                assert pause['start'] == pause['end']
            else:
                assert retained and pindex == len(pauses) - 1
                assert pause['start'] == dict(game_time=final['game_time'], remaining=final['remaining'][0])
        for row in session['supply_commands'] + session['market_transfers']:
            assert any(row['before']['game_time'] == row['after']['game_time'] == p['start']['game_time'] and
                       row['before']['remaining'][0] == row['after']['remaining'][0] == p['start']['remaining'] for p in pauses)
        for key, wanted in [('awQTask', ('UInt', str(final['streak']))), ('Money', ('Int', str(money)))]:
            assert values[key] == wanted
        for n in range(19):
            assert values[f'WGoodCount{n}'] == ('UInt', str(final['stock'][n]))
            assert values[f'MGoodCount{n}'] == ('UInt', str(final['market'][n]))
            assert values[f'MGoodPrice{n}'] == ('UInt', '99')
        while last_epoch < len(epochs) - 1:
            last_epoch += 1
            timeline += [epochs[last_epoch]['before'], epochs[last_epoch]['game_time']]
        timeline += [final['game_time'], float(values['GameTime'][1])]
        prices, notes, wraps, rounding = timeline_with_contract_notes(timeline, successful_notes, before_values, events)
        assert len(wraps) == len(epochs) - 1
        assert all(values[key] == value for key, value in prices.items())
        expected_binary = binary_expected(before_save, notes, epochs[-1]['words'])
        assert saved.read_bytes() == expected_binary, ('Complete saved binary differs', session['kind'])
        branches.append(dict(kind=session['kind'], initial_streak=initial, final_streak=final['streak'],
            process_closed_normally=True, requested_target_reached=not retained, retained_active_contract=retained_active,
            actual_receipts=receipt_rows, supply_commands=len(session['supply_commands']), market_transfers=len(session['market_transfers']),
            actual_pause_intervals=pauses, all_19_product_ledgers=True, final_money=money,
            additions=additions, transferred=transfers, consumed=consumed, final_stock=final['stock'], final_market=final['market'],
            coefficient_epochs=coefficient_audits, clock_wraps=wraps, clock_rounding=rounding,
            independently_expected_notes=len(notes), complete_expected_saved_bytes=len(expected_binary),
            independently_expected_contract_win_notes=len(successful_notes),
            complete_saved_sha256=digest(saved), saved_profile_sha256=digest(work / f'{index}-{suffix}/Profiles/00000001.xml'),
            entire_binary_unchanged=saved.read_bytes() == (base / '00000001.save').read_bytes(), normalizations=[]))
    return report, branches


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--runtime-report', type=Path, required=True)
    parser.add_argument('--prefix', required=True)
    parser.add_argument('--allow-retained-failure', action='store_true')
    args = parser.parse_args()
    report, branches = audit(args.runtime_report, args.allow_retained_failure)
    full = read(ROOT / 'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT / name) == sha for name, sha in full['production_source_files'].items())
    for row in read(ROOT / 'analysis/inventory.json')['files']:
        assert digest(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159' / row['path']) == row['sha256']
    state_name = args.prefix + '-state-validation.json'
    regression_name = args.prefix + '-regression.json'
    original_runtime_status = report['status']
    scope_status = ('passed_retained_completed_prefix_and_active_contract_ledgers_only' if original_runtime_status == 'failed' else
                    'passed_actual_paired_contract_receipts_and_complete_ledgers_only' if len(branches) == 2 else
                    'passed_actual_single_contract_receipts_and_complete_ledgers_only')
    report.update(status=scope_status, original_runtime_status=original_runtime_status,
        auditor_sha256=digest(__file__), runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(branches=branches, ordinary_farming_supply_claimed=False,
                        supply_commands_are_original_diagnostic_fixture=True, original_memory_or_profile_written=False,
                        normalizations=[]), whole_game_complete=False)
    write(ROOT / 'analysis' / state_name, report)
    result = dict(status='passed_real_contract_receipts_paused_fixture_supply_and_whole_save_audit_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), recorder_sha256=digest(__file__),
        reports={state_name: digest(ROOT / 'analysis' / state_name)}, target=report['requested_target'],
        actual_receipts_per_executable=[len(b['actual_receipts']) for b in branches],
        complete_expected_saved_bytes=[b['complete_expected_saved_bytes'] for b in branches],
        production_changed=False, current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json', new_full_ctest_run=False,
        original_files_unchanged=17, whole_game_complete=False)
    write(ROOT / 'analysis' / regression_name, result)
    verification = read(ROOT / 'analysis/verification.json')
    for name in [state_name, regression_name]:
        if 'analysis/' + name not in verification['reports']:
            verification['reports'].append('analysis/' + name)
    if 'analysis/' + state_name not in verification['original_game_runtime']['scope_reports']:
        verification['original_game_runtime']['scope_reports'].append('analysis/' + state_name)
    verification['latest_scoped_regression'] = dict(report='analysis/' + regression_name, recorded_at_utc=result['recorded_at_utc'], whole_game_complete=False)
    paths = [ROOT / 'CMakeLists.txt'] + [p for folder in ['src', 'tools', 'tests'] for p in (ROOT / folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows = [p.relative_to(ROOT).as_posix() + '\0' + digest(p) + '\n' for p in sorted(paths, key=lambda p: p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/' + regression_name
    write(ROOT / 'analysis/verification.json', verification)
    print(json.dumps(dict(status=result['status'], branches=branches), ensure_ascii=True))


if __name__ == '__main__':
    main()
