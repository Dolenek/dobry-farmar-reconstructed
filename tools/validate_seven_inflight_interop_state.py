#!/usr/bin/env python3
"""Audit seven independent progress restores and fourteen actual completed units."""
import argparse,xml.etree.ElementTree as ET,zipfile
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--complete',action='store_true');args=p.parse_args()
    suite='seven-completion' if args.complete else 'seven-inflight-interop'
    fixture=read(ROOT/'.local/seven-inflight-state-validation.json');fixture_work=checked(fixture)
    report=read(ROOT/'.local'/f'{suite}-validation.json');work=checked(report)
    assert report['status']==('explored_actual_seven_restored_completion_only' if args.complete else 'explored_actual_seven_inflight_bidirectional_restores_only')
    assert report['fixture_report_sha256']==digest(ROOT/'.local/seven-inflight-state-validation.json')
    assert len(report['sessions'])==(2 if args.complete else 4)
    def location(n):return (fixture_work if n<2 else work)/f'{n}-persisted/Profiles'
    def props(n):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(location(n)/'00000001.xml').getroot()}
    with zipfile.ZipFile(ROOT.parent/'farmář/Dobrý farmář/Data/Data.pack') as z:
        events=ET.fromstring(z.read(next(n for n in z.namelist() if n.lower()=='gameplay/gameplay.xml'))).find('Events')
    arrivals={}
    def seconds(event):
        h,m,s=map(int,event.get('Time').split(':'));return h*3600+m*60+s
    # GameplayDefinitions inserts by time; profile letter IDs index
    # the stable time-sorted event list, not source XML order.
    for index,event in enumerate(sorted(events,key=seconds)):
        h,m,s=map(int,event.get('Time').split(':'));time=h*3600+m*60+s
        if event.get('Type')=='5' and time==1110:arrivals[index]=time
    assert len(arrivals)==1
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'ProdPct{n}' for n in range(7)}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    transfers=[]
    for session,(before,after) in zip(report['sessions'],[(0,2),(1,3)] if args.complete else [(0,2),(1,3),(2,4),(4,5)]):
        assert session['status']=='completed' and session['exit_code']==0
        first,final=session['first_observed_production'],session['final_production_state']
        assert first['counts']==[2]*7 and first['milk']==6 and first['processed']==6
        left,right=props(before),props(after);assert set(left)==set(right) and all(left[k][0]==right[k][0] for k in left)
        changed={k:[left[k],right[k]] for k in left if left[k]!=right[k]};allowed=set(varying)
        for values in [left,right]:
            assert values['Money']==('Int','7084') and values['awardShow9']==('Bool','1')
            for n in range(7):assert values[f'Upgrade{n}']==('Int','0') and values[f'ProdType{n}']==('UInt','2') and values[f'ProdStop{n}']==('UInt','0')
        for n in range(7):
            assert left[f'ProdCount{n}']==('UInt','2') and 0<float(left[f'ProdPct{n}'][1])<100
            assert right[f'ProdCount{n}']==('UInt','0' if args.complete else '2')
            if args.complete:assert right[f'ProdPct{n}']==('Float','0')
            else:assert float(left[f'ProdPct{n}'][1])<=float(right[f'ProdPct{n}'][1])<100
        arrival=[]
        for slot in [0,1]:
            key=f'letter{slot}'
            if key in changed:
                event=int(right[key][1]);assert left[key]==('Int','-1') and event in arrivals
                when=arrivals[event];assert float(left['GameTime'][1])<when<=float(right['GameTime'][1])
                assert right[f'letState{slot}']==('Int','0')
                assert 95<float(right[f'letTime{slot}'][1])<=120
                allowed|={key,f'letState{slot}'};arrival.append(dict(slot=slot,event_index=event,event_seconds=when,remaining=float(right[f'letTime{slot}'][1])))
        if args.complete:
            assert final['counts']==[0]*7 and final['progress']==[0.]*7 and final['milk']==20 and final['processed']==20 and final['warehouse']==94
            assert 1130<=final['game_time']<1135 and len(arrival)==1
            assert right['WGoodCount10']==right['awQProcess']==('UInt','20')
            allowed|={f'ProdCount{n}' for n in range(7)}|{'WGoodCount10','awQProcess'}
            observations=session['completion_observations'];assert observations[0]['counts']==[2]*7 and observations[-1]['counts']==[0]*7
            for old,new in zip(observations,observations[1:]):assert all(0<=b<=a<=2 for a,b in zip(old['counts'],new['counts']))
        else:
            assert final['counts']==[2]*7 and final['milk']==6 and final['processed']==6 and final['warehouse']==80
            assert right['WGoodCount10']==right['awQProcess']==('UInt','6')
        assert set(changed)<=allowed,(before,after,changed)
        a,b=[(location(n)/'00000001.save').read_bytes() for n in [before,after]]
        assert a==b and len(a)==4089,('Complete seven-building binary save differs',before,after)
        transfers.append(dict(before=before,after=after,changed_properties=changed,actual_letter_arrivals=arrival,identical_complete_save_bytes=4089,normalizations=[]))
    if args.complete:
        a,b=props(2),props(3);differences={k:[a[k],b[k]] for k in a if a[k]!=b[k]}
        assert set(differences)<=varying and all(a[k][0]=='Float' for k in differences),differences
        report['independent_completed_profile_float_differences']=differences
    report.update(status='passed_for_actual_seven_restored_completions_only' if args.complete else 'passed_for_actual_seven_inflight_bidirectional_saves_only',
        comparison=dict(transfer_comparisons=transfers,actual_production_units_per_completion=14 if args.complete else 0,normalizations=[]),auditor_sha256=digest(__file__),
        whole_game_complete=False,pending=['Remaining crops, capacities and real gameplay systems','Remaining Squall97, JNG and isolated uninstaller scopes'])
    write(ROOT/'.local'/f'{suite}-state-validation.json',report);print(report['status'])
if __name__=='__main__':main()
