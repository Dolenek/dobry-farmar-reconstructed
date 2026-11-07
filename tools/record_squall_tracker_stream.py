#!/usr/bin/env python3
"""Retain exact sequential observations without calling silent native output active playback."""
import datetime
from record_live_production_verification import ROOT,digest,read,write


def main():
    value=read(ROOT/'.local/squall-tracker-stream-validation.json');work=ROOT/value['work_directory']
    assert value['status']=='passed_actual_sequential_tracker_PCM_on_owned_constructor_storage_only'
    assert len(value['cases'])==12 and value['isolated_processes']==36 and not value['deviations']
    for name,sha in value['source_files'].items():assert digest(ROOT/name)==sha and digest(work/'snapshots'/name)==sha,name
    total_bytes=0
    for case in value['cases']:
        buffers=[]
        for backend,outcome in case['outcomes'].items():
            folder=ROOT/outcome['directory'];assert outcome['exit_code']==0
            for name,sha in outcome['artifacts'].items():assert digest(folder/name)==sha
            state=outcome['state'];assert state['blocks']==128 and state['wide_guards_intact']
            # These are the observed original results, not a desired mixer model.
            assert state['samples']==7056000 and state['nonzero_destination_bytes']==0 and state['block_returns']==[0]*128
            buffers.append((folder/'decoded-pcm.bin').read_bytes())
        assert buffers[0]==buffers[1]==buffers[2];total_bytes+=len(buffers[0])
    failed=ROOT/'.local/squall-tracker-stream/20261107T085843365597Z/report.json';first=read(failed)
    assert first['status']=='failed' and first['error']=='AssertionError()'
    native=read(failed.parent/'0-pc53-slot1-native/stdout.json')
    assert native['samples']==0 and native['nonzero_destination_bytes']==0 and len(native['block_returns'])==128
    assert set(native['block_returns'])=={0,4096}
    name='analysis/squall-tracker-stream-validation.json';write(ROOT/name,value)
    result=dict(status='passed_exact_silent_native_sequential_constructor_observations_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        report=name,report_sha256=digest(ROOT/name),isolated_processes=36,blocks_per_process=128,
        exact_guarded_output_bytes_per_candidate=total_bytes,actual_nonzero_PCM_bytes=0,actual_returned_frames_for_five_orders=0,
        requested_precision_reapplied_after_both_DLL_loads=True,
        historical_constructor_probe_precision_limit='Older ctor probe set PC before LoadLibrary;native startup could replace PC24 with PC53. This new probe reapplies and measures PC after both loads and constructor.',
        native_factory_used=False,code_or_IAT_written=False,active_tracker_playback_proved=False,
        initial_failed_nonzero_assumption=dict(report=failed.relative_to(ROOT).as_posix(),sha256=digest(failed),native_observation=native),
        interpretation='Original five-order constructor reports7056000 samples but sequential reads return0 and silence; both sources preserve it. Original one-order constructor reports0 total samples and initially returns4096-frame silent blocks. This does not replace the separately verified active mixer kernels.',
        production_implementation_changed=False,whole_game_complete=False,
        pending=['Original unsafe factory/title behavior and broader tracker formats remain explicitly bounded','Continue actual40-contract threshold and original whole-project checks'])
    out='analysis/squall-tracker-stream-observations.json';write(ROOT/out,result)
    verification=read(ROOT/'analysis/verification.json')
    for n in [name,out]:
        if n not in verification['reports']:verification['reports'].append(n)
    verification['squall_tracker_sequential_observations']=dict(report=out,active_playback_proved=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded36 normal sequential processes and exact silent native observations; no active-playback claim.')


if __name__=='__main__':main()
