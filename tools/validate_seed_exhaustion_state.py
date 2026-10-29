#!/usr/bin/env python3
"""Audit actual seed consumption, deferred package removal and three newly watered crops."""
import argparse,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/seed-exhaustion-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/seed-exhaustion-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_natural_seed_package_exhaustion_and_three_new_crops_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    def props(i):return {p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()}
    left,right=props(0),props(1);assert len(left)==794 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    varying={'GameTime','RunTime'}|{f'{prefix}{n}' for prefix in ['FFP','FFW','FFF'] for n in [14,15,16,24,25,26,33,34]}
    changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
    for values in [left,right]:
        for name,value in {'Money':('Int','2'),'PCarry':('UInt','1'),'fcu':('Int','1'),'fua':('Int','3'),'WaterLevel':('Float','120'),'MEffPrice0':('Int','-1'),'MEffPrice1':('Int','8'),'awQCollect':('UInt','10'),'letter1':('Int','-1')}.items():assert values[name]==value,(name,values[name])
        for field in [14,15,16]:
            assert values[f'FFC{field}']==('UInt','1') and values[f'FFS{field}']==('UInt','0')
            assert float(values[f'FFW{field}'][1])>118 and float(values[f'FFF{field}'][1])>118
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];states=[load_save(p) for p in paths]
    assert paths[0].read_bytes()==paths[1].read_bytes()
    for state in states:
        assert state['bytes']==2650 and state['seeds']==[] and len(state['sales'])==1
        assert len(state['messages'])==4 and len(state['coefficient_bits'])==383
    for session in report['sessions']:
        rows={row['label']:row['state'] for row in session['work_steps']}
        assert len(rows)==13 and rows['take-three-remaining-seeds']['held']==10
        assert rows['sow-14']['held']==rows['sow-15']['held']==10
        assert rows['sow-16']['held']==rows['empty-package-removed']['held']==0
        assert rows['water-15']['farm_available']==2 and rows['water-16']['farm_available']==3
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    for row in inventory['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_natural_seed_package_exhaustion_and_three_new_crops_only',runtime_report_sha256=digest(args.runtime_report),
        audit_files={n:digest(ROOT/n) for n in ['tools/validate_seed_exhaustion_state.py','tools/game_save_state.py']},
        comparison=dict(profile_properties=794,changed_properties=changed,identical_complete_save_bytes=2650,normalizations=[],remaining_seed_packages=0,new_tomato_fields=[14,15,16],remaining_bucket_water=120,coefficient_bits_equal=383,messages_equal=4),
        pending=['Bidirectional saves of naturally exhausted seed package state','Other crops, upgrades, buildings and all remaining full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
