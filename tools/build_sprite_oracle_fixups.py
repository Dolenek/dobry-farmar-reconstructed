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
    parser.add_argument('--sound-manager',action='store_true',help='Record SoundManager files, native lifetime and volume operands')
    parser.add_argument('--progress-sprite',action='store_true',help='Record native ProgressBar sprite and switch table operands')
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
    if args.progress_sprite:
        addresses=[0x4f9290,0x4ed4f0,0x4455e0,0x5809b3];prefix='progress-sprite'
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
        addresses += [0x521020,0x521060]
        todo=[0x4fb040,0x4fafc0,0x4fb0c0,0x4fa8c0,0x4794f0,0x4fa800,0x51f100,
            0x521100,0x5206e0,0x5212a0,0x521080,0x51e6d0,0x51ec50,0x51d920,
            0x4f5e70,0x4f8580,0x4f8ce0,0x4f8e10,0x4f7e10,0x4f7250,0x4f7700,0x4f6a80,0x4f6130,
            0x4db210, # wrap calls TextLine width through vtable +10c
            0x501b60,0x500df0,0x5014d0,0x501530,0x501590,0x501970,
            0x500650,0x500a20,0x501a20,0x501430,0x501480,
            0x4fffb0,0x4fca60,0x4fcea0,0x4fefa0,0x4fe870,0x4fef60,0x4fe9a0,0x4fe9b0,
            0x4fdfd0,0x4fe020,0x4fe070,0x4fc680,0x4feaa0,0x4fd780,0x4fdbb0,
            0x4ffb70,0x4ff410,0x4ffa00,0x4db9f0,0x50b3d0,
            0x479b30,0x4795b0,0x479110,0x478e90,0x478f00,0x478ef0,
            0x47b1c0,0x47a4e0,0x47ac90,0x479c70,0x479c90,0x479c80,
            0x479cb0,0x479ce0,0x479d10,0x479d40,0x47a110,0x421520,
            0x504180,0x5042a0,0x504780,0x504b80,0x504210,0x504280,
            0x502970,0x503bf0,0x5036d0,0x5031a0,0x502dc0,0x502170,
            0x5021a0,0x502590,0x5020f0,0x5020c0,0x502130,0x502150,0x489770,0x487ae0,0x487a30,0x487ac0,0x488f10,0x4892f0,0x488770,0x488b40,
            0x4f91e0,0x4f9230,0x4f9270,0x4f9290,0x4f9b20,0x4f9f80,0x413750,
            0x480770,0x47fff0,0x4800f0,0x480110,0x481130,0x541700,0x480d10,
            0x4c8d10,0x4c8de0,0x4c8e00,0x4c8e80,0x4c95c0,0x4c9a80,0x4d2080,0x4d2130,0x4d2160,
            0x4c6a40,0x4c61e0,0x4c60e0,0x4c61c0,0x4c5b70,0x4c5bc0,0x421520,
            0x4728c0,0x472d50,0x474da0,0x474dc0,0x473150,0x473520,0x473940,0x473d50,0x474130,0x474520,0x474910,
            0x4787e0,0x478dc0,0x478da0,0x4763a0,0x476ae0,0x4776b0,0x477ab0,0x477e80,0x40ab50,
            0x4761c0,0x475300,0x475ae0,0x475700,0x475b00,0x4783d0,0x476ec0,0x53fdf0,0x53f2d0,
            0x484da0,0x483e90,0x4842a0,0x4842b0,0x4836d0,0x483ab0,0x4817a0,0x4817c0,0x4817d0,0x4816d0,
            0x4818f0,0x481860,0x485490,0x4826c0,0x482ef0,0x482ae0,0x484700,0x5306c0,0x403610,0x403660,0x4036b0,0x4036c0,0x40af00,0x40b560,0x40e6e0,0x4adac0,0x4b50d0,0x4b09e0,0x4b0a40,0x4b0a60,0x4b0a80,0x4b4a70,0x4b0b10,0x4b0b30,0x4b0b50,0x4b1240,0x4b1710,0x4b1bd0,0x4b2040,0x4b2500,0x4b29c0,0x4b3300,0x4b3a00,0x4b3e70,0x4b4340,0x4b4b60,0x4b2e80,0x4aeb10];seen=set(addresses)
        todo += [0x4b59c0,0x4b5c60,0x4ae570,0x4b4ac0,0x4b4ad0,0x42f030,0x581021,0x4169c0,0x416970,0x417050,0x417430,0x4174b0,0x416a30,0x40ba00,0x410ce0,0x404170,0x428370,0x4284c0,0x428590,0x429bb0,0x4297c0,0x42a1b0,0x428a90,0x428ff0,0x4293e0,0x499ca0,0x499cc0,0x49a340,0x4cdee0,0x423550,0x421600,0x4222b0,0x4230b0,0x421be0,0x4236f0,0x4227b0,0x422cc0,0x421800,0x4215e0,0x48fb30,0x48fb50,0x4bb830,0x4b92c0,0x4b9330,0x4bb800,0x4ba730,0x4bd2f0,0x4bc460,0x4bc840,0x4bcc20,0x4b98e0,0x4ba0b0,0x4bbee0,0x4b9350,0x4b93e0,0x467fa0,0x4b1080,0x4036e0,0x4cdef0,0x48bd30,0x48bd60,0x48bd80,0x48bde0,0x48be00,0x48c210,0x415b70,0x415bf0,0x415c80,0x415d10,0x4161a0,0x416580,0x4aadb0,0x4a8b30,0x4ab360,0x4ab380,0x4aa690,0x4aa890,0x4aa040,0x4aa640,0x4aa660,0x4a8d50,0x4a94e0,0x4a9c60,0x4b73f0,0x4b65b0,0x4b6b90,0x4b7010,0x4b7960,0x4b7980,0x4b8ce0,0x4b7a00,0x4b7fe0,0x4b8510,0x4b8900,0x4b9270,0x4b9290,0x4d3790,0x4ad4b0,0x4abef0,0x4ac570,0x4accc0,0x4ad0d0,0x4abea0,0x4ada70,0x4ada90,0x4c02c0,0x487330,0x487980,0x487960,0x487950,0x485680,0x485d90,0x486170,0x485560,0x485590,0x485630,0x485660,0x486550,0x487210,0x486d90,0x4870a0,0x489e80,0x4c48e0,0x4c4a60,0x42c7d0,0x4c4a80,0x4c4eb0,0x4c5270,0x4c5630,0x4c56f0,0x4c3430,0x4c31f0,0x4c3900,0x4c32c0,0x4c1f90,0x4c30a0,0x4c3130,0x4c3920,0x4c4390,0x4c44a0,0x4c4430,0x4c32d0,0x4c06b0,0x4c0710,0x4c0750,0x4c0b50,0x4c0f70,0x4c1350,0x4c1770,0x4c1b60,0x4bef70,0x4bf4d0,0x4bf9e0,0x4bd950,0x4be740,0x499240,0x4b9cc0,0x4bd8a0,0x4bd830,0x4bd8f0,0x4bd920,0x502130,0x4bd9d0,0x4bdf00,0x4be340,0x4c2cc0,0x4beb40,0x4bfee0,0x4bb330,0x4bb7e0]
        todo += [0x4cdde0,0x4cc150,0x4cc700,0x4ca6e0,0x4cac90,0x4ca700,0x4cd400,0x4cb8f0,0x4cc780,0x4cb4d0,0x4cbd60,0x4cc7c0,0x4cb7e0,0x4cc290,0x4ca7d0,0x4caf70,0x4ca690,0x4cac10,0x4cda00,0x4cc320,0x4ccc40,0x4cd020,0x4f6260,0x4f6a00,0x4f6630,0x4f6e80,0x4cc800,0x4cb840,0x4ca720]
        todo += [0x420cb0,0x4201f0,0x4200f0,0x420f50,0x420f30,0x41d520,0x41d580,0x41d5e0,0x41d610,0x41e050,0x41e4c0,0x41da60,0x41d9a0,0x41d9f0,0x417910,0x403740,0x41d500,0x413780,0x413fa0,0x413f80,0x414070,0x414430,0x4147f0,0x4148e0,0x414d60,0x4cfce0,0x4d0490,0x4ce160,0x4cdf70,0x4cdfa0,0x4cdfd0,0x4d0990,0x4d0a20,0x4d0ac0,0x4d0c30,0x4ce650,0x4ceb00,0x4cef60,0x4cf3c0,0x4cf7d0,0x4d1160,0x4cfbe0,0x4cdf10,0x4c9fc0,0x4c9fd0,0x4c9fe0,0x4c9ff0,0x4ca050,0x4ca080,0x4ca0b0,0x4ca0d0,0x4ca0f0,0x4ca110,0x4ca140,0x4ca160,0x4ca190,0x4ca1b0,0x4ca1e0,0x4ca1f0,0x4ca210,0x4ca230,0x4ca260,0x4ca290,0x4ca2b0,0x4ca310,0x4ca330,0x4ca350,0x4ca450,0x4ca4e0,0x4ca570,0x4ca610]
        todo += [0x49dc20,0x49a270,0x49a370,0x49ddc0,0x49b7c0,0x49a650,0x49c880,0x49c490,0x49bcc0,0x49c0b0,0x49a340,0x499c80,0x491690,0x4928e0,0x48edf0,0x48eef0,0x48f9e0,0x48dea0,0x48e130,0x48e550,0x48e980,0x48e9e0,0x48df00,0x48df50,0x48e030,0x48de70,0x48de90,0x48fa00,0x48fba0,0x48fb80,0x48fc60,0x490160,0x4906a0,0x490a90,0x490e70,0x491260,0x491900,0x5070d0,0x507160,0x507190,0x5071b0,0x507660,0x507af0,0x413a00]
        todo += [4822832, 4812704, 4815120, 4815264, 4801424, 4817920, 4803104, 4820064, 4806592, 4807632, 4804208, 4799344, 4812912, 4813952, 4814960, 4800464, 4799952, 4805600, 4830624, 4799584, 4822576, 4808656, 4809792, 4799408, 4847568, 4799136, 4810016, 4824048, 4829616, 4797200, 4796240, 4798496, 4797440, 4819008, 4814992, 4815152, 4211056]
        todo += [0x4a7480,0x4a50e0,0x4a3f10,0x4a6b90,0x4a3620,0x4a47f0,0x4a8660,0x4a59c0,0x4a7d70,0x4a2d30,0x4a62a0]
        todo += [0x49e9f0,0x49b080,0x49cd30,0x49cd50,0x49cd60,0x4a0330,0x4a2490,0x4a1c90,0x49f010,0x49f0c0,0x49f170,0x49f210,0x49f300,0x49f3c0,0x49fdf0,0x49f8c0,0x49fc30,0x499f00,0x499c00,0x499e30,0x499fe0,0x49a0b0,0x49d340,0x49d7b0,0x49dc00,0x49a120,0x49a180,0x499ad0,0x49ab60,0x499b90,0x499dd0,0x499db0,0x499b50,0x49a050,0x499ea0,0x49a080,0x4a0000,0x4a0230,0x4a02c0,0x4a7850,0x4a54b0,0x4a42e0,0x4a6f60,0x4a39f0,0x4a4bc0,0x4a8a30,0x4a5d90,0x4a8140,0x4a3100,0x4a6670,0x4178c0,0x417d10,0x417b90,0x4f0510]
        boundaries={0x494390,0x42c960,0x431df0,0x432260,0x498400,0x437d90,0x436d90,0x45cea0,0x4683a0,0x44ec00,0x44f120,0x44edd0,0x49f010,0x499c80,0x467fa0,0x4d0070,0x4d0c30,0x466a40,0x469400,0x447b40,0x426f50,0x465e70,0x46eb70,0x46f370,0x465050,0x465460,0x4654b0,0x449820,0x44ad50,0x449880,0x449860,0x449840,0x44a820,0x42cde0,0x446d50,0x4d0ac0,0x4b5c60,0x486970,0x4341b0,0x4c02c0,0x4d43a0,0x4ecb20,0x4e8900,0x4e8d20,0x42fe80,0x401cc0,0x40a670,0x4105e0,0x477290,0x4842d0,0x4832f0,0x4b0b70,0x4106a0,0x410770,0x406b00,0x405060,0x406700,0x406570,0x40a450,0x4059e0,0x4b6b90,0x4b7fe0,0x4a94e0,0x4c06b0,0x4c44a0,0x410d60,0x4682d0,0x4d0a20,0x4cf3c0,0x4a8be0,0x403900,0x4050d0,0x4087d0}
        todo += [0x499c90] # right-button callback's unindexed caller does not export callees
        todo += [0x41be20,0x4182b0,0x41c0e0,0x41b010,0x41b520,0x419e70,0x41ab10,0x41a250,0x41b910,0x419680,0x4192a0,0x419a90]
        boundaries |= {0x493b70,0x494a20,0x4957c0,0x495bd0,0x494e70,0x498c60,0x467ec0}
        while todo:
            address=todo.pop()
            if address in seen:continue
            seen.add(address);addresses.append(address)
            if address in rows:
                todo += [int(c,16) for c in rows[address]['callees'] if re.fullmatch(r'[0-9a-fA-F]+',c) and int(c,16)<0x558000 and int(c,16) not in boundaries]
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
    if args.sound_manager:
        prefix='sound-manager';seen=set();todo=[0x535d50,0x535280,0x535900,0x533ea0,0x533a90,0x5342c0,0x532f40,0x532f90,0x532e40,0x580f98,0x5809b3,0x534730,0x534b70,0x534be0,0x533620,0x5346e0,0x4fb8b0,0x4fb700,0x4fbda0,0x4fc1d0,0x559570,0x559970,0x50ae80,0x50ada0,0x50ae20,0x50ae30,0x50ae50,0x532e00]
        boundaries={0x4019c0,0x5573b2,0x55739a,0x5573ac,0x5573a6,0x55737c,0x557382,0x580375,0x4016d0,0x401cc0,0x408860,0x406700,0x40a450,0x4d1e00,0x40a470,0x4fb740}
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
                0x493dd0:0x493fc8,0x493c60:0x493ca2,0x499630:0x499729,0x496440:0x496513,0x493bb0:0x493bcb,0x49a050:0x49a080,0x499ea0:0x499f00,0x49a080:0x49a0b0,0x49f8c0:0x49fc22,0x49fc30:0x49fdaf,0x4a0000:0x4a0224,0x4a0230:0x4a02c0,0x4a02c0:0x4a0330,0x413a00:0x413a0c,0x48e980:0x48e9d6,0x41d610:0x41d666,0x41d5e0:0x41d601,0x4c9fc0:0x4c9fd0,0x4c9fd0:0x4c9fe0,0x4c9fe0:0x4c9ff0,0x4c9ff0:0x4ca050,0x4ca050:0x4ca080,0x4ca080:0x4ca0b0,0x4ca0b0:0x4ca0d0,0x4ca0d0:0x4ca0f0,0x4ca0f0:0x4ca110,0x4ca110:0x4ca140,0x4ca140:0x4ca160,0x4ca160:0x4ca190,0x4ca190:0x4ca1b0,0x4ca1b0:0x4ca1e0,0x4ca1e0:0x4ca1f0,0x4ca1f0:0x4ca210,0x4ca210:0x4ca230,0x4ca230:0x4ca260,0x4ca260:0x4ca290,0x4ca290:0x4ca2b0,0x4ca2b0:0x4ca310,0x4ca310:0x4ca330,0x4ca330:0x4ca350,0x4ca350:0x4ca44d,0x4cdf10:0x4cdf19,
                0x4c32c0:0x4c32c8,0x4bd830:0x4bd835,0x4bd8f0:0x4bd91b,0x4bd920:0x4bd94b,0x42c7d0:0x42c7d5,0x485560:0x485584,0x485630:0x485654,0x485660:0x485679,0x487950:0x487958,0x48bd80:0x48bdce,0x4b9350:0x4b93e0,0x4b4340:0x4b4345,0x413a10:0x413a47,0x413b20:0x413b41,0x413b50:0x413b74,0x413b80:0x413bb6,
                0x413bc0:0x413be7,0x51f570:0x51f5a0,0x481770:0x48179e,0x4817a0:0x4817bc,0x4817c0:0x4817cb,0x4816d0:0x4816f5,0x4842a0:0x4842a8,
                0x536480:0x536498,0x5364a0:0x5364a7,0x536500:0x536519,0x50d260:0x50d2ad,0x531a40:0x531a44,0x521500:0x52181f,
                0x521020:0x521057,0x521060:0x52107e,0x4fe9a0:0x4fe9a8,0x4fe9b0:0x4fea9c,0x478ef0:0x478ef8,
                0x479c80:0x479c89,0x479cb0:0x479cd1,0x479ce0:0x479d01,0x479d10:0x479d3b,0x532e00:0x532e20,0x50ae20:0x50ae2d,0x50ae30:0x50ae4b,0x50ae50:0x50ae57}[address]
            # The exported constructor range stops before its allocation tail.
            # Preserve the original contiguous normal path through RET 8.
            if args.text_buttons and address==0x4fffb0: end=max(end,0x5002e4)
            if args.text_buttons and address==0x5042a0: end=max(end,0x50441b)
            if args.text_buttons and address==0x4f9b20: end=max(end,0x4f9c0b)
            result=subprocess.run(['objdump','-d','-Mintel',f'--start-address={address}',f'--stop-address={end}',reference.name],cwd=directory,capture_output=True,text=True,check=True)
            for line in result.stdout.splitlines():
                match=re.match(r'\s*([0-9a-f]+):\s*((?:[0-9a-f]{2} )+)\s*(.*)',line)
                if not match: continue
                instruction,encoded,text=int(match[1],16),bytes.fromhex(match[2]),match[3]
                memory=re.search(r'ds:0x([0-9a-f]+)',text)
                if (args.button_state or args.progress_sprite) and not memory and text.startswith('jmp'):
                    candidate=re.search(r'\[[^\]]*\+0x([0-9a-f]+)\]',text)
                    if candidate and pe.image_base<=int(candidate[1],16)<pe.image_base+0x300000: memory=candidate
                if (args.view or args.timer or args.cursor or args.profile_headers or args.profile_properties or args.sound_manager) and not memory and text.split() and text.split()[0] in (('mov','push','lea','add') if args.profile_lifecycle or (args.text_buttons and address in (0x40ab50,0x496520)) else ('mov','push')):
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
                    assert 0<=operand<=(7 if args.view or args.timer or args.cursor or args.profile_headers or args.profile_properties or args.sound_manager else 3),line
                fixups.append(dict(rva=instruction-pe.image_base+operand,original_address=target,function=f'{address:08x}',instruction=text))
    if args.progress_sprite:
        for slot in range(4):
            rva=0xf9b10+slot*4;target=int.from_bytes(pe.data[pe.offset(rva):pe.offset(rva)+4],'little')
            assert 0x4f9534<=target<0x4f961f
            fixups.append(dict(rva=rva,original_address=target,function='ProgressBar-switch',instruction=f'side {slot}'))
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
            if args.text_buttons: tables += [(0x1f6b28,65),(0x1f6940,88),(0x1f6c68,88),(0x1fdba0,88),(0x107ae0,4),(0x1f02c0,65),(0x1f7370,88),(0x1ef1c0,65),(0x1fb458,14),(0x1fb1dc,15),(0x1fb144,15),(0x1faf14,15),(0x1faff0,15),(0x1fa3f0,88),(0x1fa954,14),(0x1fa950,1),(0x1fa9f8,15),(0x1f62a4,14),(0x1f62dc,1),(0x1f9820,14),(0x1f962c,14),(0x1f9538,14),(0x1f9e04,14),(0x1f9f48,14),(0x1f65e0,72),(0x1f65d4,2),(0x1ef408,72),(0x1ef400,2),(0x1f9f80,14),(0x1fa158,89),(0x1fa14c,2),(0x1f04a0,89),(0x1f0c40,89),(0x1ff0d8,88),(0x1f0020,88),(0x1efe20,72),(0x1f104,4),(0x11f994,4),(0x1fcf04,14),(0x1ff2b0,72),(0x1fef38,65),(0x1fca60,67),(0x1fd360,65),(0x1fd18c,14),(0x1fd1c4,3),(0x1f5870,14),(0x1f58a8,1),(0x1f5900,14),(0x1fd8b0,92),(0x1fd618,92),(0x1fd4b0,88),(0x1f62e4,14),(0x1f631c,3)]
            if args.button_state: tables += [(0x1fc8d0,88),(0x1fc4c0,88),(0x1fdd98,88)]
            if args.text_buttons: tables += [(0x1fcd30,67),(0xf9b10,4),(0xf9f70,4),(0x1f5e0c,14),(0x141c60,5),(0x1fad98,65),(0x1fb568,2),(0x1faad8,65),(0xc60b8,9),(0x1f51a0,88),(0x74d88,4),(0x1f5830,14),(0x1f5868,3),(0x1f54b0,65),(0x1f60a8,14),(0x1f60e0,1),(0x1f98c8,2),(0x1f98d0,18),(0x1f9c60,2)]
            if args.text_buttons: tables += [(2063096, 89), (2063072, 3), (2063084, 2), (2061880, 88), (2061872, 2), (2062240, 92), (2062232, 2), (2061520, 88), (2068136, 91), (2068116, 3), (2068128, 2), (2066232, 91), (2066212, 3), (2066224, 2), (2065272, 91), (2065252, 3), (2065264, 2), (2067664, 91), (2067640, 3), (2067652, 2), (2064784, 91), (2064764, 3), (2064776, 2), (2065752, 91), (2065728, 3), (2065740, 2), (2069104, 91), (2069080, 3), (2069092, 2), (2066712, 91), (2066692, 3), (2066704, 2), (2068616, 91), (2068596, 3), (2068608, 2), (2064312, 91), (2064288, 3), (2064300, 2), (2067192, 91), (2067172, 3), (2067184, 2)]
            if args.text_buttons: tables += [(0x1f7134,15),(0x1f7130,1),(0x1f6e10,65),(0x1f6e04,2),(0x949f0,11),(0x1ef7a0,89),(0x1ef798,2)]
            if args.text_buttons: tables += [(0x1f7918,89),(0x1f6dc8,1)] # ExtendedButton HideTips virtual at +160
            if args.button_state: tables += [(0x0f20d4,4),(0x108110,4)] # state switch destinations
            for table,count in tables:
                for slot in range(count):
                    rva=table+slot*4; target=int.from_bytes(pe.data[pe.offset(rva):pe.offset(rva)+4],'little')
                    assert pe.image_base<=target<pe.image_base+0x200000
                    fixups.append(dict(rva=rva,original_address=target,function='cControl/Picture-vtable',instruction=f'virtual slot {slot}'))
    if args.sound_manager:
        for table,count in [(0x60028c,11),(0x601678,7),(0x5fe0d8,7),(0x5fcf5c,1)]:
            for slot in range(count):
                rva=table-pe.image_base+slot*4
                target=int.from_bytes(pe.data[pe.offset(rva):pe.offset(rva)+4],'little')
                assert pe.image_base<=target<pe.image_base+0x200000
                fixups.append(dict(rva=rva,original_address=target,function='SoundManager/resource/channel-vtable',instruction=f'virtual slot {slot}'))
    (ROOT/f'analysis/{prefix}-oracle-fixups.json').write_text(json.dumps(dict(reference_sha256=expected,test_only=True,fixups=fixups),indent=2)+'\n',encoding='utf-8')
    lines=['// Generated by build_sprite_oracle_fixups.py; test-only data operand addresses.']
    lines += [f'{{0x{row["rva"]:08x}U,0x{row["original_address"]:08x}U}},' for row in fixups]
    (ROOT/'tools'/f'{prefix.replace("-","_")}_oracle_fixups.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print(f'Recorded {len(fixups)} test-only operand fixups')
if __name__=='__main__': main()
