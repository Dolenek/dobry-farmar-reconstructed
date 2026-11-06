"""Resolve current full regression without silently reusing obsolete source hashes."""
from record_live_production_verification import ROOT, digest, read


def current_regression():
    name=read(ROOT/'analysis/verification.json')['tests']['last_full_ctest_report']
    report=read(ROOT/name)
    assert report['tests_passed']==102 and report['tests_failed']==0
    assert report['production_source_files']
    for path,sha in report['production_source_files'].items():assert digest(ROOT/path)==sha,path
    return name,report
