#!/usr/bin/env python3
"""Audit actual market price controls and exact original restore-time sale replay."""
from record_live_production_verification import ROOT, digest, read, write, checked
from validate_counter_supply_state import props
from counter_save_audit import exact_restored_binary, replay_sales
from game_save_state import load_save


def main():
    report = read(ROOT/'.local/counter-quiescence-validation.json')
    work = checked(report)
    assert report['status'] == 'explored_actual_counter_outputs_price99_quiescence_only'
    fixture_path = ROOT/report['fixture_report']['path']
    assert digest(fixture_path) == report['fixture_report']['sha256']
    fixture = read(fixture_path)
    assert fixture['status'] == 'passed_actual_native_source_thousand_counter_supply_only'
    baseline = ROOT/fixture['work_directory']
    branches = []
    for index, session in enumerate(report['sessions']):
        assert session['status'] == 'completed' and session['exit_code'] == 0
        before = baseline/f'{index}-persisted/Profiles'
        after = work/f'{index}-persisted/Profiles'
        for name, sha in session['seed_profiles_sha256'].items():
            assert digest(before/name) == sha
        left, right = [props(p/'00000001.xml') for p in [before, after]]
        assert set(left) == set(right)
        assert all(left[k][0] == right[k][0] for k in left)
        clicks = session['actual_price_clicks']
        price = 2
        for increment, click in enumerate(clicks, 1):
            assert click['before'] == price and click['after'] == min(price+increment, 99)
            price = click['after']
            assert click['money'] == int(left['Money'][1])
            assert click['market_milk'] == int(left['MGoodCount10'][1])
        assert price == 99 and len(clicks) == 14
        assert left['MGoodPrice10'] == ('UInt', '2') and right['MGoodPrice10'] == ('UInt', '99')
        assert int(right['marketIt'][1]) == int(left['marketIt'][1])+1
        assert right['MSellTime10'] == ('Float', '1.#INF' if index == 0 else '-1')
        assert session['stable_price99_observations'] == [[int(left['Money'][1]), int(left['MGoodCount10'][1])]]*2
        varying = {'GameTime', 'RunTime', 'letTime0', 'letTime1', 'letTime2', 'MGoodPrice10', 'marketIt', 'MSellTime10'}
        varying |= {f'{prefix}{n}' for prefix in ['FFP', 'FFW', 'FFF'] for n in range(60)}
        changed = {k: [left[k], right[k]] for k in left if left[k] != right[k]}
        assert set(changed) <= varying, changed
        for name in ['GameTime', 'RunTime']:
            assert 0 < float(right[name][1])-float(left[name][1]) < 15
        binary = exact_restored_binary(before/'00000001.save', after/'00000001.save')
        saved = load_save(after/'00000001.save')
        assert replay_sales(saved['sales']) == saved['sales']
        assert binary['journal_records'] == 71 and binary['coefficient_words'] == 383
        branches.append(dict(kind=session['kind'], total_price_increase=97, actual_accelerated_click_pairs=len(clicks),
                             changed_properties=changed, binary=binary,
                             subsequent_restore_sale_replay_is_identity=True,
                             output_save_sha256=digest(after/'00000001.save'), normalizations=[]))
    report.update(status='passed_actual_price99_quiescence_and_exact_original_sale_replay_only',
                  auditor_sha256=digest(__file__),
                  audit_files={name: digest(ROOT/name) for name in ['tools/counter_save_audit.py', 'tools/validate_counter_supply_state.py']},
                  comparison=dict(branches=branches, original_profile_fields_modified_by_probe=False,
                                  subsequent_saves_expected_byte_identical=True, normalizations=[]))
    write(ROOT/'.local/counter-quiescence-state-validation.json', report)
    print(report['status'])
    print([(b['kind'], b['binary']) for b in branches])


if __name__ == '__main__':
    main()
