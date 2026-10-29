#!/usr/bin/env python3
"""Audit three real harvests and the money gate without hiding timed price events."""
import argparse,hashlib,json,xml.etree.ElementTree as ET,zipfile
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/cucumber-harvest-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/cucumber-harvest-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_three_cucumber_harvest_sale_cycles_and_money_gate_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    props=[{p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert len(left)==795 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    # The final states straddle the real791s/810s popularity definitions.
    original=ROOT.parent/'farmář/Dobrý farmář'
    with zipfile.ZipFile(original/'Data/Data.pack') as archive:events=ET.fromstring(archive.read('Gameplay/gameplay.xml')).find('Events')
    prices={}
    for event in events:
        if event.attrib.get('Type')=='6' and event.attrib.get('Time') in ['0:13:11','0:13:30']:
            h,m,s=map(int,event.attrib['Time'].split(':'));prices[h*3600+m*60+s]={int(row.get('ProductType')):int(row.get('Value')) for row in event.find('Data')}
    assert prices=={791:{0:8,1:6,3:20},810:{0:-1,1:4,3:6}},prices
    timed=[]
    for values in props:
        gt=float(values['GameTime'][1]);assert 791<=gt<840,gt;event_time=810 if gt>=810 else 791
        for product,value in prices[event_time].items():assert values[f'MEffPrice{product}']==('Int',str(value))
        timed.append(dict(game_time=gt,event_time=event_time,prices=prices[event_time]))
        for name,value in {'Money':('Int','184'),'PCarry':('UInt','6'),'MGoodPrice3':('UInt','10'),'PCC3':('UInt','0'),
                           'WGoodCount3':('UInt','0'),'MGoodCount3':('UInt','0'),'mTrigger':('Int','0'),'fcu':('Int','1'),
                           'awQCollect':('UInt','30'),'Upgrade0':('Int','-1'),'awQDestroy':('UInt','1')}.items():assert values[name]==value,(name,values[name])
        for field in [17,18,23,27]:assert values[f'FFC{field}']==('UInt','3') and values[f'FFS{field}']==('UInt','0') and float(values[f'FFW{field}'][1])>0
    varying={'GameTime','RunTime','letTime0','MEffPrice0','MEffPrice1','MEffPrice3'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in [14,15,16,17,18,23,24,25,26,27,33,34,35]}
    changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
    gates=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and len(session['work_steps'])==27
        assert session['initial_state']['money']==64 and session['final_state']['money']==184
        assert session['initial_cucumber_price']==2 and session['price_clicks']==[dict(before=n,after=n+1) for n in range(2,10)]
        rows={row['label']:row['state'] for row in session['work_steps']}
        first,second,third=[rows[f'{i}-sold-four-at-ten'] for i in range(3)]
        assert [row['money'] for row in [first,second,third]]==[104,144,184]
        assert rows['0-four-cucumbers-ready']['money_trigger']==first['money_trigger']==rows['1-four-cucumbers-ready']['money_trigger']==140
        frozen=rows['0-four-cucumbers-ready']['game_time'];assert first['game_time']==rows['1-four-cucumbers-ready']['game_time']==frozen
        assert second['money_trigger']==third['money_trigger']==0 and second['game_time']>frozen
        gates.append(dict(frozen_game_time=frozen,money_before_unlock=104,money_at_unlock=144,resumed_game_time=second['game_time']))
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];states=[load_save(path) for path in paths]
    for state in states:assert state['bytes']==3557 and state['seeds']==[] and len(state['sales'])==4 and all(row['amount']==10 for row in state['sales']) and len(state['messages'])==7 and len(state['coefficient_bits'])==383
    assert states[0]['messages']==states[1]['messages'] and states[0]['coefficient_bits']==states[1]['coefficient_bits']
    ranges=states[0]['sale_clock_ranges'];assert ranges==states[1]['sale_clock_ranges'];payloads=[bytearray(p.read_bytes()) for p in paths]
    for payload in payloads:
        for first,last in ranges:payload[first:last]=b'\0'*(last-first)
    assert payloads[0]==payloads[1],'Save differs outside independent latest sale clocks/cutoff'
    for row in json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))['files']:assert digest(original/row['path'])==row['sha256']
    report.update(status='passed_for_three_cucumber_cycles_money_gate_and_recorded_timed_event_cutoffs_only',runtime_report_sha256=digest(args.runtime_report),
        audit_files={n:digest(ROOT/n) for n in ['tools/validate_cucumber_harvest_state.py','tools/game_save_state.py']},
        comparison=dict(profile_properties=795,changed_properties=changed,timed_popularity_contexts=timed,money_gate_contexts=gates,
            save_bytes=3557,normalized_byte_ranges=ranges,latest_sales=[s['sales'] for s in states],messages_equal=7,coefficient_bits_equal=383,
            actual_harvested_cucumbers=12,actual_sales_income=120,discrete_prices_claimed_equal=False,
            actual_time_shortcut='Original Alt+Insert15x for growth/sale countdown, restored1x before each harvest'),
        pending=['Timed frame replay across exact event cutoffs, latest sale-history transfers','Paid live production, tools, other crops and remaining full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
