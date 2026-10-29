#!/usr/bin/env python3
"""Audit paid farm tool upgrades and the natural all-upgraded award in actual games."""
import argparse,json,xml.etree.ElementTree as ET,zipfile
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/tool-upgrades-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/tool-upgrades-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_paid_tool_upgrades_and_natural_award_only' and len(report['sessions'])==2
    original=ROOT.parent/'farmář/Dobrý farmář'
    with zipfile.ZipFile(original/'Data/Data.pack') as archive:costs=[[int(cell.text) for cell in row] for row in ET.fromstring(archive.read('Game.xml')).find('Costs')]
    labels=[f'{name}-level-{level}' for name,last in [('dig',3),('mow',3),('well',2)] for level in range(1,last+1)]+['natural-all-farm-tools-award']
    comparable=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0
        rows=session['catalog_steps'];assert [row['label'] for row in rows]==labels and session['expected_money']==5134
        cash=8134;levels=dict(dig=0,mow=0,well=0);index=0
        for name,item,last in [('dig',14,3),('mow',11,3),('well',13,2)]:
            for level in range(1,last+1):
                cash-=costs[item][level];levels[name]=level;state=rows[index]['state'];index+=1
                assert state['money']==cash and state['warehouse_level']==3 and all(state[key]==value for key,value in levels.items())
        comparable.append([{key:value for key,value in row['state'].items() if key not in ['game_time','progress']} for row in rows])
    assert comparable[0]==comparable[1]
    props=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{index}-persisted/Profiles/00000001.xml').getroot()} for index in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    changed={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(changed)<=varying and all(left[key][0]=='Float' for key in changed),changed
    expected={'Money':('Int','5134'),'Upgrade31':('Int','3'),'Upgrade32':('Int','3'),'Upgrade33':('Int','2'),'Upgrade34':('Int','3'),
              'Award07UpgradeAllFarmDepots':('Bool','1'),'awardShow7':('Bool','1'),'awQProcess':('UInt','6'),'awQDestroy':('UInt','6')}
    for values in props:
        for key,value in expected.items():assert values[key]==value,(key,values[key],value)
    paths=[work/f'{index}-persisted/Profiles/00000001.save' for index in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes() and paths[0].stat().st_size==4089
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/row['path'])==row['sha256']
    report.update(status='passed_for_actual_paid_all_farm_tool_upgrades_and_natural_award7_only',runtime_report_sha256=digest(args.runtime_report),auditor_sha256=digest(__file__),
        comparison=dict(actual_paid_upgrades=8,actual_total_cost=3000,business_snapshots_per_executable=9,all_profile_discretes_equal=True,independent_float_differences=changed,
            complete_identical_save_bytes=4089,normalizations=[],award_requested_by_cheat=False,profile_or_game_memory_written_by_probe=False),
        whole_game_complete=False,pending=['Further farming with each upgraded tool and maximum-capacity water/mow/dig operations','Remaining natural awards, factory levels, in-flight/full-storage production and full-goal scopes'])
    write(args.output,report);print(report['status'])
if __name__=='__main__':main()
