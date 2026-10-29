#!/usr/bin/env python3
"""Audit the complete artifacts of the opt-in natural tomato-harvest process pair."""
import argparse,datetime,hashlib,json,xml.etree.ElementTree as ET
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--runtime-report',type=Path,default=ROOT/'.local/tomato-harvest-validation.json');p.add_argument('--output',type=Path,default=ROOT/'.local/tomato-harvest-state-validation.json');args=p.parse_args()
 report=json.loads(args.runtime_report.read_text(encoding='utf-8'));assert report['status']=='explored_natural_purchased_tomatoes_growth_harvest_and_sale_only' and len(report['sessions'])==2
 for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
 work=ROOT/report['work_directory']
 for name,value in report['screenshots'].items():assert digest(work/name)==value,name
 def props(index):return {x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(work/f'{index}-persisted/Profiles/00000001.xml').getroot()}
 left,right=props(0),props(1);assert len(left)==794 and set(left)==set(right) and all(left[n][0]==right[n][0] for n in left)
 varying={'RunTime','GameTime','letTime0','letTime1','FFF34','MSellTime1'}|{f'{prefix}{n}' for prefix in ['FFP','FFW'] for n in [24,25,26,33,34]}
 changed={n:[left[n],right[n]] for n in left if left[n]!=right[n]};assert set(changed)<=varying,changed
 for values in [left,right]:
  expected={'Money':('Int','72'),'SeedAvail1':('UInt','1'),'awQCollect':('UInt','10'),'PCarry':('UInt','6'),'PCC0':('UInt','0'),'WGoodCount0':('UInt','0'),'MGoodCount0':('UInt','0'),'mTrigger':('Int','0'),'tutJustProc':('Int','12')}
  for n,v in expected.items():assert values[n]==v,(n,values[n],v)
  for step in [12,20,21,22,23,24]:assert values[f'tip{step}']==('Bool','1'),step
  assert values['fcu']==('Int','0') and values['FFC34']==('UInt','1') and values['FFS34']==('UInt','0')
  assert float(values['FFW34'][1])>90 and float(values['FFF34'][1])>100
  assert values['PCC1']==values['WGoodCount1']==values['MGoodCount1']==('UInt','0')
  assert values['MGoodPrice1']==('UInt','2') and values['awQTask']==('UInt','0')
 saves=[load_save(work/f'{i}-persisted/Profiles/00000001.save') for i in [0,1]]
 for state in saves:
  assert state['bytes']==2515 and len(state['seeds'])==1 and len(state['sales'])==3 and len(state['coefficient_bits'])==383
  seed=dict(state['seeds'][0]);seed.pop('padding_hex');position=seed.pop('x');assert 122.9<position<123.1 and seed==dict(y=280.,kind=1,count=3,status=0,unpaid=0),seed
  assert all(row['amount']==2 for row in state['sales']) and len(state['messages'])==3 and all(row['read']==1 for row in state['messages'])
 assert saves[0]['messages']==saves[1]['messages'] and saves[0]['coefficient_bits']==saves[1]['coefficient_bits']
 # Classify the observed seed-X difference explicitly; it is pending a repeated reference transition audit.
 ranges=[[4,8],[25,28],[32,36],[52,56]]
 assert saves[0]['seed_padding_ranges']==saves[1]['seed_padding_ranges']==[[25,28]]
 raw=[bytearray((work/f'{i}-persisted/Profiles/00000001.save').read_bytes()) for i in [0,1]]
 for data in raw:
  for a,b in ranges:data[a:b]=b'\0'*(b-a)
 assert raw[0]==raw[1],'Harvest save differs outside recorded X drift, padding and independently timed latest sale/cutoff'
 inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
 for row in inventory['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256'],row['path']
 report['status']='passed_for_natural_purchased_tomatoes_gameplay_with_recorded_seed_position_drift_only';report['runtime_report_sha256']=digest(args.runtime_report)
 report['audit_files']={n:digest(ROOT/n) for n in ['tools/validate_tomato_harvest_state.py','tools/game_save_state.py']}
 report['comparison']=dict(profile_properties=794,changed_properties=changed,seed=saves[1]['seeds'][0],save_bytes=2515,normalized_byte_ranges=ranges,sale_records=saves[0]['sales'],coefficient_bits_equal=383,messages_equal=3,seed_position_drift_pending=True)
 report['pixel_regions']=[row['pixel_comparison'] for row in report['sessions'][1]['stages']]
 assert [row['step'] for row in report['sessions'][1]['stages']]==[12]
 for session in report['sessions']:
  rows={row['label']:row['state'] for row in session['work_steps']}
  assert rows['tomatoes-ready']['field_state']==1 and rows['tomatoes-ready']['crop_progress']==0
  assert rows['tomatoes-harvested']['field_state']==0
  assert session['tomato_sale_price']==2 and session['final_state']['money']==72
 report['recorded_at_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();report['pending']=['Repeated native/source seed-coordinate drift investigation and actual crop save transfers','Remaining game/engine/audio/full-goal scopes']
 args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(report['status'])
if __name__=='__main__':main()
