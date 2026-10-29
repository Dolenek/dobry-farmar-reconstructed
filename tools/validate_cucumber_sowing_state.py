#!/usr/bin/env python3
"""Audit purchased cucumber seeds, natural regrowth, four plots and complete saves."""
import argparse,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/cucumber-sowing-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/cucumber-sowing-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_natural_cucumber_purchase_sowing_and_watering_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    props=[{p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert len(left)==795 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    fields=[14,15,16,17,18,23,24,25,26,27,33,34,35]
    varying={'GameTime','RunTime'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in fields}
    changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
    for values in props:
        for name,value in {'Money':('Int','64'),'PCarry':('UInt','1'),'WaterLevel':('Float','0'),'SeedAvail3':('UInt','1'),
                           'fcu':('Int','1'),'Upgrade0':('Int','-1'),'awQDestroy':('UInt','1'),'MEffPrice3':('Int','6'),
                           'Upgrade16843009':('Int','-1163067136')}.items():assert values[name]==value,(name,values[name])
        for field in [17,18,23,27]:
            assert values[f'FFC{field}']==('UInt','3') and values[f'FFS{field}']==('UInt','0')
            assert float(values[f'FFW{field}'][1])>118 and float(values[f'FFF{field}'][1])>118
        for field in [14,15,16,34]:assert values[f'FFC{field}']==('UInt','1') and values[f'FFS{field}']==('UInt','1')
    expected=['bought-cucumber-package','previous-tilled-fields-regrew','dig-17','dig-18','dig-23','dig-27','take-cucumber-seeds','sow-17','sow-18','sow-23','sow-27','empty-cucumber-package-removed','filled-bucket','water-17','water-18','water-23','water-27']
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and [row['label'] for row in session['work_steps']]==expected
        assert session['initial_state']['money']==94 and session['final_state']['money']==64
        rows={row['label']:row['state'] for row in session['work_steps']}
        assert rows['take-cucumber-seeds']['held']==10 and rows['empty-cucumber-package-removed']['held']==0 and rows['filled-bucket']['held']==1
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes()
    for path in paths:
        state=load_save(path);assert state['bytes']==3557 and state['seeds']==[] and len(state['sales'])==4 and len(state['messages'])==7 and len(state['coefficient_bits'])==383
    for row in json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_natural_cucumber_purchase_regrowth_sowing_and_watering_only',runtime_report_sha256=digest(args.runtime_report),
        audit_files={n:digest(ROOT/n) for n in ['tools/validate_cucumber_sowing_state.py','tools/game_save_state.py']},
        comparison=dict(profile_properties=795,changed_properties=changed,identical_complete_save_bytes=3557,normalizations=[],messages_equal=7,coefficient_bits_equal=383,
            new_cucumber_fields=[17,18,23,27],mature_tomato_fields=[14,15,16,34],purchased_seed_cost=30,remaining_packages=0,remaining_bucket_water=0,
            original_extra_profile_property_preserved=True,original_ploughed_to_grass_seconds=60,inserted_game_state=False),
        pending=['Cucumber harvest/sale and bidirectional saves of eight planted crops','Paid tools, live production, other crops and remaining full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
