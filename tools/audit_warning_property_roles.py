#!/usr/bin/env python3
"""Correct an auxiliary observer label from native instructions and retained saves;no game execution."""
import datetime
import struct
import xml.etree.ElementTree as ET
from pe import PE
from record_live_production_verification import ROOT,digest,read,write
from tutorial_warning_session import WarningSuite


def properties(directory):
    return {node.get('Name'):node.get('Val') for node in ET.parse(ROOT/directory/'00000001.xml').getroot()}


def main():
    binary=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe';pe=PE(binary)
    def at(address,size):
        offset=pe.offset(address-pe.image_base,size);return pe.data[offset:offset+size]
    witnesses=[]
    for role,index,item,push_address,item_address,call_address in [
        ('warehouse',32,12,0x468045,0x46804d,0x468056),('well',33,13,0x46b394,0x46b39c,0x46b3a5)]:
        assert at(push_address,2)==bytes([0x6a,index]) and at(item_address,2)==bytes([0x6a,item])
        call=at(call_address,5);assert call[0]==0xe8
        assert call_address+5+struct.unpack('<i',call[1:])[0]==0x4169c0
        witnesses.append(dict(role=role,property_id=index,item_type=item,
            property_push=dict(address=f'{push_address:08x}',bytes=at(push_address,2).hex()),
            item_push=dict(address=f'{item_address:08x}',bytes=at(item_address,2).hex()),
            upgrade_constructor_call=dict(address=f'{call_address:08x}',bytes=call.hex(),target='004169c0')))
    names=['.local/tutorial-warning-runtime/20261107T210415175462Z/report.json',
        '.local/tutorial-warning-runtime/20261107T210917905903Z/report.json']
    observations=[]
    for name in names:
        raw=read(ROOT/name)
        for session in raw['sessions']:
            for key,folder in [('initial_state',session['seed_profile_directory']),
                ('final_state',session['persisted_profile_directory']),('failure_live_state',session['persisted_profile_directory'])]:
                if key not in session:continue
                saved=properties(folder);old=session[key]['warehouse_level']
                assert old==int(saved['Upgrade33'])
                actual=int(saved['Upgrade32'])
                observations.append(dict(report=name,kind=session['kind'],case=session['case'],state=key,
                    captured_auxiliary_warehouse_label=old,actual_warehouse_level=actual,actual_well_level=int(saved['Upgrade33']),
                    profile_directory=folder,profile_xml_sha256=digest(ROOT/folder/'00000001.xml'),
                    auxiliary_label_was_wrong=old!=actual))
    # Exercise the actual corrected getter on an independently retained fixture
    # where the two values differ. The mock only supplies get-only profile data;
    # constructing a game, window, or Windows input is not part of this audit.
    fixture=next(row for row in observations if row['actual_warehouse_level']==3 and row['actual_well_level']==2)
    saved=properties(fixture['profile_directory'])
    class Observer:
        def step(self):return -1
        def property(self,store,name):
            return saved[name]=='1' if store=='booleans' else float(saved[name]) if store=='floats' else int(saved[name])
    suite=WarningSuite.__new__(WarningSuite);suite.observer=Observer();suite.counters=lambda:{}
    current=suite.state();assert current['warehouse_level']==3 and current['well_level']==2
    for row in observations:
        if row['case'] in ['full_warehouse','mixed_products','poor_contract']:
            assert row['actual_warehouse_level']==row['captured_auxiliary_warehouse_label']==0
    result=dict(status='corrected_warning_observer_warehouse32_well33_auxiliary_label_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reference_sha256=digest(binary),instruction_witnesses=witnesses,
        raw_reports={name:digest(ROOT/name) for name in names},observations=observations,
        corrected_actual_getter_distinguishes_fixture_levels=dict(warehouse=3,well=2),
        source_files={name:digest(ROOT/name) for name in ['tools/tutorial_warning_session.py','src/game/ui/Warehouse.cpp','src/game/ui/WaterSource.cpp']},
        impact='Full-carry auxiliary label was well level2 instead of warehouse3. Mower capacity16,actual transfers,pixels and ledgers remain independently verified. Other reviewed fixtures have both levels0;their capacity decisions are unchanged.',
        original_raw_reports_rewritten=False,production_changed=False,games_or_input_started=False,whole_game_complete=False)
    write(ROOT/'analysis/tutorial-warning-property-role-correction.json',result)
    v=read(ROOT/'analysis/verification.json')
    if 'analysis/tutorial-warning-property-role-correction.json' not in v['reports']:
        v['reports'].append('analysis/tutorial-warning-property-role-correction.json')
    write(ROOT/'analysis/verification.json',v)
    print('Verified native Warehouse32/Well33,corrected actual observer getter,and retained historical label errors without rewriting raw reports.')


if __name__=='__main__':main()
