#!/usr/bin/env python3
"""Audit natural delayed offer37, charged acceptance and active230-second save."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image,ImageChops
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from validate_factory_history_state import resource
from contract_offer_pixels import audit_offer


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/contract-delay-acceptance-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/contract-delay-acceptance-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_natural_rose_contract37_delay_acceptance_and_active_save_only' and len(report['sessions'])==2
    definition=dict(index=37,penalty=30,bonus=90,win_time_seconds=240,product=4,count=4,initial_stock=2,coefficient_bits=0)
    comparable=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and session['contract_definition']==definition
        assert session['fixture_report_sha256']==digest(ROOT/'analysis/irrigation-interop-validation.json')
        steps=session['work_steps'];assert [r['label'] for r in steps]==['natural-rose-contract37-arrived','offer-delayed-without-charge','rose-contract-accepted','actual-countdown230']
        assert [r['state']['money'] for r in steps]==[4534,4534,4504,4504]
        assert all(r['state']['stock_roses']==2 and r['state']['sprinklers']==[True]*3 for r in steps)
        assert steps[1]['state']['letters'][session['offer_slot']]['id']==37 and all(t['id']==-1 for t in steps[1]['state']['tasks'])
        assert steps[2]['state']['tasks'][0]==dict(id=37,remaining=240.,pass_cancel=True)
        assert steps[3]['state']['tasks'][0]==session['final_state']['tasks'][0]==dict(id=37,remaining=230.,pass_cancel=True)
        comparable.append([{k:v for k,v in r['state'].items() if k not in ['game_time','run_time','letters']}|dict(letter_ids=[n['id'] for n in r['state']['letters']]) for r in steps])
    assert comparable[0]==comparable[1]
    props=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
    differences={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
    varying={'GameTime','RunTime','letTime0','letTime1','letTime2'}|{f'{prefix}{n}' for prefix in ['FFF','FFW','FFP'] for n in range(60)}
    assert set(differences)<=varying,differences
    for values in props:
        for k,v in dict(Money=('Int','4504'),task0=('Int','37'),taskTime0=('Float','230'),taskPassCancel0=('Bool','1'),WGoodCount4=('UInt','2'),letter0=('Int','-1')).items():assert values[k]==v,(k,values[k])
    fixture=read(ROOT/'analysis/irrigation-interop-validation.json');before=load_save(ROOT/fixture['work_directory']/'3-persisted/Profiles/00000001.save')
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]]
    assert paths[0].read_bytes()==paths[1].read_bytes() and paths[0].stat().st_size==4089
    assert load_save(paths[0])==before
    events=list(resource('Gameplay/gameplay.xml').find('Events'));events.sort(key=lambda e:sum(int(v)*scale for v,scale in zip(e.get('Time').split(':'),[3600,60,1])))
    event=events[37];assert event.get('Time')=='0:18:30' and event.find('Data').get('Penalty')=='30' and before['coefficient_bits'][37]==0
    pixels=[]
    for index,name in enumerate(['rose-offer-before-wait','rose-offer-reopened']):
        offers=[session['offers'][index] for session in report['sessions']]
        assert all(offer['label']==name for offer in offers)
        pixels.append(audit_offer(ROOT,work,name,[offer['scene'] for offer in offers]))
    report.update(status='passed_for_natural_rose_contract_delay_acceptance_and_active_save_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=4,all_profile_discretes_equal=True,independent_float_differences=differences,original_contract=definition,
            actual_offer_pixels=pixels,actual_remaining_seconds=230,active_task_saved=True,complete_identical_save_bytes=4089,normalizations=[]),whole_game_complete=False,
        pending=['Opposite-program restoration and actual deadline failure','Further complete-process saves of failed contract','Remaining gameplay, Squall, JNG and uninstaller scopes'])
    write(args.output,report);print(report['status']);print('two118-sprite offers equal; two36400-pixel notes exact; translucent backdrop bound1;4089 bytes exact; active task37/time230')


if __name__=='__main__':main()
