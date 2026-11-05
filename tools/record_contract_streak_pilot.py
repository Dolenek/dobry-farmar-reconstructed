#!/usr/bin/env python3
"""Audit the first complete real contract receipt and the supply/transfer ledgers."""
import datetime
import hashlib
import math
import struct
from record_live_production_verification import ROOT,digest,read,write,checked
from validate_counter_supply_state import props
from validate_factory_history_state import resource
from game_save_state import load_save
from gameplay_coefficient_audit import audit_coefficients,f32


def main():
    report=read(ROOT/'.local/contract-streak-validation.json');work=checked(report)
    assert report['status']=='explored_actual_contract_streak_with_original_supply_command_only' and report['requested_target']==1
    baseline_report=ROOT/report['baseline_report']['path'];assert digest(baseline_report)==report['baseline_report']['sha256']
    base=ROOT/read(baseline_report)['work_directory']/'0-persisted/Profiles'
    base_values=props(base/'00000001.xml');base_save=load_save(base/'00000001.save')
    baseline_stock=[int(base_values[f'WGoodCount{n}'][1]) for n in range(19)]
    events=list(resource('Gameplay/gameplay.xml').find('Events'))
    events.sort(key=lambda e:sum(int(v)*m for v,m in zip(e.get('Time').split(':'),[3600,60,1])))
    assert digest(ROOT/report['gameplay_layout']['path'])==report['gameplay_layout']['sha256']
    assert report['gameplay_layout']['value']['coefficient_vector']==48
    branches=[]
    for index,session in enumerate(report['sessions']):
        assert session['kind']==['native','source'][index] and session['status']=='completed' and session['exit_code']==0
        assert session['initial_streak']==0 and len(session['contract_receipts'])==1
        assert len(session['coefficient_epochs'])==1
        assert session['coefficient_epochs'][0]['words']==session['final_coefficient_words']==base_save['coefficient_bits']
        coefficients=audit_coefficients(base_save['coefficient_bits'],events)
        assert len(session['supply_commands'])==6 and len(session['market_transfers'])==102
        assert not session.get('acceptance_input_retries') and not session.get('confirmation_input_retries')
        additions=[0]*19
        for row in session['supply_commands']:
            assert row['command']=='cheatproduct'
            left,right=row['before'],row['after'];expected=left['stock'].copy();free=100-sum(expected)
            assert free>=0
            for product in range(19):
                amount=min(2,free);free-=amount;expected[product]+=amount;additions[product]+=amount
            assert right['stock']==expected and left['market']==right['market'] and left['money']==right['money']
            assert left['streak']==right['streak']==0
        for row in session['market_transfers']:
            product=row['product'];left,right=row['before'],row['after'];count=left['stock'][product]
            assert count==row['quantity']>0
            expected=left['stock'].copy();expected[product]=0
            expected_market=left['market'].copy();expected_market[product]+=count
            assert right['stock']==expected and right['market']==expected_market and left['money']==right['money']
            assert left['streak']==right['streak']==0
        receipt=session['contract_receipts'][0]
        assert receipt['offer']==368 and receipt['epoch']==0
        definition=events[368];assert definition.get('Type')=='5'
        data=definition.find('Data');coefficient=struct.unpack('<f',struct.pack('<I',base_save['coefficient_bits'][368]))[0]
        expected_requirements=[]
        for entry in list(data.find('Requirements'))[:2]:
            original=int(entry.get('Count'));expected_requirements.append(dict(product=int(entry.get('ProductType')),
                initial_count=original,count=original+math.trunc(f32(original*coefficient))))
        assert receipt['model']['requirements']==expected_requirements==[dict(product=4,initial_count=8,count=8),dict(product=12,initial_count=12,count=13)]
        expected_money={}
        for key in ['Bonus','Penalty']:
            original=int(data.get(key));value=original+math.trunc(f32(original*coefficient))
            if value>100:value=math.trunc(f32(math.trunc(f32(value*f32(.1)))*10))
            expected_money[key.lower()]=value
        assert expected_money==dict(bonus=880,penalty=220)
        assert all(receipt['model'][key]==value for key,value in expected_money.items())
        assert receipt['before_acceptance']['money']==2504 and receipt['before_completion']['money']==2284 and receipt['after_completion']['money']==3164
        assert receipt['before_acceptance']['streak']==receipt['before_completion']['streak']==0 and receipt['after_completion']['streak']==1
        consumed=[0]*19
        for requirement in expected_requirements:consumed[requirement['product']]+=requirement['count']
        final=session['final_streak_state']
        assert final['streak']==1 and final['money']==3164 and final['tasks']==[-1]*3
        for n in range(19):assert baseline_stock[n]+additions[n]==final['stock'][n]+final['market'][n]+consumed[n]
        values=props(work/f'{index}-persisted/Profiles/00000001.xml')
        assert values['awQTask']==('UInt','1') and values['Money']==('Int','3164')
        assert values['Award04Make20Task']==values['Award05Make40Task']==('Bool','0')
        assert all(values[f'MGoodPrice{n}']==('UInt','99') and int(values[f'MGoodCount{n}'][1])==final['market'][n] and int(values[f'WGoodCount{n}'][1])==final['stock'][n] for n in range(19))
        assert [int(values[f'PCC{n}'][1]) for n in range(8)]==[12]+[0]*7
        saved=work/f'{index}-persisted/Profiles/00000001.save'
        assert saved.read_bytes()==(base/'00000001.save').read_bytes() and saved.stat().st_size==15808
        branches.append(dict(kind=session['kind'],actual_offer=368,actual_receipts=1,
            actual_requirement_goods=expected_requirements,actual_deposit=220,actual_bonus=880,
            actual_supply_commands=6,actual_market_transfers=102,coefficient_words_verified=383,
            coefficient_audit=coefficients,final_streak=1,money=3164,final_stock=final['stock'],final_market=final['market'],
            entire_binary_save_unchanged_bytes=15808,normalizations=[]))
    assert branches[0]['final_stock']==branches[1]['final_stock'] and branches[0]['final_market']==branches[1]['final_market']
    initial_path=ROOT/'.local/contract-streak/20261105T214247557521Z/report.json';initial=read(initial_path)
    assert initial['status']=='failed' and 'Actual offer was not accepted' in initial['error']
    initial_work=ROOT/initial['work_directory']
    for name,sha in initial['source_files'].items():
        old=ROOT/initial['source_file_snapshots'][name] if name in initial['source_file_snapshots'] else ROOT/name
        assert digest(old)==sha,name
    assert initial['sessions'][0]['status']=='completed' and initial['sessions'][0]['exit_code']==0
    assert initial['sessions'][1]['failure_cleanup_exit_code']==0
    failed=props(initial_work/'1-failure-persisted/Profiles/00000001.xml')
    assert failed['awQTask']==('UInt','0') and failed['Money']==('Int','2504') and failed['task0']==('Int','-1')
    full=read(ROOT/'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT/name)==sha for name,sha in full['production_source_files'].items())
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path'])==row['sha256']
    observation=dict(status='retained_unaccepted_source_offer_and_completed_fresh_paired_receipts',
        initial_report=initial_path.relative_to(ROOT).as_posix(),initial_report_sha256=digest(initial_path),
        completed_initial_native_receipts=1,initial_native_exit_code=0,initial_source_cleanup_exit_code=0,
        source_offer_remained_open_after_first_accept_click=True,source_counter_and_deposit_unchanged=True,
        cause_of_first_unaccepted_click_established=False,
        diagnostic_change='Wait0.5s after completed offer transition;retry only while same offer remains shown and no task or deposit change occurred',
        acceptance_or_confirmation_retry_branch_exercised_in_successful_pair=False,
        successful_fresh_native_and_source_exit_codes=[0,0],production_changed=False,whole_game_complete=False)
    report.update(status='passed_for_actual_contract_streak_receipts_with_original_supply_fixture_only',
        auditor_sha256=digest(__file__),comparison=dict(branches=branches,
            supply_commands_are_original_test_fixture=True,ordinary_farming_supply_claimed=False,
            original_profile_or_counter_fields_written_by_probe=False,normalizations=[]),whole_game_complete=False)
    state_name='contract-streak-pilot-state-validation.json';write(ROOT/'analysis'/state_name,report)
    obs_name='contract-streak-pilot-initial-observations.json';write(ROOT/'analysis'/obs_name,observation)
    result=dict(status='passed_paired_first_actual_contract_receipt_and_complete_supply_ledgers_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reports={n:digest(ROOT/'analysis'/n) for n in [state_name,obs_name]},actual_receipts_per_executable=1,
        original_supply_commands_per_executable=6,actual_market_transfers_per_executable=102,
        actual_deposit=220,actual_bonus=880,final_streak=1,final_money=3164,whole_binary_save_unchanged_bytes=15808,
        production_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report='analysis/squall-tracker-loaders-regression.json',new_full_ctest_run=False,
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Continue actual contract streak through20/40 thresholds and final all-awards condition',
                 'Remaining75 native Squall ABI signatures,broader DLL behavior,JNG,uninstaller and whole original goal scopes'])
    name='contract-streak-pilot-regression.json';write(ROOT/'analysis'/name,result)
    verification=read(ROOT/'analysis/verification.json')
    for entry in [state_name,obs_name,name]:
        if 'analysis/'+entry not in verification['reports']:verification['reports'].append('analysis/'+entry)
    if 'analysis/'+state_name not in verification['original_game_runtime']['scope_reports']:
        verification['original_game_runtime']['scope_reports'].append('analysis/'+state_name)
    verification['latest_scoped_regression']=dict(report='analysis/'+name,recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    paths=[ROOT/'CMakeLists.txt']+[p for folder in ['src','tools','tests'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows=[p.relative_to(ROOT).as_posix()+'\0'+digest(p)+'\n' for p in sorted(paths,key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest();verification['source_tree_hash_as_of_report']='analysis/'+name
    write(ROOT/'analysis/verification.json',verification)
    print('Verified both real first receipts,12 supply commands,204 market transfers,all383 coefficients and complete15808-byte saves.')


if __name__=='__main__':main()
