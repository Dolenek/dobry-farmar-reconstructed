#!/usr/bin/env python3
"""Record global configuration scope for both builds; keep full Squall coverage pending."""
import datetime,hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text(encoding='utf-8'))
def write(path,value):Path(path).write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def main():
    reports={}
    for suffix in ['', '-fresh']:
        name=f'squall-config{suffix}-validation.json';report=read(ROOT/'.local'/name)
        assert report['status']=='passed_for_actual_global_configuration_and_hardware_output_scope' and report['result_comparisons']==4634 and report['guarded_output_snapshots']==1320
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256'] and digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        for row in report['precision_contexts']:assert row['result_comparisons']==2317
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    methods=['Get3DAlgorithm','GetBufferSize','GetDevice','GetHardwareAcceleration','GetNumDevice','GetSpeakerMode','Set3DAlgorithm','SetBufferSize','SetDevice','SetHardwareAcceleration','SetSpeakerMode']
    audit=read(ROOT/'analysis/squall-export-audit.json');addresses={}
    for name in methods:
        row=next(row for row in audit['exports'] if row['name']=='SQUALL_'+name);addresses[row['name']]=row['original_address']
        row['status']='partially_compared';row['loaded_DLL_configuration_verified_scope']='Actual pre-Init/DirectSound/post-Free, two precisions/two builds; buffer1..5000,3D support table,speaker bounds,36 raw HW flag pairs/device recreation without channels,scalar getters and guarded optional HW outputs'
        assert not row['native_signature_fully_verified']
    audit['configuration_reports']=['analysis/'+name for name in reports];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/live-production-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_for_actual_global_configuration_scope_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        recorder_sha256=digest(__file__),reports=reports,original_export_addresses=addresses,api_results_per_build=4634,guarded_snapshots_per_build=1320,
        implementation_changed=False,current_full_ctest_report='analysis/live-production-regression.json',current_full_ctest_source_hashes_match=True,
        original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Prepared/playing channels across device recreation,device loss and failure rollback','All physical endpoints/algorithms and no-device host','Remaining97-export and full-game scopes'])
    write(ROOT/'analysis/squall-config-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['squall-config-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-config-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded11 global exports, two builds4634 results each; full97-export scope remains incomplete')
if __name__=='__main__':main()
