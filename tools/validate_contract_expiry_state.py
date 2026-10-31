#!/usr/bin/env python3
"""Audit opposite-program active restoration, actual expiry and failure saves."""
import argparse
import collections
import io
import struct
import zipfile
import zlib
import xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image, ImageChops
from record_live_production_verification import ROOT, digest, read, write, checked
from game_save_state import load_save
from validate_factory_history_state import resource


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/contract-expiry-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/contract-expiry-state-validation.json')
    args=parser.parse_args();report=read(args.runtime_report);work=checked(report)
    assert report['status']=='explored_cross_program_natural_contract37_expiry_and_failure_dialog_only' and len(report['sessions'])==2
    fixture=read(ROOT/'.local/contract-delay-acceptance-state-validation.json');checked(fixture)
    assert fixture['status']=='passed_for_natural_rose_contract_delay_acceptance_and_active_save_only'
    for session in report['sessions']:
        assert session['status']=='completed' and session['exit_code']==0
        assert session['fixture_report_sha256']==digest(ROOT/'.local/contract-delay-acceptance-state-validation.json')
        assert session['restore_from_kind']==('source' if session['kind']=='native' else 'native')
        initial=session['initial_state'];assert initial['money']==4504 and initial['stock_roses']==2
        assert initial['tasks'][0]['id']==37 and 229<=initial['tasks'][0]['remaining']<=230 and initial['tasks'][0]['pass_cancel']
        steps=session['work_steps'];assert [r['label'] for r in steps]==['active-rose-contract-restored','natural-rose-deadline-expired','failed-contract-confirmed','original-time-cutoff1390']
        assert all(r['state']['money']==4504 and r['state']['stock_roses']==2 and r['state']['completed_streak']==0 for r in steps)
        assert all(r['state']['sprinklers']==[True]*3 for r in steps)
        for row in steps[1:]:assert row['state']['tasks'][0]==dict(id=-1,remaining=0.,pass_cancel=True)
        final=session['final_state'];assert 1390<=final['game_time']<1395 and final['run_time']<1800 and final['tutorial']==-1
        assert all(letter==dict(id=-1,remaining=0.) for letter in final['letters'])
    props=[{x.get('Name'):(x.get('Type'),x.get('Val')) for x in ET.parse(work/f'{i}-persisted/Profiles/00000001.xml').getroot()} for i in [0,1]]
    a,b=props;assert set(a)==set(b) and all(a[k][0]==b[k][0] for k in a)
    differences={k:[a[k],b[k]] for k in a if a[k]!=b[k]}
    varying={'GameTime','RunTime'}|{f'{prefix}{n}' for prefix in ['FFF','FFW','FFP'] for n in range(60)}
    assert set(differences)<=varying,differences
    for values in props:
        for key,value in dict(Money=('Int','4504'),task0=('Int','-1'),taskTime0=('Float','0'),taskPassCancel0=('Bool','1'),WGoodCount4=('UInt','2'),awQTask=('UInt','0')).items():assert values[key]==value,(key,values[key])
    paths=[work/f'{i}-persisted/Profiles/00000001.save' for i in [0,1]]
    assert paths[0].read_bytes()==paths[1].read_bytes() and paths[0].stat().st_size==4563
    before=load_save(ROOT/fixture['work_directory']/'0-persisted/Profiles/00000001.save');after=load_save(paths[0])
    for key in ['seeds','sales','cutoff','coefficient_bits','seed_padding_ranges','sale_clock_ranges']:assert before[key]==after[key],key
    assert after['messages'][:len(before['messages'])]==before['messages']
    events=resource('Gameplay/gameplay.xml').find('Events');notes=[]
    for event in events:
        seconds=sum(int(v)*scale for v,scale in zip(event.get('Time').split(':'),[3600,60,1]))
        note=event.find('Note')
        if 1110<seconds<=1390 and event.get('Type') in ['0','1'] and note is not None and note.text:notes.append(dict(read=0,text=note.text))
    assert len(notes)==2 and after['messages'][len(before['messages']):]==notes
    def modal(scene):return collections.Counter((tuple(g['geometry']),g['deep'],g['texture'],tuple(g['sprite']),g['flags']) for g in scene if 2029<=g['deep']<3600 and g['texture'])
    left,right=[modal(s['failure_scene']) for s in report['sessions']];assert left==right and sum(left.values())==29
    assert sum(n for row,n in left.items() if row[2]=='ui/menu_font.dds')==26
    images=[Image.open(work/f'{i}-natural-rose-failure.png').convert('RGB') for i in [0,1]]
    caption=(265,150,535,205);assert images[0].crop(caption).tobytes()==images[1].crop(caption).tobytes()
    with zipfile.ZipFile(ROOT.parent/'farmář/Dobrý farmář/Data/Data.pack') as archive:
        data=archive.read(next(n for n in archive.namelist() if n.lower()=='ui/gameui/dogovor.jng'))
    offset=8;alpha_data=[];alpha_png=[]
    while offset<len(data):
        length=struct.unpack_from('>I',data,offset)[0];kind=data[offset+4:offset+8];payload=data[offset+8:offset+8+length];offset+=length+12
        if kind==b'JDAA':alpha_data.append(payload)
        if kind==b'JHDR':assert list(payload)==[0,0,1,244,0,0,1,124,14,8,8,8,8,0,0,0]
        if kind==b'IDAT':alpha_png.append(payload)
    assert not alpha_data and alpha_png
    def chunk(kind,payload):return struct.pack('>I',len(payload))+kind+payload+struct.pack('>I',zlib.crc32(kind+payload)&0xffffffff)
    png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',500,380,8,0,0,0,0))+chunk(b'IDAT',b''.join(alpha_png))+chunk(b'IEND',b'')
    alpha=Image.open(io.BytesIO(png)).convert('L');assert alpha.size==(500,380)
    rect=(210,170,590,405);assert alpha.crop((60,65,440,300)).getextrema()==(254,254)
    delta=ImageChops.difference(*(im.crop(rect) for im in images));maximum=max(hi for lo,hi in delta.getextrema());assert maximum<=1
    report.update(status='passed_for_opposite_program_contract37_restore_natural_expiry_and_failure_save_only',auditor_sha256=digest(__file__),runtime_report_sha256=digest(args.runtime_report),
        comparison=dict(business_snapshots_per_executable=4,opposite_program_active_restores=2,actual_restored_seconds=[s['initial_state']['tasks'][0]['remaining'] for s in report['sessions']],
            failure_money=4504,additional_money_charged_at_expiry=0,roses_retained=2,observed_streak_before_and_after=0,nonzero_streak_reset_verified=False,
            all_profile_discretes_equal=True,independent_float_differences=differences,complete_identical_save_bytes=4563,new_original_event_messages=2,
            equal_actual_failure_modal_sprites=29,equal_actual_failure_glyph_sprites=26,exact_caption_rgb_pixels=270*55,caption_rect=list(caption),
            backdrop_pixels=89300,backdrop_different_pixels=sum(any(pixel) for pixel in delta.getdata()),backdrop_max_channel_difference=maximum,original_panel_alpha=254,background_animation_phases_synchronized=False,
            normalizations=[]),whole_game_complete=False,pending=['Four further bidirectional failure-state saves','Nonzero award streak reset through natural tasks','Remaining full original goal scopes'])
    write(args.output,report);print(report['status']);print('two active cross-restores;29 failure sprites equal;4563 bytes exact;two actual event notes')


if __name__=='__main__':main()
