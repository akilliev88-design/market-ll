"""Copy standalone desktop deliveries into the project, without modifying deliveries."""
import json, shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DESKTOP=ROOT.parent
OUT=ROOT/'AssetInbox/Environment/Stores/Desktop'
PACKS=[('Miras_Blender_Ekipman_2026-10-03','Yeni_Modeller'),
       ('Miras_Manav_Modelleri_2026-10-04','Models'),
       ('Miras_Magaza_Araclari_2026-10-04','Models'),
       ('Miras_Square_POS_Modeller_2026-10-04','Modeller'),
       ('Miras_Self_Servis_Kasa_Varyantlari_2026-10-04','Modeller'),
       ('Miras_Kantarci_Alternatif_Modeller_2026-10-04','Yeni_Modeller')]
def prepare():
    entries=[]
    for pack,sub in PACKS:
        parent=DESKTOP/pack/sub
        if not parent.exists(): raise RuntimeError('Missing delivery '+str(parent))
        for source in sorted(parent.iterdir()):
            if not source.is_dir(): continue
            metadata=source/'equipment.json'
            if not metadata.exists(): metadata=source/'metadata.json'
            if not metadata.exists(): continue
            info=json.loads(metadata.read_text(encoding='utf-8'))
            asset_id=info.get('id',source.name)
            target=OUT/asset_id
            if target.exists():
                saved=json.loads((target/'equipment.json').read_text(encoding='utf-8'))
                if saved.get('deliverySource')!=str(source): raise RuntimeError('Conflicting source '+asset_id)
                entries.append(dict(id=asset_id,folder=str(target.relative_to(ROOT)).replace('\\','/'),source=str(source),mesh=saved['unrealMesh']))
                continue
            shutil.copytree(source,target,ignore=shutil.ignore_patterns('*.blend1','*.blend2'))
            fbx=next(p for p in target.glob('*.fbx') if 'lod' not in p.stem.lower())
            blend=next(target.rglob('*.blend'))
            family=info.get('family','produce' if 'fillZones' in info else 'prop')
            canonical=dict(info, id=asset_id, family=family, mesh=fbx.name,
                unrealMesh=f'/Game/Stores/Desktop/{asset_id}/{fbx.stem}.{fbx.stem}',
                checkouts=info.get('checkouts',1 if asset_id.startswith('checkout_') and 'impulse' not in asset_id and 'tower' not in asset_id else 0))
            canonical.setdefault('displayName',asset_id)
            canonical.setdefault('zones',[])
            canonical.setdefault('planogram',dict(doubleSided=False,widthCm=info['dimensionsMm']['width']/10,depthCm=info['dimensionsMm']['depth']/10,frontY=0,meshYaw=180,showCategorySign=False))
            canonical['deliverySource']=str(source)
            canonical['sourceBlend']=str(blend.relative_to(target)).replace('\\','/')
            canonical['staticPlacement']=True
            (target/'equipment.json').write_text(json.dumps(canonical,ensure_ascii=False,indent=2),encoding='utf-8')
            entries.append(dict(id=asset_id,folder=str(target.relative_to(ROOT)).replace('\\','/'),source=str(source),mesh=canonical['unrealMesh']))
    (OUT/'manifest.json').write_text(json.dumps(dict(assets=entries),ensure_ascii=False,indent=2),encoding='utf-8')
    print('DESKTOP_ASSETS_PREPARED='+str(len(entries)))
if __name__=='__main__': prepare()
