#!/usr/bin/env python3
"""Inventory shipped XML against explicitly unrecovered optional graphics branches."""
import datetime
import hashlib
import xml.etree.ElementTree as ET
import zipfile
from record_live_production_verification import ROOT,digest,read,write


def main():
    data=ROOT.parent/'farmář/Dobrý farmář/Data';entries={};archives={}
    for name in ['Data.pack','cz.pack']:
        archives[name]=digest(data/name)
        with zipfile.ZipFile(data/name) as archive:
            for info in archive.infolist():
                if not info.is_dir():entries[info.filename.lower()]=(name,info.filename,archive.read(info))
    mesh=[];animation=[];masks=[];xmls={};errors=[]
    for key,(archive,name,body) in sorted(entries.items()):
        if not key.endswith('.xml'):continue
        try:root=ET.fromstring(body)
        except ET.ParseError as error:errors.append(dict(archive=archive,entry=name,error=str(error)));continue
        xmls[key]=dict(archive=archive,entry=name,sha256=hashlib.sha256(body).hexdigest())
        for node in root.iter():
            for attribute in ['Mesh','Animation']:
                if node.get(attribute):
                    (mesh if attribute=='Mesh' else animation).append(dict(entry=name,tag=node.tag,value=node.get(attribute)))
            if node.get('texture_mask'):
                target=node.get('texture_mask').lower();source,entry,raw=entries[target]
                kind='JNG' if raw.startswith(b'\x8bJNG') else 'TGA32' if target.endswith('.tga') and len(raw)>18 and raw[2]==2 and raw[16]==32 else 'other'
                masks.append(dict(xml=name,tag=node.tag,entry=entry,archive=source,format=kind,sha256=hashlib.sha256(raw).hexdigest()))
    assert not errors,errors
    assert not mesh and not animation,(mesh,animation)
    assert masks and all(row['format'] in ['JNG','TGA32'] for row in masks)
    sources=['src/game/graphics/ClickMask.cpp','src/game/resources/AnimableAppearance.cpp',
        'src/game/ui/Control.cpp','src/game/graphics/NativeImage.cpp','src/game/graphics/ImageTexture.cpp']
    result=dict(status='passed_shipped_xml_uses_no_mesh_animation_factories_and_only_supported_click_masks',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        archives=archives,effective_xml_count=len(xmls),effective_xml_files=xmls,parse_errors=errors,
        nonempty_mesh_attributes=mesh,nonempty_animation_attributes=animation,click_mask_references=masks,
        implementation_files={name:digest(ROOT/name) for name in sources},production_changed=False,
        explicitly_unrecovered_optional_branches=['3D Mesh/Animation factories and rendering',
            'Non-BGRA native click-mask D3DX conversion','Native image types outside reconstructed formats',
            'D3DX texture-format fallback on incompatible graphics devices'],
        optional_branches_claimed_implemented=False,whole_game_complete=False)
    write(ROOT/'analysis/original-resource-scope-audit.json',result)
    print('Shipped XML:',len(xmls),'supported click masks:',len(masks),'Mesh/Animation:',len(mesh),len(animation))


if __name__=='__main__':main()
