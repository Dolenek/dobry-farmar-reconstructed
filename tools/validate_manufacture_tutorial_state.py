#!/usr/bin/env python3
"""Audit actual manufacture input, original View alias words and whole saves."""
import argparse,hashlib,json,struct,xml.etree.ElementTree as ET
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def signed(value):return struct.unpack('<i',struct.pack('<I',value&0xffffffff))[0]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/manufacture-focused-fix-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/manufacture-tutorial-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_natural_manufacture_tutorial_build_feed_upgrade_and_demolition_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    properties=[{p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=properties;assert len(left)==795 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    aliases=[];expected_steps=[30,31,32,35,36,37,38,48,34,33]
    for index,(values,session) in enumerate(zip(properties,report['sessions'])):
        assert session['status']=='completed' and session['exit_code']==0 and [row['step'] for row in session['stages']]==expected_steps
        for name,value in {'Money':('Int','94'),'PCarry':('UInt','2'),'Upgrade0':('Int','-1'),'ProdType0':('UInt','2'),
                           'ProdCount0':('UInt','0'),'ProdPct0':('Float','0'),'WGoodCount0':('UInt','0'),'awQDestroy':('UInt','1'),
                           'awQProcess':('UInt','0'),'awManDepotBuild2':('Bool','1'),'XProdPos0':('Float','400'),'YProdPos0':('Float','250')}.items():assert values[name]==value,(index,name,values[name])
        for step in expected_steps:assert values[f'tip{step}']==('Bool','1')
        rows={row['label']:row['state'] for row in session['work_steps']};assert len(rows)==3
        assert rows['loaded-first-building']['building_level']==0 and rows['loaded-first-building']['building_loaded']==1 and rows['loaded-first-building']['grass_stock']==0 and rows['loaded-first-building']['extra_upgrade'] is None
        assert rows['tutorial-free-upgrade']['building_level']==1 and rows['tutorial-free-upgrade']['building_loaded']==1
        assert rows['tutorial-demolition']['building_level']==-1 and rows['tutorial-demolition']['building_loaded']==0
        words=session['focused_view_alias_words'];key='Upgrade'+str(words['property_id']);limit=signed(words['level_count_bits']);expected=signed(limit-1) if 0>=limit else 0
        assert values[key]==('Int',str(expected)) and rows['tutorial-free-upgrade']['extra_upgrade']==expected
        aliases.append(dict(key=key,property_id=words['property_id'],level_count_bits=words['level_count_bits'],signed_level_count=limit,stored_value=expected,padding_high16=words['level_count_bits']>>16))
    assert aliases[0]['key']==aliases[1]['key']=='Upgrade16843009'
    varying={'GameTime','RunTime','letTime0','WaterLevel',aliases[0]['key']}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in [14,15,16,17,18,23,24,25,26,27,33,34,35]}
    changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes()
    for path in paths:
        state=load_save(path);assert state['bytes']==3557 and state['seeds']==[] and len(state['sales'])==4 and len(state['messages'])==7 and len(state['coefficient_bits'])==383
    pixels=[row['pixel_comparison'] for row in report['sessions'][1]['stages']];exact=sum(row['pixels'] for row in pixels if row['different_pixels']==0)
    assert exact==200200 and sum(row['pixels'] for row in pixels)==286000
    for row in json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_manufacture_tutorial_with_recorded_allocation_derived_property_only',runtime_report_sha256=digest(args.runtime_report),
        audit_files={n:digest(ROOT/n) for n in ['tools/validate_manufacture_tutorial_state.py','tools/game_save_state.py']},
        comparison=dict(profile_properties=795,changed_properties=changed,allocation_derived_profile_values=aliases,allocation_value_normalized_as_equal=False,
            identical_complete_save_bytes=3557,normalizations=[],messages_equal=7,coefficient_bits_equal=383,exact_tip_rgb_pixels=exact,compared_tip_rgb_pixels=286000,
            pending_real_production=True,actual_tutorial_build_type=2,actual_feed=1,actual_upgrade=1,actual_demolition=1),
        pending=['Independent-process allocation padding and full original object ABI remain distinct from identical-state native replay',
            'Live-building restores and production, cucumber completion, paid upgrades and all remaining full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
