#!/usr/bin/env python3
"""Reference independently completed genuine checkpoints without merging or editing profiles."""
import argparse
import datetime
from pathlib import Path
import xml.etree.ElementTree as ET
from record_live_production_verification import ROOT,digest,read,write
from audit_contract_streak_state import audit


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-report',type=Path,required=True)
    parser.add_argument('--source-report',type=Path,required=True)
    parser.add_argument('--streak',type=int,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();assert 0<args.streak<=40
    sets=[]
    for kind,path in [('native',args.native_report),('source',args.source_report)]:
        path=path.resolve(strict=True);value,branches=audit(path,allow_retained_failure=True)
        index,session=next((i,s) for i,s in enumerate(value['sessions']) if s['kind']==kind)
        branch=next(b for b in branches if b['kind']==kind)
        assert ((session.get('status')=='completed' and session['exit_code']==0) or
            (branch.get('completed_target_saved_after_driver_deadline') and session['failure_cleanup_exit_code']==0))
        assert branch['final_streak']==args.streak and branch['requested_target_reached']
        directory=ROOT/branch['persisted_profile_directory']
        props={n.get('Name'):n.get('Val') for n in ET.parse(directory/'00000001.xml').getroot()}
        assert int(props['awQTask'])==args.streak and all(props[f'task{i}']=='-1' for i in range(3))
        if args.streak>=20:assert props['Award04Make20Task']==props['awardShow4']=='1'
        if args.streak>=40:assert props['Award05Make40Task']==props['awardShow5']=='1'
        assert digest(directory/'00000001.save')==branch['complete_saved_sha256']
        sets.append(dict(kind=kind,profile_directory=directory.relative_to(ROOT).as_posix(),
            files={p.name:digest(p) for p in directory.iterdir() if p.is_file()},
            source_session_index=index,source_report=path.relative_to(ROOT).as_posix(),source_report_sha256=digest(path),
            original_driver_status=value['status'],independently_audited_target_saved_after_driver_deadline=branch.get('completed_target_saved_after_driver_deadline',False),
            streak=args.streak,money=int(props['Money']),complete_expected_saved_bytes=branch['complete_expected_saved_bytes']))
    result=dict(status='verified_actual_contract_streak_checkpoint_profiles',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        work_directory='.',requested_target=args.streak,profile_sets=sets,profile_or_counter_fields_written=False,
        native_and_source_random_streams_claimed_matched=False,normalizations=[],whole_game_complete=False)
    assert not args.output.exists(),'Checkpoint descriptors are immutable once published'
    write(args.output,result);print('Referenced2 independently completed original/source checkpoints;all profile bytes preserved.')


if __name__=='__main__':main()
