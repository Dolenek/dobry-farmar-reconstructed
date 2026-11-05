#!/usr/bin/env python3
"""Audit real late-world observations without replacing random outcomes."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from validate_factory_history_state import resource

def seconds(event):return sum(int(v)*scale for v,scale in zip(event.get('Time').split(':'),[3600,60,1]))
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/late-timeline-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/late-timeline-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_later_timeline_and_three_hour_award_only'
    assert [s['kind'] for s in report['sessions']]==['native','source']
    assert report['baseline']['report']=='analysis/field-disasters-state-validation.json'
    baseline=read(ROOT/report['baseline']['report']);assert digest(ROOT/report['baseline']['report'])==report['baseline']['report_sha256']
    seed=ROOT/baseline['work_directory']/'2-persisted/Profiles'
    before=load_save(seed/'00000001.save')
    assert all(digest(seed/name)==value for name,value in report['baseline']['files'].items())
    events=list(resource('Gameplay/gameplay.xml').find('Events'));events.sort(key=seconds)
    monetary=[(i,e) for i,e in enumerate(events) if e.get('Type')=='2' and 3600<seconds(e)<=10440]
    assert len(monetary)==24
    crop_events=[dict(index=i,time=seconds(e),quantity=int(e.find('Data').get('Value')),probability=float(e.find('Data').get('RandomFactor'))) for i,e in enumerate(events) if e.get('Type')=='3' and 3600<seconds(e)<=10440]
    assert crop_events==[dict(index=189,time=6301,quantity=4,probability=1.),dict(index=266,time=9120,quantity=4,probability=1.)]
    ledgers=[];saves=[];non_money=[]
    note_texts={e.findtext('Note') for i,e in monetary};assert len(note_texts)==24 and None not in note_texts
    for cycle,s in enumerate(report['sessions']):
        assert s['status']=='completed' and s['exit_code']==s['cleanup_exit_code']==0
        assert 10<s['observed_clock_ratio']<17
        initial,end=s['initial_state'],s['final_state']
        assert 3989<=initial['run_time']<4000 and 3600<=initial['game_time']<3610
        assert initial['money']==2904 and not initial['awards'][14]
        assert 10820<=end['run_time']<10860 and 10400<=end['game_time']<10480 and end['awards'][14]
        assert s['last_returned_awards']==[7,13,14,16,17] and len(s['awards'])==1
        award=s['awards'][0];assert 10800<=award['run_time']<10803 and not award['before'][14]
        assert all(not g['flags']&2 for g in award['glyphs']) and award['glyphs']
        assert s['original_crop_ids']==[34] and len(s['final_fields'])==60
        assert all(f['active'] and f['culture']==0 for f in s['final_fields'])
        saved=load_save(work/f'{cycle}-persisted/Profiles/00000001.save');saves.append(saved)
        for key in ['seeds','sales','cutoff','coefficient_bits','seed_padding_ranges','sale_clock_ranges']:assert saved[key]==before[key],(cycle,key)
        assert saved['messages'][:len(before['messages'])]==before['messages']
        additions=saved['messages'][len(before['messages']):];texts=[m['text'] for m in additions]
        rows=[]
        for index,event in monetary:
            data=event.find('Data');count=texts.count(event.findtext('Note'));chance=float(data.get('RandomFactor'))
            assert count in [0,1] and (chance!=1 or count==1),(cycle,index,count)
            rows.append(dict(index=index,time=seconds(event),value=int(data.get('Value')),probability=chance,taken=bool(count)))
        expected=2904+sum(e['value'] for e in rows if e['taken']);assert end['money']==expected
        ledgers.append(dict(kind=s['kind'],initial_money=2904,events=rows,expected_final_money=expected,actual_final_money=end['money']))
        non_money.append([m for m in additions if m['text'] not in note_texts])
        assert [t['target'] for t in s['timeline']]==report['timeline_targets']
        for t in s['timeline']:
            at=t['state'];assert t['target']<=at['run_time']<t['target']+5
            assert at['money']==2904+sum(e['value'] for e in rows if e['taken'] and e['time']<=at['game_time']),(cycle,t['target'])
            fields=t['fields'];assert len(fields)==60
            cultures=[f['id'] for f in fields if f['culture']]
            assert cultures==([34] if at['game_time']<6301 else [])
            assert at['current_upgrade']==at['available_upgrade']==3 and at['tutorial']==-1
            assert at['held']==6 and at['stock_roses']==2 and at['sprinklers']==[True]*3
            assert at['money_gate']==0 and at['ware_gate']==-1
        props={p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(work/f'{cycle}-persisted/Profiles/00000001.xml').getroot()}
        assert props['awardShow14']==('Bool','1') and props['Money']==('Int',str(expected))
    assert non_money[0]==non_money[1]
    for a,b in zip(*[s['timeline'] for s in report['sessions']]):assert a['market_prices']==b['market_prices'],a['target']
    completion=[__import__('json').loads(line) for line in report['sessions'][0]['launcher_completion'].splitlines()]
    assert completion==[dict(event='exit',exit_code=0,watchdog_triggered=False)]
    images=[Image.open(work/f'{i}-award-0.png').convert('RGB') for i in [0,1]]
    assert all(im.size==(800,600) for im in images)
    left,right=[im.tobytes() for im in images]
    equal=sum(left[i:i+3]==right[i:i+3] for i in range(0,len(left),3))
    box=(250,380,550,440);assert images[0].crop(box).tobytes()==images[1].crop(box).tobytes()
    report.update(status='passed_actual_later_timeline_three_hour_award_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(complete_processes=2,timeline_checkpoints_per_executable=13,actual_natural_award=14,
            money_event_ledger=ledgers,non_monetary_journal_messages_equal=True,market_price_checkpoints_equal=13,
            actual_later_crop_destruction=crop_events,original_last_crop_id=34,final_crops_both=0,
            complete_save_bytes=[s['bytes'] for s in saves],complete_saves_claimed_equal=False,
            identical_whole_award_rgb_pixels=equal,whole_award_pixels=480000,award_text_rectangle=box,identical_award_text_rgb_pixels=18000,
            matched_rng_and_particle_timing_verified=False,normalizations=[]),whole_game_complete=False,
        pending=['Cross-program restoration of both late-world outcomes','Remaining natural awards, campaign and whole original goal scopes'])
    write(args.output,report);print(report['status']);print('2 real processes;13 late checkpoints each;24-event ledgers;actual three-hour award')
if __name__=='__main__':main()
