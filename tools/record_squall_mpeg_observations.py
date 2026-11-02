#!/usr/bin/env python3
"""Preserve the first failing MPEG comparison without relabeling it a pass."""
import datetime,json,struct
from record_live_production_verification import ROOT,digest,read,write

def main():
    report=read(ROOT/'.local/squall-mpeg-formats-validation.json');work=ROOT/report['work_directory']
    assert report['status']=='found_actual_mpeg_decoder_differences_or_failures' and report['failed_cases']==14
    for name,value in report['source_files'].items():
        snapshot=work/'source-snapshots'/name
        assert digest(snapshot if snapshot.exists() else ROOT/name)==value,name
    assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256']
    assert digest(ROOT/report['oracle_path'])==report['oracle_sha256']
    for name,value in report['fixture_files'].items():assert digest(work/name)==value,name
    for row in report['cases']:
        folder=ROOT/row['directory']
        for name,value in row['artifacts'].items():assert digest(folder/name)==value,name
        if (folder/'failed-source-pcm.bin').exists():
            a=(folder/'failed-source-pcm.bin').read_bytes();b=(folder/'failed-native-pcm.bin').read_bytes();assert len(a)==len(b)
            words=[(struct.unpack_from('<h',a,i)[0]-struct.unpack_from('<h',b,i)[0]) for i in range(0,len(a),2)]
            row['pcm_observation']=dict(bytes=len(a),different_bytes=sum(x!=y for x,y in zip(a,b)),different_words=sum(x!=0 for x in words),delta_counts={str(x):words.count(x) for x in sorted(set(words)-{0})},first_byte=next(i for i,(x,y) in enumerate(zip(a,b)) if x!=y))
    report['state_observations']=[]
    for pc,index in [('PC53',0),('PC53',3),('PC24',0),('PC24',3)]:
        folder=work/pc/str(index)/'state-probe';j=read(folder/'stdout.json');assert j['decoder_bytes']==47744
        report['state_observations'].append(dict(precision=pc,fixture_index=index,state=j,artifacts={p.relative_to(work).as_posix():digest(p) for p in folder.iterdir() if p.is_file()}))
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(observation_recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),state_probe_sources={name:digest(ROOT/name) for name in ['tools/squall_mpeg_state_probe.cpp','tools/squall_mpeg_formats_probe/CMakeLists.txt']},
        native_allocation_instruction='1002db04 PUSH 0xba80 before constructor10012960',fixture_encoder_used_only_to_encode_owned_integer_pcm=True,numerical_tolerance_used=False,
        observations=['13 processes stopped at differing PCM words; one stopped at differing final seek read byte count','PC24 MP2 44100 mono and stereo completed all40 guarded reads and four physical buffers each','Initial table differences are evidence only; cause and decoded differences remain unresolved'],
        production_changed=False,whole_game_complete=False,whole_squall_complete=False)
    write(ROOT/'analysis/squall-mpeg-initial-observations.json',report)
    print('Recorded14 MPEG failures and two scoped passes; production unchanged.')
if __name__=='__main__':main()
