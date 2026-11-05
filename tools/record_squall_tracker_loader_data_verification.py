#!/usr/bin/env python3
"""Record additional actual loaders, complete object states and child allocations."""
import datetime,hashlib,struct,collections
from record_live_production_verification import ROOT,digest,read,write

def main():
    reports={};runs=[]
    for label,suffix in [('main',''),('fresh','-fresh')]:
        value=read(ROOT/'.local'/f'squall-tracker-loader-data{suffix}-validation.json');work=ROOT/value['work_directory']
        assert value['status']=='passed_additional_loader_data_scope' and value['different_pairs']==0 and len(value['cases'])==88
        assert value['object_pairs']==66 and value['initialized_storage_bytes']==3751968 and value['exact_allocated_bytes']==197524 and value['address_slot_states']==71742
        assert [f['format'] for f in value['fixtures']]==['MTM','669','STM','ULT','FAR','OKT','DSM','PTM','DBM','AMF14','AMF0']
        for name,sha in value['source_files'].items():assert digest(ROOT/name)==digest(work/'snapshots'/name)==sha,name
        for name,sha in value['binaries'].items():assert digest(work/name)==sha,name
        for name,sha in value['fixture_hashes'].items():assert digest(work/'fixtures'/name)==sha,name
        for row in value['cases']:
            assert row['exit_code']==0 and row['actual_call_state']['create_result']==row['actual_call_state']['destroy_result']==1
            expected=0x20000 if row['precision']==24 else 0x10000
            for name,number in row['actual_call_state'].items():
                if name.endswith('control_before') or name.endswith('control_after'):assert number&0x30000==expected,(row['format'],name,number)
            for name,sha in row['artifacts'].items():assert digest(ROOT/row['directory']/name)==sha,name
        name=f'squall-tracker-loader-data-{label}-validation.json';write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name);runs.append(value)
    failure_path=ROOT/'.local/squall-tracker-loader-data/20261105T045617640951Z/report.json';failure=read(failure_path)
    assert collections.Counter(row['exit_code'] for row in failure['cases'])=={0:8,1:80}
    for row in failure['cases']:
        if row['exit_code']:
            assert row['stderr'].strip()=='Missing allocated tracker data'
            obj=(ROOT/row['directory']/'after-create.bin').read_bytes()
            assert struct.unpack_from('<I',obj,0x9a0c)[0]==0 and struct.unpack_from('<H',obj,0x9dca)[0]==64
    observation=dict(status='diagnostic_unused_pattern_allocation_assumption_rejected_then_repaired',
        initial_report_path=failure_path.relative_to(ROOT).as_posix(),initial_report_sha256=digest(failure_path),normal_processes=8,diagnostic_exit1_processes=80,
        actual_original_unused_pattern_size=64,actual_original_unused_pattern_pointer=0,
        resolution='Copy second pattern only when its actual allocation pointer exists;row count alone is not allocation evidence',
        recorded_as_production_failure=False,production_changed=False,
        actual_requested_precision_scope='Current probe selects precision after module/fixture preparation and captures before/after Create,OrderInfo,Destroy;older preliminary helper labels did not independently measure this boundary',
        whole_squall_complete=False,whole_game_complete=False)
    name='squall-tracker-loader-data-initial-observations.json';write(ROOT/'analysis'/name,observation);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    assert [v['binaries']['source.dll'] for v in runs]==full['candidate_sha256']
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_eleven_additional_tracker_inputs_complete_initialized_objects_and_owned_data_in_both_builds_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        original_instructions_sha256=digest(ROOT/'analysis/squall-tracker-loader-data-native-instructions.txt'),
        format_inputs=[f['format'] for f in runs[0]['fixtures']],actual_processes_per_build=88,loader_order_case_pairs_per_build=44,
        complete_object_pairs_per_build=66,initialized_storage_bytes_per_build=3751968,explicit_address_slot_class_pairs_per_build=71742,
        actual_allocated_bytes_per_build=197524,actual_instrument_header_bytes=632,actual_669_comment_allocation_bytes=109,
        requested_precision_before_after_actual_calls_verified=True,precision_contexts=['PC53','PC24'],
        excluded_address_slots=1087,external_nonnull_pointer_contents_claimed_equal_only_for_dumped_allocations=True,
        native_factory_underallocation_claimed_safe=False,unsafe_title_cleanup_claimed_fixed=False,leaked_legacy_comment_cleanup_claimed_verified=False,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        candidate_sha256=full['candidate_sha256'],original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,
        pending=['Remaining AMS/DMF/MDL/MED/MT2/PSM/UMX loaders and broader variants','Further tracker effects,public playback and original export signatures','Remaining whole original goal scopes'])
    write(ROOT/'analysis/squall-tracker-loader-data-regression.json',result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');provenance['additional_tracker_loaders_verification']=dict(report='analysis/squall-tracker-loader-data-regression.json',production_implementation_changed=False,whole_squall_complete=False)
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-tracker-loader-data-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-tracker-loader-data-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/squall-tracker-loader-data-regression.json'
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both88-process loader suites,197524 allocated bytes and3751968 initialized storage bytes per build.')

if __name__=='__main__':main()
