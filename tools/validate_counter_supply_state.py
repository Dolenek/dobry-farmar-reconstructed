#!/usr/bin/env python3
"""Independently audit actual thousand-counter supply chains and native/source saves."""
import argparse,json,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image,ImageChops
from record_live_production_verification import ROOT,digest,read,write,checked
from validate_factory_history_state import resource
from game_save_state import load_save
from gameplay_coefficient_audit import audit_coefficients

def props(path):return {node.get('Name'):(node.get('Type'),node.get('Val')) for node in ET.parse(path).getroot()}
def seconds(event):return sum(int(v)*scale for v,scale in zip(event.get('Time').split(':'),[3600,60,1]))

def verify_ledger(session,paid_before):
    initial=session['counter_cycle_initial'];rows=session['conservation_steps'];previous=(initial['collected'],initial['processed']);paid=paid_before
    costs=[int(x.text) for x in list(resource('Game.xml').find('Costs'))[2]];assert costs==[150,250,400,500]
    for row in rows:
        label=row['label']
        if label=='paid-cowshed-created':paid=150
        elif label.startswith('paid-cowshed-level-'):paid=sum(costs[:int(label.rsplit('-',1)[1])+1])
        assert row['collected']>=previous[0] and row['processed']>=previous[1];previous=(row['collected'],row['processed'])
        assert row['harvested']==row['collected']-initial['collected'] and row['produced']==row['processed']-initial['processed']
        assert 69+row['collected']-100==row['grass']+row['carried']+row['loaded']+row['processed']-6,('Global grass ledger',row)
        sold=5+row['processed']-6-row['milk']-row['market_milk'];assert sold>=0
        assert row['money']==2504-paid+2*sold,('Global sale money ledger',row,paid,sold)
        assert row['milk_sold']==initial['milk']+initial['market_milk']+row['produced']-row['milk']-row['market_milk']
        assert 0<=row['carried']<=16 and 0<=row['loaded']<=12 and 34+row['grass']+row['milk']<=100
        assert row['stable_read_attempts']>=1
    harvests=[row for row in session['work_steps'] if '-harvest-field-' in row['label']]
    assert len(harvests)==sum(len(row['harvested_fields']) for row in session['ui_cycles'])
    for n,row in enumerate(harvests,initial['collected']+1):assert row['state']['collected']==n
    return dict(ledger_snapshots=len(rows),actual_harvest_actions=len(harvests),actual_productions=rows[-1]['processed']-initial['processed'],last=rows[-1])

