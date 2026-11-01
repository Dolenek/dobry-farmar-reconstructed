#!/usr/bin/env python3
"""Audit real paid expansions using all60 actual field activation flags."""
import argparse
import xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked
from game_save_state import load_save
from validate_factory_history_state import resource


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/farm-expansions-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/farm-expansions-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_paid_farm_expansions_and_expanded_sprinkler_watering_only' and len(report['sessions'])==2
    meta=report['field_layout'];assert meta['metadata']==dict(object_size=2096,id=1120,upgrade_index=1124,active=1128)
    assert meta['source_sha256']==digest(ROOT/'tools/live_farm_field_layout.cpp') and meta['cmake_sha256']==digest(ROOT/'tools/live_farm_field_layout/CMakeLists.txt')
    assert meta['executable_sha256']==digest(ROOT/'build/live-farm-field-layout-vs/Release/farm-field-live-layout.exe')
    xml=resource('UI/gameUI/farm.xml');groups=[int(n.get('UpgradeIndex')) for n in xml.find('FieldArray')]
    assert [int(n.get('Price')) for n in xml.find('UpgradePrices')]==[0,70,500,1000]
    labels=['initial-level1','paid-level2','paid-level3','expanded-live-watering','persisted-level3'];comparable=[]
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0 and session['fixture_report_sha256']==digest(ROOT/'analysis/contract-expiry-state-validation.json')
        rows=session['expansion_snapshots'];assert [r['label'] for r in rows]==labels
        assert [r['state']['money'] for r in rows]==[4504,4004,3004,3004,3004]
        assert [r['state']['current_upgrade'] for r in rows]==[1,2,3,3,3]
        assert all(r['state']['available_upgrade']==3 and r['state']['stock_roses']==2 and r['state']['held']==6 and r['state']['sprinklers']==[True]*3 for r in rows)
        assert [len(r['coverage_ids']) for r in rows]==[20,28,30,30,30]
        assert [sum(f['active'] for f in r['fields']) for r in rows]==[20,38,60,60,60]
        assert [list(r['overlays'].values()) for r in rows]==[[22,18,1],[0,22,1],[0,0,0],[0,0,0],[0,0,0]]
        for row in rows:
            assert [f['id'] for f in row['fields']]==list(range(60))
            assert [f['upgrade'] for f in row['fields']]==groups
            assert [f['active'] for f in row['fields']]==[g<=row['state']['current_upgrade'] for g in groups]
        new=sorted(set(rows[-1]['coverage_ids'])-set(rows[0]['coverage_ids']));assert len(new)==10 and new==session['new_covered_field_ids']
        for n in new:
            first=rows[1] if groups[n]==2 else rows[2]
            assert rows[0]['fields'][n]['water']==0 and rows[0]['fields'][n]['active'] is False
            assert rows[3]['fields'][n]['water']>1 and rows[4]['fields'][n]['water']>118
            assert rows[4]['fields'][n]['progress']<first['fields'][n]['progress']
        for n in set(range(60))-set(rows[-1]['coverage_ids']):assert rows[-1]['fields'][n]['water']==0
        final=session['final_state'];assert final['tutorial']==-1 and 1400<=final['game_time']<1405 and final['run_time']<1800
        comparable.append([dict(label=r['label'],money=r['state']['money'],level=r['state']['current_upgrade'],coverage=r['coverage_ids'],overlays=r['overlays'],fields=[{k:f[k] for k in ['id','upgrade','active','position','culture','status']} for f in r['fields']]) for r in rows])
    assert comparable[0]==comparable[1]
    props=[{n.get('Name'):(n.get('Type'),n.get('Val')) for n in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    a,b=props;assert set(a)==set(b) and all(a[k][0]==b[k][0] for k in a)
    differences={k:[a[k],b[k]] for k in a if a[k]!=b[k]};varying={'GameTime','RunTime'}|{f'{p}{n}' for p in ['FFF','FFW','FFP'] for n in range(60)}
    assert set(differences)<=varying,differences
    for values in props:assert values['Money']==('Int','3004') and values['fcu']==('Int','3') and values['fua']==('Int','3')
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert paths[0].read_bytes()==paths[1].read_bytes() and paths[0].stat().st_size==4563
    fixture=read(ROOT/'analysis/contract-expiry-state-validation.json');before=ROOT/fixture['work_directory']/'0-persisted/Profiles/00000001.save'
    assert paths[0].read_bytes()==before.read_bytes() and load_save(paths[0])==load_save(before)
    report.update(status='passed_for_actual_paid_two_farm_expansions_sixty_active_fields_and_thirty_watered_fields_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(expansion_snapshots_per_executable=5,live_field_records_per_executable=300,active_field_counts=[20,38,60],watered_field_counts=[20,28,30],
            paid_costs=[500,1000],money_after=3004,new_automatically_watered_field_ids=report['sessions'][0]['new_covered_field_ids'],new_fields_actually_growing=True,
            all_profile_discretes_equal=True,independent_float_differences=differences,complete_identical_save_bytes=4563,normalizations=[]),
        whole_game_complete=False,pending=['Four further expanded-farm bidirectional restores','Remaining full original goal scopes'])
    write(args.output,report);print(report['status']);print('two paid expansions;20/38/60 actual flags;20/28/30 live watering;4563 bytes exact')


if __name__=='__main__':main()
