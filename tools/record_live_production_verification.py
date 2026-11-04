#!/usr/bin/env python3
"""Record actual paid production, crash diagnosis and complete live-house transfers."""
import datetime,hashlib,json,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text(encoding='utf-8'))
def write(path,value):
    # Keep recorded JSON bytes identical to the repository's explicit LF policy.
    # Windows text-mode CRLF translation otherwise invalidates cross-report SHA256s.
    Path(path).write_bytes((json.dumps(value,ensure_ascii=False,indent=2)+'\n').encode('utf-8'))
def checked(report):
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    for name,value in report['implementation_files'].items():
        assert digest(ROOT/'build/windows/Release'/('Dobrý farmář.exe' if name=='FarmarSourceInterop.exe' else name))==value,name
    return work
def main():
    paid=read(ROOT/'.local/paid-production-state-validation.json');checked(paid)
    assert paid['status']=='passed_for_actual_paid_cowshed_grass_harvest_and_two_milk_cycles_only'
    assert paid['comparison']['identical_complete_save_bytes']==3730 and not paid['comparison']['normalizations']
    for name,value in paid['audit_files'].items():assert digest(ROOT/name)==value,name
    interop=read(ROOT/'.local/production-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_paid_live_cowshed_and_milk_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/paid-production-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for session in interop['sessions']:assert session['exit_code']==0 and session['status']=='completed'
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==3730 and not row['normalizations']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    initial=read(ROOT/'.local/production-interop/20261029T210306249364Z/report.json')
    diagnosis=read(ROOT/'.local/production-interop/20261029T210928922761Z/report.json')
    trace_path=ROOT/diagnosis['work_directory']/'2-crash-trace.jsonl'
    trace=[json.loads(line) for line in trace_path.read_text(encoding='utf-8').splitlines()]
    crashes=[row for row in trace if row['event']=='access_violation'];assert len(crashes)==2
    assert crashes[0]['ecx']==0 and crashes[0]['access_address']==1524 and trace[-1]['exit_code']==3221225477
    failure=dict(status='source_live_house_restore_crash_reproduced_then_repaired',
        initial_runtime_report_sha256=digest(ROOT/'.local/production-interop/20261029T210306249364Z/report.json'),
        diagnostic_runtime_report_sha256=digest(ROOT/diagnosis['work_directory']/'report.json'),
        failed_implementation_files=initial['implementation_files'],trace_sha256=digest(trace_path),trace=trace,
        resolution=dict(original_constructor='00413210 restores Manufacture houses before Warehouse',
            original_goods='00467fa0 indexes UInt WGoodCount directly in profile map0065ff44; no Warehouse receiver',
            observed_source_chain=['Warehouse::goods(null)','ProductionBuilding::setProductionCount','ProductionBuilding::refresh','Upgrade::setLevel','ProductionBuilding::loadXML','Manufacture::Manufacture','Game::World::construct'],
            instruction_evidence_sha256=digest(ROOT/'.local/warehouse-goods-native-disassembly.txt'),
            change='World::warehouseGoods reads bounded profile WGoodCount directly; original construction order retained',
            debuggee_memory_written=False),whole_game_complete=False)
    write(ROOT/'analysis/production-restore-crash.json',failure)
    write(ROOT/'analysis/paid-production-state-validation.json',paid)
    write(ROOT/'analysis/production-interop-validation.json',interop)
    junit=ROOT/'.local/live-production-ctest.xml';suite=ET.parse(junit).getroot();cases=suite.findall('.//testcase')
    assert len(cases)==102 and not any(suite.findall('.//'+tag) for tag in ['failure','error','skipped'])
    names=subprocess.check_output(['git','ls-files','src/game','src/squall'],cwd=ROOT,text=True).splitlines()
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_paid_cowshed_two_milk_cycles_live_restore_and_complete_current_102_regression',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        base_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        production_source_files={name:digest(ROOT/name) for name in names},
        implementation_files=paid['implementation_files'],reference_files_unchanged=17,
        reports={name:digest(ROOT/'analysis'/name) for name in ['paid-production-state-validation.json','production-interop-validation.json','production-restore-crash.json']},
        tests_passed=102,tests_failed=0,sum_test_seconds=sum(float(case.get('time','0')) for case in cases),
        junit_sha256=digest(junit),ctest_log_sha256=digest(ROOT/'.local/live-production-ctest.log'),
        build_logs={name:digest(ROOT/'.local'/name) for name in ['live-production-build.log','live-production-fresh-build.log']},
        source_debugger_sha256=digest(ROOT/'build/source-crash-trace-vs/Release/source-crash-trace.exe'),
        restored_source_process_traces=traces,whole_game_complete=False,
        pending=['Remaining10 production kinds in complete EXEs','Save and resume while production is in flight','Remaining whole-game, full Squall, JNG and uninstaller scopes'])
    write(ROOT/'analysis/live-production-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(result['reports'])+['live-production-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/live-production-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    verification['tests'].update(ctest_passed=102,last_full_ctest_report='analysis/live-production-regression.json',full_ctest_repeated_for_latest_sample_fix=True)
    verification['tests'].pop('last_full_ctest_implementation_commit',None)
    for name in ['paid-production-state-validation.json','production-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded actual paid cowshed, two milk cycles, four exact live-house transfers and current102/102; whole goal remains incomplete')
if __name__=='__main__':main()
