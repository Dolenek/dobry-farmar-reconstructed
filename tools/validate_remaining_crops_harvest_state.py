#!/usr/bin/env python3
"""Audit opposite-program harvests, timed25 loss, storage and complete crop saves."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from validate_factory_history_state import resource


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/remaining-crops-harvest-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/remaining-crops-harvest-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_cross_program_harvest_and_storage_of_five_remaining_crops_only' and len(report['sessions'])==2
    fixture=read(ROOT/'.local/remaining-crops-growth-state-validation.json');fixture_work=checked(fixture)
    props=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    kinds=[2,4,5,6,7];fields=[24,25,26,33,35];business=[];saves=[];masked=[];time_deductions=[]
    event=next(e for e in resource('Gameplay/gameplay.xml').find('Events') if e.get('Type')=='2' and e.get('Time')=='0:25:15')
    assert event.find('Data').get('Value')=='-25' and event.find('Data').get('RandomFactor')=='1.0'
    for i,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0 and len(session['work_steps'])==12
        assert session['fixture_report_sha256']==digest(ROOT/'.local/remaining-crops-growth-state-validation.json')
        assert session['restore_from_kind']==('source' if session['kind']=='native' else 'native')
        assert session['final_collected']==session['initial_collected']+5
        steps=session['work_steps'];assert steps[0]['label']=='take-mower' and steps[-1]['label']=='original-time-cutoff1598'
        product_rows=[]
        for n,(crop,field) in enumerate(zip(kinds,fields)):
            for stored in [False,True]:
                row=steps[1+n*2+int(stored)];state=row['state']
                assert row['label']==(f'store-kind-{crop}' if stored else f'harvest-kind-{crop}-field-{field}')
                expected=[0]*8
                if not stored:expected[crop]=1
                assert state['carried']==expected
                stock=[int(c in kinds[:n+int(stored)]) for c in range(8)];assert state['stock']==stock
                assert state['held']==6 and state['fields'][n]['crop']==crop and state['fields'][n]['status']==0
                product_rows.append(dict(carried=state['carried'],stock=state['stock'],held=state['held']))
        for row in steps:
            at=row['state'];assert at['money']==4959-(25 if at['game_time']>=1515 else 0)
        assert 1598<=steps[-1]['state']['game_time']<1603
        time_deductions.append([dict(label=row['label'],game_time=row['state']['game_time'],money=row['state']['money']) for row in steps])
        business.append(product_rows)
        values=props[i]
        assert values['Money']==('Int','4934') and values['PCarry']==('UInt','6') and values['awGrowType']==('Int','11')
        for key in ['Award11GrowAllAtOneTime','Award13Play30Minute','awardShow11','awardShow13','tip49']:assert values[key]==('Bool','1')
        for crop,field in zip(kinds,fields):assert values[f'WGoodCount{crop}']==('UInt','1') and values[f'PCC{crop}']==('UInt','0') and values[f'FFS{field}']==('UInt','0')
        path=work/f'{i}-persisted/Profiles/00000001.save';saved=load_save(path);before=load_save(fixture_work/f'{1-i}-persisted/Profiles/00000001.save')
        assert saved['messages']==before['messages']+[dict(read=0,text=event.findtext('Note'))]
        assert saved['sales']==before['sales'] and saved['cutoff']==before['cutoff'] and saved['coefficient_bits']==before['coefficient_bits']
        for old,new in zip(before['seeds'],saved['seeds']):
            assert new['x']==old['x']+1 and all(new[k]==old[k] for k in ['y','kind','count','status','unpaid'])
        assert len(saved['seeds'])==5
        raw=bytearray(path.read_bytes())
        for a,b in saved['seed_padding_ranges']:raw[a:b]=b'\0'*(b-a)
        saves.append(saved);masked.append(raw)
    assert business[0]==business[1] and masked[0]==masked[1]
    left,right=props;assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
    differences={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
    varying={'GameTime','RunTime','letTime0','letTime1','letTime2'}|{f'{prefix}{n}' for prefix in ['FFP','FFW','FFF'] for n in range(60)}
    assert set(differences)<=varying,differences
    report.update(status='passed_for_cross_program_five_crop_harvests_and_storage_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(actual_cross_program_harvest_restores=2,business_snapshots_per_executable=12,matching_product_snapshots_per_executable=10,all_profile_discretes_equal=True,independent_float_differences=differences,
            actual_timed_money_deduction=dict(event_time=1515,amount=-25,random_factor=1,observations=time_deductions),complete_save_bytes=saves[0]['bytes'],complete_save_equal_except_seed_padding=True,
            normalizations=[dict(kind='original_uninitialized_seed_padding_only',ranges=saves[0]['seed_padding_ranges'])],new_journal_message=event.findtext('Note'),remaining_seed_packages=5,stocked_new_kinds=kinds),whole_game_complete=False,
        pending=['Further complete-process bidirectional transfers of stocked crops and packages','Remaining weather, irrigation, contract and natural-award gameplay scopes','Remaining Squall, JNG and uninstaller scopes'])
    write(args.output,report);print(report['status']);print('save bytes',saves[0]['bytes'])


if __name__=='__main__':main()
