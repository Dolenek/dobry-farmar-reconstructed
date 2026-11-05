#!/usr/bin/env python3
"""Audit real all-field sprinkler maintenance and the natural no-dry award."""
import argparse,struct,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from validate_factory_history_state import resource

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/no-dry-award-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/no-dry-award-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_all60_fields_maintained_and_natural_no_dry_award_only'
    assert [s['kind'] for s in report['sessions']]==['native','source']
    path=ROOT/report['baseline']['report'];fixture=read(path)
    assert fixture['status']=='passed_actual_main_story_end_and_first_late_world_clock_loop_only'
    assert digest(path)==report['baseline']['report_sha256']
    seed=ROOT/fixture['work_directory']/'0-persisted/Profiles';before=load_save(seed/'00000001.save')
    assert all(digest(seed/name)==value for name,value in report['baseline']['files'].items())
    field=resource('Farm/ActionAreas/FarmField.xml');threshold=float(field.get('water_sign_level'));assert threshold==15
    centers=[tuple(map(float,node.get('Center').split())) for node in resource('UI/gameUI/farm.xml').find('FieldArray')]
    radius=float(resource('Farm/ActionAreas/Sprinkle.xml').get('Radius'));assert radius==100
    events=list(resource('Gameplay/gameplay.xml').find('Events'))
    events.sort(key=lambda e:sum(int(v)*scale for v,scale in zip(e.get('Time').split(':'),[3600,60,1])))
    event=events[356];assert event.get('Type')=='3' and event.get('Time')=='3:42:00'
    note=event.findtext('Note');assert note
    note_bytes=note.encode('utf-16-le');record=struct.pack('<BI',0,len(note_bytes)//2)+note_bytes;assert len(record)==203
    old=(seed/'00000001.save').read_bytes();count_offset=12+24*len(before['seeds'])+8*len(before['sales'])
    coefficient_offset=len(old)-4-4*len(before['coefficient_bits'])
    assert struct.unpack_from('<I',old,count_offset)[0]==len(before['messages'])
    expected_binary=old[:count_offset]+struct.pack('<I',len(before['messages'])+1)+old[count_offset+4:coefficient_offset]+record+old[coefficient_offset:]
    sizes=[];summaries=[]
    for cycle,s in enumerate(report['sessions']):
        assert s['status']=='completed' and s['exit_code']==s['cleanup_exit_code']==0 and 10<s['observed_clock_ratio']<17
        start,end=s['initial_state'],s['final_state'];assert not start['awards'][12] and not start['aw_no_dry'] and start['aw_no_dry_time']==0
        assert end['awards'][12] and end['aw_no_dry'] and 1200<=end['aw_no_dry_time']<1240
        assert not end['main_game'] and 13600<end['game_time']<13900
        assert s['original_crop_ids']==[] and len(s['final_fields'])==60 and all(f['active'] and f['culture']==0 and f['water']>threshold for f in s['final_fields'])
        plan=s['watering_plan'];assert plan['moving_sprinkler']==0 and plan['actual_drag_inputs_only']
        assert plan['radius']==radius and plan['water_sign_threshold']==threshold
        assert plan['fixed_sprinklers']==[[530,260],[390,380]]
        def coverage(point):return [n for n,p in enumerate(centers) if (p[0]-point[0])**2+(p[1]-point[1])**2<=radius**2]
        assert plan['coverage']==[coverage(p) for p in plan['targets']]
        assert len(set().union(*(set(coverage(p)) for p in plan['targets']+plan['fixed_sprinklers'])))==60
        steps=s['watering_steps'];assert len(steps)>=12
        activated=False;previous=0.;minimum=120.;active_steps=0
        for index,step in enumerate(steps):
            assert step['target']==plan['targets'][index%6] and step['round']==index//6
            assert len(step['water'])==60 and all(0<=v<=120 for v in step['water'])
            assert step['no_dry']==(min(step['water'])>threshold)
            if step['no_dry']:activated=True
            if activated:
                assert step['no_dry'] and step['no_dry_time']>=previous
                previous=step['no_dry_time'];minimum=min(minimum,min(step['water']));active_steps+=1
            else:assert step['no_dry_time']==0
        assert 1140<=previous<1180 and len(s['awards'])==1
        award=s['awards'][0];assert not award['before'][12] and 1200<=award['no_dry_time']<1204
        assert len(award['field_water'])==60 and min(award['field_water'])>threshold
        assert s['last_returned_awards']==[7,12,13,14,16,17]
        assert [i for i,(a,b) in enumerate(zip(start['awards'],end['awards'])) if a!=b]==[12]
        props={p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{cycle}-persisted/Profiles/00000001.xml').getroot()}
        assert props['awardShow12']==('Bool','1') and props['awNoDry']==('Bool','1') and float(props['awNoDryTime'][1])>=1200
        saved=load_save(work/f'{cycle}-persisted/Profiles/00000001.save');sizes.append(saved['bytes'])
        assert (work/f'{cycle}-persisted/Profiles/00000001.save').read_bytes()==expected_binary
        for key in ['seeds','sales','cutoff','coefficient_bits','seed_padding_ranges','sale_clock_ranges']:assert saved[key]==before[key],(cycle,key)
        assert saved['messages']==before['messages']+[dict(read=0,text=note)] and start['money']==end['money']==2504
        assert end['held']==6 and end['stock_roses']==2 and end['sprinklers']==[True]*3
        for n,(x,y) in enumerate(plan['fixed_sprinklers'],start=1):
            assert props[f'SPXPos{n}']==('Float',str(x)) and props[f'SPYPos{n}']==('Float',str(y)) and props[f'SPStatus{n}']==('UInt','0')
        assert [float(props['SPXPos0'][1]),float(props['SPYPos0'][1])]==steps[-1]['target']
        summaries.append(dict(kind=s['kind'],actual_drag_moves=len(steps),maintained_water_snapshots=active_steps,
            minimum_water_at_maintained_snapshots=minimum,award_no_dry_time=award['no_dry_time'],minimum_water_at_award=min(award['field_water']),
            final_moving_sprinkler_position=steps[-1]['target'],complete_save_bytes=saved['bytes']))
    completion=[__import__('json').loads(line) for line in report['sessions'][0]['launcher_completion'].splitlines()]
    assert completion==[dict(event='exit',exit_code=0,watchdog_triggered=False)]
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]]
    assert all(im.size==(800,600) for im in images) and images[0].tobytes()==images[1].tobytes()
    report.update(status='passed_actual_all60_fields_maintained_and_natural_no_dry_award_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(complete_processes=2,actual_natural_award=12,required_no_dry_seconds=1200,water_sign_threshold=threshold,
            field_count=60,sprinkler_radius=radius,branches=summaries,identical_whole_award_rgb_pixels=480000,
            whole_binary_saves_equal_to_independently_encoded_expected_result=True,expected_original_journal_event=356,expected_added_note_bytes=203,
            complete_save_bytes=sizes,normalizations=[],matched_rng_and_particle_timing_verified=False),
        whole_game_complete=False,pending=['Bidirectional restoration of both actual water outcomes and award12','Remaining natural awards,contract streak and whole original goal scopes'])
    write(args.output,report);print(report['status']);print('2 actual full processes;all60 fields maintained;natural1200-second award;480000 award pixels exact')

if __name__=='__main__':main()
