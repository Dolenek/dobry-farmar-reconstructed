#!/usr/bin/env python3
"""Audit actual paused cowshed input count and uncompleted production progress."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/inflight-production-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/inflight-production-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_paused_inflight_cowshed_only' and len(report['sessions'])==2
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0
        assert [row['label'] for row in session['catalog_steps']]==['warehouse-filled','one-grass-loaded','two-grass-loaded','partial-production']
        frozen=session['frozen_inflight_state'];assert frozen==session['final_state'] and frozen['building_loaded']==2 and 10<frozen['building_progress']<50 and frozen['milk']==2 and frozen['processed']==2
    props=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{index}-persisted/Profiles/00000001.xml').getroot()} for index in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    changed={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    varying={'GameTime','RunTime','letTime0','letTime1','ProdPct0'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(changed)<=varying and all(left[key][0]=='Float' for key in changed),changed
    expected={'Money':('Int','34'),'Upgrade0':('Int','0'),'ProdType0':('UInt','2'),'ProdCount0':('UInt','2'),'ProdStop0':('UInt','0'),
              'WGoodCount0':('UInt','0'),'WGoodCount10':('UInt','2'),'awQProcess':('UInt','2')}
    for values in props:
        for key,value in expected.items():assert values[key]==value,(key,values[key],value)
        assert 10<float(values['ProdPct0'][1])<50
        assert sum(int(values[f'WGoodCount{kind}'][1]) for kind in range(19))==13
    paths=[work/f'{index}-persisted/Profiles/00000001.save' for index in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes() and paths[0].stat().st_size==3730
    report.update(status='passed_for_actual_paused_inflight_cowshed_fixture_only',runtime_report_sha256=digest(args.runtime_report),auditor_sha256=digest(__file__),
        comparison=dict(all_profile_discretes_equal=True,independent_float_differences=changed,complete_identical_save_bytes=3730,normalizations=[],
            loaded_grass=2,milk_before_completion=2,warehouse_occupied=13,original_Alt_Pause=True,inserted_profile_or_memory_state=False),
        whole_game_complete=False,pending=['Actual cross-executable restoration and completion of partial production','Warehouse-full blocking, production levels and remaining full-goal scopes'])
    write(args.output,report);print(report['status'])
if __name__=='__main__':main()
