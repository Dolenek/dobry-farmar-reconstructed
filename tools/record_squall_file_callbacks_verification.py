#!/usr/bin/env python3
"""Record native file callback stores, reader ownership and diagnostic failures."""
import datetime,hashlib
from record_live_production_verification import ROOT,digest,read,write

def main():
    reports={};runs=[]
    for label,name in [('main','squall-file-callbacks-validation.json'),('fresh','squall-file-callbacks-fresh-validation.json')]:
        value=read(ROOT/'.local'/name);work=ROOT/value['work_directory']
        assert value['status']=='passed_actual_file_callbacks_pointer_storage_and_reader_ownership_scope'
        for path,sha in value['source_files'].items():assert digest(ROOT/path)==sha,path
        assert digest(ROOT/value['candidate_path'])==value['rebuilt_sha256']
        assert digest(ROOT/value['oracle_path'])==value['oracle_sha256']
        assert digest((ROOT/value['candidate_path']).with_suffix('.map'))==value['map_sha256']
        assert value['pointer_combinations']==24576 and value['actual_pcm_reads']==30720 and value['guarded_pcm_bytes']==1683456
        for row in value['precision_contexts']:
            assert digest(work/row['raw_callback_event_file'])==row['raw_callback_event_file_sha256']
            assert row['pointer_fpu_checks']==12288 and row['forced_failures_per_dll']==9
        for row in value['processes']:assert row['exit_code']==0
        target=f'squall-file-callbacks-{label}-validation.json';write(ROOT/'analysis'/target,value);reports[target]=digest(ROOT/'analysis'/target);runs.append(value)
    failures=[]
    for stamp in ['20261105T040127453736Z','20261105T040430252657Z','20261105T040523563203Z','20261105T040627225254Z']:
        folder=ROOT/'.local/squall-file-callbacks'/stamp;failed=read(folder/'report.json')
        assert failed['status']=='failed' and 'Precision changed' in failed['error']
        assert failed['precision_contexts'][0]['precision']=='PC53'
        failures.append(dict(work_directory=failed['work_directory'],report_sha256=digest(folder/'report.json'),
            source_snapshot_sha256=digest(folder/'squall_file_callbacks_reference_test.cpp'),
            driver_snapshot_sha256=digest(folder/'validate_squall_file_callbacks.py'),error=failed['error'],
            recorded_as_production_failure=False,pc24_run_claimed_passed=False))
    buffered=ROOT/'.local/squall-file-callbacks/20261105T040627225254Z/PC24.stderr.log';trace=buffered.read_text(encoding='utf-8')
    assert 'before-pointer-pages pc=131072' in trace and 'after-pointer-pages pc=65536' in trace
    observation=dict(status='diagnostic_fixture_allocation_fpu_change_identified_and_precision_scope_restored',failures=failures,
        buffered_trace_sha256=digest(buffered),buffered_trace=trace,
        resolution='Prepare three pointer-validity pages before selecting PC53/PC24;no control-word resets between measured original/source calls;check all12288 pointer cases and lifecycle boundaries per precision',
        allocation_side_effect_root_cause_claimed_known=False,production_changed=False,
        initial_map_path_error=dict(driver_sha256=digest(ROOT/'.local/squall-file-callbacks-before-map-fix/validate_squall_file_callbacks.py'),log_sha256=digest(ROOT/'.local/squall-file-callbacks-before-map-fix/run.log'),reference_probe_process_created=False),
        initial_missing_string_header_build_log_sha256=digest(ROOT/'.local/squall-file-callbacks-initial-build.log'),whole_squall_complete=False,whole_game_complete=False)
    name='squall-file-callbacks-initial-observations.json';write(ROOT/'analysis'/name,observation);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/path)==sha for path,sha in full['production_source_files'].items())
    assert [v['rebuilt_sha256'] for v in runs]==full['candidate_sha256']
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_file_callback_pointer_storage_actual_reader_ownership_and_guarded_decode_in_both_builds_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        native_instructions_sha256=digest(ROOT/'analysis/squall-file-callbacks-native-instructions.txt'),
        original_addresses=dict(export='10003c10',store='100085a0',callback_slots='10075990..1007599c'),
        pointer_combinations_per_build=24576,exact_pointer_store_pairs_per_build=98304,actual_pcm_reads_per_build=30720,
        guarded_pcm_bytes_per_build=1683456,actual_file_callback_events_per_dll_per_build=16020,
        successful_sample_loads_per_dll_per_build=1182,controlled_initialization_failures_per_dll_per_build=18,
        precision_contexts=['PC53','PC24'],lifecycle_phases=['before_Init','actual_DirectSound_initialized','after_Free'],
        current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',
        new_full_ctest_run=False,production_implementation_changed=False,candidate_sha256={label:value['rebuilt_sha256'] for label,value in zip(['main','fresh'],runs)},
        original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,
        pending=['Invoked invalid callbacks and concurrent setter mutation','Remaining complete export signatures,codec and whole original goal scopes'])
    write(ROOT/'analysis/squall-file-callbacks-regression.json',result)
    audit=read(ROOT/'analysis/squall-export-audit.json');row=next(row for row in audit['exports'] if row['name']=='SQUALL_SetFileCallbacks')
    row['verified_scope']='Both builds/PC53/PC24:24576 pointer combinations,exact stores even on invalid return,all16 A/B callback-generation combinations,retained readers,guarded PCM and controlled open/seek/read failures'
    row['loaded_DLL_verified_scope']=row['verified_scope']
    row['file_callback_scope']=dict(report='analysis/squall-file-callbacks-regression.json',original_store_addresses='10075990..1007599c',invalid_returns_still_store_exact_pointers=True)
    assert row['status']=='partially_compared' and not row['native_signature_fully_verified']
    write(ROOT/'analysis/squall-export-audit.json',audit)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json')
    provenance['file_callback_behavior_verification']=dict(report='analysis/squall-file-callbacks-regression.json',production_implementation_changed=False,whole_squall_complete=False)
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-file-callbacks-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-file-callbacks-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/squall-file-callbacks-regression.json'
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both actual file-callback builds,24576 pointer combinations and30720 guarded PCM reads per build.')

if __name__=='__main__':main()
