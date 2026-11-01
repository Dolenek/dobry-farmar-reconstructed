#!/usr/bin/env python3
"""Audit the actual later timeline, stochastic money ledger and crop destruction."""
import argparse
import xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from validate_factory_history_state import resource


def seconds(event):return sum(int(v)*scale for v,scale in zip(event.get('Time').split(':'),[3600,60,1]))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/field-disasters-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/field-disasters-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_natural_world_progression_and_first_three_crop_destruction_events_only'
    assert [(s['kind'],s['variant']) for s in report['sessions']]==[('native','prefix'),('source','prefix'),('native','disasters'),('source','disasters')]
    retained=report.get('retained_completed_segments')
    if retained:
        prior_path=ROOT/retained['report_path'];assert digest(prior_path)==retained['report_sha256']
        prior=read(prior_path);prior_work=ROOT/prior['work_directory'];assert prior['status']=='failed'
        assert digest(prior_work/'driver.py')==retained['driver_snapshot_sha256']==prior['source_files']['tools/validate_game_field_disasters.py']
        assert retained['completed_cycles']==[0,1,2] and 'Injected pointer missed owned window target' in prior['error']
        for name,value in prior['source_files'].items():
            if name!='tools/validate_game_field_disasters.py':assert digest(ROOT/name)==value,name
        for i in [0,1,2]:
            old=prior['sessions'][i];current=dict(report['sessions'][i]);current.pop('retained_from_report');assert current==old
            assert old['status']=='completed' and old['exit_code']==0
            for name in ['00000001.xml','00000001.save']:assert (work/f'{i}-persisted/Profiles'/name).read_bytes()==(prior_work/f'{i}-persisted/Profiles'/name).read_bytes()
    fixture=read(ROOT/'analysis/farm-expansions-state-validation.json');checked(fixture)
    before=load_save(ROOT/fixture['work_directory']/'0-persisted/Profiles/00000001.save')
    events=list(resource('Gameplay/gameplay.xml').find('Events'));events.sort(key=seconds)
    definitions=[]
    for index in [112,113,114]:
        event=events[index];assert event.get('Type')=='3'
        definitions.append(dict(index=index,time=seconds(event),quantity=int(event.find('Data').get('Value')),probability=float(event.find('Data').get('RandomFactor'))))
    assert definitions==[dict(index=112,time=3585,quantity=4,probability=.4),dict(index=113,time=3586,quantity=2,probability=.6),dict(index=114,time=3587,quantity=3,probability=.4)]
    crops=[14,15,16,17,18,23,27,34];destroyed=[14,15,16,17,18,23,27]
    saves=[load_save(work/f'{i}-persisted/Profiles/00000001.save') for i in range(4)]
    for saved in saves:
        for key in ['seeds','sales','cutoff','coefficient_bits','seed_padding_ranges','sale_clock_ranges']:assert saved[key]==before[key],key
    ledger=[];non_money_messages=[]
    monetary=[(i,e) for i,e in enumerate(events) if e.get('Type')=='2' and 1400<seconds(e)<=3550]
    assert len(monetary)==11 and len({e.findtext('Note') for i,e in monetary})==11
    note_texts={e.findtext('Note') for i,e in monetary};assert None not in note_texts
    for i,s in enumerate(report['sessions']):
        assert s['status']=='completed' and s['exit_code']==0 and s['fixture_report_sha256']==digest(ROOT/'analysis/farm-expansions-state-validation.json')
        assert s['original_crop_ids']==crops and s['final_state']['current_upgrade']==s['final_state']['available_upgrade']==3
        assert s['final_state']['stock_roses']==2 and s['final_state']['held']==6 and s['final_state']['sprinklers']==[True]*3
        assert len(s['final_fields'])==60 and all(f['active'] for f in s['final_fields'])
        if i<2:
            assert s['initial_state']['money']==3004 and len(s['awards'])==1
            rows=s['timeline'];assert [r['target'] for r in rows]==[1462,1820,2120,2670,3150,3550]
            assert 3550<=s['final_state']['game_time']<3580
            for row in rows:
                assert row['target']<=row['state']['game_time']<row['target']+5
                assert all(f['culture']!=0 and f['status']==1 and f['water']>118 and not f['marked'] for f in row['crops'])
            texts=[m['text'] for m in saves[i]['messages']];expected=3004;entries=[]
            for index,event in monetary:
                value=int(event.find('Data').get('Value'));chance=float(event.find('Data').get('RandomFactor'));count=texts.count(event.findtext('Note'))
                assert count in [0,1] and (chance!=1 or count==1)
                entries.append(dict(index=index,time=seconds(event),value=value,probability=chance,taken=bool(count)))
                expected+=value*count
            assert expected==s['final_state']['money']
            for row in rows:assert row['state']['money']==3004+sum(e['value'] for e in entries if e['taken'] and e['time']<=row['state']['game_time'])
            ledger.append(dict(kind=s['kind'],initial_money=3004,events=entries,expected_final_money=expected,actual_final_money=s['final_state']['money']))
            non_money_messages.append([m for m in saves[i]['messages'] if m['text'] not in note_texts])
        else:
            prefix=i-2;assert s['initial_state']['money']==report['sessions'][prefix]['final_state']['money']
            assert s['initial_state']['money']==s['final_state']['money']
            assert 3550<=s['initial_state']['game_time']<3580 and 3600<=s['final_state']['game_time']<3605
            start,end=s['restart_clock_begin'],s['restart_clock_end'];ratio=(end['game_time']-start['game_time'])/(end['wall']-start['wall']);assert .8<ratio<1.2
            assert all(f['culture']!=0 and f['status']==1 and f['water']>118 and not f['marked'] for f in s['before_disasters']['crops'])
            assert 3584<=s['before_disasters']['state']['game_time']<3585
            assert len(s['event_windows'])>=2 and s['event_windows'][0]['state']['game_time']<3585
            assert [f['id'] for f in s['after_disasters']['crops'] if f['culture']==0]==destroyed
            assert [f['id'] for f in s['final_fields'] if f['id'] in crops and f['culture']==0]==destroyed
            survivor=s['final_fields'][34];assert survivor['culture']==1 and survivor['status']==1 and survivor['water']>118
            assert saves[i]['messages'][:-1]==saves[prefix]['messages']
            assert saves[i]['messages'][-1]==dict(read=0,text=events[112].findtext('Note'))
            assert saves[i]['bytes']-saves[prefix]['bytes']==219
            for name in ['00000001.xml','00000001.save']:assert s['seed_profile_hashes'][name]==digest(work/f'{prefix}-persisted/Profiles'/name)
    assert non_money_messages[0]==non_money_messages[1]
    for left,right in zip(report['sessions'][0]['timeline'],report['sessions'][1]['timeline']):assert left['market_prices']==right['market_prices']
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]];assert images[0].tobytes()==images[1].tobytes()
    props=[{n.get('Name'):(n.get('Type'),n.get('Val')) for n in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [2,3]]
    a,b=props;assert set(a)==set(b) and all(a[k][0]==b[k][0] for k in a)
    differences={k:[a[k],b[k]] for k in a if a[k]!=b[k]};discrete={k:v for k,v in differences.items() if v[0][0] not in ['Float','Double']}
    assert discrete==dict(Money=[('Int','2904'),('Int','2804')])
    for values in props:assert values['awardShow13']==('Bool','1')
    report.update(status='passed_for_actual_later_world_timeline_stochastic_money_and_first_crop_destruction_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(complete_processes=4,opposite_program_prefix_restores=2,ordinary_saved_prefix_restores=2,timeline_checkpoints_per_executable=6,
            money_event_ledger=ledger,explicit_stochastic_discrete_difference=discrete,independent_profile_differences=differences,non_monetary_journal_messages_equal=True,
            actual_original_destruction_events=definitions,initial_ready_crop_ids=crops,destroyed_crop_ids=destroyed,surviving_last_crop=34,
            independent_live_marker_sets=[[f['id'] for f in s['after_disasters']['crops'] if f['marked']] for s in report['sessions'][2:]],
            matched_rng_and_particle_timing_verified=False,natural_award=13,identical_whole_award_rgb_pixels=480000,
            complete_save_bytes=[s['bytes'] for s in saves],complete_saves_claimed_equal=False,actual_rodent_note_bytes_per_branch=219,normalizations=[]),
        whole_game_complete=False,pending=['Cross-program restoration of both different disaster outcomes','Guaranteed later field destruction and other world progression','Remaining whole original goal scopes'])
    write(args.output,report);print(report['status']);print('4 real processes;7 actual destroyed crops each;11-event money ledgers;480000 award pixels exact')


if __name__=='__main__':main()
