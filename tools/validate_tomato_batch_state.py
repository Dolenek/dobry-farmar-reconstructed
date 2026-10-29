#!/usr/bin/env python3
"""Audit actual four-crop capacity, price clicks, sales and saved state."""
import argparse,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/tomato-batch-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/tomato-batch-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_original_fast_mode_four_crop_batch_and_market_price_input_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    properties=[{p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=properties;assert len(left)==794 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    varying={'GameTime','RunTime','letTime0'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in [14,15,16,24,25,26,33,34]}
    changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
    for values in properties:
        for name,value in {'Money':('Int','34'),'PCarry':('UInt','6'),'fcu':('Int','1'),'fua':('Int','3'),'MGoodPrice1':('UInt','8'),
                           'PCC1':('UInt','0'),'WGoodCount1':('UInt','0'),'MGoodCount1':('UInt','0'),'awQCollect':('UInt','14'),
                           'UpAvail11':('UInt','1'),'UpAvail14':('UInt','1'),'letter0':('Int','16'),'tip28':('Bool','1')}.items():assert values[name]==value,(name,values[name])
        for field in [14,15,16,34]:assert values[f'FFC{field}']==('UInt','1') and values[f'FFS{field}']==('UInt','0')
    expected_labels=['four-tomato-fields-ready','take-mower','harvest-14','harvest-15','harvest-16','store-three-tomatoes','harvest-fourth-tomato','store-four-tomatoes','sold-four-tomatoes-at-eight']
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0
        assert [row['label'] for row in session['work_steps']]==expected_labels
        assert session['initial_state']['money']==2 and session['final_state']['money']==34
        assert session['initial_tomato_price']==2 and session['price_clicks']==[dict(before=n,after=n+1) for n in range(2,8)]
        assert [row['step'] for row in session['stages']]==[28]
    states=[load_save(work/f'{i}-persisted/Profiles/00000001.save') for i in [0,1]]
    for state in states:
        assert state['bytes']==3013 and state['seeds']==[] and len(state['sales'])==4 and all(s['amount']==8 for s in state['sales'])
        assert len(state['messages'])==5 and len(state['coefficient_bits'])==383
    assert states[0]['messages']==states[1]['messages'] and states[0]['coefficient_bits']==states[1]['coefficient_bits']
    ranges=states[0]['sale_clock_ranges'];assert ranges==states[1]['sale_clock_ranges']
    payloads=[bytearray((work/f'{i}-persisted/Profiles/00000001.save').read_bytes()) for i in [0,1]]
    for payload in payloads:
        for first,last in ranges:payload[first:last]=b'\0'*(last-first)
    assert payloads[0]==payloads[1],'Save differs outside independently timed sales and cutoff'
    for row in json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_actual_four_tomato_crops_capacity_price_input_and_batch_sale_only',runtime_report_sha256=digest(args.runtime_report),
        audit_files={n:digest(ROOT/n) for n in ['tools/validate_tomato_batch_state.py','tools/game_save_state.py']},
        comparison=dict(profile_properties=794,changed_properties=changed,save_bytes=3013,normalized_byte_ranges=ranges,
            sale_records=[s['sales'] for s in states],sale_amounts=[8]*4,messages_equal=5,coefficient_bits_equal=383,seed_packages=0,
            actual_time_shortcut='Original Alt+Insert15x for growth, restored1x before work; no profile/time writes'),
        pixel_regions=[row['pixel_comparison'] for row in report['sessions'][1]['stages']],
        pending=['Same-input bidirectional saves of this aged four-sale history','Contract16, natural upgrades, other crops, production and remaining full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
