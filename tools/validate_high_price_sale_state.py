#!/usr/bin/env python3
"""Audit actual price80 sales, natural award15, note356 and every saved byte."""
import struct
from PIL import Image, ImageChops
from record_live_production_verification import ROOT, digest, read, write, checked
from validate_counter_supply_state import props, seconds, audit_timeline
from validate_factory_history_state import resource
from game_save_state import load_save


def main():
    report = read(ROOT/'.local/high-price-sale-validation.json')
    work = checked(report)
    assert report['status'] == 'explored_actual_price80_sale_natural_award15_and_price99_quiescence_only'
    fixture_path = ROOT/report['fixture_report']['path']
    assert digest(fixture_path) == report['fixture_report']['sha256']
    base = ROOT/read(fixture_path)['work_directory']/'0-persisted/Profiles'
    original = props(base/'00000001.xml')
    old = load_save(base/'00000001.save')
    assert len(old['sales']) == 650 and len(old['messages']) == 71
    events = list(resource('Gameplay/gameplay.xml').find('Events'))
    events.sort(key=seconds)
    branches = []
    for index, session in enumerate(report['sessions']):
        assert session['status'] == 'completed' and session['exit_code'] == 0
        for name, sha in session['seed_profiles_sha256'].items():
            assert digest(base/name) == sha
        clicks = session['actual_price_clicks']
        assert len(clicks) == 38
        assert [(c['before'],c['after']) for c in clicks[:19]] == [(p,p-1) for p in range(99,80,-1)]
        assert [(c['before'],c['after']) for c in clicks[19:]] == [(p,p+1) for p in range(80,99)]
        assert all(c['money'] == 3180 and c['milk'] == 10 for c in clicks[:19])
        assert all(c['money'] == 3340 and c['milk'] == 8 for c in clicks[19:])
        assert 13360 <= session['actual_slowdown_time'] < 13380
        contexts = session['actual_award_contexts']
        assert len(contexts) == 1
        context = contexts[0]
        assert context['high_sold'] and context['money'] == 3260 and context['market_milk'] == 9 and context['price'] == 80
        values = props(work/f'{index}-persisted/Profiles/00000001.xml')
        path = work/f'{index}-persisted/Profiles/00000001.save'
        saved = load_save(path)
        changed = {k:[original[k],values[k]] for k in original if original[k] != values[k]}
        fixed = dict(Money=('Int','3340'), MGoodCount10=('UInt','8'), MGoodPrice10=('UInt','99'),
            Award15SellItemPer40=('Bool','1'), awHighSold=('Bool','1'), awardShow15=('Bool','1'),
            awQCollect=('UInt','1036'), awQProcess=('UInt','1011'), WGoodCount10=('UInt','12'),
            ProdCount0=('UInt','0'), PCC0=('UInt','0'), letter0=('Int','-1'), letTime0=('Float','0'))
        assert all(values[k] == v for k,v in fixed.items())
        assert values['MSellTime10'] == ('Float','1.#INF')
        assert session['stable_price99_observations'] == [[3340,8]]*2
        prices, notes, wraps, rounding = audit_timeline([float(values['GameTime'][1])],original,events)
        assert not wraps and not rounding and len(notes) == 1
        assert all(values[k] == v for k,v in prices.items())
        assert values['MEffPrice10'] == ('Int','80')
        assert int(values['marketIt'][1]) == int(original['marketIt'][1])+1
        allowed = {'GameTime','RunTime','letTime0','letter0','Money','MGoodCount10','marketIt',
                   'Award15SellItemPer40','awHighSold','awardShow15'} | set(prices)
        allowed |= {f'{prefix}{n}' for prefix in ['FFP','FFW','FFF','FFS'] for n in range(60)}
        assert set(changed) <= allowed, changed
        assert 13380 < float(values['GameTime'][1]) < 13410
        assert not saved['seeds'] and saved['coefficient_bits'] == old['coefficient_bits']
        assert saved['messages'] == old['messages']+notes
        sales = saved['sales']
        assert sales[:650] == old['sales'] and len(sales) == 652
        assert all(s['amount'] == 80 and 13380 <= s['time'] <= float(values['GameTime'][1]) for s in sales[650:])
        cutoff = struct.unpack('<f',struct.pack('<f',sales[-1]['time']-60))[0]
        assert saved['cutoff'] == cutoff
        expected = struct.pack('<IIf',0,652,cutoff)
        expected += b''.join(struct.pack('<fI',s['time'],s['amount']) for s in sales)
        messages = old['messages']+notes
        expected += struct.pack('<I',len(messages))
        for message in messages:
            encoded = message['text'].encode('utf-16-le')
            expected += struct.pack('<BI',message['read'],len(encoded)//2)+encoded
        expected += struct.pack('<I',383)+struct.pack('<383I',*old['coefficient_bits'])
        assert path.read_bytes() == expected and len(expected) == 23694
        branches.append(dict(kind=session['kind'], actual_price80_sales=sales[650:],
            original_price_spike_time=13380, original_high_sale_predicate='signed sold>79',
            actual_money_before=3180, actual_money_after=3340, market_before=10, market_after=8,
            exact_complete_expected_save_bytes=23694, independently_encoded_new_notes=notes,
            unchanged_coefficient_words=383, changed_properties=changed, normalizations=[]))
    images = [Image.open(work/f'{i}-award-0.png').convert('RGB') for i in range(2)]
    assert all(i.size == (800,600) for i in images)
    a,b = [i.tobytes() for i in images]
    pixels = dict(compared_pixels=480000,different_pixels=sum(a[n:n+3] != b[n:n+3] for n in range(0,len(a),3)),
                  difference_bounds=ImageChops.difference(*images).getbbox(),exact=a == b)
    # Keep the first pair's actual difference. A second real original process
    # and full live sprite observations establish both original variants.
    from audit_award_pixel_order import audit_original_variants
    rendering = audit_original_variants(work)
    report.update(status='passed_actual_price80_sales_natural_award15_and_complete_encoded_saves_only',
        auditor_sha256=digest(__file__), runtime_report_sha256=digest(ROOT/'.local/high-price-sale-validation.json'),
        comparison=dict(branches=branches,actual_award_panel_pixels=pixels,
            actual_original_rendering_variants=rendering,
            exact_complete_expected_save_bytes_per_executable=23694,
            actual_native_source_sale_timestamps_preserved=True,normalizations=[]), whole_game_complete=False)
    write(ROOT/'.local/high-price-sale-state-validation.json',report)
    print(report['status'],pixels)


if __name__ == '__main__':
    main()
