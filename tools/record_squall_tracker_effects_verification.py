#!/usr/bin/env python3
"""Record complete owned-object comparisons of actual tracker effects."""
import datetime,hashlib
from record_live_production_verification import ROOT,digest,read,write

def main():
    reports={};runs=[]
    for label,suffix in [('main',''),('fresh','-fresh')]:
        value=read(ROOT/'.local'/f'squall-tracker-effects{suffix}-validation.json');work=ROOT/value['work_directory']
        assert value['status']=='passed_actual_effect_methods_only' and value['exit_code']==0
        summary=value['summary'];assert summary==dict(total_pairs=1409024,different_pairs=0,control_differences=0,guard_differences=0,object_bytes=61196,whole_object_bytes_compared=86226632704)
        assert len(value['methods'])==43 and all(row['pairs']==32768 and row['different_pairs']==0 for row in value['methods'])
        assert [row['method'] for row in value['methods'][11:]]==[f'ProcessEffects_{n}' for n in range(32)]
        for name,sha in value['source_files'].items():assert digest(ROOT/name)==digest(work/'snapshots'/name)==sha,name
        for name,sha in value['binaries'].items():assert digest(work/name)==sha,name
        for name,sha in value['artifacts'].items():assert digest(work/name)==sha,name
        name=f'squall-tracker-effects-{label}-validation.json';write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name);runs.append(value)
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    assert [v['binaries']['source.dll'] for v in runs]==full['candidate_sha256']
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_eleven_direct_effect_methods_and_all32_actual_dispatch_commands_in_both_builds_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        original_instructions_sha256=digest(ROOT/'analysis/squall-tracker-effects-native-instructions.txt'),
        actual_call_pairs_per_build=1409024,complete_object_bytes_per_pair=61196,initialized_object_bytes_compared_per_build=86226632704,
        precision_contexts=['PC53','PC24'],parameters='all0..255',module_types=runs[0]['module_types'],
        all_pointer_fields_zero_in_fixture=True,no_address_exclusions=True,no_original_factory_used=True,
        initial_manifest_observation=dict(directory='.local/squall-tracker-effects/20261105T130404847891Z',
            failure='Original preliminary manifest requested out-of-line source Vibrato,which optimization inlined;no probe executed',
            resolution='Test actual production ProcessEffects dispatcher instead;no production flags or code changed'),
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        candidate_sha256=full['candidate_sha256'],original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,
        pending=['Active instrument/sample note lifecycle,volume-column commands and sequential effect histories','Public tracker playback and unsafe original factory/layout boundaries','Complete independent export signature semantics and remaining whole original goal scopes'])
    name='squall-tracker-effects-regression.json';write(ROOT/'analysis'/name,result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');provenance['tracker_effects_verification']=dict(report='analysis/'+name,production_implementation_changed=False,whole_squall_complete=False);write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    verification=read(ROOT/'analysis/verification.json')
    for entry in list(reports)+[name]:
        if 'analysis/'+entry not in verification['reports']:verification['reports'].append('analysis/'+entry)
    verification['latest_scoped_regression']=dict(report='analysis/'+name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded both1409024-pair actual effect suites;whole original goal remains incomplete.')

if __name__=='__main__':main()
