#!/usr/bin/env python3
"""Bind native DLL FP startup to four actual whole-game restores and97 current ABI proofs."""
import datetime
import hashlib
import json
from record_live_production_verification import ROOT,digest,read,write,checked
from current_production_regression import current_regression
from counter_save_audit import exact_restored_binary


def main():
    path=ROOT/'.local/squall-loader-contract-interop-validation.json';report=read(path);work=checked(report)
    assert report['status']=='passed_actual_contract_streak_and_four_whole_bidirectional_saves_only'
    assert report['expected_streak']==35
    baseline=ROOT/report['fixture_report']['path'];assert digest(baseline)==report['fixture_report_sha256']
    fixture=read(baseline);seed_work=ROOT/fixture['work_directory']
    transfers=report['comparison']['transfer_comparisons'];assert len(transfers)==len(report['sessions'])==4
    sizes=[]
    for i,(session,row) in enumerate(zip(report['sessions'],transfers)):
        assert session['kind']==['source','native','native','source'][i]
        assert session['status']=='completed' and session['exit_code']==0
        live=session['restored_expanded_farm'];assert live['streak']==35 and live['task20_owned'] and live['awards'][4]
        assert not live['task40_owned'] and not live['awards'][5]
        location=lambda n:(seed_work if n<2 else work)/f'{n}-persisted/Profiles'
        result=exact_restored_binary(location(row['before'])/'00000001.save',location(row['after'])/'00000001.save')
        assert result==row['binary'] and result['whole_binary_unchanged'] and not row['normalizations']
        assert result['complete_expected_output_bytes']==[17166,17200,17166,17166][i]
        sizes.append(result['complete_expected_output_bytes'])
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        events=[json.loads(s) for s in path.read_text(encoding='utf8').splitlines()]
        assert events[-1]==dict(event='exit',exit_code=0)
        assert not any(e['event']=='access_violation' for e in events)
        traces[path.name]=dict(sha256=digest(path),events=events)
    assert len(traces)==2
    full_name,full=current_regression();assert report['implementation_files']['Squall.dll']==full['candidate_sha256'][0]
    audit=read(ROOT/'analysis/squall-export-audit.json');assert audit['native_signature_semantics_verified_count']==97
    assert audit['rebuilt_sha256']==full['candidate_sha256'][0]
    seen={};evidence={};returns=0
    for row in audit['exports']:
        assert row['native_signature_fully_verified'] and row['status']=='partially_compared'
        name=row['native_signature_semantic_evidence']['report'];value=read(ROOT/name)
        assert value['current_candidate_sha256']==full['candidate_sha256']
        assert value['auditor_sha256']==digest(ROOT/'tools'/('audit_squall_'+name.split('/')[-1][7:-5].replace('-','_')+'.py'))
        actual=next(e for e in value['exports'] if e['name']==row['name'])
        assert actual['original_address']==row['original_address']
        seen[row['name']]=actual['all_native_return_sites'];returns+=len(actual['all_native_return_sites'])
        evidence[name]=digest(ROOT/name)
    assert len(seen)==97 and returns==223
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'
    for item in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/item['path'])==item['sha256']
    name='analysis/squall-loader-contract-interop-validation.json'
    report.update(independent_source_exit_traces=traces,recorder_sha256=digest(__file__))
    write(ROOT/name,report)
    result_name='analysis/squall-loader-game-regression.json'
    result=dict(status='passed_native_loader_FP_startup_new_full102_four_current_whole_game_restores_and97_physical_abi_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reports={full_name:digest(ROOT/full_name),name:digest(ROOT/name),**evidence},
        abi_exports=97,native_public_return_sites=223,whole_bidirectional_save_bytes=sizes,
        source_restore_processes_without_access_violation=2,current_candidate_sha256=full['candidate_sha256'],
        current_full_ctest_report=full_name,new_full_ctest_run_for_this_record=False,original_files_unchanged=17,
        whole_game_complete=False,whole_squall_complete=False,
        pending=['Continue actual40 contract thresholds and final all-awards condition','Wider DLL,JNG and uninstall scopes of original goal'])
    write(ROOT/result_name,result)
    verification=read(ROOT/'analysis/verification.json')
    for n in [name,result_name]:
        if n not in verification['reports']:verification['reports'].append(n)
    if name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append(name)
    verification['latest_scoped_regression']=dict(report=result_name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf8')).hexdigest()
    verification['source_tree_hash_as_of_report']=result_name;write(ROOT/'analysis/verification.json',verification)
    print('Verified4 current whole-game restores,17166/17200 bytes,2 source exits without AV and97/223 native ABI interfaces/returns.')


if __name__=='__main__':main()
