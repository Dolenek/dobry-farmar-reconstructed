#!/usr/bin/env python3
"""Audit five paid, partially consumed packages and actual growing fields."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',default=ROOT/'.local/remaining-crops-sowing-validation.json',type=Path)
    parser.add_argument('--output',default=ROOT/'.local/remaining-crops-sowing-state-validation.json',type=Path)
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_five_remaining_crop_kinds_purchase_sowing_and_watering_only' and len(report['sessions'])==2
    kinds=[2,4,5,6,7];fields=[24,25,26,33,35];costs=[70,50,40,90,25]
    props=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    business=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and len(session['work_steps'])==35
        assert session['fixture_report_sha256']==digest(ROOT/'analysis/factory-history-interop-validation.json')
        money=5234;rows=[]
        for n,(kind,field,cost) in enumerate(zip(kinds,fields,costs)):
            labels=[f'kind-{kind}-take-spade',f'kind-{kind}-dig-{field}',f'kind-{kind}-bought-package',f'kind-{kind}-take-seeds',f'kind-{kind}-sown-{field}',f'kind-{kind}-take-water',f'kind-{kind}-watered-{field}']
            steps=session['work_steps'][n*7:(n+1)*7];assert [r['label'] for r in steps]==labels
            for stage,row in enumerate(steps):
                state=row['state'];assert state['money']==money-(cost if stage>=2 else 0)
                assert state['held']==[2,2,2,10,10,1,1][stage] and state['carried']==state['stock']==[0]*8
                assert state['fields'][n]['crop']==(kind if stage>=4 else 0)
                if stage==1:assert state['fields'][n]['status']==3 and state['fields'][n]['fertility']>119
                if stage==6:assert state['fields'][n]['water']>118 and state['water']==360
                rows.append({k:state[k] for k in ['money','held','carried','stock','grow_mask','water']}|dict(fields=[{k:v[k] for k in ['id','crop','status']} for v in state['fields']]))
            money-=cost
        business.append(rows)
    assert business[0]==business[1]
    left,right=props;assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
    differences={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
    varying={'GameTime','RunTime'}|{f'letTime{n}' for n in range(3)}|{f'{p}{n}' for p in ['FFP','FFW','FFF'] for n in range(60)}
    assert set(differences)<=varying,differences
    for values in props:
        assert values['Money']==('Int','4959') and values['PCarry']==('UInt','1') and values['WaterLevel']==('Float','360')
        for crop,field in zip(kinds,fields):assert values[f'FFC{field}']==('UInt',str(crop)) and values[f'FFS{field}']==('UInt','0')
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];saves=[load_save(path) for path in paths];masked=[]
    for path,state in zip(paths,saves):
        assert len(state['seeds'])==5 and [s['kind'] for s in state['seeds']]==kinds
        assert all(s['count']==3 and s['status']==0 and not s['unpaid'] for s in state['seeds'])
        raw=bytearray(path.read_bytes())
        for a,b in state['seed_padding_ranges']:raw[a:b]=b'\0'*(b-a)
        masked.append(raw)
    assert masked[0]==masked[1], 'Save differs beyond original seed padding'
    assert saves[0]['sales']==saves[1]['sales'] and saves[0]['messages']==saves[1]['messages'] and saves[0]['coefficient_bits']==saves[1]['coefficient_bits']
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_five_remaining_crop_kinds_purchase_sowing_and_watering_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=35,kinds=kinds,fields=fields,paid_cost=sum(costs),profile_properties=len(left),all_profile_discretes_equal=True,independent_float_differences=differences,
            save_bytes=saves[0]['bytes'],save_bytes_exact=paths[0].read_bytes()==paths[1].read_bytes(),normalizations=[dict(kind='original_uninitialized_seed_padding_only',ranges=saves[0]['seed_padding_ranges'])],
            five_packages_remaining_count=3,messages_equal=len(saves[0]['messages']),coefficient_bits_equal=len(saves[0]['coefficient_bits']),original_profile_fields_modified_by_probe=False),whole_game_complete=False,
        pending=['Resume and mature these five crop kinds in the opposite program','Natural award11, actual harvests and further saves','Remaining full-game, Squall, JNG and uninstaller scopes'])
    write(args.output,report);print(report['status']);print('save bytes',saves[0]['bytes'],'raw equal',report['comparison']['save_bytes_exact'])


if __name__=='__main__':main()
