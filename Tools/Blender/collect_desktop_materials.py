import bpy,json
from pathlib import Path
root=Path(__file__).resolve().parents[2]
result={}
for entry in json.loads((root/'AssetInbox/Environment/Stores/Desktop/manifest.json').read_text(encoding='utf-8'))['assets']:
    asset=entry['id']
    folder=root/'AssetInbox/Environment/Stores/Desktop'/asset
    info=json.loads((folder/'equipment.json').read_text(encoding='utf-8'))
    bpy.ops.wm.open_mainfile(filepath=str(folder/info['sourceBlend']))
    mesh=bpy.data.objects.get(Path(info['mesh']).stem)
    result[asset]=[]
    palette=[]
    for m in mesh.data.materials:
        shader=next((n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED'),None) if m.use_nodes else None
        data=dict(name=m.name,color=list(shader.inputs['Base Color'].default_value) if shader else list(m.diffuse_color),roughness=shader.inputs['Roughness'].default_value if shader else .5,metallic=shader.inputs['Metallic'].default_value if shader else 0)
        if shader:
            for socket,key in (('Base Color','baseTexture'),('Emission Color','emissionTexture')):
                links=shader.inputs[socket].links
                if links and links[0].from_node.type=='TEX_IMAGE' and links[0].from_node.image:
                    im=links[0].from_node.image
                    candidates=list((folder/'Textures').glob(Path(im.filepath).name)) or list(folder.rglob(Path(im.filepath).name))
                    if candidates:data[key]=str(candidates[0].relative_to(root)).replace('\\','/')
        palette.append(data)
        result[asset].append(dict(name=m.name,color=list(m.diffuse_color),nodes=[dict(type=n.type,name=n.name,inputs={s.name:list(s.default_value) if hasattr(s.default_value,'__len__') else s.default_value for s in n.inputs if hasattr(s,'default_value') and s.type in ('RGBA','VALUE')}) for n in m.node_tree.nodes] if m.use_nodes else []))
    info['sourceMaterialPalette']=palette
    (folder/'equipment.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
(root/'Saved/produce-materials.json').write_text(json.dumps(result,indent=2))
