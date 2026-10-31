#!/usr/bin/env python3
"""Audit actual prepositioned mouse input during native/source Logic pause."""
import xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    path=ROOT/'.local/paused-production-input-validation.json';report=read(path);work=checked(report)
    assert report['status']=='explored_actual_pause_mouse_resume_and_production_only' and len(report['sessions'])==2
    comparable=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and session['paused_input']['game_clock_frozen']
        rows={row['label']:row['state'] for row in session['catalog_steps']}
        paused=rows['paused-feed-attempt'];resumed=rows['resumed-after-queued-press'];active=rows['active-feed'];advanced=rows['actual-production-advances']
        assert paused['count']==1 and paused['progress']==0 and paused['goods'][0]==1
        assert paused['game_time']==session['paused_input']['frozen_game_time']
        assert resumed['count']==1 and resumed['goods'][0]==1
        for state in [active,advanced]:assert state['count']==2 and state['goods'][0]==0 and state['processed']==6
        assert advanced['progress']>active['progress']>0
        comparable.append([{key:value for key,value in row['state'].items() if key not in ['game_time','progress']} for row in session['catalog_steps']])
    assert comparable[0]==comparable[1]
    props=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    changed={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    allowed={'GameTime','RunTime','letTime0','letTime1','ProdPct0'}|{f'{prefix}{i}' for prefix in ['FFP','FFW','FFF'] for i in range(60)}
    assert set(changed)<=allowed and all(left[key][0]=='Float' for key in changed),changed
    a,b=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert a.read_bytes()==b.read_bytes() and a.stat().st_size==4089
    report.update(status='passed_for_actual_prepositioned_mouse_during_logic_pause_and_resume_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(path),
        comparison=dict(frozen_game_clock_and_production=True,actual_paused_loaded_input=1,actual_next_active_input=1,production_resumed=True,
            all_profile_discretes_equal=True,complete_identical_save_bytes=4089,independent_float_differences=changed,normalizations=[]),whole_game_complete=False,
        pending=['Mouse relocation and other controls during pause','Remaining complete goal scopes'])
    write(ROOT/'.local/paused-production-input-state-validation.json',report);print(report['status'])
if __name__=='__main__':main()
