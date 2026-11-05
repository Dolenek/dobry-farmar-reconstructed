"""Publish original ABI evidence without upgrading behavioral coverage."""
import datetime,hashlib
from record_live_production_verification import ROOT,digest,read,write

def publish(filename,rows,auditor,details):
    abi=[read(ROOT/'analysis'/n) for n in ['squall-tracker-loaders-abi.json','squall-tracker-loaders-fresh-abi.json']]
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(v['status']=='matches_export_contract' for v in abi) and [v['rebuilt_sha256'] for v in abi]==full['candidate_sha256']
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    native=ROOT.parent/'farmář/Dobrý farmář/Squall.dll'
    for v in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/v['path'])==v['sha256']
    report_name='analysis/'+filename
    result=dict(status='passed_listed_native_argument_and_return_abi_semantics_only',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        auditor_sha256=digest(auditor),publisher_sha256=digest(__file__),original_sha256=digest(native),exports=rows,
        current_candidate_sha256=full['candidate_sha256'],current_source_header_sha256=digest(ROOT/'src/squall/runtime/Squall.h'),
        new_runtime_audio_test_claimed=False,production_changed=False,original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,details=details)
    write(ROOT/report_name,result)
    audit=read(ROOT/'analysis/squall-export-audit.json');by_name={row['name']:row for row in rows}
    for row in audit['exports']:
        if row['name'] in by_name:
            value=by_name[row['name']];assert row['original_address']==value['original_address']
            row['native_signature_fully_verified']=True
            row['native_signature_semantic_evidence']=dict(report=report_name,argument=value['argument_role'],return_role=value['return_role'],whole_behavior_verified=False)
    count=sum(bool(row.get('native_signature_fully_verified')) for row in audit['exports'])
    audit['native_signature_semantics_verified_count']=count;audit['rebuilt_sha256']=abi[0]['rebuilt_sha256'];write(ROOT/'analysis/squall-export-audit.json',audit)
    verification=read(ROOT/'analysis/verification.json')
    if report_name not in verification['reports']:verification['reports'].append(report_name)
    verification['native_squall_signature_semantics']=dict(report=report_name,verified_export_count=count,remaining_export_count=97-count,whole_behavior_verified=False)
    verification['latest_scoped_regression']=dict(report=report_name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    hashes=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(hashes).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']=report_name
    write(ROOT/'analysis/verification.json',verification)
    return count
