#!/usr/bin/env python3
"""Record a complete regression for the committed focused-View repair."""
import datetime,hashlib,json,subprocess,xml.etree.ElementTree as ET
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text(encoding='utf-8'))
def write(path,value):Path(path).write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def main():
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip();assert head.startswith('8599644'),head
    tracked=subprocess.check_output(['git','ls-files','src/game','src/squall'],cwd=ROOT,text=True).splitlines()
    # The checkout uses both LF and configured CRLF files. Let Git apply its
    # recorded filters; raw current file hashes remain in the runtime manifests.
    subprocess.run(['git','diff','--exit-code',head,'--','src/game','src/squall'],cwd=ROOT,check=True,capture_output=True)
    path=ROOT/'.local/focused-upgrade-ctest.xml';suite=ET.parse(path).getroot();cases=suite.findall('.//testcase')
    assert len(cases)==102 and not any(suite.findall('.//'+name) for name in ['failure','error','skipped'])
    required=next(c for c in cases if c.get('name')=='original-upgrade-state');assert float(required.get('time','0'))>0
    upgrade=read(ROOT/'.local/upgrade-validation.json');assert upgrade['focused_view_alias_states']==6144
    for name,value in upgrade['source_files'].items():assert digest(ROOT/name)==value,name
    runtime=read(ROOT/'analysis/cucumber-sowing-state-validation.json')
    for name,value in runtime['implementation_files'].items():
        filename='Dobrý farmář.exe' if name=='FarmarSourceInterop.exe' else name
        assert digest(ROOT/'build/windows/Release'/filename)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    log=ROOT/'.local/focused-upgrade-ctest.log';interrupted=ROOT/'.local/focused-upgrade-ctest-interrupted.log'
    result=dict(status='passed_complete_102_check_run_after_focused_View_repair',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        implementation_commit=head,recorder_sha256=digest(__file__),tests_passed=102,tests_failed=0,
        production_source_files={name:digest(ROOT/name) for name in tracked},
        sum_test_seconds=sum(float(c.get('time','0')) for c in cases),junit_sha256=digest(path),log_sha256=digest(log),
        prior_interrupted_run=dict(log_sha256=digest(interrupted),completed=False,recorded_as_pass=False),
        implementation_files=runtime['implementation_files'],upgrade_alias_states=6144,original_files_unchanged=17,whole_game_complete=False)
    write(ROOT/'analysis/focused-view-upgrade-full-regression.json',result)
    verification=read(ROOT/'analysis/verification.json');verification['tests']['ctest_passed']=102
    verification['tests']['last_full_ctest_report']='analysis/focused-view-upgrade-full-regression.json'
    verification['tests']['last_full_ctest_implementation_commit']=head
    verification['tests']['full_ctest_repeated_for_latest_game_alias_fix']=True
    verification['latest_scoped_regression']=dict(report='analysis/focused-view-upgrade-full-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    if 'analysis/focused-view-upgrade-full-regression.json' not in verification['reports']:verification['reports'].append('analysis/focused-view-upgrade-full-regression.json')
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded complete102/102 regression for8599644; prior interrupted run remains separate, entire reconstruction incomplete')
if __name__=='__main__':main()
