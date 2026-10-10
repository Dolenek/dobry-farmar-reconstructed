#!/usr/bin/env python3
"""Record test-only absolute data operand relocations from inventoried instructions."""
import hashlib
import argparse
import json
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from pe import PE
from read_function import ROOT,NAMES
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--draw',action='store_true',help='Record sprite draw oracle operands instead of geometry')
    parser.add_argument('--font',action='store_true',help='Record original font lookup oracle operands')
    parser.add_argument('--text-layout',action='store_true',help='Record original TextLine layout oracle operands')
    parser.add_argument('--letter',action='store_true',help='Record original Letter transform oracle operands')
    parser.add_argument('--string-manager',action='store_true',help='Record StringManager getString normal-path operands')
    parser.add_argument('--control',action='store_true',help='Record Control makeSprite oracle operands')
    parser.add_argument('--view',action='store_true',help='Record cView hierarchy/registration oracle operands')
    parser.add_argument('--control-state',action='store_true',help='Record complete 2D control state/lifetime oracle operands')
    parser.add_argument('--text-state',action='store_true',help='Record TextLine/Letter lifecycle oracle operands')
    parser.add_argument('--button-state',action='store_true',help='Record ButtonPrototype/AnimationButton/Button oracle operands')
    parser.add_argument('--button-text',action='store_true',help='Record animated ButtonText/Letter oracle operands')
    parser.add_argument('--text-buttons',action='store_true',help='Record owned text button constructors and menu XML loading operands')
    parser.add_argument('--click-mask',action='store_true',help='Record original GPU JNG click-mask oracle operands')
    parser.add_argument('--input-bindings',action='store_true',help='Record original Input binding mutation operands')
    parser.add_argument('--ui-frame',action='store_true',help='Record original main-frame UI traversal with explicit service fixtures')
    parser.add_argument('--timer',action='store_true',help='Record native timer construction, processing and scalar methods')
    parser.add_argument('--cursor',action='store_true',help='Record native cursor/window coordinate operands')
    parser.add_argument('--profile-headers',action='store_true',help='Record profile header construction/XML/tree operations')
    parser.add_argument('--profile-properties',action='store_true',help='Record thirteen profile property trees and typed dispatch')
    parser.add_argument('--profile-lifecycle',action='store_true',help='Record profile constructors, full load/save/create/destructor')
    args=parser.parse_args()
    if args.profile_lifecycle: args.profile_properties=True
    if args.text_buttons: args.button_text=True; args.button_state=True
    if args.button_text: args.text_state=True
    if args.click_mask: args.view=True
    if args.input_bindings: args.view=True
    if args.ui_frame: args.view=True
    if args.text_state: args.control_state=True
    if args.button_state: args.control_state=True
    if args.control_state: args.view=True
    original=ROOT.parent/'farmář/Dobrý farmář'/NAMES['game']
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    pe=PE(original)
    rows={int(row['address'],16):row for row in map(json.loads,(ROOT/'analysis/raw/game/functions.jsonl').read_text().splitlines())}
    addresses=[0x515830,0x5809b3] if args.draw else [0x517f40,0x417f80,0x4455e0,0x5809b3]
    prefix='sprite-draw' if args.draw else 'sprite'
    if args.font:
        assert not args.draw
        addresses=[0x50b3d0,0x50b450,0x5809b3]; prefix='font'
    if args.text_layout:
        assert not args.draw and not args.font
        addresses=[0x4dc800,0x50b450,0x5809b3]; prefix='text-layout'
    if args.letter:
        assert not args.draw and not args.font and not args.text_layout
        addresses=[0x50c500,0x417f80,0x4455e0,0x5809b3]; prefix='letter'
    if args.string_manager:
        assert not args.draw and not args.font and not args.text_layout and not args.letter
        addresses=[0x52ed20,0x5809b3]; prefix='string-manager'
    if args.control:
        assert not args.draw and not args.font and not args.text_layout and not args.letter and not args.string_manager
        addresses=[0x4ef3b0,0x4f0b70,0x4ed510,0x413a10,0x4455e0,0x5809b3]; prefix='control'
    if args.view:
        assert sum((args.draw,args.font,args.text_layout,args.letter,args.string_manager,args.control))==0
        addresses=[0x4d55b0,0x4d58a0,0x4d3460,0x4d37d0,0x4d3720,0x4d3bb0,0x4d3fb0,
            0x4d4590,0x4d4980,0x4d4d80,0x4d59c0,0x4d2ec0,0x4d2f50,0x4d2cf0,0x4d2d30,
            0x4d2d70,0x4d2db0,0x4d2df0,0x4d2e20,0x506f90,0x506fe0,0x507030,0x507080,
            0x5069d0,0x506b90,0x506d90,0x4d2ff0,0x4d31a0,0x4d34c0,0x478f20,0x5809b3]
        prefix='view'
        if args.control_state:
            prefix='control-state'
            addresses += [0x4edc80,0x4d2080,0x4ed480,0x460ba0,0x4f16f0,0x4f1560,0x4d2130,
                0x4f1780,0x4ed670,0x4f1810,0x4f17a0,0x4f17c0,0x4f1680,0x4edbe0,0x4418c0,
                0x4f14e0,0x4f1510,0x4f0110,0x4ed5a0,0x4ed6d0,0x4f1080,0x4f0a60,0x4ed610,
                0x4eda30,0x48c710,0x48c900,0x48c970,0x48cd10,0x48d280,0x48c8c0,
                0x52fe50,0x4ee7e0,0x4f0b70,0x413a10,0x4ed510,0x4ef3b0,0x4ed4f0,0x4455e0,
                0x413a50,0x4eebb0,0x413b20,0x413b50,0x413b80,0x413bc0,0x4d3750,0x506a10,0x50db80]
            addresses=list(dict.fromkeys(addresses))
            if args.text_state:
                prefix='text-state'
                addresses += [0x4daca0,0x4da840,0x4da2f0,0x4da2d0,0x4dae40,0x4dbdc0,0x4dd350,
                    0x4dd030,0x4dc800,0x4dc1c0,0x4da400,0x50c490,0x50c500,0x4d2260,0x50b450,0x417f80]
                if args.button_text:
                    prefix='button-text'; addresses += [0x51e040,0x41fa40]
            if args.button_state:
                prefix='button-state'
                addresses += [0x4f53b0,0x4f5480,0x4f28c0,0x4f2770,0x4f2cc0,0x4f30e0,0x4f34c0,
                    0x4f3a00,0x4f3e00,0x4f41f0,0x4f4620,0x4f2620,0x4f26b0,0x4f1c30,0x4f1ce0,
                    0x507ec0,0x507fd0,0x508120]
    if args.click_mask:
        prefix='click-mask'; seen=set(); todo=[0x51c040,0x4f5200,0x5809b3]
        while todo:
            address=todo.pop()
            if address in seen: continue
            seen.add(address)
            if address in rows:
                todo += [int(c,16) for c in rows[address]['callees'] if int(c,16)<0x558000 and int(c,16)!=0x50f580]
        addresses=sorted(seen)
    if args.text_buttons:
        prefix='text-buttons'
        addresses += [0x51fe90,0x41e8d0,0x51f5a0,0x41ed10,0x51f9b0,0x41f120,
            0x51dbf0,0x41f5f0,0x4dd980,0x4d43a0,0x4eded0,0x4d22a0,
            0x4f1860,0x4f4b80,0x4f20f0,0x508560,0x51f570,0x51f520,0x46be80,
            0x51f540,0x46bea0,0x4f1c80,0x5202d0,0x4fe470,0x401050,0x481e10,0x481990,0x481770,
            0x5213c0,0x521430,0x521440,0x521460,0x521500,0x521820,0x521840,0x521860,0x521f00,0x522320,0x522860,0x522c40,0x4db9b0]
        todo=[0x4fb040,0x4fafc0,0x4fb0c0,0x4fa8c0,0x4794f0,0x4fa800];seen=set(addresses)
        while todo:
            address=todo.pop()
            if address in seen:continue
            seen.add(address);addresses.append(address)
            if address in rows:
                todo += [int(c,16) for c in rows[address]['callees'] if int(c,16)<0x558000 and int(c,16) not in (0x4f5e70,0x4d43a0)]
        addresses=list(dict.fromkeys(addresses))
    if args.input_bindings:
        prefix='input-bindings'; seen=set(); todo=[0x526190,0x5280f0,0x526580,0x5284e0,0x526970,0x5288e0,0x524640,0x524a30,0x524e60,
            0x527310,0x5245e0,0x525620,0x525240,0x525a00,0x523e20,0x524200,0x5809b3]
        while todo:
            address=todo.pop()
            if address in seen: continue
            seen.add(address)
            if address in rows: todo += [int(c,16) for c in rows[address]['callees'] if int(c,16)<0x558000 and int(c,16)!=0x552e20]
        addresses=sorted(seen)
    if args.ui_frame:
        prefix='ui-frame';seen=set(addresses);todo=[0x531b20,0x4fb180]
        while todo:
            address=todo.pop()
            if address in seen:continue
            seen.add(address);addresses.append(address)
            if address in rows:
                todo += [int(c,16) for c in rows[address]['callees'] if int(c,16)<0x558000 and int(c,16) not in (0x530290,0x556210,0x556600,0x556c60,0x42fe80,0x401cc0,0x401be0,0x401da0)]
    if args.timer:
        prefix='timer'
        addresses=[0x536ae0,0x536430,0x536550,0x536690,0x5366e0,0x536720,0x536480,0x5364a0,0x536500,0x5809b3]
    if args.cursor:
        prefix='cursor';addresses=[0x530290,0x512330,0x50d260,0x531a40,0x5809b3]
    if args.profile_headers:
        prefix='profile-headers';seen=set();todo=[0x4ecb20,0x4ea000,0x4dfd90,0x4e8900,0x4e75b0,0x4e4110,0x5809b3]
        boundaries={0x401050,0x4017c0,0x401ab0,0x4d94a0,0x4d9670,0x4e17e0,0x4ec310}
        while todo:
            address=todo.pop()
            if address in seen: continue
            seen.add(address)
            if address in rows and address not in boundaries:
                todo += [int(c,16) for c in rows[address]['callees'] if int(c,16)<0x558000]
        addresses=sorted(seen)
    if args.profile_properties:
        prefix='profile-properties';seen=set();todo=[0x4ebd10,0x4e9d80,0x4106a0,0x410770,0x4105e0,
            0x4e98c0,0x4e9990,0x4e9a70,0x4e9b40,0x4e9c00,0x4e9cc0,0x4eac20,0x4eb1d0,0x5809b3,
            0x4e4c70,0x4e4e90,0x4e50c0,0x4e52e0,0x4e5500,0x4e5720,0x4e5940,0x4e3840,
            0x4e5b60,0x4e5d90,0x4e30f0,0x4e3320,0x4e35a0]
        boundaries={0x4016d0,0x4017c0,0x403960,0x401c70,0x40da20,0x4d95c0,0x4d9a90,
            0x4d9720,0x4d97d0,0x4d9880,0x4d94a0,0x4d9930,0x4d93b0,0x4d9670,0x4d99e0,0x4d9240,
            0x4ae240,0x4df0a0,0x40daf0,0x4e0280,0x4deaa0,0x4df4f0,0x4a8be0,0x406400,0x4df6b0,
            0x4de930,0x4df380,0x4de7c0,0x4ded80,0x4455e0,0x4019c0}
        if args.profile_lifecycle:
            prefix='profile-lifecycle';todo += [0x4ecb20,0x4ec310,0x4ea000,0x4ea5d0,0x4e3a70,0x4e6cf0,0x4ead60,0x411390,0x4e8d20,0x4e14e0,0x4e15e0]
            boundaries |= {0x401050,0x4e1010,0x4e17e0,0x4087d0,0x405710,0x40a430,0x50ce30,0x40a470,0x401cc0,0x41d680,0x40a670,0x4cfbe0,0x401ab0}
        while todo:
            address=todo.pop()
            if address in seen: continue
            seen.add(address)
            if address in rows and address not in boundaries:
                todo += [int(c,16) for c in rows[address]['callees'] if int(c,16)<0x558000]
        addresses=sorted(seen)
    scratch=ROOT/'.local/function-inspection'; scratch.mkdir(parents=True,exist_ok=True)
    fixups=[]
    with tempfile.TemporaryDirectory(dir=scratch) as directory:
        reference=Path(directory)/'reference.exe'; shutil.copyfile(original,reference)
        for address in addresses:
            end=int(rows[address]['body_max'],16)+1 if address in rows else {
                0x413a10:0x413a47,0x413b20:0x413b41,0x413b50:0x413b74,0x413b80:0x413bb6,
                0x413bc0:0x413be7,0x51f570:0x51f5a0,0x481770:0x48179e,
                0x536480:0x536498,0x5364a0:0x5364a7,0x536500:0x536519,0x50d260:0x50d2ad,0x531a40:0x531a44,0x521500:0x52181f}[address]
            result=subprocess.run(['objdump','-d','-Mintel',f'--start-address={address}',f'--stop-address={end}',reference.name],cwd=directory,capture_output=True,text=True,check=True)
            for line in result.stdout.splitlines():
                match=re.match(r'\s*([0-9a-f]+):\s*((?:[0-9a-f]{2} )+)\s*(.*)',line)
                if not match: continue
                instruction,encoded,text=int(match[1],16),bytes.fromhex(match[2]),match[3]
                memory=re.search(r'ds:0x([0-9a-f]+)',text)
                if args.button_state and not memory and text.startswith('jmp'):
                    candidate=re.search(r'\[[^\]]*\+0x([0-9a-f]+)\]',text)
                    if candidate and pe.image_base<=int(candidate[1],16)<pe.image_base+0x300000: memory=candidate
                if (args.view or args.timer or args.cursor or args.profile_headers or args.profile_properties) and not memory and text.split() and text.split()[0] in (('mov','push','lea','add') if args.profile_lifecycle else ('mov','push')):
                    # A member displacement can precede an immediate pointer,
                    # e.g. mov [esi+0xc],0x536550. Inspect every operand token.
                    candidates=[candidate for candidate in re.finditer(r'0x([0-9a-f]+)\b',text)
                        if pe.image_base<=int(candidate[1],16)<pe.image_base+0x300000]
                    if candidates: memory=candidates[-1]
                # The normal getString path loads the shared fallback via an
                # immediate address, in addition to absolute memory operands.
                if args.string_manager and not memory:
                    memory=re.search(r'0x(6605c8)\b',text)
                if not memory: continue
                target=int(memory[1],16)
                assert target>=pe.image_base and target<pe.image_base+0x300000, line
                operand=encoded.find(target.to_bytes(4,'little'))
                # objdump wraps long instructions. The memory operand starts in
                # its first line, and may finish on the continuation line.
                if operand<0:
                    binary=pe.data[pe.offset(instruction-pe.image_base):pe.offset(instruction-pe.image_base)+15]
                    operand=binary.find(target.to_bytes(4,'little'))
                    assert 0<=operand<=(7 if args.view or args.timer or args.cursor or args.profile_headers or args.profile_properties else 3),line
                fixups.append(dict(rva=instruction-pe.image_base+operand,original_address=target,function=f'{address:08x}',instruction=text))
    if args.view:
        # cView's base destructor reinstates its original virtual table before
        # destroying children; those children call the parent's removeChild.
        for slot in range(14):
            rva=0x1fb770+slot*4
            target=int.from_bytes(pe.data[pe.offset(rva):pe.offset(rva)+4],'little')
            assert pe.image_base<=target<pe.image_base+0x200000
            fixups.append(dict(rva=rva,original_address=target,function='cView-vtable',instruction=f'virtual slot {slot}'))
        if args.control_state:
            tables=[(0x1fbfd8,59),(0x1fc388,65)]
            if args.text_state: tables += [(0x1fb818,72),(0x1fe198,69)]
            if args.button_text: tables += [(0x1fee10,72)]
            if args.text_buttons: tables += [(0x1ff0d8,88),(0x1f0020,88),(0x1efe20,72),(0x1f104,4),(0x11f994,4),(0x1fcf04,14),(0x1ff2b0,72)]
            if args.button_state: tables += [(0x1fc8d0,88),(0x1fc4c0,88),(0x1fdd98,88)]
            if args.button_state: tables += [(0x0f20d4,4),(0x108110,4)] # state switch destinations
            for table,count in tables:
                for slot in range(count):
                    rva=table+slot*4; target=int.from_bytes(pe.data[pe.offset(rva):pe.offset(rva)+4],'little')
                    assert pe.image_base<=target<pe.image_base+0x200000
                    fixups.append(dict(rva=rva,original_address=target,function='cControl/Picture-vtable',instruction=f'virtual slot {slot}'))
    (ROOT/f'analysis/{prefix}-oracle-fixups.json').write_text(json.dumps(dict(reference_sha256=expected,test_only=True,fixups=fixups),indent=2)+'\n',encoding='utf-8')
    lines=['// Generated by build_sprite_oracle_fixups.py; test-only data operand addresses.']
    lines += [f'{{0x{row["rva"]:08x}U,0x{row["original_address"]:08x}U}},' for row in fixups]
    (ROOT/'tools'/f'{prefix.replace("-","_")}_oracle_fixups.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print(f'Recorded {len(fixups)} test-only operand fixups')
if __name__=='__main__': main()
