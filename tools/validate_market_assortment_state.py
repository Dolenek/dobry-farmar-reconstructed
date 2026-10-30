#!/usr/bin/env python3
"""Audit all actual market rows and the naturally earned complete-assortment award."""
import argparse,collections,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/market-assortment-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/market-assortment-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual19_product_market_assortment_and_natural_award_only'
    assert len(report['sessions'])==2
    comparable=[];clicks=[];sequences=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and session['awards']
        initial=session['initial_goods'];assert len(initial)==19 and all(initial)
        rows=session['catalog_steps'];assert [row['label'] for row in rows]==['all-prices99']+[f'product-{i}-transferred' for i in range(19)]+['natural-max-market-assortment-award']
        for index,row in enumerate(rows):
            state=row['state'];assert state['money']==8134 and state['warehouse_level']==3 and state['prices']==[99]*19
            sent=min(index,19)
            assert state['market']==[initial[i] if i<sent else 0 for i in range(19)]
            assert state['goods']==[0 if i<sent else initial[i] for i in range(19)]
        assert rows[-1]['state']['award'] and rows[-1]['state']['award_shown']
        grouped=collections.defaultdict(list)
        for click in session['price_clicks']:grouped[click['product']].append(click)
        assert set(grouped)==set(range(19))
        for product,events in grouped.items():
            assert all(1<=event['before']<=event['after']<=99 for event in events)
            assert events[-1]['after']==99 and any(event['after']-event['before']>1 for event in events)
            assert all(old['after']==new['before'] for old,new in zip(events,events[1:]))
        clicks.append(len(session['price_clicks']))
        sequences.append([{key:value for key,value in event.items() if key!='game_time'} for event in session['price_clicks']])
        comparable.append([{key:value for key,value in row['state'].items() if key!='game_time'} for row in rows])
    assert comparable[0]==comparable[1]
    assert sequences[0]==sequences[1]
    props=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    for values in props:
        assert values['Award19MaxMarketAssortiment']==values['awardShow18']==('Bool','1')
        for i in range(19):
            assert values[f'MGoodCount{i}']==('UInt',str(report['sessions'][0]['initial_goods'][i]))
            assert values[f'MGoodPrice{i}']==('UInt','99') and values[f'WGoodCount{i}']==('UInt','0')
            assert values[f'MSellTime{i}']==('Float','1.#INF')
    differences={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    discrete={key:value for key,value in differences.items() if left[key][0] not in ['Float','Double']}
    saves=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]]
    from PIL import Image,ImageChops
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]]
    assert images[0].size==images[1].size
    difference=ImageChops.difference(*images)
    pixels=dict(compared_pixels=images[0].width*images[0].height,
        difference_bounds=difference.getbbox(),exact=images[0].tobytes()==images[1].tobytes())
    # Count RGB pixels, including any change restricted to green or blue.
    a,b=images[0].tobytes(),images[1].tobytes();pixels['different_pixels']=sum(a[i:i+3]!=b[i:i+3] for i in range(0,len(a),3))
    report.update(status='passed_for_actual19_market_rows_physical_price_acceleration_and_natural_award18_only',
        auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=21,all_market_goods_and_prices_equal=True,all19_infinite_clocks_spelled_exactly=True,
            physical_price_clicks_per_executable=clicks,price_acceleration_observed_for_all19_rows=True,
            all_physical_price_change_sequences_equal=True,actual_award_panel_pixels=pixels,
            independent_profile_differences=differences,independent_profile_discrete_differences=discrete,
            all_profile_discretes_equal=not discrete,complete_save_equal=saves[0].read_bytes()==saves[1].read_bytes(),
            save_bytes=[path.stat().st_size for path in saves],normalizations=[],original_profile_or_memory_written=False),
        whole_game_complete=False,pending=['Bidirectional restore of all19 infinity counters','Any explicitly listed independently timed profile/save differences','Other market inputs, remaining natural awards and complete goal scopes'])
    write(args.output,report);print(report['status']);print(report['comparison'])
if __name__=='__main__':main()
