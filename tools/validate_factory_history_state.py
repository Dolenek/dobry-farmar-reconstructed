#!/usr/bin/env python3
"""Audit one persistent eleven-kind building history and its natural award8."""
import argparse,struct,xml.etree.ElementTree as ET,zipfile
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save

def resource(name):
    data=None
    for packed in ['Data.pack','cz.pack']:
        with zipfile.ZipFile(ROOT.parent/'farmář/Dobrý farmář/Data'/packed) as z:
            for n in z.namelist():
                if n.lower()==name.lower():data=z.read(n)
    assert data is not None;return ET.fromstring(data)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/factory-history-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/factory-history-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_all_factory_kinds_one_profile_and_natural_award8_only' and len(report['sessions'])==2
    costs=[int(list(row)[0].text) for row in resource('Game.xml').find('Costs')][:11];assert sum(costs[4:])==1850
    values=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    comparable=[];alias_keys=set()
    for i,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0 and len(session['awards'])==1 and session['costs']==costs
        assert session['fixture_report_sha256']==digest(ROOT/'analysis/squall-channel-vectors-game-interop-validation.json')
        steps=session['catalog_steps'];assert len(steps)==16 and session['initial_destroyed']==6
        for n,kind in enumerate(range(4,11)):
            for built in [False,True]:
                row=steps[2*n+int(built)];s=row['state'];assert row['label']==(f'kind-{kind}-built' if built else f'before-kind-{kind}-destroyed')
                assert s['money']==7084-sum(costs[4:kind+int(built)]) and s['destroyed']==7+n
                assert s['levels']==[0]*6+([0] if built else [-1])+[-1]*13 and s['loaded']==[0]*7
                assert s['types']==[2]*6+[kind if built else 2 if kind==4 else kind-1]
                count=kind+int(built);assert s['history']==[True]*count+[False]*(11-count)
        assert steps[-2]['label']=='natural-all-factory-kind-history-award' and steps[-1]['label']=='original-time-cutoff1150'
        assert steps[-1]['state']['award'] and steps[-1]['state']['award_shown'] and 1150<=steps[-1]['state']['game_time']<1155
        entries=session['all_purchase_alias_words'];assert len(entries)==7;last={}
        for kind,entry in zip(range(4,11),entries):
            assert entry['kind']==kind and len(entry['before'])==7;expected={}
            for word in entry['before']:
                key='Upgrade'+str(word['property_id']);alias_keys.add(key);old=expected.get(key,word['previous']);limit=word['level_count']
                expected[key]=old if old<limit else struct.unpack('<i',struct.pack('<I',(limit-1)&0xffffffff))[0]
            assert expected==entry['expected']==entry['actual'];last.update(expected)
        for key,value in last.items():assert values[i][key]==('Int',str(value))
        comparable.append([{k:v for k,v in r['state'].items() if k not in ['game_time','award','award_shown']} for r in steps])
    assert comparable[0]==comparable[1]
    left,right=values;assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
    differences={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(differences)<=varying|alias_keys,differences
    for props in values:
        for key,value in dict(Money=('Int','5234'),awQDestroy=('UInt','13'),Award08BuildAllManufactureDepots=('Bool','1'),awardShow8=('Bool','1'),UpAvail13=('UInt','1'),letter0=('Int','37')).items():assert props[key]==value,(key,props[key])
        assert all(props[f'awManDepotBuild{n}']==('Bool','1') for n in range(11))
    saves=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert saves[0].read_bytes()==saves[1].read_bytes() and saves[0].stat().st_size==4252
    fixture=read(ROOT/'analysis/squall-channel-vectors-game-interop-validation.json');old=load_save(ROOT/fixture['work_directory']/'3-persisted/Profiles/00000001.save');new=load_save(saves[0])
    assert old['seeds']==new['seeds']==[] and old['sales']==new['sales'] and old['cutoff']==new['cutoff'] and old['coefficient_bits']==new['coefficient_bits']
    event=next(e for e in resource('Gameplay/gameplay.xml').find('Events') if e.get('Time')=='0:19:00' and e.get('Type')=='0')
    note=event.find('Note').text;assert new['messages']==old['messages']+[dict(read=0,text=note)]
    from PIL import Image,ImageChops
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]];assert images[0].size==images[1].size
    a,b=[im.tobytes() for im in images];pixels=dict(compared_pixels=images[0].width*images[0].height,different_pixels=sum(a[n:n+3]!=b[n:n+3] for n in range(0,len(a),3)),difference_bounds=ImageChops.difference(*images).getbbox(),exact=a==b)
    assert pixels['exact'],pixels
    for session in report['sessions']:
        glyphs=session['award_visible_congratulation_lines']
        assert len(glyphs)==1 and len(glyphs[0])==79
        assert len({row['geometry'][1] for row in glyphs[0]})==4
    alias_diff={k:v for k,v in differences.items() if k in alias_keys}
    report.update(status='passed_for_all_factory_kinds_one_profile_and_natural_award8_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=16,paid_kinds=list(range(4,11)),paid_cost=1850,actual_alias_word_snapshots=98,alias_padding_claimed_equal=False,recorded_allocation_derived_alias_differences=alias_diff,
            all_profile_discretes_equal=not alias_diff,non_alias_profile_discretes_equal=True,independent_profile_differences=differences,complete_identical_save_bytes=4252,actual_award_panel_pixels=pixels,
            natural_new_journal_note=dict(event_time=1140,text=note,read=False,save_bytes_before=4089,save_bytes_after=4252),normalizations=[]),whole_game_complete=False,
        pending=['Bidirectional restore of cumulative eleven-kind history and natural award8','Remaining crops, natural awards, audio and full-goal scopes'])
    write(args.output,report);print(report['status']);print(report['comparison']['actual_award_panel_pixels'])
if __name__=='__main__':main()
