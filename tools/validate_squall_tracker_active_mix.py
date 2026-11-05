#!/usr/bin/env python3
"""Compare actual full active-channel mixer dispatch and loop state."""
import argparse,datetime,json,shutil,subprocess
from pathlib import Path
from validate_squall_tracker_stages import ROOT,digest,symbol
from record_live_production_verification import write,read

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-tracker-active-mix-validation.json')
    parser.add_argument('--allow-differences',action='store_true')
    args=parser.parse_args();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work=ROOT/'.local/squall-tracker-active-mix'/stamp;work.mkdir(parents=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=read(ROOT/'analysis/inventory.json')
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged()
    binaries={'native.dll':original/'Squall.dll','source.dll':args.candidate.resolve(),'source.map':args.candidate.resolve().with_suffix('.map'),
              'probe.exe':ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-active-mix-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    paths={name:ROOT/name for name in ['tools/validate_squall_tracker_active_mix.py','tools/squall_tracker_active_mix_probe.cpp','tools/squall_tracker_probe/CMakeLists.txt']}
    for p in (ROOT/'src/squall').rglob('*'):
        if p.suffix in ['.cpp','.h']:paths[p.relative_to(ROOT).as_posix()]=p
    for name,path in paths.items():
        out=work/'snapshots'/name;out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,out)
    native=['17fe0','759ac','819d4','819d8','819c8'];own=[symbol(work/'source.map',name) for name in ['?CreateStereoMix@CDecompressTrack@@','?MixSoundBuffer@@','?gnDryROfsVol@@','?gnDryLOfsVol@@','?gdwSoundSetup@CDecompressTrack@@']]
    report=dict(status='running_actual_active_channel_mix',work_directory=work.relative_to(ROOT).as_posix(),
        binaries={name:digest(work/name) for name in binaries},source_files={name:digest(path) for name,path in paths.items()},
        native_rvas=native,source_rvas=own,runs=[],whole_squall_complete=False,whole_game_complete=False)
    try:
        states=[];destinations=[];channels=[]
        for backend,rvas in [('native',native),('source',own)]:
            folder=work/backend;folder.mkdir();config=folder/'config.txt';config.write_bytes(('\n'.join(rvas)+'\n').encode('ascii'))
            run=subprocess.run([str(work/'probe.exe'),str(work/f'{backend}.dll'),str(config)],cwd=folder,capture_output=True,timeout=90)
            (folder/'stdout.json').write_bytes(run.stdout);(folder/'stderr.log').write_bytes(run.stderr)
            report['runs'].append(dict(backend=backend,exit_code=run.returncode,stderr=run.stderr.decode('utf-8','replace'),
                directory=folder.relative_to(ROOT).as_posix(),artifacts={p.name:digest(p) for p in folder.iterdir()}))
            write(work/'report.json',report);assert run.returncode==0,report['runs'][-1]
            state=json.loads(run.stdout);assert state['case_count']==8640 and state['guards_intact'] and state['sample_read_only']
            states.append(state);destinations.append((folder/'destinations.bin').read_bytes());channels.append((folder/'channel-scalars.bin').read_bytes())
        assert len(channels[0])==len(channels[1])==26127360 and len(destinations[0])==len(destinations[1])==5944320
        offset=0;channel_offset=0;groups={};different_cases=0
        for i,(left,right) in enumerate(zip(states[0]['cases'],states[1]['cases'])):
            count=left['count'];size=count*8+32;state_size=left['channels']*252
            data_a=destinations[0][offset:offset+size];data_b=destinations[1][offset:offset+size];offset+=size
            channel_a=channels[0][channel_offset:channel_offset+state_size];channel_b=channels[1][channel_offset:channel_offset+state_size];channel_offset+=state_size
            delta=sum(x!=y for x,y in zip(data_a,data_b));scalar_delta=sum(x!=y for x,y in zip(channel_a,channel_b))
            key=tuple(left[n] for n in ['channels','mode','variant','loop']);row=groups.setdefault(key,dict(channels=key[0],mode=key[1],variant=key[2],loop=key[3],case_pairs=0,different_case_pairs=0,different_destination_bytes=0,different_channel_scalar_bytes=0))
            row['case_pairs']+=1;bad=left!=right or delta or scalar_delta
            row['different_case_pairs']+=bool(bad);row['different_destination_bytes']+=delta;row['different_channel_scalar_bytes']+=scalar_delta;different_cases+=bool(bad)
        assert offset==len(destinations[0]) and channel_offset==len(channels[0])
        report.update(status='passed_actual_active_channel_mix' if not different_cases else 'different_actual_active_channel_mix',
            case_pairs=8640,different_case_pairs=different_cases,active_channel_counts=[1,2,33],
            different_destination_bytes=sum(row['different_destination_bytes'] for row in groups.values()),
            different_channel_scalar_bytes=sum(row['different_channel_scalar_bytes'] for row in groups.values()),
            guarded_destination_bytes=5944320,channel_scalar_bytes=26127360,groups=list(groups.values()),
            address_field_excluded='Only per-channel pCurrentSample address fields;actual cleared/retained state is compared for each channel;full input sample remains read-only',
            code_or_IAT_written=False,production_forwarding=False,original_files_unchanged=17,
            pending=['Further active-voice effects and real complete tracker playback','Remaining tracker loader/format branches and whole original goal scopes'])
        for name,path in paths.items():assert digest(path)==report['source_files'][name],name
        unchanged()
    except Exception as error:
        report.update(status='failed',error=repr(error))
        import traceback;report['traceback']=traceback.format_exc()
    write(work/'report.json',report);write(args.output,report)
    print(json.dumps({k:report.get(k) for k in ['status','work_directory','case_pairs','different_case_pairs','different_destination_bytes','different_channel_scalar_bytes','error']}))
    assert report['status']!='failed' and (args.allow_differences or not report['different_case_pairs']),'Actual active mixer observations retained'

if __name__=='__main__':main()
