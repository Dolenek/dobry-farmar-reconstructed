#!/usr/bin/env python3
"""Compare actual group APIs in original/source DLLs with real DirectSound channels."""
import argparse,datetime,hashlib,json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/squall-groups-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-groups-validation.json')
    args=parser.parse_args();candidate=args.candidate.resolve(strict=True);oracle=args.oracle.resolve(strict=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged();candidate_hash=digest(candidate);contexts=[]
    for precision,output in run_precision_cases(oracle,[original/'Squall.dll',candidate,original/'Data'],timeout=40):
        row=json.loads(output);assert row['export_lookups']==194 and row['api_comparisons']==420
        assert row['empty_group_calls']==288 and row['prepared_channels']==3 and row['group_ids']==[17,-7]
        assert row['real_directsound'] and row['original_entry_point_executed'] and not row['audible_output_compared']
        row['precision']=precision;contexts.append(row);unchanged();assert digest(candidate)==candidate_hash
    paths=[p for p in (ROOT/'src/squall').rglob('*') if p.is_file()]+[ROOT/n for n in ['tools/squall_groups_reference_test.cpp','tools/validate_squall_groups.py','tools/squall_exports.inc','tools/oracle_precision.py','tools/cmake/SquallCandidate.cmake']]
    report=dict(status='passed_for_loaded_DLL_group_operations_on_real_ambient_channels_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=digest(original/'Squall.dll'),
        rebuilt_sha256=candidate_hash,oracle_sha256=digest(oracle),candidate_path=candidate.relative_to(ROOT).as_posix(),oracle_path=oracle.relative_to(ROOT).as_posix(),
        source_files={p.relative_to(ROOT).as_posix():digest(p) for p in sorted(paths)},precision_contexts=contexts,
        original_entry_point_executed=True,reference_files_unchanged=17,original_program_code_used_in_implementation=False,
        api_comparisons=sum(c['api_comparisons'] for c in contexts),
        original_functions=['10007100','10007190','10007220','100072c0','10002260','10001f40','10001f80','10001fe0'],
        coverage=['Four public group APIs before Init, initialized empty, retired empty and after Free',
            'Two actual ambient channels in group17, a third in group-7; unrelated group fields/status preserved',
            'Group volume table indices0,1,43,99,100,101,255 and actual public volume getters',
            'Group frequency boundaries-1,0,99,100,22050,44100,88200,100000,100001,INT_MAX and all three physical getters',
            'Prepared buffers retain status3 under group pause; actual Channel_Start and individual pause enter status2',
            'Actual group resume/pause with0,1,-1,2; public status verifies live/paused channels and group separation',
            'Both actual workers retire stopped group channels; only original transient-24 and terminal0 allowed for2s',
            'Negative/large group IDs and non-indexed invalid volumes when no matching live channel exists'],
        test_adapters=['Only public GetProcAddress calls, actual OS DirectSound buffers/workers and recovered read-only file callbacks',
            'Original reference loaded normally with its DLL entrypoint; no original code linked into implementation',
            'Asynchronous retirement compared at terminal state, not by matching scheduler epochs'],
        pending=['Other channel types and group changes concurrent with worker callbacks/device teardown',
            'Original unchecked volume-table reads outside0..255 with matching live channels',
            'Audible output and sample-accurate playback timing, other devices and unsupported backend conditions',
            'All remaining unverified signatures/behavior across97 Squall exports and full-game scope'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Squall groups:840 public results in PC53/PC24, real ambient channels and workers;17 references unchanged')
if __name__=='__main__':main()
