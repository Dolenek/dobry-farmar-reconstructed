#!/usr/bin/env python3
"""Compare every original general/fast active-voice kernel using owned data."""
import argparse,datetime,json,shutil,subprocess
from pathlib import Path
from validate_squall_tracker_stages import ROOT,digest,symbol
from record_live_production_verification import write,read

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',type=Path,default=ROOT/'build/windows/squall-candidate/Release/Squall.dll')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/squall-tracker-voices-validation.json')
    parser.add_argument('--allow-differences',action='store_true')
    args=parser.parse_args();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work=ROOT/'.local/squall-tracker-voices'/stamp;work.mkdir(parents=True)
    original=ROOT.parent/'farmář/Dobrý farmář';inventory=read(ROOT/'analysis/inventory.json')
    def unchanged():
        for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
    unchanged()
    binaries={'native.dll':original/'Squall.dll','source.dll':args.candidate.resolve(),'source.map':args.candidate.resolve().with_suffix('.map'),
              'probe.exe':ROOT/'build/squall-tracker-probe-vs/Release/squall-tracker-voice-probe.exe'}
    for name,path in binaries.items():shutil.copyfile(path,work/name)
    paths={name:ROOT/name for name in ['tools/validate_squall_tracker_voices.py','tools/squall_tracker_voice_probe.cpp','tools/squall_tracker_probe/CMakeLists.txt']}
    for p in (ROOT/'src/squall').rglob('*'):
        if p.suffix in ['.cpp','.h']:paths[p.relative_to(ROOT).as_posix()]=p
    for name,path in paths.items():
        out=work/'snapshots'/name;out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,out)
    native=['4851c','4861c'];own=[symbol(work/'source.map',name) for name in ['?gpMixFunctionTable@@','?gpFastMixFunctionTable@@']]
    report=dict(status='running_actual_active_voice_kernels',work_directory=work.relative_to(ROOT).as_posix(),
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
            state=json.loads(run.stdout);assert state['case_count']==9216 and state['guards_intact'] and state['sample_read_only']
            states.append(state);destinations.append((folder/'destinations.bin').read_bytes());channels.append((folder/'channel-scalars.bin').read_bytes())
        assert len(channels[0])==len(channels[1])==9216*252 and len(destinations[0])==len(destinations[1])==2359296
        offset=0;groups={};different_cases=0
        for i,(left,right) in enumerate(zip(states[0]['cases'],states[1]['cases'])):
            a=dict(left);b=dict(right);a.pop('kernel_rva');b.pop('kernel_rva')
            count=left['count'];size=count*8+64
            data_a=destinations[0][offset:offset+size];data_b=destinations[1][offset:offset+size];offset+=size
            channel_a=channels[0][i*252:(i+1)*252];channel_b=channels[1][i*252:(i+1)*252]
            delta=sum(x!=y for x,y in zip(data_a,data_b));scalar_delta=sum(x!=y for x,y in zip(channel_a,channel_b))
            key=(left['table'],left['index']);row=groups.setdefault(key,dict(table=key[0],index=key[1],native_kernel_rva=left['kernel_rva'],source_kernel_rva=right['kernel_rva'],case_pairs=0,different_case_pairs=0,different_destination_bytes=0,different_channel_scalar_bytes=0))
            row['case_pairs']+=1;bad=a!=b or delta or scalar_delta
            row['different_case_pairs']+=bool(bad);row['different_destination_bytes']+=delta;row['different_channel_scalar_bytes']+=scalar_delta;different_cases+=bool(bad)
        assert offset==len(destinations[0])
        report.update(status='passed_actual_active_voice_kernels' if not different_cases else 'different_actual_active_voice_kernels',
            case_pairs=9216,different_case_pairs=different_cases,function_table_entries=128,
            different_destination_bytes=sum(row['different_destination_bytes'] for row in groups.values()),
            different_channel_scalar_bytes=sum(row['different_channel_scalar_bytes'] for row in groups.values()),
            guarded_destination_bytes=2359296,channel_scalar_bytes=2322432,groups=list(groups.values()),
            address_field_excluded='Only pCurrentSample at channel offset0..3;every call independently proves pointer unchanged;full input sample remains read-only',
            code_or_IAT_written=False,production_forwarding=False,original_files_unchanged=17,
            pending=['Actual complete CreateStereoMix dispatch with active channels','More increments,buffer boundaries,looping and simultaneous channels','Remaining whole original goal scopes'])
        for name,path in paths.items():assert digest(path)==report['source_files'][name],name
        unchanged()
    except Exception as error:
        report.update(status='failed',error=repr(error))
        import traceback;report['traceback']=traceback.format_exc()
    write(work/'report.json',report);write(args.output,report)
    print(json.dumps({k:report.get(k) for k in ['status','work_directory','case_pairs','different_case_pairs','different_destination_bytes','different_channel_scalar_bytes','error']}))
    assert report['status']!='failed' and (args.allow_differences or not report['different_case_pairs']),'Actual active mixer observations retained'

if __name__=='__main__':main()
