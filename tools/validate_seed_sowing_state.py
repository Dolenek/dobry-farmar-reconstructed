#!/usr/bin/env python3
"""Audit the complete artifacts of the opt-in natural seed-sowing process pair."""
import argparse,datetime,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/seed-sowing-validation.json');p.add_argument('--output',type=Path,default=ROOT/'.local/seed-sowing-state-validation.json');args=p.parse_args()
 report=json.loads(args.runtime_report.read_text(encoding='utf-8'));assert report['status']=='explored_natural_seed_sowing_and_watering_only' and len(report['sessions'])==2
 for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
 work=ROOT/report['work_directory']
 for name,value in report['screenshots'].items():assert digest(work/name)==value,name
 def props(index):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(work/f'{index}-persisted/Profiles/00000001.xml').getroot()}
 left,right=props(0),props(1);assert len(left)==794 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
 varying={'RunTime','GameTime','letTime0','letTime1','FFF34'}|{f'{prefix}{n}' for prefix in ['FFP','FFW'] for n in [24,25,26,33,34]}
 changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
 for values in [left,right]:
  expected={'Money':('Int','70'),'SeedAvail1':('UInt','1'),'awQCollect':('UInt','9'),'PCarry':('UInt','1'),'PCC0':('UInt','0'),'WGoodCount0':('UInt','0'),'MGoodCount0':('UInt','0'),'mTrigger':('Int','0'),'tutJustProc':('Int','24')}
  for n,v in expected.items():assert values[n]==v,(n,values[n],v)
  for step in [20,21,22,23,24]:assert values[f'tip{step}']==('Bool','1'),step
  assert values['fcu']==('Int','0') and values['FFC34']==('UInt','1') and values['FFS34']==('UInt','0')
  assert float(values['FFW34'][1])>118 and float(values['FFF34'][1])>118
 saves=[load_save(work/f'{i}-persisted/Profiles/00000001.save') for i in [0,1]]
 for state in saves:
  assert state['bytes']==2523 and len(state['seeds'])==1 and len(state['sales'])==4 and len(state['coefficient_bits'])==383
  seed=dict(state['seeds'][0]);seed.pop('padding_hex');assert seed==dict(x=122.,y=280.,kind=1,count=3,status=0,unpaid=0),seed
  assert all(row['amount']==2 for row in state['sales']) and len(state['messages'])==3 and all(row['read']==1 for row in state['messages'])
 assert saves[0]['messages']==saves[1]['messages'] and saves[0]['coefficient_bits']==saves[1]['coefficient_bits']
 ranges=saves[0]['seed_padding_ranges'];assert ranges==saves[1]['seed_padding_ranges']
 raw=[bytearray((work/f'{i}-persisted/Profiles/00000001.save').read_bytes()) for i in [0,1]]
 for data in raw:
  for a,b in ranges:data[a:b]=b'\0'*(b-a)
 assert raw[0]==raw[1],'Sowing save differs outside original three padding bytes'
 inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
 for row in inventory['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256'],row['path']
 report['status']='passed_for_natural_digging_seed_sowing_and_watering_only';report['runtime_report_sha256']=digest(args.runtime_report)
 report['audit_files']={n:digest(ROOT/n) for n in ['tools/validate_seed_sowing_state.py','tools/game_save_state.py']}
 report['comparison']=dict(profile_properties=794,changed_properties=changed,seed=saves[1]['seeds'][0],save_bytes=2523,normalized_byte_ranges=ranges,sale_records=saves[0]['sales'],coefficient_bits_equal=383,messages_equal=3,five_exact_tutorial_rgb_pixels=143000)
 report['pixel_regions']=[row['pixel_comparison'] for row in report['sessions'][1]['stages']]
 assert [row['step'] for row in report['sessions'][1]['stages']]==[20,21,22,23,24]
 assert all(row['different_pixels']==0 for row in report['pixel_regions'])
 report['recorded_at_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();report['pending']=['Sown crop save transfers and purchased-crop growth/harvest','Remaining game/engine/audio/full-goal scopes']
 args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
