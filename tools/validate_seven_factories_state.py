#!/usr/bin/env python3
"""Audit seven paid placements, observed View/Upgrade alias words and natural award9."""
import argparse,struct,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/seven-factories-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/seven-factories-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_seven_simultaneous_buildings_and_natural_award9_only'
    plan=read(ROOT/'.local/seven-factory-placement-plan.json');positions=plan['positions'];assert not plan['mask_overlap_pairs']
    assert len(report['sessions'])==2
    comparable=[];alias_keys=set()
    values=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    for i,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0 and len(session['awards'])==1
        assert session['placement_plan_sha256']==digest(ROOT/'.local/seven-factory-placement-plan.json')
        entries=session['all_purchase_alias_words'];assert len(entries)==7
        last={}
        for count,entry in enumerate(entries,start=1):
            assert len(entry['before'])==count
            expected={}
            for word in entry['before']:
                key='Upgrade'+str(word['property_id']);alias_keys.add(key)
                old=expected.get(key,word['previous']);limit=word['level_count']
                expected[key]=old if old<limit else struct.unpack('<i',struct.pack('<I',(limit-1)&0xffffffff))[0]
            assert expected==entry['expected']==entry['actual'];last.update(expected)
        for key,value in last.items():assert values[i][key]==('Int',str(value))
        steps=session['catalog_steps'];assert [r['label'] for r in steps]==[f'built-{n}' for n in range(1,8)]+['natural-seven-buildings-award','original-time-cutoff1098']
        for n,row in enumerate(steps):
            count=min(n+1,7);s=row['state']
            assert s['money']==8134-150*count
            assert s['levels']==[0]*count+[-1]*(20-count)
            assert s['types']==[2]*count+[4294967295]*(7-count) and s['loaded']==[0]*7
            assert s['positions']==positions[:count]+[[0.,0.]]*(7-count)
        assert steps[-1]['state']['award'] and steps[-1]['state']['award_shown']
        assert 1098<=steps[-1]['state']['game_time']<1105
        comparable.append([{k:v for k,v in row['state'].items() if k not in ['game_time','award','award_shown']} for row in steps])
    assert comparable[0]==comparable[1]
    left,right=values;assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
    differences={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(differences)<=varying|alias_keys,differences
    for props in values:
        assert props['Money']==('Int','7084') and props['Award09UpgradeAllManufactureDepots']==props['awardShow9']==('Bool','1')
        assert sum(int(props[f'WGoodCount{n}'][1]) for n in range(19))==0
        assert sum(int(props[f'MGoodCount{n}'][1]) for n in range(19))==40
        for n in range(7):
            assert props[f'Upgrade{n}']==('Int','0') and props[f'ProdType{n}']==('UInt','2') and props[f'ProdCount{n}']==('UInt','0')
    saves=[(work/f'{i}-persisted/Profiles/00000001.save').read_bytes() for i in [0,1]]
    assert saves[0]==saves[1] and len(saves[0])==4089
    from PIL import Image,ImageChops
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]]
    assert images[0].size==images[1].size
    a,b=[im.tobytes() for im in images]
    pixels=dict(compared_pixels=images[0].width*images[0].height,different_pixels=sum(a[n:n+3]!=b[n:n+3] for n in range(0,len(a),3)),difference_bounds=ImageChops.difference(*images).getbbox(),exact=a==b)
    alias_differences={k:v for k,v in differences.items() if k in alias_keys}
    report.update(status='passed_for_actual_seven_simultaneous_level0_buildings_and_natural_award9_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=9,positions=positions,paid_cost=1050,actual_alias_word_snapshots=56,alias_padding_claimed_equal=False,recorded_allocation_derived_alias_differences=alias_differences,
            non_alias_profile_discretes_equal=True,all_profile_discretes_equal=not alias_differences,independent_profile_differences=differences,complete_identical_save_bytes=4089,actual_award_panel_pixels=pixels,normalizations=[]),
        whole_game_complete=False,pending=['Bidirectional restore of seven simultaneous buildings','Other factory input paths and remaining complete-game scopes'])
    write(args.output,report);print(report['status']);print(report['comparison'])
if __name__=='__main__':main()
