#!/usr/bin/env python3
"""Record completed real-DLL group comparisons without claiming complete export equivalence."""
import hashlib,json
from pathlib import Path
from pe import PE
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def write(path,value):path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def main():
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    for row in inventory['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    reports=[]
    for name in ['squall-groups-validation','squall-groups-fresh-validation']:
        report=json.loads((ROOT/'.local'/f'{name}.json').read_text(encoding='utf-8'))
        assert report['status']=='passed_for_loaded_DLL_group_operations_on_real_ambient_channels_only' and report['api_comparisons']==840
        assert len(report['precision_contexts'])==2 and report['reference_files_unchanged']==17
        assert digest(ROOT/report['candidate_path'])==report['rebuilt_sha256']
        assert digest(ROOT/report['oracle_path'])==report['oracle_sha256']
        for path,value in report['source_files'].items():assert digest(ROOT/path)==value,path
        write(ROOT/'analysis'/f'{name}.json',report);reports.append(report)
    normalized=[];timestamp_ranges=[]
    for report in reports:
        pe=PE(ROOT/report['candidate_path']);ranges=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]]
        assert pe.directory(6)[1]==28 and pe.u32(ranges[0][0])==pe.u32(ranges[1][0]),'PE/debug linker timestamps differ'
        raw=bytearray(pe.data)
        for begin,end in ranges:raw[begin:end]=b'\0'*(end-begin)
        normalized.append(raw);timestamp_ranges.append(ranges)
    assert timestamp_ranges[0]==timestamp_ranges[1] and normalized[0]==normalized[1], 'Fresh DLL differs outside its two linker timestamps'
    write(ROOT/'analysis/squall-groups-build-comparison.json',dict(status='passed_identical_except_two_linker_timestamps',
        raw_sha256=[r['rebuilt_sha256'] for r in reports],normalized_byte_ranges=timestamp_ranges[0],
        bytes=len(normalized[0]),normalized_sha256=hashlib.sha256(normalized[0]).hexdigest(),
        files_modified=False))
    audit=json.loads((ROOT/'analysis/squall-export-audit.json').read_text(encoding='utf-8'))
    assert audit['rebuilt_sha256']==reports[0]['rebuilt_sha256']
    scopes={
        'SQUALL_ChannelGroup_Pause':'Real groups17/-7; prepared no-op; actual start/pause/resume with0,1,-1,2; unrelated group preserved; empty contexts before/after Init/Free',
        'SQUALL_ChannelGroup_Stop':'Real groups17/-7, unrelated group preserved, bounded original-24/0 asynchronous retirement and empty contexts',
        'SQUALL_ChannelGroup_SetVolume':'Actual groups and public getters for table indices0,1,43,99,100,101,255; invalid volumes only with no live matching channels',
        'SQUALL_ChannelGroup_SetFrequency':'Actual groups and public getters across-1,0,99,100,22050,44100,88200,100000,100001,INT_MAX; unrelated group preserved',
        'SQUALL_Channel_Start':'Three actual prepared ambient buffers per precision transition from public status3 into playback before individual pause',
        'SQUALL_Channel_Pause':'Three actual started ambient buffers per precision enter paused public status2 before group operations',
        'SQUALL_Channel_Status':'Actual prepared3, live1, paused2, transient stop-24 and retired0 in group scenarios'}
    for row in audit['exports']:
        if row['name'] in scopes:
            row['status']='partially_compared';row['loaded_DLL_group_verified_scope']=scopes[row['name']]
            assert not row['native_signature_fully_verified']
    audit['group_operation_reports']=['analysis/squall-groups-validation.json','analysis/squall-groups-fresh-validation.json']
    write(ROOT/'analysis/squall-export-audit.json',audit)
    print('Recorded two builds/PC53/PC24 group comparisons; all97 full-signature/behavior audit remains incomplete')
if __name__=='__main__':main()
