#!/usr/bin/env python3
"""Audit actual purchase/feed/production/demolition sequences for selected native factories."""
import argparse,hashlib,json,xml.etree.ElementTree as ET,zipfile
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/factory-catalog-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/factory-catalog-state-validation.json')
    args=p.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_actual_catalog_recipes_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    kinds=report['catalog_kinds'];definitions=[('SheepFold',3,9),('HenHouse',7,8),('CowShed',0,10),('PigPen',1,11),('CheeseDairy',13,12),('Creamery',10,13),('Weavery',9,14),('Incubator',8,15),('TextileDep',14,16),('Bakery',17,18),('Mill',7,17)]
    original=ROOT.parent/'farmář/Dobrý farmář'
    with zipfile.ZipFile(original/'Data/Data.pack') as archive:
        cost_table=[[int(cell.text) for cell in row] for row in ET.fromstring(archive.read('Game.xml')).find('Costs')]
        costs=[row[0] for row in cost_table]
        times={kind:float(ET.fromstring(archive.read(f'Manufacture/{definitions[kind][0]}.xml')).find('Upgrades')[0].get('MaxProductionTime')) for kind in kinds}
    comparable=[];durations=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and [row['kind'] for row in session['recipes']]==kinds
        steps=session['catalog_steps'];expected_labels=['funded']+[f'warehouse-level-{n}' for n in [1,2,3]]+['all-recipe-inputs','remove-baseline-cowshed']+[f'kind-{kind}-{phase}' for kind in kinds for phase in ['built','loaded','produced','destroyed']]
        assert [row['label'] for row in steps]==expected_labels
        rows={row['label']:row['state'] for row in steps};assert rows['funded']['money']==10034
        cash=10034
        for level in [1,2,3]:
            cash-=cost_table[12][level]
            assert rows[f'warehouse-level-{level}']['money']==cash and rows[f'warehouse-level-{level}']['warehouse_level']==level
        goods=[2]*19;goods[10]=4;assert rows['all-recipe-inputs']['goods']==goods and cash==8934
        processed=2
        for kind in kinds:
            name,input_kind,output_kind=definitions[kind];built=rows[f'kind-{kind}-built'];loaded=rows[f'kind-{kind}-loaded'];produced=rows[f'kind-{kind}-produced'];destroyed=rows[f'kind-{kind}-destroyed']
            cash-=costs[kind];assert built['money']==cash and built['kind']==kind and built['level']==0 and built['goods']==goods
            goods[input_kind]-=1;assert loaded['goods']==goods and loaded['count']==1 and loaded['processed']==processed
            goods[output_kind]+=1;processed+=1
            assert produced['goods']==goods and produced['processed']==processed and produced['count']==0 and produced['progress']==0
            assert destroyed['level']==-1 and destroyed['goods']==goods and destroyed['money']==cash
            duration=produced['game_time']-loaded['game_time'];assert duration>=times[kind]-1,(kind,duration,times[kind])
            durations.append(dict(executable=session['kind'],kind=kind,xml_seconds_per_unit=times[kind],observed_game_seconds=duration))
        comparable.append([{key:value for key,value in row['state'].items() if key not in ['game_time','progress']} for row in steps])
    assert comparable[0]==comparable[1],'Actual factory business state differs'
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];states=[load_save(path) for path in paths]
    assert paths[0].read_bytes()==paths[1].read_bytes(),'Complete factory save differs'
    props=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    changed={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(changed)<=varying and all(left[key][0]=='Float' for key in changed),changed
    for values in props:
        for key,value in {'Money':('Int',str(cash)),'Upgrade32':('Int','3'),'Upgrade0':('Int','-1'),'awQProcess':('UInt',str(processed)),
                          'awQDestroy':('UInt',str(2+len(kinds))),'awardShow16':('Bool','1'),'awardShow17':('Bool','1')}.items():assert values[key]==value,(key,values[key],value)
        for kind in kinds:assert values[f'awManDepotBuild{kind}']==('Bool','1')
        for product,value in enumerate(goods):assert values[f'WGoodCount{product}']==('UInt',str(value))
    for row in json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))['files']:assert digest(original/row['path'])==row['sha256']
    report.update(status='passed_for_selected_actual_factory_recipes_purchase_feed_produce_destroy_only',
        runtime_report_sha256=digest(args.runtime_report),auditor_sha256=digest(__file__),
        comparison=dict(factory_kinds=kinds,matching_business_snapshots=len(comparable[0]),complete_identical_save_bytes=states[0]['bytes'],normalizations=[],
            all_profile_names_types_and_discrete_values_equal=True,recorded_independent_float_differences=changed,all_profile_values_claimed_equal=False,
            unit_durations=durations,commands='Original cheatupgrades,10*cheatmoney,cheatproduct via WM_CHAR/GameInput; no state-memory or fixture writes',
            original_time_shortcut_used=False,source_game_code_calls_reference=False),whole_game_complete=False,
        pending=['Other factory kinds, levels, in-flight saves, simultaneous houses and other full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
