#!/usr/bin/env python3
"""Audit three paid sprinklers, real water distribution and actual movement rejection."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
from validate_factory_history_state import resource
from game_save_state import load_save


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/irrigation-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/irrigation-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_three_sprinklers_purchase_watering_invalid_drag_and_restore_only' and len(report['sessions'])==2
    config=resource('UI/gameUI/farm.xml');positions=[tuple(map(float,n.get('Center').split())) for n in config.find('FieldArray')]
    active=[n for n,row in enumerate(config.find('FieldArray')) if int(row.get('UpgradeIndex'))<=1]
    radius=float(resource('Farm/ActionAreas/Sprinkle.xml').get('Radius'));assert radius==100
    targets=[(400,270),(530,260),(390,380)];affected=[[n for n in active if (positions[n][0]-x)**2+(positions[n][1]-y)**2<=radius**2] for x,y in targets]
    assert sorted(set().union(*map(set,affected)))==active and len(active)==20
    business=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and len(session['work_steps'])==17
        assert session['fixture_report_sha256']==digest(ROOT/'analysis/tool-upgrades-state-validation.json')
        assert session['affected_fields']==affected and session['active_fields']==active
        steps=session['work_steps'];rows=[]
        for index in range(3):
            for stage,label in enumerate(['purchase-preview','paid-installed','watered-fields']):
                row=steps[index*3+stage];state=row['state'];assert row['label']==f'sprinkler-{index}-{label}'
                assert state['money']==5134-200*(index+int(stage>0))
                current=state['sprinklers'][index];assert current['exists']==(stage>0) and current['status']==(2 if stage==0 else 0)
                if stage>0:assert (current['x'],current['y'])==targets[index]
                if stage==2:
                    values={r['id']:r['water'] for r in state['fields']};assert all(values[n]>118 for n in affected[index])
        labels=['fourth-purchase-ui-hidden','installed-invalid-drag-preview','invalid-release-stays-moving','valid-click-drop-empty-radius','old-radius-water-decays','valid-drag-release-restores-radius','all-active-fields-automatically-watered','original-time-cutoff1070']
        assert [r['label'] for r in steps[9:]]==labels
        for row in steps:
            state=row['state'];assert state['held']==6 and state['carried']==[0]*8
            rows.append({key:state[key] for key in ['money','held','carried','sprinklers']})
        for row in steps[9:]:assert row['state']['money']==4534 and all(s['exists'] for s in row['state']['sprinklers'])
        for index in [10,11]:
            s=steps[index]['state']['sprinklers'][0];assert s['status']==2 and (s['x'],s['y'])==(400,270)
        for index in [12,13]:
            s=steps[index]['state']['sprinklers'][0];assert s['status']==0 and (s['x'],s['y'])==(130,280)
        field14=next(r for r in steps[13]['state']['fields'] if r['id']==14);assert field14['water']<119
        for index in [14,15,16]:
            s=steps[index]['state']['sprinklers'][0];assert s['status']==0 and (s['x'],s['y'])==(400,270)
        assert all(f['water']>118 for f in steps[15]['state']['fields'])
        roots=[r for r in session['invalid_preview_scene'] if (r['texture'] or '').lower()=='farm/actionareas/sprinkle/default.jng'];assert len(roots)==3
        moved=next(r for r in roots if r['geometry'][:2]==[120.,100.]);assert all(moved['sprite'][n]==0xff804040 for n in [2,5,8,11])
        business.append(rows)
    assert business[0]==business[1]
    props=[{r.get('Name'):(r.get('Type'),r.get('Val')) for r in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=props;assert set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
    differences={n:[left[n],right[n]] for n in left if left[n]!=right[n]}
    varying={'GameTime','RunTime','letTime0','letTime1','letTime2'}|{f'{prefix}{n}' for prefix in ['FFP','FFF','FFW'] for n in range(60)}
    discrete={n:v for n,v in differences.items() if n not in varying}
    maturations={}
    for name,values in discrete.items():
        assert name.startswith('FFS') and int(name[3:]) in active,name
        assert set(values)=={('UInt','0'),('UInt','1')},values
        pending=props[0] if values[0]==('UInt','0') else props[1]
        ready=props[1] if pending is props[0] else props[0]
        field=int(name[3:]);assert 0<float(pending[f'FFP{field}'][1])<20 and ready[f'FFP{field}']==('Float','0')
        assert all(float(p[f'FFW{field}'][1])>118 and float(p[f'FFF{field}'][1])>1 for p in props)
        maturations[name]=dict(values=values,native_remaining_seconds=left[f'FFP{field}'][1],source_remaining_seconds=right[f'FFP{field}'][1])
    for values in props:
        assert values['Money']==('Int','4534') and values['awNoDry']==('Bool','0') and values['awNoDryTime']==('Float','0')
        for i,(x,y) in enumerate(targets):
            assert values[f'SPExist{i}']==('Bool','1') and values[f'SPStatus{i}']==('UInt','0')
            assert values[f'SPXPos{i}']==('Float',str(x)) and values[f'SPYPos{i}']==('Float',str(y))
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes() and paths[0].stat().st_size==4089
    fixture=read(ROOT/'analysis/tool-upgrades-state-validation.json');before=load_save(ROOT/fixture['work_directory']/'0-persisted/Profiles/00000001.save');saved=load_save(paths[0]);assert before==saved
    report.update(status='passed_for_actual_three_paid_sprinklers_watering_and_movement_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=17,paid_cost=600,final_money=4534,sprinkler_targets=targets,affected_field_ids=affected,automatically_watered_active_fields=20,
            empty_radius_drop=[130,280],rejected_warehouse_preview=[120,100],actual_invalid_preview_argb=0xff804040,movement_cost=0,complete_identical_save_bytes=4089,normalizations=[],
            all_profile_discretes_equal=not discrete,independent_profile_differences=differences,time_dependent_discrete_difference=maturations,
            inactive_dry_fields_preserve_awNoDry_false=True),whole_game_complete=False,
        pending=['Actual bidirectional restore of sprinkler positions and active watering','Controlled whole-frame replay of live irrigation particles','Remaining gameplay, Squall, JNG and uninstaller scopes'])
    write(args.output,report);print(report['status']);print('17 work snapshots per EXE,20 active fields,4089 exact bytes; field16 timed maturity remains explicit')


if __name__=='__main__':main()
