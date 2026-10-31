#!/usr/bin/env python3
"""Audit actual fourteen inputs and seven independent partial production counters."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/seven-inflight-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/seven-inflight-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_seven_simultaneous_loaded_inflight_buildings_only' and len(report['sessions'])==2
    sequences=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0
        steps=session['production_steps'];assert len(steps)==18
        goods=[0]*19;counts=[0]*7;market=session['initial_market'].copy();expected=[]
        for index in range(7):
            for product in range(19):goods[product]+=min(2,100-sum(goods))
            expected.append((f'supply-{index}',counts.copy(),goods.copy(),market.copy()))
            goods[0]-=2;counts[index]+=2
            expected.append((f'slider-feed-{index}',counts.copy(),goods.copy(),market.copy()))
        assert sum(goods)==98
        for product in [1,2,3]:
            assert goods[product]==6;market[product]+=goods[product];goods[product]=0
            expected.append((f'make-space-product-{product}',counts.copy(),goods.copy(),market.copy()))
        expected.append(('paused-seven-inflight',counts,goods,market))
        for row,(label,count,stock,sent) in zip(steps,expected):
            state=row['state'];assert row['label']==label
            assert state['money']==7084 and state['counts']==count and state['goods']==stock and state['market']==sent
            assert state['processed']==6 and state['stopped']==[0]*7
        frozen=session['frozen_inflight_state'];assert frozen==steps[-1]['state']
        assert frozen['counts']==[2]*7 and all(0<p<60 for p in frozen['progress']) and 1105<=frozen['game_time']<1107
        assert sum(frozen['goods'])==80 and frozen['goods'][10]==6
        sequences.append([{k:v for k,v in row['state'].items() if k not in ['game_time','progress']} for row in steps])
    assert sequences[0]==sequences[1]
    props=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{index}-persisted/Profiles/00000001.xml').getroot()} for index in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    changed={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'ProdPct{n}' for n in range(7)}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(changed)<=varying and all(left[key][0]=='Float' for key in changed),changed
    for values in props:
        assert values['Money']==('Int','7084') and values['awQProcess']==('UInt','6')
        for n in range(7):
            assert values[f'Upgrade{n}']==('Int','0') and values[f'ProdType{n}']==('UInt','2') and values[f'ProdCount{n}']==('UInt','2') and values[f'ProdStop{n}']==('UInt','0')
            assert 0<float(values[f'ProdPct{n}'][1])<60
        assert sum(int(values[f'WGoodCount{n}'][1]) for n in range(19))==80 and values['WGoodCount10']==('UInt','6')
    saves=[(work/f'{n}-persisted/Profiles/00000001.save').read_bytes() for n in [0,1]]
    assert saves[0]==saves[1] and len(saves[0])==4089
    report.update(status='passed_for_actual_seven_simultaneous_inflight_buildings_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=18,actual_loaded_grass=14,simultaneous_active_factories=7,all_profile_discretes_equal=True,independent_float_differences=changed,complete_identical_save_bytes=4089,normalizations=[],paused_state_frozen=True,original_profile_or_memory_written=False),
        whole_game_complete=False,pending=['Actual bidirectional restore and completion of all fourteen units','Remaining whole-game, DLL and uninstaller scopes'])
    write(args.output,report);print(report['status'])
if __name__=='__main__':main()
