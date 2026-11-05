#!/usr/bin/env python3
"""Audit the actual story end, clock wrap and new saved RNG coefficients."""
import argparse,re,struct,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from gameplay_coefficient_audit import audit_coefficients
from validate_factory_history_state import resource

def seconds(event):return sum(int(v)*scale for v,scale in zip(event.get('Time').split(':'),[3600,60,1]))
def properties(path):return {p.get('Name'):(p.get('Type'),p.get('Val')) for p in ET.parse(path).getroot()}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/campaign-loop-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/campaign-loop-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_main_story_end_and_first_late_world_clock_loop_only'
    assert [s['kind'] for s in report['sessions']]==['native','source']
    baseline_path=ROOT/report['baseline']['report'];baseline=read(baseline_path)
    assert digest(baseline_path)==report['baseline']['report_sha256']
    assert baseline['status']=='passed_actual_later_timeline_three_hour_award_only'
    seed=ROOT/baseline['work_directory']/'0-persisted/Profiles';before=load_save(seed/'00000001.save');initial=properties(seed/'00000001.xml')
    assert all(digest(seed/name)==value for name,value in report['baseline']['files'].items())
    events=list(resource('Gameplay/gameplay.xml').find('Events'));events.sort(key=seconds)
    assert len(events)==383 and events[312].get('Type')=='9' and seconds(events[312])==11190
    assert max(seconds(e) for e in events)==14400
    prior_rng=audit_coefficients(before['coefficient_bits'],events)
    new_rng=[];ledgers=[];sizes=[];non_money=[]
    monetary=[(i,e) for i,e in enumerate(events) if e.get('Type')=='2' and seconds(e)>float(initial['GameTime'][1])]
    assert [i for i,e in monetary]==[306,311,319,333,344,360,382]
    for cycle,s in enumerate(report['sessions']):
        assert s['status']=='completed' and s['exit_code']==s['cleanup_exit_code']==0 and 10<s['observed_clock_ratio']<17
        at=s['initial_state'];end=s['final_state'];assert at['main_game'] and not end['main_game']
        assert at['money']==3054 and 10820<=at['run_time']<10880
        assert 16000<=end['run_time']<16040 and 12400<=end['game_time']<12450
        assert len(s['actual_clock_loops'])==1
        loop=s['actual_clock_loops'][0];a,b=loop['before'],loop['after']
        assert 14395<a['game_time']<14400 and 11190<=b['game_time']<11196
        assert 14780<a['run_time']<14810 and 0<b['run_time']-a['run_time']<6
        assert abs((b['game_time']-a['game_time'])-(b['run_time']-a['run_time'])+3210)<5
        assert not a['main_game'] and not b['main_game']
        assert s['awards']==[] and end['awards']==at['awards'] and end['awards'][14]
        assert len(s['final_fields'])==60 and all(f['active'] and f['culture']==0 for f in s['final_fields'])
        saved=load_save(work/f'{cycle}-persisted/Profiles/00000001.save');sizes.append(saved['bytes'])
        for key in ['seeds','sales','cutoff','seed_padding_ranges','sale_clock_ranges']:assert saved[key]==before[key],(cycle,key)
        assert saved['coefficient_bits']!=before['coefficient_bits']
        new_rng.append(dict(kind=s['kind'],**audit_coefficients(saved['coefficient_bits'],events)))
        additions=saved['messages'][len(before['messages']):];assert saved['messages'][:len(before['messages'])]==before['messages']
        texts=[m['text'] for m in additions];rows=[]
        for index,event in monetary:
            data=event.find('Data');raw=data.get('RandomFactor');chance=float(re.match(r'[+-]?[0-9]+(?:\.[0-9]*)?',raw).group())
            count=texts.count(event.findtext('Note'));assert count in [0,1] and (chance!=1 or count==1)
            rows.append(dict(index=index,time=seconds(event),value=int(data.get('Value')),raw_probability=raw,parsed_probability=chance,taken=bool(count)))
        expected=3054+sum(e['value'] for e in rows if e['taken']);assert end['money']==expected
        ledgers.append(dict(kind=s['kind'],initial_money=3054,events=rows,expected_final_money=expected,actual_final_money=end['money']))
        non_money.append([m for m in additions if m['text'] not in {e.findtext('Note') for i,e in monetary}])
        assert [t['target'] for t in s['timeline']]==report['timeline_targets']
        assert s['timeline'][1]['state']['main_game'] and not s['timeline'][2]['state']['main_game']
        for t in s['timeline']:
            state=t['state'];assert t['target']<=state['run_time']<t['target']+12
            wrapped=state['run_time']>=b['run_time'];clock_limit=14400 if wrapped else state['game_time']
            assert state['money']==3054+sum(e['value'] for e in rows if e['taken'] and e['time']<=clock_limit)
            assert state['main_game']==(state['game_time']<11190 and not wrapped)
            prices=[int(initial[f'MEffPrice{n}'][1]) for n in range(19)]
            def apply(start,finish):
                for index,event in enumerate(events):
                    if event.get('Type')=='6' and start<seconds(event)<=finish:
                        assert saved['coefficient_bits'][index]==before['coefficient_bits'][index]==0
                        for entry in event.find('Data'):prices[int(entry.get('ProductType'))]=int(entry.get('Value'))
            apply(float(initial['GameTime'][1]),clock_limit)
            if wrapped:apply(11190,state['game_time'])
            assert prices==t['market_prices'],(cycle,t['target'],prices,t['market_prices'])
            assert len(t['fields'])==60 and all(f['culture']==0 for f in t['fields'])
            assert state['current_upgrade']==state['available_upgrade']==3 and state['tutorial']==-1
            assert state['held']==6 and state['stock_roses']==2 and state['sprinklers']==[True]*3
        props=properties(work/f'{cycle}-persisted/Profiles/00000001.xml')
        assert props['MainGame']==('Bool','0') and props['Money']==('Int',str(expected))
    assert non_money[0]==non_money[1]
    completion=[__import__('json').loads(line) for line in report['sessions'][0]['launcher_completion'].splitlines()]
    assert completion==[dict(event='exit',exit_code=0,watchdog_triggered=False)]
    report.update(status='passed_actual_main_story_end_and_first_late_world_clock_loop_only',auditor_sha256=digest(__file__),
        coefficient_auditor_sha256=digest(ROOT/'tools/gameplay_coefficient_audit.py'),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(complete_processes=2,timeline_checkpoints_per_executable=13,main_game_end_event=312,main_game_end_time=11190,
            actual_clock_loops_per_executable=1,clock_wrap_from=14400,clock_wrap_to=11190,
            initial_coefficient_rng_audit=prior_rng,new_saved_coefficient_rng_audits=new_rng,money_event_ledger=ledgers,
            market_payloads_independently_verified=True,non_monetary_journal_messages_equal=True,
            complete_save_bytes=sizes,complete_saves_claimed_equal=False,matched_rng_claimed=False,normalizations=[]),
        whole_game_complete=False,pending=['Cross-program restoration after real coefficient regeneration','Remaining natural awards,contract streak and whole original goal scopes'])
    write(args.output,report);print(report['status']);print('2 real processes;story end;14400->11190 wrap;383 new coefficient words each independently reconstructed')

if __name__=='__main__':main()