def audit_timeline(values,baseline,events):
    time=float(baseline['GameTime'][1]);prices={f'MEffPrice{n}':baseline[f'MEffPrice{n}'] for n in range(19)};notes=[];wraps=[];rounding=[]
    for target in values:
        if target<time:
            if time-target>1000:
                intervals=[(time,14400),(11190,target)];wraps.append(dict(before=time,after=target))
            else:
                assert time-target<.1,(time,target);rounding.append(dict(before=time,after=target));intervals=[]
        else:intervals=[(time,target)]
        for low,high in intervals:
            for index,event in enumerate(events):
                when=seconds(event)
                if not low<when<=high:continue
                kind=event.get('Type');note=event.findtext('Note')
                if kind in ['0','1','3','6','7'] and note:notes.append(dict(read=0,text=note))
                if kind=='6':
                    assert float(event.find('Data').get('Range','0'))==0,(index,event.attrib)
                    prices.update({f'MEffPrice{v.get("ProductType")}':('Int',v.get('Value')) for v in event.find('Data')})
        time=target
    return prices,notes,wraps,rounding

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/counter-supply-cycle-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/counter-supply-state-validation.json');args=parser.parse_args()
    report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_counter_supply_cycle_only' and [s['kind'] for s in report['sessions']]==['native','source']
    assert report['requested_targets']==dict(harvest=1032,production=1000)
    for name,path in report['source_file_snapshots'].items():assert digest(ROOT/path)==report['source_files'][name]
    base_report=ROOT/report['baseline_report']['path'];assert digest(base_report)==report['baseline_report']['sha256']
    base=ROOT/read(base_report)['work_directory']/'0-persisted/Profiles';base_props=props(base/'00000001.xml');base_save=load_save(base/'00000001.save')
    prefix_meta=report['retained_native_prefix'];prefix_path=ROOT/prefix_meta['report'];assert digest(prefix_path)==prefix_meta['report_sha256']
    prefix=read(prefix_path);prefix_session=prefix['sessions'][0];assert prefix['status']=='failed' and 'Actual bulk load slider did not take grass' in prefix['error']
    for name,sha in prefix['source_files'].items():
        old=ROOT/prefix['source_file_snapshots'][name] if name in prefix['source_file_snapshots'] else ROOT/name;assert digest(old)==sha,name
    checkpoint=ROOT/prefix_meta['ordinary_shutdown_profile_directory']
    for name,sha in prefix_meta['files'].items():assert digest(checkpoint/name)==sha,name
    checkpoint_values=props(checkpoint/'00000001.xml');checkpoint_save=load_save(checkpoint/'00000001.save')
    assert checkpoint_values['awEarn']==('Bool','1') and checkpoint_values['Award06Earn1000perMinute']==checkpoint_values['awardShow6']==('Bool','0')
    assert len(checkpoint_save['sales'])==500 and sum(v['amount'] for v in checkpoint_save['sales'])==1000
    prefix_ledger=verify_ledger(prefix_session,0);assert prefix_ledger['actual_harvest_actions']==792 and prefix_ledger['actual_productions']==837
    events=list(resource('Gameplay/gameplay.xml').find('Events'));events.sort(key=seconds)
    branches=[];profiles=[];saves=[]
    for i,session in enumerate(report['sessions']):
        assert session['status']=='completed' and session['exit_code']==0
        if i==0:
            assert session['counter_cycle_initial']['collected']==892 and session['counter_cycle_initial']['processed']==843 and session['paid_cost']==0
            completion=[json.loads(line) for line in session['launcher_completion'].splitlines()];assert completion==[dict(event='exit',exit_code=0,watchdog_triggered=False)]
        else:
            assert session['counter_cycle_initial']==dict(collected=100,processed=6,grass=61,carried=8,milk=5,market_milk=0,money=2504) and session['paid_cost']==1300
        ledger=verify_ledger(session,1300 if i==0 else 0)
        assert ledger['actual_harvest_actions']+(prefix_ledger['actual_harvest_actions'] if i==0 else 0)==936
        assert ledger['actual_productions']+(prefix_ledger['actual_productions'] if i==0 else 0)==1005
        values=props(work/f'{i}-persisted/Profiles/00000001.xml');profiles.append(values);saved=load_save(work/f'{i}-persisted/Profiles/00000001.save');saves.append(saved)
        for key,wanted in dict(awQCollect=('UInt','1036'),awQProcess=('UInt','1011'),PCC0=('UInt','0'),WGoodCount0=('UInt','0'),ProdCount0=('UInt','0'),WGoodCount10=('UInt','12'),Upgrade0=('Int','3'),ProdType0=('UInt','2')).items():assert values[key]==wanted,(i,key,values[key])
        assert all(values[f'Upgrade{n}']==('Int','-1') for n in range(1,20))
        assert all(values[f'WGoodCount{n}']==base_props[f'WGoodCount{n}'] and values[f'MGoodCount{n}']==('UInt','0') for n in range(19) if n not in [0,10])
        for n in range(1,8):assert values[f'PCC{n}']==('UInt','0')
        assert values['MGoodPrice10']==('UInt','2')
        for index,key in [(0,'Award00Grow100'),(1,'Award01Produce100'),(2,'Award02Grow1000'),(3,'Award03Produce1000'),(6,'Award06Earn1000perMinute')]:assert values[key]==values[f'awardShow{index}']==('Bool','1')
        sold=1010-12-int(values['MGoodCount10'][1]);assert int(values['Money'][1])==1204+2*sold
        timeline=([v['game_time'] for v in prefix_session['conservation_steps']] if i==0 else [])+[v['game_time'] for v in session['conservation_steps']]+[float(values['GameTime'][1])]
        expected_prices,notes,wraps,rounding=audit_timeline(timeline,base_props,events)
        assert all(values[k]==v for k,v in expected_prices.items()),(i,expected_prices)
        assert saved['messages']==base_save['messages']+notes,(i,len(saved['messages']),len(base_save['messages']),len(notes))
        assert saved['seeds']==base_save['seeds']==[] and len(saved['coefficient_bits'])==383
        coefficients=audit_coefficients(saved['coefficient_bits'],events)
        assert all(v['amount']==2 and 11190<=v['time']<=14400 for v in saved['sales'])
        assert sum(v['amount'] for v in saved['sales'])>999
        contexts=session['live_award_before_ok']
        assert any(v['collected']==1000 and not v['owned'][1] for v in contexts)
        assert any(v['processed']==1000 and not v['owned'][2] for v in contexts)
        branches.append(dict(kind=session['kind'],final_collected=1036,final_processed=1011,actual_harvest_actions=936,actual_productions=1005,
            conservation=ledger,retained_prefix_conservation=prefix_ledger if i==0 else None,milk_sold=sold,money=int(values['Money'][1]),remaining_market_milk=int(values['MGoodCount10'][1]),
            clock_wraps=wraps,actual_profile_clock_rounding=rounding,independently_checked_price_words=19,independently_expected_new_notes=len(notes),coefficient_audit=coefficients,
            saved_bytes=saved['bytes'],saved_sales=len(saved['sales']),saved_sales_total=sum(v['amount'] for v in saved['sales']),saved_cutoff=saved['cutoff']))
    panels=[]
    for name,left,right in [('production100',ROOT/prefix['work_directory']/'0-award-0.png',work/'1-award-0.png'),('income1000',work/'0-award-0.png',work/'1-award-1.png'),('harvest1000',work/'0-award-1.png',work/'1-award-2.png'),('production1000',work/'0-award-2.png',work/'1-award-3.png')]:
        a,b=[Image.open(p).convert('RGB') for p in [left,right]];assert a.size==b.size==(800,600)
        x,y=a.tobytes(),b.tobytes();pixel=dict(award=name,compared_pixels=480000,different_pixels=sum(x[k:k+3]!=y[k:k+3] for k in range(0,len(x),3)),difference_bounds=ImageChops.difference(a,b).getbbox(),exact=x==y);assert pixel['exact'];panels.append(pixel)
    assert int(profiles[1]['Money'][1])-int(profiles[0]['Money'][1])==2*(int(profiles[0]['MGoodCount10'][1])-int(profiles[1]['MGoodCount10'][1]))==20
    report.update(status='passed_actual_native_source_thousand_counter_supply_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(branches=branches,actual_harvest_actions_per_executable=936,actual_productions_per_executable=1005,final_counters=[1036,1011],actual_award_panels=panels,exact_rgb_pixels=1920000,
            final_money_difference=20,final_market_milk_difference=10,exact_material_and_sale_money_ledgers=True,
            independent_final_binary_saves_claimed_identical=False,matched_random_stream_claimed=False,normalizations=[],
            native_prefix_exit_code_captured=False,native_prefix_checkpoint_unmodified_and_reloaded=True,
            short_feed_interval_explanation=dict(actual_saved_sales_total=1000,pending_income_predicate=True,earned_income_flag_before_resume=False,live_effect_phase_at_failure_captured=False,inference='Frozen short interval is consistent with the only new pending income award;resume actually displays its original panel')),
        whole_game_complete=False,pending=['Native/source save restoration with live sale histories and independent coefficient vectors','Remaining natural contract streaks,high-price sales,planning and all-awards conditions','Remaining Squall/JNG/uninstaller and whole original goal scopes'])
    write(args.output,report);print(report['status']);print('2 actual completed processes;936 harvests/1005 productions each;4 complete panels/1920000 exact RGB pixels')

if __name__=='__main__':main()
