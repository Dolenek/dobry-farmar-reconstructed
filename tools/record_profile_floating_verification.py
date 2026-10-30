#!/usr/bin/env python3
"""Record actual CRT numeric comparison, independent replay and whole-game warehouse regression."""
import datetime,json,platform,re,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write,checked

def main():
    reports={};replays=[]
    fresh=ROOT/'build/squall-source-repro-vs20261010/Release/profile-floating-probe.exe'
    for precision in ['PC24','PC53']:
        name=f'profile-floating-{precision}-validation.json'
        report=read(ROOT/'.local'/f'profile-floating-final-{precision}-validation.json')
        assert report['status']=='observed_actual_native_profile_floating_formats'
        assert report['native_precision']==precision and len(report['cases'])==4324
        assert report['diagnostic_native_fp_context_changed']==(precision=='PC53')
        assert not report['original_program_code_used_in_implementation'] and report['reference_files_unchanged']==17
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        assert digest(ROOT/'build/windows/Release/profile-floating-probe.exe')==report['oracle_sha256']
        work=ROOT/report['work_directory'];assert digest(work/'fixture.tsv')==report['fixture_sha256']
        assert digest(work/'parser-trace.jsonl')==report['parser_trace_sha256']
        rows=report['precision_contexts'];assert len(rows)==1;row=rows[0]
        assert all(row[key]==0 for key in ['same_seed_parse_differences','same_seed_format_differences','assigned_parse_differences','raw_format_differences'])
        assert row['parse_differences']==60 # Context-dependent retained addresses/stack words, explicitly unequal.
        prefix=work/'independent-replay'
        subprocess.run([str(fresh),str(work/'fixture.tsv'),str(prefix),*(['pc24'] if precision=='PC24' else [])],check=True,timeout=20)
        words={parts[0]:parts[3] for parts in (line.split('\t') for line in prefix.with_suffix('.tsv').read_text(encoding='ascii').splitlines())}
        text=Path(str(prefix)+'.seeded.xml')
        values=dict(re.findall(r'<prop\s+Name="([^"]*)"\s+Val="([^"]*)"',text.read_text(encoding='utf-16')))
        for item in row['rows']:
            assert words[item['key']]==item['native_bits'],item
            assert values[item['key']]==item['native_text'],item
        replays.append(dict(precision=precision,cases=len(words),same_seed_bit_differences=0,format_differences=0,
            executable_sha256=digest(fresh),tsv_sha256=digest(prefix.with_suffix('.tsv')),
            xml_sha256=digest(text),native_report_sha256=digest(ROOT/'.local'/f'profile-floating-final-{precision}-validation.json')))
        write(ROOT/'analysis'/name,report);reports[name]=digest(ROOT/'analysis'/name)
    warehouse=read(ROOT/'.local/full-warehouse-state-validation.json');checked(warehouse)
    assert warehouse['status']=='passed_for_actual_full_warehouse_blocking_market_release_and_milk_completion_only'
    assert warehouse['comparison']['complete_identical_save_bytes']==4089 and not warehouse['comparison']['normalizations']
    assert warehouse['auditor_sha256']==digest(ROOT/'tools/validate_full_warehouse_state.py')
    write(ROOT/'analysis/full-warehouse-state-validation.json',warehouse);reports['full-warehouse-state-validation.json']=digest(ROOT/'analysis/full-warehouse-state-validation.json')
    interop=read(ROOT/'.local/full-warehouse-interop-validation.json');interop_work=checked(interop)
    assert interop['status']=='passed_for_full_warehouse_market_infinity_clock_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/full-warehouse-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    assert all(row['identical_complete_save_bytes']==4089 and not row['normalizations'] for row in interop['comparison']['transfer_comparisons'])
    for row in interop['sessions']:assert row['exit_code']==0 and row['clock_after_restore']['recomputed']=='inf'
    traces={}
    for path in interop_work.glob('*-crash-trace.jsonl'):
        events=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert events[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in events)
        traces[path.name]=dict(sha256=digest(path),events=events)
    assert len(traces)==2
    write(ROOT/'analysis/full-warehouse-interop-validation.json',interop);reports['full-warehouse-interop-validation.json']=digest(ROOT/'analysis/full-warehouse-interop-validation.json')
    suite=ET.parse(ROOT/'.local/profile-floating-ctest.xml').getroot();cases=suite.findall('.//testcase')
    assert len(cases)==102 and not any(suite.findall('.//'+tag) for tag in ['failure','error','skipped'])
    before=read(ROOT/'.local/profile-floating-before-validation.json')
    assert len(before['cases'])==252 and all(item['format_differences']==151 and item['parse_differences']==26 for item in before['precision_contexts'])
    initial=dict(status='actual_original_streams_exposed_modern_conversion_and_output_differences',
        report_sha256=digest(ROOT/'.local/profile-floating-before-validation.json'),work_directory=before['work_directory'],
        initial_format_differences=151,initial_parse_differences=26,initial_cases=252,
        original_instruction_evidence={name:digest(ROOT/'.local'/name) for name in ['profile-floating-setters-native.txt','profile-floating-gather-native.txt','profile-floating-scale-native.txt']},
        additional_probes=[],earlier_whole_warehouse_probes=[],whole_game_complete=False)
    failed_interop=ROOT/'.local/full-warehouse-interop/20261030T173150680649Z'
    failed_report=read(failed_interop/'report.json');assert failed_report['status']=='failed'
    failed_trace=[json.loads(line) for line in (failed_interop/'2-crash-trace.jsonl').read_text(encoding='utf-8').splitlines()]
    assert failed_trace[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in failed_trace)
    initial['initial_interop_timeout']=dict(report_sha256=digest(failed_interop/'report.json'),error=failed_report['error'],
        trace=failed_trace,menu_screenshot_sha256=digest(failed_interop/'2-03-menu.png'),recorded_as_pass=False,
        resolution='Visible menu and normal source exit without AV;20s readiness budget also included debugger attach. Separate60s timer after attach passes all four retries. Exact initial delay cause unproved.')
    for name in ['profile-floating-parser-extended-validation.json','profile-floating-final-validation.json']:
        probe=read(ROOT/'.local'/name);payload={item['key']:item['payload'] for item in probe['cases']}
        differences=[dict(item,payload=payload[item['key']]) for item in probe['precision_contexts'][0]['rows'] if item['native_bits']!=item['same_seed_source_bits']]
        initial['additional_probes'].append(dict(report=name,sha256=digest(ROOT/'.local'/name),status=probe['status'],work_directory=probe['work_directory'],
            same_seed_differences=differences,recorded_as_pass=False))
    for stamp,reason in [('20261030T154355630507Z','Observer caught a render-tree rotation; no source game crash proved'),
        ('20261030T154806396593Z','Both games completed; audit exposed inf spelling and a945 timed event cutoff'),
        ('20261030T161244475412Z','Transient loaded-count observation missed after a slow screenshot'),
        ('20261030T170621309166Z','Both games completed; independent970/977 clocks straddled field18 water exhaustion')]:
        path=ROOT/'.local/full-warehouse-production'/stamp/'report.json';probe=read(path)
        initial['earlier_whole_warehouse_probes'].append(dict(work_directory=probe['work_directory'],runtime_status=probe['status'],error=probe.get('error'),
            report_sha256=digest(path),reason=reason,recorded_as_current_full_audit_pass=False))
    write(ROOT/'analysis/profile-floating-initial-probe.json',initial);reports['profile-floating-initial-probe.json']=digest(ROOT/'analysis/profile-floating-initial-probe.json')
    for item in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/item['path'])==item['sha256']
    source={path.relative_to(ROOT).as_posix():digest(path) for folder in ['src/game','src/squall'] for path in sorted((ROOT/folder).rglob('*')) if path.is_file()}
    result=dict(status='passed_actual_profile_numeric_crt_and_full_warehouse_current_regression',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        base_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        production_source_files=source,reports=reports,independent_numeric_replays=replays,source_restore_debugger_traces=traces,
        implementation_files=warehouse['implementation_files'],tests_passed=102,tests_failed=0,
        sum_test_seconds=sum(float(case.get('time','0')) for case in cases),
        junit_sha256=digest(ROOT/'.local/profile-floating-ctest.xml'),ctest_log_sha256=digest(ROOT/'.local/profile-floating-ctest.log'),
        build_logs={name:digest(ROOT/'.local'/name) for name in ['profile-floating-full-build.log','profile-floating-fresh-build.log']},
        windows_system_crt_dependency_verified_only_on_recorded_system=True,original_program_code_used_in_implementation=False,
        validation_system=platform.platform(),
        original_files_unchanged=17,whole_game_complete=False,
        pending=['Other numeric types and non-C locales, malformed XML and exception paths','Maximum factory capacities, simultaneous factories and further natural awards','Remaining game, Squall97 exports, JNG and uninstaller scopes'])
    write(ROOT/'analysis/profile-floating-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['profile-floating-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['full-warehouse-state-validation.json','full-warehouse-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:
            verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/profile-floating-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests'].update(ctest_passed=102,last_full_ctest_report='analysis/profile-floating-regression.json')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded actual numeric contexts, both independent replays, full warehouse4089 bytes and102/102; complete goal remains in progress')

if __name__=='__main__':main()
