#!/usr/bin/env python3
"""Audit natural planning, real queue markers, all32 field jobs and complete saves."""
from PIL import Image, ImageChops
from record_live_production_verification import ROOT, digest, read, write, checked
from validate_counter_supply_state import props, seconds, audit_timeline
from validate_factory_history_state import resource


def main():
    report = read(ROOT/'.local/planning-queue-validation.json')
    work = checked(report)
    assert report['status'] == 'explored_actual32_queued_paid_digs_and_natural_planning_award_only'
    assert [s['kind'] for s in report['sessions']] == ['native', 'source']
    fixture_path = ROOT/'analysis/no-dry-award-state-validation.json'
    base = ROOT/read(fixture_path)['work_directory']/'0-persisted/Profiles'
    original = props(base/'00000001.xml')
    # The initial diagnostic name said "paid". Original XML supplies an empty
    # DigCost attribute; the recovered numeric field's default is zero.
    assert resource('Farm/ActionAreas/FarmField.xml').get('DigCost') == ''
    header = (ROOT/'src/game/ui/FarmField.h').read_text(encoding='utf-8')
    assert 'digCost=0' in header
    events = list(resource('Gameplay/gameplay.xml').find('Events'))
    events.sort(key=seconds)
    branches = []
    for index, session in enumerate(report['sessions']):
        assert session['status'] == 'completed' and session['exit_code'] == 0
        assert session['fixture_report']['sha256'] == digest(fixture_path)
        rows = session['actual_enqueued_targets']
        fields = session['dug_fields']
        assert len(rows) == len(fields) == len(set(fields)) == 32
        assert [r['target']['field'] for r in rows] == fields
        assert [r['visible_marker_count'] for r in rows] == list(range(1, 33))
        assert all(r['game_time'] == session['paused_start_time'] and r['collected'] == 44 and r['planned'] == 0 for r in rows)
        if index == 0:
            assert [r['actual_native_queue_size'] for r in rows] == list(range(1, 33))
        observations = session['actual_work_counter_observations']
        counts = [r['planned'] for r in observations]
        assert counts == sorted(set(counts)) and counts[-1] == 32
        assert 30 in counts and all(r['money'] == 2504 for r in observations)
        assert all(r['planning_started'] for r in observations if 0 < r['planned'] < 32)
        values = props(work/f'{index}-persisted/Profiles/00000001.xml')
        assert values['Money'] == original['Money'] == ('Int', '2504')
        for key, expected in dict(awQCollect=('UInt','44'), awQProcess=('UInt','6'),
            awAdecvClick=('UInt','32'), PCarry=('UInt','2'),
            Award20Megaplanning=('Bool','1'), awardShow19=('Bool','1')).items():
            assert values[key] == expected
        for prefix, count in [('PCC', 8), ('WGoodCount', 19), ('MGoodCount', 19)]:
            assert all(values[f'{prefix}{n}'] == original[f'{prefix}{n}'] for n in range(count))
        # Some first fields have already naturally returned to growing/ready by
        # the end. Require an actual completed state3 observation for every job.
        for slot, field in enumerate(fields):
            assert any(r['fields'][slot] == 3 for r in observations) or values[f'FFS{field}'] == ('UInt','3'), field
        prices, notes, wraps, rounding = audit_timeline([float(values['GameTime'][1])], original, events)
        assert not notes and not wraps and not rounding
        assert all(values[key] == value for key, value in prices.items())
        assert values['letter1'] == ('Int','-1') and values['letTime1'] == ('Float','0')
        assert values['letter0'] in [('Int','-1'), ('Int','370')]
        if values['letter0'] == ('Int','370'):
            assert 0 < float(values['letTime0'][1]) <= 120
        else:
            assert values['letTime0'] == ('Float','0')
        saved = work/f'{index}-persisted/Profiles/00000001.save'
        assert saved.read_bytes() == (base/'00000001.save').read_bytes()
        assert saved.stat().st_size == 15808
        assert len(session['award_visible_congratulation_lines']) == 1
        branches.append(dict(kind=session['kind'], real_enqueued_fields=fields,
            actual_marker_counts=list(range(1,33)), actual_planning_count=32,
            actual_dig_cost=0, money_unchanged=2504, input_and_output_binary_exact_bytes=15808,
            real_work_counter_samples=len(observations), final_game_time=values['GameTime'],
            actual_offer_370=values['letter0'], actual_price_words=prices,
            entire_final_profile=values, normalizations=[]))
    images = [Image.open(work/f'{i}-award-0.png').convert('RGB') for i in range(2)]
    assert all(i.size == (800,600) for i in images)
    left, right = [i.tobytes() for i in images]
    pixels = dict(compared_pixels=480000, different_pixels=sum(left[n:n+3] != right[n:n+3] for n in range(0,len(left),3)),
                  difference_bounds=ImageChops.difference(*images).getbbox(), exact=left == right)
    assert pixels['exact']
    report.update(status='passed_actual32_task_queue_natural_planning_award_and_original_zero_dig_cost_only',
        auditor_sha256=digest(__file__), runtime_report_sha256=digest(ROOT/'.local/planning-queue-validation.json'),
        comparison=dict(branches=branches, actual_award_panel_pixels=pixels,
            saved_bytes=15808, independent_random_offer_and_field_regrowth_preserved=True,
            source_native_clock_or_rng_stream_equality_claimed=False,
            runtime_identifier_paid_digs_is_not_a_claim_of_nonzero_payment=True, normalizations=[]),
        whole_game_complete=False)
    write(ROOT/'.local/planning-queue-state-validation.json', report)
    print(report['status'], pixels)


if __name__ == '__main__':
    main()
