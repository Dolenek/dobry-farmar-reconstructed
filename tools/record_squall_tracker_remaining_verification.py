#!/usr/bin/env python3
"""Record both actual remaining-loader suites without broadening their scope."""
import datetime,hashlib,struct
from record_live_production_verification import ROOT,digest,read,write

def main():
    reports={};runs=[]
    for label,suffix in [('main',''),('fresh','-fresh')]:
        value=read(ROOT/'.local'/f'squall-tracker-remaining-loaders{suffix}-validation.json');work=ROOT/value['work_directory']
        assert value['status']=='passed_additional_loader_data_scope' and value['different_pairs']==0 and len(value['cases'])==176
        assert value['object_pairs']==132 and value['initialized_storage_bytes']==7503936 and value['exact_allocated_bytes']==362932 and value['address_slot_states']==143484
        assert [f['format'] for f in value['fixtures']]==['MTM','669','STM','ULT','FAR','OKT','DSM','PTM','DBM','AMF14','AMF0','UMX_IT','UMX_XM','UMX_S3M','UMX_MOD','MMD0','MMD1','MDL','PSM','DMF','MT2','AMS']
        for name,sha in value['source_files'].items():assert digest(ROOT/name)==digest(work/'snapshots'/name)==sha,name
        for name,sha in value['binaries'].items():assert digest(work/name)==sha,name
        for name,sha in value['fixture_hashes'].items():assert digest(work/'fixtures'/name)==sha,name
        for row in value['cases']:
            assert row['exit_code']==0 and row['actual_call_state']['create_result']==row['actual_call_state']['destroy_result']==1
            expected=0x20000 if row['precision']==24 else 0x10000
            for name,number in row['actual_call_state'].items():
                if name.endswith('control_before') or name.endswith('control_after'):assert number&0x30000==expected,(row['format'],name,number)
            folder=ROOT/row['directory']
            for name,sha in row['artifacts'].items():assert digest(folder/name)==sha,name
            assert any((folder/'pattern0.bin').read_bytes())
            sample=(folder/'instrument1-pcm.bin').read_bytes();assert len(sample)==2048 and len(set(sample))>128
            if row['format']=='AMS':
                assert (folder/'legacy-pattern-names.bin').read_bytes()==bytes(32)
                fixture=next(f for f in value['fixtures'] if f['format']=='AMS')
                assert sample==(work/'fixtures'/fixture['name']).read_bytes()[-2048:]
        name=f'squall-tracker-remaining-{label}-validation.json';write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name);runs.append(value)
    failure_path=ROOT/'.local/squall-tracker-remaining-loaders/20261105T125501875469Z/report.json';failure=read(failure_path)
    assert failure['different_pairs']==4 and all(r['exit_code']==0 for r in failure['cases'])
    assert all(row['format']=='AMS' and row['differences']==['scalar states'] for row in failure['differences'])
    for a,b in zip(failure['cases'][::2],failure['cases'][1::2]):
        if a['format']=='AMS':
            for stage,x in a['states'].items():
                y=b['states'][stage];assert {k for k,v in x.items() if v!=y[k]}=={'legacy_pattern_name_pointer'}
    observation=dict(status='diagnostic_raw_pointer_scalar_comparison_replaced_with_address_class_and_actual_owned_bytes',
        initial_report_path=failure_path.relative_to(ROOT).as_posix(),initial_report_sha256=digest(failure_path),
        normal_processes=88,ams_pointer_only_differences=4,production_changed=False,
        resolution='Retain raw addresses and snapshots;compare null/object-relative/external address classes and separately copy all32 owned AMS pattern-name bytes',
        original_ams_sample_flags={'RS_AMS8':17,'RS_AMS16':21},native_ReadSample='0x1002CBB0',
        native_jump_table='0x1002CFE0',actual_ams_default_copy_target='0x1002CF86',
        general_ams_compression_claimed_verified=False,whole_squall_complete=False)
    name='squall-tracker-remaining-initial-observations.json';write(ROOT/'analysis'/name,observation);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    assert [r['binaries']['source.dll'] for r in runs]==full['candidate_sha256']
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_twenty_two_controlled_tracker_loader_inputs_in_both_builds_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        original_instructions_sha256=digest(ROOT/'analysis/squall-tracker-remaining-native-instructions.txt'),
        format_inputs=[f['format'] for f in runs[0]['fixtures']],actual_processes_per_build=176,loader_order_case_pairs_per_build=88,
        complete_object_pairs_per_build=132,initialized_storage_bytes_per_build=7503936,explicit_address_slot_class_pairs_per_build=143484,
        actual_allocated_bytes_per_build=362932,ams_pattern_names_bytes_per_case=32,
        requested_precision_before_after_actual_calls_verified=True,precision_contexts=['PC53','PC24'],
        excluded_address_slots=1087,external_pointer_contents_claimed_equal_only_for_dumped_allocations=True,
        native_factory_underallocation_claimed_safe=False,unsafe_title_cleanup_claimed_fixed=False,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        candidate_sha256=full['candidate_sha256'],original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,
        pending=['Broader format variants including AMS2,compressed AMS/DMF and MT2 effects','Public tracker playback and unsafe original factory/layout boundaries','Complete independent export signature semantics and remaining whole original goal scopes'])
    name='squall-tracker-remaining-regression.json';write(ROOT/'analysis'/name,result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');provenance['remaining_tracker_loaders_verification']=dict(report='analysis/'+name,production_implementation_changed=False,whole_squall_complete=False);write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json')
    for entry in list(reports)+[name]:
        if 'analysis/'+entry not in verification['reports']:verification['reports'].append('analysis/'+entry)
    verification['latest_scoped_regression']=dict(report='analysis/'+name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both176-process suites,362932 actual allocation bytes and7503936 initialized object bytes per build.')

if __name__=='__main__':main()
