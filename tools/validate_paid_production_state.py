#!/usr/bin/env python3
"""Audit actual paid construction and grass-to-milk production in complete EXEs."""
import argparse,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/paid-production-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/paid-production-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_paid_cowshed_and_two_actual_milk_cycles_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    props=[{p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert len(left)==795 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in [14,15,16,17,18,23,24,25,26,27,33,34,35]}
    changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
    expected={'Money':('Int','34'),'PCarry':('UInt','6'),'WaterLevel':('Float','240'),'Upgrade0':('Int','0'),'ProdType0':('UInt','2'),
              'XProdPos0':('Float','400'),'YProdPos0':('Float','250'),'ProdCount0':('UInt','0'),'ProdPct0':('Float','0'),'ProdStop0':('UInt','0'),
              'WGoodCount10':('UInt','2'),'WGoodCount0':('UInt','0'),'awQCollect':('UInt','32'),'awQProcess':('UInt','2'),
              'TProdCompl':('Bool','1'),'tip43':('Bool','1'),'tip26':('Bool','1'),'letter0':('Int','26'),'letter1':('Int','30')}
    for values in props:
        for name,value in expected.items():assert values[name]==value,(name,values[name])
    labels=['filled-grass-bucket','water-grass-24','water-grass-25','two-grass-plots-ready','take-grass-mower','harvest-grass-24','harvest-grass-25','stored-two-grass','paid-cowshed-built','loaded-one-grass','loaded-two-grass','first-real-milk','second-real-milk']
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and [row['label'] for row in session['work_steps']]==labels
        assert session['initial_state']['money']==184 and session['initial_state']['building_level']==-1
        rows={row['label']:row['state'] for row in session['work_steps']}
        assert rows['stored-two-grass']['stock']==2 and rows['paid-cowshed-built']['money']==34 and rows['paid-cowshed-built']['building_level']==0
        assert rows['loaded-two-grass']['stock']==0 and rows['loaded-two-grass']['building_loaded']==2 and rows['loaded-two-grass']['milk']==0
        for label,produced,loaded in [('first-real-milk',1,1),('second-real-milk',2,0)]:
            row=rows[label];assert row['milk']==row['processed']==produced and row['building_loaded']==loaded and row['money']==34
        assert [row['step'] for row in session['stages']]==[26,43]
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes()
    for path in paths:
        state=load_save(path);assert state['bytes']==3730 and state['seeds']==[] and len(state['sales'])==4 and len(state['messages'])==8 and len(state['coefficient_bits'])==383
    for row in json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_actual_paid_cowshed_grass_harvest_and_two_milk_cycles_only',runtime_report_sha256=digest(args.runtime_report),
        audit_files={n:digest(ROOT/n) for n in ['tools/validate_paid_production_state.py','tools/game_save_state.py']},
        comparison=dict(profile_properties=795,changed_properties=changed,identical_complete_save_bytes=3730,normalizations=[],messages_equal=8,coefficient_bits_equal=383,
            actual_building_cost=150,harvested_grass=2,produced_milk=2,remaining_live_building=dict(kind=2,level=0,x=400,y=250),
            original_time_shortcut='15x only to mature watered grass; all harvest/build/feed/production actions1x',
            tutorial_pixels=[row['pixel_comparison'] for row in report['sessions'][1]['stages']],inserted_game_state=False),
        pending=['Bidirectional save restoration with the purchased live building and milk stock','Remaining10 building types, interrupted production, paid tools and all full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
