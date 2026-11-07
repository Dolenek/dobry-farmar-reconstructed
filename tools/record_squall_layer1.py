#!/usr/bin/env python3
"""Record defined Layer-I PCM equality and explicit native packet-size defects."""
import datetime
from record_live_production_verification import ROOT,digest,read,write
from current_production_regression import current_regression
from pe import PE


def main():
    value=read(ROOT/'.local/squall-layer1-validation.json');work=ROOT/value['work_directory']
    assert value['status']=='passed_owned_MPEG_LayerI_defined_frame_prefix_PCM_and_native_metadata_both_builds_only'
    assert len(value['fixtures'])==8 and len(value['cases'])==32 and value['guarded_PCM_reads']==1280
    for name,sha in value['source_files'].items():assert digest(ROOT/name)==sha and digest(work/'snapshots'/name)==sha,name
    for item in value['fixtures']:
        path=work/item['name'];assert digest(path)==item['input_sha256']
        pcm=path.with_suffix('.independent.s16le');assert digest(pcm)==item['independent_PCM_sha256']
        assert pcm.stat().st_size==item['independent_PCM_bytes']==9216*item['channels']*2
        assert bool(any(pcm.read_bytes()))!=item['silent']
    totals={}
    for backend in ['source','fresh']:
        cases=[c for c in value['cases'] if c['backend']==backend];assert len(cases)==16
        for case in cases:
            assert case['exit_code']==0;folder=ROOT/case['directory']
            for name,sha in case['artifacts'].items():assert digest(folder/name)==sha,name
            state=read(folder/'stdout.json');assert state==case['result']
            assert state['layer']==1 and state['guarded_pcm_reads']==40 and state['physical_buffers']==0 and not state['actual_directsound']
        totals[backend]=dict(pcm_bytes=sum(c['result']['pcm_bytes'] for c in cases),
            nonzero_pcm_bytes=sum(c['result']['nonzero_pcm_bytes'] for c in cases),guarded_reads=640)
    assert totals['source']==totals['fresh'] and totals['source']['pcm_bytes']==490560 and totals['source']['nonzero_pcm_bytes']==395452
    failed=ROOT/'.local/squall-layer1/20261107T134719145765Z/report.json';initial=read(failed)
    assert initial['status']=='failed' and 'MPEG defined seek range' in initial['error']
    state_path=ROOT/'.local/squall-layer1-state/stdout.json';metadata=read(state_path)
    assert all(row['different']==0 for row in metadata['fields'])
    m=metadata['mpeg_metadata'];assert all(m[k]['native']==m[k]['source']==expected for k,expected in [('_frames',24),('_samplesInFrame',1152),('_samplesInFile',8064)])
    original=ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159';pe=PE(original/'Squall.dll');witnesses=[]
    for address,data in [(0x10011f90,'b880040000d3f833c983ff010f95c141'),
        (0x10012ac2,'89854ca70000'),(0x10012bca,'89853ca700000fafc8')]:
        expected=bytes.fromhex(data);offset=pe.offset(address-pe.image_base,len(expected));assert pe.data[offset:offset+len(expected)]==expected
        witnesses.append(dict(address=f'{address:08x}',bytes_hex=data))
    full_name,full=current_regression()
    assert [value['binaries'][n+'.dll'] for n in ['source','fresh']]==full['candidate_sha256']
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(original/row['path'])==row['sha256']
    name='analysis/squall-layer1-validation.json';write(ROOT/name,value)
    out='analysis/squall-layer1-regression.json'
    result=dict(status='passed_LayerI_defined384_frame_prefixes_both_builds_preserving_native_metadata_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        report=name,report_sha256=digest(ROOT/name),candidate_sha256=full['candidate_sha256'],
        independent_valid_inputs=8,precision_contexts=['PC53','PC24'],isolated_pair_processes=32,guarded_PCM_reads=1280,
        per_build=totals,actual_frame_prefix_samples=384,native_header_samples=[1152,576],
        native_instruction_witnesses=witnesses,whole_declared_native_packets_claimed_defined=False,
        native_state_evidence=dict(path=state_path.relative_to(ROOT).as_posix(),sha256=digest(state_path),value=metadata),
        initial_failed_broad_seek_assumption=dict(path=failed.relative_to(ROOT).as_posix(),sha256=digest(failed),error=initial['error']),
        production_implementation_changed=False,unchanged_full102_report=full_name,new_full_ctest_run=False,
        original_files_unchanged=17,original_program_code_used_in_implementation=False,whole_squall_complete=False,whole_game_complete=False,
        pending=value['pending'])
    write(ROOT/out,result)
    v=read(ROOT/'analysis/verification.json')
    for n in [name,out]:
        if n not in v['reports']:v['reports'].append(n)
    v['squall_layer1']=dict(report=out,defined_prefix_samples=384,guarded_reads=1280,general_playback_claimed=False)
    write(ROOT/'analysis/verification.json',v)
    print('Recorded8 independently valid LayerI files,32 processes,1280 exact defined reads,and preserved native1152/576 metadata defects.')


if __name__=='__main__':main()
