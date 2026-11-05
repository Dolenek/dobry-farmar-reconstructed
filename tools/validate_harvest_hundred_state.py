#!/usr/bin/env python3
"""Audit actual harvesting,carried capacity,warehouse ledger and natural award0."""
import argparse,json,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image,ImageChops
from record_live_production_verification import ROOT,digest,read,write,checked
from validate_factory_history_state import resource
from game_save_state import load_save

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/harvest-hundred-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/harvest-hundred-state-validation.json');args=parser.parse_args()
    report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual56_grass_harvests_and_natural_award0_only' and len(report['sessions'])==2
    baseline=read(ROOT/'analysis/no-dry-award-state-validation.json');base=ROOT/baseline['work_directory']/'0-persisted/Profiles'
    def props(path):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(path).getroot()}
    original=props(base/'00000001.xml');values=[];business=[];fields=[n for n in range(60) if n not in [18,27,42,50]]
    creation=float(resource('Game.xml').find('ProductionInfo')[0].get('CreationTime'))
    stock=[int(original[f'WGoodCount{n}'][1]) for n in range(19)];initial_carried=[int(original[f'PCC{n}'][1]) for n in range(8)]
    assert stock[0]==1 and sum(stock)==40 and initial_carried==[12]+[0]*7
    for i,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0 and len(session['awards'])==1 and session['harvested_fields']==fields
        assert session['initial_collected']==44 and session['initial_carried']==initial_carried and session['initial_stock']==stock and session['initial_money']==2504
        assert session['fixture_report_sha256']==digest(ROOT/'analysis/no-dry-award-state-validation.json')
        steps=session['work_steps'];assert len(steps)==61;compact=[]
        for row in steps:
            label=row['label'];s=row['state'];assert s['money']==2504 and len(s['fields'])==60
            assert all(f['kind']==0 and f['status'] in [0,1] and 0<=f['progress']<=creation and f['water']>=0 for f in s['fields'])
            if label.startswith('harvest-'):
                _,number,_,field=label.split('-');number=int(number);assert fields[number-1]==int(field)
                expected_stock=stock.copy();expected_stock[0]+=12+16*((number-1)//16)
                assert s['collected']==44+number and s['carried']==[1+(number-1)%16]+[0]*7 and s['stock']==expected_stock
                if number<56:assert not s['award'] and not s['award_shown']
            elif label=='initial-carried-products-stored' or label.startswith('batch-'):
                number=0 if label.startswith('initial-') else int(label.split('-')[1]);assert number in [0,16,32,48]
                expected_stock=stock.copy();expected_stock[0]+=12+number
                assert s['collected']==44+number and s['carried']==[0]*8 and s['stock']==expected_stock
            else:
                assert label=='natural-hundred-harvest-award' and s['collected']==100 and s['award'] and s['award_shown']
            compact.append({k:s[k] for k in ['money','collected','carried','stock']})
        assert session['final_harvest_count']==100 and session['final_carried']==[8]+[0]*7
        expected_stock=stock.copy();expected_stock[0]+=60;assert session['final_stock']==expected_stock and sum(expected_stock)==100
        v=props(work/f'{i}-persisted/Profiles/00000001.xml');values.append(v)
        for k,w in dict(Money=('Int','2504'),awQCollect=('UInt','100'),awQProcess=('UInt','6'),Award00Grow100=('Bool','1'),awardShow0=('Bool','1')).items():assert v[k]==w,(k,v[k])
        assert [int(v[f'PCC{n}'][1]) for n in range(8)]==[8]+[0]*7 and [int(v[f'WGoodCount{n}'][1]) for n in range(19)]==expected_stock
        saved=work/f'{i}-persisted/Profiles/00000001.save';assert saved.read_bytes()==(base/'00000001.save').read_bytes() and saved.stat().st_size==15808
        business.append(compact)
        glyphs=session['award_visible_congratulation_lines'];assert len(glyphs)==1 and len(glyphs[0])==40
    assert business[0]==business[1]
    left,right=values;assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
    differences={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
    varying={'GameTime','RunTime','letTime0'}|{f'{prefix}{n}' for prefix in ['FFP','FFW','FFF'] for n in range(60)}
    readiness=[]
    for n in range(60):
        name=f'FFS{n}'
        if name in differences:
            assert n in fields and left[name]==('UInt','1') and right[name]==('UInt','0')
            assert left[f'FFP{n}']==('Float','0') and 0<float(right[f'FFP{n}'][1])<2
            readiness.append(dict(field=n,native_progress=0,source_seconds_remaining=float(right[f'FFP{n}'][1])));varying.add(name)
    # Keep the actual independent random offer difference visible. This checks
    # the original conditional event and valid outcomes,not an equal RNG stream.
    events=list(resource('Gameplay/gameplay.xml').find('Events'));event=events[370]
    assert event.get('Type')=='5' and event.get('Time')=='3:50:00' and event.find('Data').get('RandomFactor')=='0.5'
    assert left['letter0']==('Int','370') and right['letter0']==('Int','-1')
    assert 0<float(left['letTime0'][1])<=120 and right['letTime0']==('Float','0')
    assert all(float(v['GameTime'][1])>13800 for v in values);varying.add('letter0')
    assert set(differences)<=varying,differences
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]];assert all(im.size==(800,600) for im in images)
    a,b=[im.tobytes() for im in images];pixels=dict(compared_pixels=480000,different_pixels=sum(a[n:n+3]!=b[n:n+3] for n in range(0,len(a),3)),difference_bounds=ImageChops.difference(*images).getbbox(),exact=a==b);assert pixels['exact'],pixels
    completion=[json.loads(line) for line in report['sessions'][0]['launcher_completion'].splitlines()];assert completion==[dict(event='exit',exit_code=0,watchdog_triggered=False)]
    report.update(status='passed_actual56_harvests_natural100_award_and_capacity_ledger_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=61,actual_field_snapshots_per_executable=3660,actual_harvest_actions_per_executable=56,
            initial_collected=44,final_collected=100,carried_capacity=16,warehouse_capacity=100,final_carried_grass=8,final_stored_grass=61,
            complete_identical_save_bytes=15808,entire_save_unchanged_from_native_baseline=True,actual_award_panel_pixels=pixels,
            original_independent_profile_differences=differences,regrowth_readiness_differences=readiness,
            random_offer_difference=dict(event_index=370,probability=0.5,native_offer=True,source_offer=False,actual_rng_stream_equality_claimed=False),normalizations=[]),
        whole_game_complete=False,pending=['Bidirectional restores of natural100 award,full warehouse and carried8','Natural1000 harvest/production counters,contract streaks and whole original goal scopes'])
    write(args.output,report);print(report['status']);print(pixels)

if __name__=='__main__':main()
