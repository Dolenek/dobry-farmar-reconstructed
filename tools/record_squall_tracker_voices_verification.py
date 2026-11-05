#!/usr/bin/env python3
"""Record actual voice kernels and complete active mixer state comparisons."""
import datetime,hashlib,struct
from record_live_production_verification import ROOT,digest,read,write

def cap_observation(report):
    result={};work=ROOT/report['work_directory']
    for backend in ['native','source']:
        folder=work/backend;rows=read(folder/'stdout.json')['cases'];data=(folder/'destinations.bin').read_bytes();scalar=(folder/'channel-scalars.bin').read_bytes()
        offset=state_offset=0;cases={}
        for row in rows:
            size=row['count']*8+32;state_size=row['channels']*252
            if all(row[k]==v for k,v in [('precision',53),('count',7),('channels',33),('variant',1),('increment',0x8000),('loop',1)]) and row['mode'] in [0x10,0x10010]:
                cases[row['mode']]=(data[offset:offset+size],struct.unpack_from('<2i',scalar,state_offset+80))
            offset+=size;state_offset+=state_size
        assert len(cases)==2 and cases[0x10][0]!=cases[0x10010][0]
        assert cases[0x10][1]==(0,0) and cases[0x10010][1]==(-1784768,-1784768)
        result[backend]=dict(normal_32_channel_cap_last_channel_offset=list(cases[0x10][1]),
            direct_to_disk_33rd_channel_offset=list(cases[0x10010][1]),
            normal_buffer_sha256=hashlib.sha256(cases[0x10][0]).hexdigest(),
            direct_to_disk_buffer_sha256=hashlib.sha256(cases[0x10010][0]).hexdigest())
    assert result['native']==result['source'];return result

def main():
    reports={};runs={};cap={}
    for label,suffix in [('main',''),('fresh','-fresh')]:
        runs[label]={}
        for suite,status,count in [('voices','passed_actual_active_voice_kernels',9216),('active-mix','passed_actual_active_channel_mix',8640)]:
            value=read(ROOT/'.local'/f'squall-tracker-{suite}{suffix}-validation.json');work=ROOT/value['work_directory']
            assert value['status']==status and value['case_pairs']==count and value['different_case_pairs']==value['different_destination_bytes']==value['different_channel_scalar_bytes']==0
            for name,sha in value['source_files'].items():assert digest(ROOT/name)==sha and digest(work/'snapshots'/name)==sha,name
            for name,sha in value['binaries'].items():assert digest(work/name)==sha,name
            for row in value['runs']:
                assert row['exit_code']==0
                for name,sha in row['artifacts'].items():assert digest(ROOT/row['directory']/name)==sha,name
            name=f'squall-tracker-{suite}-{label}-validation.json';write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name);runs[label][suite]=value
            if suite=='active-mix':cap[label]=cap_observation(value)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    for index,label in enumerate(['main','fresh']):
        for value in runs[label].values():assert value['binaries']['source.dll']==full['candidate_sha256'][index]
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_128_voice_table_entries_and_complete_active_mixer_in_both_builds_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        native_instructions_sha256=digest(ROOT/'analysis/squall-tracker-active-mix-native-instructions.txt'),
        original_addresses=dict(general_function_table='1004851c',fast_function_table='1004861c',create_stereo_mix='10017fe0',sample_count='10018280'),
        function_table_entries_per_build=128,kernel_case_pairs_per_build=9216,complete_mixer_case_pairs_per_build=8640,
        guarded_destination_bytes_per_build=8303616,channel_scalar_bytes_per_build=28449792,
        active_channel_counts=[1,2,33],precision_contexts=['PC53','PC24'],
        actual_cap_observations=cap,read_only_input_samples_verified=True,
        address_fields_excluded='Per-channel pCurrentSample raw address only;unchanged/cleared ownership state verified and compared for every call',
        original_heap_factory_scope_claimed_safe=False,original_loader_fallback_scope_claimed_safe=False,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        implementation_files=full['implementation_files'],candidate_sha256=full['candidate_sha256'],original_files_unchanged=17,
        whole_squall_complete=False,whole_game_complete=False,
        pending=['More complete tracker loaders,active effects and safe actual public playback scopes','Remaining original export signatures,codec and whole-game goal scopes'])
    write(ROOT/'analysis/squall-tracker-voices-regression.json',result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json')
    provenance['active_tracker_mixer_verification']=dict(report='analysis/squall-tracker-voices-regression.json',production_implementation_changed=False,whole_squall_complete=False)
    write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-tracker-voices-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-tracker-voices-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/squall-tracker-voices-regression.json'
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both builds:128 actual voice-table entries,17856 call pairs,8303616 guarded destination bytes and28449792 scalar bytes each.')

if __name__=='__main__':main()
