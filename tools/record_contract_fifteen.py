#!/usr/bin/env python3
"""Record actual streak15, literal WinNote and the retained incomplete audit."""
import datetime
import hashlib
import struct
from pe import PE
from record_live_production_verification import ROOT, digest, read, write
from audit_contract_streak_state import audit
from validate_counter_supply_state import props, seconds
from validate_factory_history_state import resource
from game_save_state import load_save


def main():
    raw_path = ROOT / '.local/contract-streak/20261106T182154720921Z/report.json'
    raw, branches = audit(raw_path)
    published = ROOT / 'analysis/contract-streak-fifteen-state-validation.json'
    assert read(published)['runtime_report_sha256'] == digest(raw_path)
    assert read(published)['auditor_sha256'] == digest(ROOT / 'tools/audit_contract_streak_state.py')
    assert [b['final_streak'] for b in branches] == [15, 15]
    assert [b['final_money'] for b in branches] == [11164, 10924]
    assert [b['supply_commands'] for b in branches] == [47, 56]
    assert [b['market_transfers'] for b in branches] == [813, 961]
    assert [b['complete_expected_saved_bytes'] for b in branches] == [16377, 16394]
    work = ROOT / raw['work_directory']
    for s in raw['sessions']:
        assert not s.get('runtime_award_contexts') and not s.get('pointer_move_retries')
        assert not s.get('acceptance_input_retries') and not s.get('confirmation_input_retries')
        before = props(ROOT / s['seed_profile_directory'] / '00000001.xml')
        assert before['Award17Getting10000Gold'] == before['awardShow17'] == ('Bool', '1')
    expected_old_path = ROOT / '.local/contract15-expected-1.save'
    old = load_save(expected_old_path)
    saved_path = work / '1-persisted/Profiles/00000001.save'
    saved = load_save(saved_path)
    assert old['bytes'] == 16377 and saved['bytes'] == 16394
    for key in ['seeds', 'sales', 'cutoff', 'coefficient_bits']:
        assert old[key] == saved[key]
    assert saved['messages'] == old['messages'] + [dict(read=1, text='+ 750$')]
    events = list(resource('Gameplay/gameplay.xml').find('Events'));events.sort(key=seconds)
    assert events[326].find('Data').findtext('WinNote') == '+ 750$'
    receipt = next(r for r in raw['sessions'][1]['contract_receipts'] if r['offer'] == 326)
    assert receipt['model']['bonus'] == 880
    assert receipt['after_completion']['money'] - receipt['before_completion']['money'] == 880
    pe = PE(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159.exe')
    instructions = [(0x004bbf80, '8b576c', 'mov edx,[edi+6c] active event'),
                    (0x004bbf83, '8b4a44', 'mov ecx,[edx+44] actual Bonus'),
                    (0x004bbf86, '03c8', 'add ecx,eax current money'),
                    (0x004bc0cb, '7414', 'je004bc0e1 skip empty WinNote'),
                    (0x004bc0cd, '8b4f6c', 'mov ecx,[edi+6c] active event'),
                    (0x004bc0d0, '83c168', 'add ecx,68 literal WinNote string'),
                    (0x004bc0d3, '6a01', 'push1 read flag'),
                    (0x004bc0d5, '51', 'push ecx WinNote pointer'),
                    (0x004bc0d6, '8b0d40f76500', 'mov ecx,[65f740] actual GameMenu'),
                    (0x004bc0dc, 'e8bf830000', 'call004c44a0 GameMenu::notify')]
    evidence = []
    for address, hex_bytes, meaning in instructions:
        data = bytes.fromhex(hex_bytes);offset = pe.offset(address - pe.image_base, len(data))
        assert pe.data[offset:offset + len(data)] == data
        evidence.append(dict(address=f'{address:08x}', bytes=hex_bytes, meaning=meaning))
    assert 0x004bc0dc + 5 + struct.unpack('<i', bytes.fromhex('bf830000'))[0] == 0x004c44a0
    observation_name = 'contract-streak-fifteen-win-note-observations.json'
    observation = dict(status='retained_incomplete_audit_then_verified_literal_contract_win_note',
        recorder_sha256=digest(__file__), raw_runtime_report=raw_path.relative_to(ROOT).as_posix(), raw_runtime_report_sha256=digest(raw_path),
        initial_audit_log='.local/contract-streak-fifteen-audit.log', initial_audit_log_sha256=digest(ROOT / '.local/contract-streak-fifteen-audit.log'),
        incomplete_expected_source_save=expected_old_path.relative_to(ROOT).as_posix(), incomplete_expected_source_save_sha256=digest(expected_old_path),
        incomplete_expected_bytes=16377, actual_and_correctly_expected_bytes=16394,
        missing_record=dict(read=1, text='+ 750$'), original_xml_event=326, actual_coefficient_adjusted_reward=880,
        actual_original_instruction_evidence=evidence,
        actual_native_runtime_receipt326_in_this_pair=False,
        static_native_literal_win_note_dispatch_verified=True,
        native_and_source_baselines_already_owned_award17=True, new_award17_panel_claimed=False,
        changed_production=False, incomplete_auditor_assumption='Only timed notes were included;actual success WinNote was omitted',
        auditor_correction='Interleave original literal read=true WinNote after its actual completion,without replacing text with adjusted bonus',
        normalizations=[], whole_game_complete=False)
    write(ROOT / 'analysis' / observation_name, observation)
    full = read(ROOT / 'analysis/squall-tracker-loaders-regression.json')
    assert all(digest(ROOT / n) == sha for n, sha in full['production_source_files'].items())
    for row in read(ROOT / 'analysis/inventory.json')['files']:
        assert digest(ROOT.parent / 'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159' / row['path']) == row['sha256']
    regression_name = 'contract-streak-fifteen-regression.json'
    result = read(ROOT / 'analysis' / regression_name)
    result.update(status='passed_paired_actual_streak15_and_exact_literal_win_note_saved_bytes_only',
        recorder_sha256=digest(__file__), recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reports={published.name: digest(published), observation_name: digest(ROOT / 'analysis' / observation_name)},
        actual_supply_commands=[47, 56], actual_market_transfers=[813, 961], final_money=[11164, 10924],
        actual_contract_win_note_records=[0, 1], full_award17_panel_claimed_new=False,
        pending=['Actual20/40 contract threshold panels and bidirectional checkpoint restores',
                 'Remaining30 native Squall ABI signatures and broader DLL/game/JNG/uninstaller scopes'])
    write(ROOT / 'analysis' / regression_name, result)
    verification = read(ROOT / 'analysis/verification.json')
    for name in [published.name, observation_name, regression_name]:
        if 'analysis/' + name not in verification['reports']:verification['reports'].append('analysis/' + name)
    verification['latest_scoped_regression'] = dict(report='analysis/' + regression_name, recorded_at_utc=result['recorded_at_utc'], whole_game_complete=False)
    paths = [ROOT / 'CMakeLists.txt'] + [p for folder in ['src', 'tools', 'tests'] for p in (ROOT / folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    rows = [p.relative_to(ROOT).as_posix() + '\0' + digest(p) + '\n' for p in sorted(paths, key=lambda p:p.relative_to(ROOT).as_posix())]
    verification['source_tree_sha256'] = hashlib.sha256(''.join(rows).encode('utf8')).hexdigest()
    verification['source_tree_hash_as_of_report'] = 'analysis/' + regression_name
    write(ROOT / 'analysis/verification.json', verification)
    print('Verified actual5 receipts per EXE,47/56 supply commands,813/961 transfers,all16377/16394 save bytes and literal WinNote+750 versus actual880 reward.')


if __name__ == '__main__':main()
