#!/usr/bin/env python3
"""Audit actual cross-program maturation, eight ready cultures and natural awards11/13."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image,ImageChops
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from validate_factory_history_state import resource


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/remaining-crops-growth-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/remaining-crops-growth-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_cross_program_five_crop_growth_and_natural_awards11_13_only' and len(report['sessions'])==2
    fixture=read(ROOT/'.local/remaining-crops-sowing-state-validation.json');fixture_work=checked(fixture)
    props=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    saves=[];masked=[]
    for i,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0 and len(session['awards'])==2
        assert session['fixture_report_sha256']==digest(ROOT/'.local/remaining-crops-sowing-state-validation.json')
        assert session['restore_from_kind']==('source' if session['kind']=='native' else 'native')
        assert {r['crop'] for r in session['all_eight_culture_snapshot']}==set(range(8))
        assert all(r['status']==1 and r['water']>0 for r in session['all_eight_culture_snapshot'])
        first,last=session['awards'];assert 1800<=first['run_time']<1802 and first['grow_mask']==191 and last['grow_mask']==255
        assert not first['before'][13] and last['before'][13] and not last['before'][11]
        values=props[i]
        for key in ['Award11GrowAllAtOneTime','Award13Play30Minute','awardShow11','awardShow13']:assert values[key]==('Bool','1')
        assert values['awGrowType']==('Int','255') and values['Money']==('Int','4959')
        for kind,field in zip([2,4,5,6,7],[24,25,26,33,35]):assert values[f'FFC{field}']==('UInt',str(kind)) and values[f'FFS{field}']==('UInt','1')
        path=work/f'{i}-persisted/Profiles/00000001.save';saved=load_save(path);before=load_save(fixture_work/f'{1-i}-persisted/Profiles/00000001.save')
        assert saved['bytes']==5142 and len(saved['seeds'])==5
        for old,new in zip(before['seeds'],saved['seeds']):
            assert new['x']==old['x']+1 and all(new[k]==old[k] for k in ['y','kind','count','status','unpaid'])
        assert saved['sales']==before['sales'] and saved['cutoff']==before['cutoff'] and saved['coefficient_bits']==before['coefficient_bits']
        expected=[]
        for event in resource('Gameplay/gameplay.xml').find('Events'):
            h,m,s=map(int,event.get('Time').split(':'));seconds=h*3600+m*60+s
            # Type5 Note is a letter's body, not an appended journal entry.
            if 1280<=seconds<1503 and event.get('Type') in ['0','1'] and event.findtext('Note'):expected.append((seconds,event.findtext('Note')))
        assert [t for t,_ in sorted(expected)]==[1335,1490]
        assert saved['messages']==before['messages']+[dict(read=0,text=t) for _,t in sorted(expected)]
        raw=bytearray(path.read_bytes())
        for a,b in saved['seed_padding_ranges']:raw[a:b]=b'\0'*(b-a)
        saves.append(saved);masked.append(raw)
    assert masked[0]==masked[1]
    left,right=props;assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
    differences={k:[left[k],right[k]] for k in left if left[k]!=right[k]}
    varying={'GameTime','RunTime','letTime0','letTime1','letTime2'}|{f'{prefix}{n}' for prefix in ['FFP','FFW','FFF'] for n in range(60)}
    discrete={k:v for k,v in differences.items() if k not in varying}
    assert discrete==dict(MEffPrice10=[('Int','16'),('Int','8')],tutJustProc=[('Int','49'),('Int','25')],tip49=[('Bool','1'),('Bool','0')]),discrete
    assert float(left['GameTime'][1])>=1500>float(right['GameTime'][1])
    event=next(e for e in resource('Gameplay/gameplay.xml').find('Events') if e.get('Time')=='0:25:00' and e.get('Type')=='6')
    assert next(r for r in event.find('Data') if r.get('ProductType')=='10').get('Value')=='16'
    assert any(r['step']==49 for r in report['sessions'][0]['stages'])
    assert any(float(left[f'FFF{n}'][1])==0 for n in [15,16,34]) and all(float(right[f'FFF{n}'][1])>0 for n in [15,16,34])
    pixel_rows=[]
    for index in range(2):
        images=[Image.open(work/f'{i}-award-{index}.png').convert('RGB') for i in [0,1]]
        a,b=[im.tobytes() for im in images];text=[im.crop((260,340,550,470)) for im in images]
        pixel_rows.append(dict(index=index,award=13 if index==0 else 11,whole_frame_different_pixels=sum(a[n:n+3]!=b[n:n+3] for n in range(0,len(a),3)),whole_frame_difference_bounds=ImageChops.difference(*images).getbbox(),congratulation_rect=[260,340,550,470],congratulation_exact=text[0].tobytes()==text[1].tobytes()))
    report.update(status='passed_for_cross_program_five_crop_growth_and_natural_awards11_13_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(actual_cross_program_growth_restores=2,eight_cultures_actually_ready_in_both=True,natural_awards=[13,11],all_profile_discretes_equal=False,independent_profile_differences=differences,
            actual_timed_discrete_differences=dict(price_event1500=discrete['MEffPrice10'],fertility_tutorial49={k:discrete[k] for k in ['tutJustProc','tip49']},trigger='0043 field enabledStep: zero fertility opens tutorial49; native actually processed it, source cutoff precedes depletion'),
            complete_save_bytes=5142,complete_save_equal_except_seed_padding=True,normalizations=[dict(kind='original_uninitialized_seed_padding_only',ranges=saves[0]['seed_padding_ranges'])],actual_award_pixels=pixel_rows,new_messages=len(saves[0]['messages'])-10),whole_game_complete=False,
        pending=['Actual five-kind harvests and storage in opposite EXE','Common later gameplay cutoff and further complete-save transfers','Remaining whole-game, Squall, JNG and uninstaller scopes'])
    write(args.output,report);print(report['status']);print('award pixels',pixel_rows)


if __name__=='__main__':main()
