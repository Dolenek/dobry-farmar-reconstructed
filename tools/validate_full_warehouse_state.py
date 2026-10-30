#!/usr/bin/env python3
"""Audit real full-store blocking and output resumption after an ordinary market transfer."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/full-warehouse-production-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/full-warehouse-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_full_warehouse_output_blocking_and_resumption_only' and len(report['sessions'])==2
    labels=['warehouse-filled','one-grass-loaded','two-grass-loaded','refilled-full-warehouse','output-blocked-at100','grass-price-eight','first-unblocked-milk','second-unblocked-milk','original-time-cutoff-1000']
    comparable=[]
    for session in report['sessions']:
        assert session['exit_code']==0 and session['status']=='completed' and [row['label'] for row in session['catalog_steps']]==labels
        rows={row['label']:row['state'] for row in session['catalog_steps']}
        assert sum(rows['warehouse-filled']['goods'])==15 and sum(rows['two-grass-loaded']['goods'])==13
        for label in ['refilled-full-warehouse','output-blocked-at100','grass-price-eight']:
            at=rows[label];assert sum(at['goods'])==15 and at['count']==2 and at['processed']==2 and at['goods'][10]==2
        for label in ['output-blocked-at100','grass-price-eight']:assert rows[label]['progress']==100
        first,last=session['running_blocked_samples'];assert last['game_time']>first['game_time']
        for at in [first,last]:assert at['building_loaded']==2 and at['building_progress']==100 and at['processed']==2 and at['milk']==2
        released=rows['first-unblocked-milk'];assert released['market_grass']==2 and released['goods'][0]==0 and released['goods'][10]==3 and released['count']==1 and released['processed']==3 and sum(released['goods'])==14
        done=rows['second-unblocked-milk'];assert done['market_grass']==2 and done['grass_price']==8 and done['money']==34 and done['goods'][10]==4 and done['count']==0 and done['processed']==4 and sum(done['goods'])==15
        comparable.append([{key:value for key,value in row['state'].items() if key not in ['progress','game_time']} for row in session['catalog_steps']])
    assert comparable[0]==comparable[1]
    props=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{index}-persisted/Profiles/00000001.xml').getroot()} for index in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    changed={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(changed)<=varying and all(left[key][0]=='Float' for key in changed),changed
    expected={'Money':('Int','34'),'Upgrade0':('Int','0'),'ProdType0':('UInt','2'),'ProdCount0':('UInt','0'),'ProdPct0':('Float','0'),
              'WGoodCount0':('UInt','0'),'WGoodCount10':('UInt','4'),'MGoodCount0':('UInt','2'),'MGoodPrice0':('UInt','8'),'awQProcess':('UInt','4')}
    for values in props:
        for key,value in expected.items():assert values[key]==value,(key,values[key],value)
        assert sum(int(values[f'WGoodCount{kind}'][1]) for kind in range(19))==15
        assert 1000<=float(values['GameTime'][1])<1050
        for field in [18,23,27]:assert values[f'FFS{field}']==('UInt','0') and values[f'FFW{field}']==('Float','0')
    paths=[work/f'{index}-persisted/Profiles/00000001.save' for index in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes()
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_actual_full_warehouse_blocking_market_release_and_milk_completion_only',runtime_report_sha256=digest(args.runtime_report),auditor_sha256=digest(__file__),
        comparison=dict(business_snapshots_per_executable=9,all_profile_discretes_equal=True,independent_float_differences=changed,
            complete_identical_save_bytes=paths[0].stat().st_size,normalizations=[],blocked_with_live_game_clock=True,real_market_transfer=2,produced_after_release=2,
            warehouse_occupied_after_release_and_completion=[14,15],market_units_unsold=2,price_set_through_input=True,original_profile_or_memory_written=False),
        whole_game_complete=False,pending=['All maximum loaded capacities and simultaneous factories','Remaining full-game/audio/JNG/uninstaller scopes'])
    write(args.output,report);print(report['status'])
if __name__=='__main__':main()
