#!/usr/bin/env python3
"""Independently audit actual paid demolitions,the native20 threshold and saves."""
import argparse,struct,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
from validate_factory_history_state import resource
from game_save_state import load_save

def main():
    p=argparse.ArgumentParser();p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/demolition-award-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/demolition-award-state-validation.json');args=p.parse_args()
    report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_paid_seven_demolitions_and_natural_award10_only' and len(report['sessions'])==2
    baseline=read(ROOT/'analysis/factory-history-state-validation.json');base=ROOT/baseline['work_directory']/'0-persisted/Profiles'
    costs=[int(list(row)[0].text) for row in resource('Game.xml').find('Costs')][:11];assert costs[7]==100
    values=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    alias_keys=set();comparable=[]
    for i,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0 and len(session['awards'])==1 and session['costs']==costs
        assert session['initial_destroyed']==13 and session['initial_money']==5234 and session['fixture_report_sha256']==digest(ROOT/'analysis/factory-history-state-validation.json')
        steps=session['catalog_steps'];assert len(steps)==17
        for n in range(7):
            for built in [False,True]:
                label=f'cycle-{n}-'+('built' if built else 'destroyed');row=next(r for r in steps if r['label']==label);s=row['state']
                assert s['money']==5234-100*(n+int(built)) and s['destroyed']==14+n
                assert s['levels']==[0]*6+([0] if built else [-1])+[-1]*13 and s['loaded']==[0]*7
                assert s['types']==[2]*6+[7 if built or n else 10] and s['history']==[True]*11
                if n<6:assert not s['award'] and not s['award_shown']
        for label in ['natural-twenty-demolition-award-before-rebuild','natural-twenty-demolition-award','original-time-cutoff1240']:
            row=next(r for r in steps if r['label']==label);assert row['state']['destroyed']==20 and row['state']['award'] and row['state']['award_shown']
        assert 1240<=steps[-1]['state']['game_time']<1245
        entries=session['all_purchase_alias_words'];assert len(entries)==7;last={}
        for entry in entries:
            assert entry['kind']==7 and len(entry['before'])==7;expected={}
            for word in entry['before']:
                key='Upgrade'+str(word['property_id']);alias_keys.add(key);old=expected.get(key,word['previous']);limit=word['level_count']
                expected[key]=old if old<limit else struct.unpack('<i',struct.pack('<I',(limit-1)&0xffffffff))[0]
            assert expected==entry['expected']==entry['actual'];last.update(expected)
        for key,value in last.items():assert values[i][key]==('Int',str(value))
        comparable.append([{k:v for k,v in r['state'].items() if k not in ['game_time','award','award_shown']} for r in steps])
        props=values[i]
        for key,value in dict(Money=('Int','4534'),awQDestroy=('UInt','20'),Award10Destroy100Depots=('Bool','1'),awardShow10=('Bool','1')).items():assert props[key]==value,(key,props[key])
        prices=next(e for e in resource('Gameplay/gameplay.xml').find('Events') if e.get('Type')=='6' and e.get('Time')=='0:20:30')
        for row in prices.find('Data'):assert props['MEffPrice'+row.get('ProductType')]==('Int',row.get('Value'))
    assert comparable[0]==comparable[1]
    left,right=values;differences={k:[left.get(k),right.get(k)] for k in set(left)|set(right) if left.get(k)!=right.get(k)}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(differences)<=varying|alias_keys,differences
    saves=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]]
    original=(base/'00000001.save').read_bytes();assert len(original)==4252
    for save in saves:assert save.read_bytes()==original
    decoded=[load_save(path) for path in saves];assert decoded[0]==decoded[1]
    from PIL import Image,ImageChops
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]];assert images[0].size==images[1].size==(800,600)
    a,b=[im.tobytes() for im in images];pixels=dict(compared_pixels=480000,different_pixels=sum(a[n:n+3]!=b[n:n+3] for n in range(0,len(a),3)),difference_bounds=ImageChops.difference(*images).getbbox(),exact=a==b)
    assert pixels['exact'],pixels
    for session in report['sessions']:
        assert len(session['award_visible_congratulation_lines'])==1 and len(session['award_visible_congratulation_lines'][0])==36
    report.update(status='passed_for_actual_paid_seven_demolitions_and_natural_twenty_threshold_award10_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=17,paid_rebuild_type=7,paid_cost=700,demolition_counter_before=13,demolition_counter_after=20,
            actual_alias_word_snapshots=98,recorded_allocation_derived_alias_differences={k:v for k,v in differences.items() if k in alias_keys},
            independent_profile_differences=differences,non_alias_profile_discretes_equal=True,complete_identical_save_bytes=4252,
            whole_save_exactly_unchanged_from_unedited_native_baseline=True,actual_award_panel_pixels=pixels,normalizations=[]),whole_game_complete=False,
        pending=['Bidirectional restores of natural award10 and actual20-demolition counter','Remaining natural counters,contract streaks and whole original goal scopes'])
    write(args.output,report);print(report['status']);print(pixels)

if __name__=='__main__':main()
