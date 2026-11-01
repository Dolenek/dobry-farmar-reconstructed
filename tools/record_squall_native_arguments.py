#!/usr/bin/env python3
"""Record independent original stack references and original callee return bytes."""
import datetime
from pe import PE
from record_live_production_verification import ROOT,digest,read,write


def main():
    report=read(ROOT/'.local/squall-native-arguments-audit.json')
    assert report['status']=='passed_all97_native_referenced_argument_slots_only' and report['reference_files_unchanged']==17
    assert report['auditor_sha256']==digest(ROOT/'tools/audit_squall_native_arguments.py')
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    assert digest(ROOT/report['declared_contract_report'])==report['declared_contract_report_sha256']
    assert digest(ROOT/'.local/squall-complete-native-disassembly.txt')==report['disassembly_sha256']
    native=ROOT.parent/'farmář/Dobrý farmář/Squall.dll';assert digest(native)==report['original_sha256'];pe=PE(native)
    exports=report['exports'];assert len(exports)==97 and sum(len(row['stack_references']) for row in exports)==230
    assert sum(row['symbolic_states'] for row in exports)==5697
    assert len(report['call_cleanup_evidence'])==603 and len(report['direct_callee_cleanup_summaries'])==63
    for row in exports:
        assert row['referenced_argument_slots']==row['declared_argument_count']
        assert row['explicit_argument_slots']==list(range(1,row['declared_argument_count']+1)) and row['balanced_return_addresses']
    for rows in report['direct_callee_return_evidence'].values():
        for row in rows:
            raw=bytes.fromhex(row['instruction_bytes']);offset=pe.offset(int(row['address'],16)-pe.image_base,len(raw));assert pe.data[offset:offset+len(raw)]==raw
            assert raw==b'\xc3' if row['pop_bytes']==0 else raw==b'\xc2'+row['pop_bytes'].to_bytes(2,'little')
    for suffix in ['', '-fresh']:
        declared=read(ROOT/f'analysis/squall-export-stack{suffix}-validation.json')
        for context in declared['precision_contexts']:
            contracts={row['name']:row for row in context['measured_exports']}
            for row in exports:assert row['referenced_argument_slots']==contracts[row['name']]['argument_count']
    write(ROOT/'analysis/squall-native-arguments-audit.json',report)
    audit=read(ROOT/'analysis/squall-export-audit.json')
    for row in audit['exports']:
        native_row=next(x for x in exports if x['name']==row['name'])
        row['native_argument_slots_verified']=dict(count=native_row['referenced_argument_slots'],slots=native_row['explicit_argument_slots'],reference_instructions=[x['address'] for x in native_row['stack_references']],
            scope='Independent original ESP/EBP analysis, native direct-callee RET pop bytes, eight explicit COM call push contracts and Win32 import contracts. All reachable tracked returns balance. Pointer/return semantics remain separately audited.')
    audit['native_argument_reports']=['analysis/squall-native-arguments-audit.json'];write(ROOT/'analysis/squall-export-audit.json',audit)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(native.parent/row['path'])==row['sha256']
    result=dict(status='passed_all97_original_explicit_argument_slot_counts_only',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        report='analysis/squall-native-arguments-audit.json',report_sha256=digest(ROOT/'analysis/squall-native-arguments-audit.json'),exports=97,
        original_argument_reference_instructions=230,symbolic_states=5697,original_callee_cleanup_summaries=63,call_cleanup_sites=603,
        native_argument_slot_count_verified=True,whole_native_pointer_and_return_semantics_verified=False,
        native_input_disassembly_sha256=report['disassembly_sha256'],production_implementation_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,whole_squall_complete=False,
        pending=['Remaining native pointer/return/callback semantics beyond current behavioral audits','Multiple-member sample selection and other remaining DLL scopes','Remaining whole original goal scopes'])
    write(ROOT/'analysis/squall-native-arguments-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in ['squall-native-arguments-audit.json','squall-native-arguments-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/squall-native-arguments-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded independent explicit argument slots for all97 originals; production remains unchanged.')


if __name__=='__main__':main()
