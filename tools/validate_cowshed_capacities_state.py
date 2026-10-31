#!/usr/bin/env python3
"""Audit actual capacities, one-unit return, partial slider return and blocked production."""
import argparse,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/cowshed-capacities-validation.json')
    p.add_argument('--output',type=Path,default=ROOT/'.local/cowshed-capacities-state-validation.json')
    args=p.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_actual_cowshed_all_four_capacities_and_transfer_controls_only' and len(report['sessions'])==2
    comparable=[];alias_keys=set()
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0
        aliases=session['upgrade_alias_words'];assert len(aliases)==4
        for alias in aliases:
            key='Upgrade'+str(alias['property_id']);alias_keys.add(key)
            expected=alias['previous'] if alias['previous']<alias['level_count'] else ((alias['level_count']-1+2**31)%2**32)-2**31
            assert alias['actual']==alias['expected']==expected
        records=session['capacity_levels'];assert [r['capacity'] for r in records]==[2,4,6,12]
        steps=session['catalog_steps'];by_label={row['label']:row['state'] for row in steps}
        for level,row in enumerate(records):
            capacity=row['capacity'];assert row['level']==level
            maximum=row['maximum'];assert maximum['level']==level and maximum['count']==capacity and maximum['processed']==6
            returned=by_label[f'level-{level}-return-one'];reloaded=by_label[f'level-{level}-reload-one']
            assert returned['count']==capacity-1 and returned['goods'][0]==maximum['goods'][0]+1
            assert reloaded['count']==capacity and reloaded['goods']==maximum['goods']
            remaining=capacity-min(capacity,row['free_before_slider']);assert row['slider_remaining']==remaining
            assert row['slider']['count']==remaining and row['slider']['goods'][0]==maximum['goods'][0]+capacity-remaining
            for key in ['overflow','blocked_return']:
                state=row[key];assert state['count']==capacity and state['processed']==6 and sum(state['goods'])==100 and state['goods'][0]>0
            assert row['overflow']['goods']==row['blocked_return']['goods']
        blocked=by_label['level3-full-store-production-blocked'];released=by_label['one-space-one-real-milk'];last=by_label['next-unit-blocked-at100']
        assert blocked['count']==12 and blocked['progress']==100 and blocked['processed']==6
        for state in [released,last]:
            assert state['count']==11 and state['processed']==7 and sum(state['goods'])==100
            assert state['goods'][0]==blocked['goods'][0]-1 and state['goods'][10]==blocked['goods'][10]+1
        assert last['progress']==100
        comparable.append([{key:value for key,value in row['state'].items() if key not in ['game_time','progress']} for row in steps])
    assert comparable[0]==comparable[1]
    values=[{row.get('Name'):(row.get('Type'),row.get('Val')) for row in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    left,right=values;assert set(left)==set(right) and all(left[key][0]==right[key][0] for key in left)
    changes={key:[left[key],right[key]] for key in left if left[key]!=right[key]}
    varying={'GameTime','RunTime','letTime0','letTime1'}|{f'{prefix}{field}' for prefix in ['FFP','FFW','FFF'] for field in range(60)}
    assert set(changes)<=varying|alias_keys,changes
    alias_differences={key:value for key,value in changes.items() if key in alias_keys}
    assert all(left[key][0]=='Float' for key in changes if key not in alias_keys)
    for index,session in enumerate(report['sessions']):
        for key in alias_keys:
            entries=[entry for entry in session['upgrade_alias_words'] if key=='Upgrade'+str(entry['property_id'])]
            if entries:assert values[index][key]==('Int',str(entries[-1]['actual']))
    for props in values:
        for key,expected in dict(Money=('Int','6834'),Upgrade0=('Int','3'),ProdType0=('UInt','2'),ProdCount0=('UInt','11'),ProdPct0=('Float','100'),awQProcess=('UInt','7'),WGoodCount10=('UInt','7'),MGoodCount0=('UInt','2')).items():assert props[key]==expected,(key,props[key])
        assert sum(int(props[f'WGoodCount{i}'][1]) for i in range(19))==100
    saves=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]];assert saves[0].read_bytes()==saves[1].read_bytes() and saves[0].stat().st_size==4089
    report.update(status='passed_for_actual_cowshed_capacities_return_slider_and_one_space_production_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(capacities=[2,4,6,12],business_snapshots_per_executable=len(comparable[0]),one_unit_returns=True,partial_slider_remaining=[r['slider_remaining'] for r in report['sessions'][0]['capacity_levels']],
            capacity_clamped_with_input_available=True,full_store_blocks_return=True,actual_produced_after_one_market_space=1,
            non_alias_profile_discretes_equal=True,all_profile_discretes_equal=not alias_differences,
            validated_actual_alias_word_snapshots=8,recorded_allocation_derived_alias_differences=alias_differences,alias_padding_claimed_equal=False,
            independent_float_differences={key:value for key,value in changes.items() if key not in alias_keys},complete_identical_save_bytes=4089,normalizations=[]),
        whole_game_complete=False,pending=['Other factory kinds maximum input paths and simultaneous buildings','Remaining game, Squall, JNG and uninstaller scopes'])
    write(args.output,report);print(report['status'])
if __name__=='__main__':main()
