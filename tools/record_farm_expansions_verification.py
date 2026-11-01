#!/usr/bin/env python3
"""Record actual field purchases, sixty live flags and expanded-farm restores."""
import datetime
import json
from record_live_production_verification import ROOT,digest,read,write,checked


def main():
    state=read(ROOT/'.local/farm-expansions-state-validation.json');checked(state)
    assert state['status']=='passed_for_actual_paid_two_farm_expansions_sixty_active_fields_and_thirty_watered_fields_only'
    c=state['comparison'];assert c['live_field_records_per_executable']==300 and c['complete_identical_save_bytes']==4563 and not c['normalizations']
    state['audit_files']={'tools/validate_farm_expansions_state.py':digest(ROOT/'tools/validate_farm_expansions_state.py')}
    interop=read(ROOT/'.local/farm-expansions-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_sixty_purchased_fields_and_thirty_watered_fields_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/farm-expansions-state-validation.json')
    transfers=interop['comparison']['transfer_comparisons'];assert len(transfers)==len(interop['sessions'])==4
    covered=state['sessions'][0]['expansion_snapshots'][-1]['coverage_ids']
    for row in transfers:assert row['identical_complete_save_bytes']==4563 and not row['normalizations']
    for s in interop['sessions']:
        assert s['exit_code']==0 and s['status']=='completed' and s['tutorial_step']==-1
        assert len(s['live_fields'])==60 and all(f['active'] for f in s['live_fields'])
        assert all(s['live_fields'][n]['water']>118 for n in covered)
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    initial_work=ROOT/'.local/farm-expansions/20261031T174052708240Z';initial=read(initial_work/'report.json')
    assert initial['status']=='failed' and 'UnicodeDecodeError' in initial['traceback']
    assert not any(s.get('expansion_snapshots') for s in initial['sessions'])
    assert digest(initial_work/'driver.py')==initial['source_files']['tools/validate_game_farm_expansions.py']
    observation=dict(status='diagnostic_map_encoding_error_before_any_purchase_only',work_directory=initial['work_directory'],report_sha256=digest(initial_work/'report.json'),
        preserved_driver_sha256=digest(initial_work/'driver.py'),initial_error_type='UnicodeDecodeError',
        resolution='MSVC map is not UTF-8; lossless Latin-1 decoding locates its ASCII decorated symbols.',
        large_error_repr='Original diagnostic serialized the map bytes into its exception. Preserve the report, but do not dump its huge error line.',
        recorded_as_game_failure=False,production_implementation_changed=False,whole_game_complete=False)
    reports={}
    for name,value in [('farm-expansions-state-validation.json',state),('farm-expansions-interop-validation.json',interop),('farm-expansions-initial-observation.json',observation)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    write(ROOT/'analysis/farm-expansions-regression.json',dict(status='passed_actual_paid_two_expansions_sixty_active_fields_and_four_complete_restores_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        expansion_snapshots_per_executable=5,live_field_records_per_executable=300,active_field_counts=[20,38,60],watered_field_counts=[20,28,30],
        paid_costs=[500,1000],money_after=3004,new_automatically_watered_field_ids=c['new_automatically_watered_field_ids'],
        native_field_vtable='005f1870',native_constructor='0043c120',native_active_offset='510',native_identity_offsets=['508','50c'],
        instruction_evidence_sha256=digest(ROOT/'.local/farm-field-constructor-live-disassembly.txt'),field_layout=state['field_layout'],
        layout_build_logs={name:digest(ROOT/'.local'/name) for name in ['live-farm-field-layout-config.log','live-farm-field-layout-build.log']},
        identical_complete_save_bytes=4563,further_bidirectional_restores=4,restored_live_field_records=240,source_restore_debugger_traces=traces,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Natural later field-destruction events and other world progression','Remaining natural awards and nonzero contract streak','Remaining complete Squall/JNG/uninstaller scopes']))
    print('Recorded actual500/1000 field purchases,300 live records per EXE and four exact4563-byte restores.')


if __name__=='__main__':main()
