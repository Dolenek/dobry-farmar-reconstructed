#!/usr/bin/env python3
"""Audit actual contract16 acceptance, fulfillment, exact dialog pixels and saves."""
import argparse,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/tomato-contract-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/tomato-contract-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_natural_tomato_contract16_acceptance_and_fulfillment_only' and len(report['sessions'])==2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    properties=[{p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=properties;assert len(left)==794 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    varying={'GameTime','RunTime','letTime0','WaterLevel'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in [14,15,16,24,25,26,33,34]}
    changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
    for values in properties:
        for name,value in {'Money':('Int','94'),'PCarry':('UInt','6'),'task0':('Int','-1'),'taskTime0':('Float','90'),
                           'SeedAvail3':('UInt','1'),'tip27':('Bool','1'),'letter0':('Int','-1'),
                           'PCC1':('UInt','0'),'WGoodCount1':('UInt','0'),'awQCollect':('UInt','18'),
                           'awQTask':('UInt','1'),'req0_0':('Int','0'),'req0_1':('Int','0')}.items():assert values[name]==value,(name,values[name])
        for field in [14,15,16,34]:assert values[f'FFC{field}']==('UInt','1') and values[f'FFS{field}']==('UInt','0') and float(values[f'FFW{field}'][1])>100
    for index,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0 and len(session['work_steps'])==16
        rows={row['label']:row['state'] for row in session['work_steps']}
        assert session['initial_state']['money']==34 and session['final_state']['money']==94
        assert rows['accepted-tomato-contract']['money']==14 and rows['accepted-tomato-contract']['task']==16
        assert rows['store-three-tomatoes']['stock']==3 and rows['store-four-tomatoes']['stock']==4
        # The recorded scalar snapshot precedes each predicate read, so a value
        # can lag the successful work predicate by a frame. Do not call it atomic.
        assert [row['step'] for row in session['stages']]==[27]
        for name,suffix in [('offer_region_sha256','10-tomato-offer'),('confirmation_region_sha256','11-confirm-completion')]:
            payload=Image.open(work/f'{index}-{suffix}.png').crop((210,170,590,405)).tobytes()
            assert hashlib.sha256(payload).hexdigest()==session[name] and session[name]==report['sessions'][0][name]
    tip=report['sessions'][1]['stages'][0]['pixel_comparison'];assert tip['pixels']==28600 and tip['different_pixels']==0 and tip['max_channel_difference']==0
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes()
    states=[load_save(p) for p in paths]
    for state in states:assert state['bytes']==3278 and state['seeds']==[] and len(state['sales'])==4 and len(state['messages'])==6 and len(state['coefficient_bits'])==383
    for row in json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_natural_tomato_contract16_acceptance_and_fulfillment_only',runtime_report_sha256=digest(args.runtime_report),
        audit_files={n:digest(ROOT/n) for n in ['tools/validate_tomato_contract_state.py','tools/game_save_state.py']},
        comparison=dict(profile_properties=794,changed_properties=changed,identical_complete_save_bytes=3278,normalizations=[],messages_equal=6,coefficient_bits_equal=383,
            actual_contract=16,deposit=20,bonus=80,final_money=94,exact_dialog_rgb_pixels=178600,exact_tip27_rgb_pixels=28600,
            work_snapshots_atomic=False,implementation_dll_scope='This gameplay pair used the committed2745060 factor-fixed DLL; newer vector fix has separate integration evidence'),
        pending=['Bidirectional saves of the completed tomato contract','Natural tool upgrades, cucumber crop, buildings and remaining full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
