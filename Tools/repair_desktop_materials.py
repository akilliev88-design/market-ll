"""Rebuild shaders with authored colors and explicitly imported UV textures.

Legacy FBX embedded textures left null TextureSample references. Rebuild their
simple Principled surfaces; preserve source meshes, UVs, colors and roughness.
"""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
base=root/'AssetInbox/Environment/Stores/Desktop'
for entry in json.loads((base/'manifest.json').read_text(encoding='utf-8'))['assets']:
    info=json.loads((root/entry['folder']/'equipment.json').read_text(encoding='utf-8'))
    mesh=unreal.load_asset(info['unrealMesh']);target='/Game/Stores/Desktop/'+entry['id']
    for slot,p in zip(mesh.get_editor_property('static_materials'),info['sourceMaterialPalette']):
        m=slot.get_editor_property('material_interface')
        if not isinstance(m,unreal.Material):continue
        unreal.MaterialEditingLibrary.delete_all_material_expressions(m)
        m.set_editor_property('used_with_instanced_static_meshes',True)
        def constant(prop,value):
            n=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant)
            n.set_editor_property('r',value);unreal.MaterialEditingLibrary.connect_material_property(n,'',prop)
        def texture(key,prop):
            task=unreal.AssetImportTask();name='T_'+Path(p[key]).stem
            for k,v in dict(filename=str(root/p[key]),destination_path=target,destination_name=name,automated=True,replace_existing=True,save=True).items():task.set_editor_property(k,v)
            unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
            tex=unreal.load_asset(target+'/'+name)
            if not tex:raise RuntimeError('Texture missing '+p[key])
            n=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionTextureSample)
            n.set_editor_property('texture',tex);unreal.MaterialEditingLibrary.connect_material_property(n,'RGB',prop)
        if p.get('baseTexture'):texture('baseTexture',unreal.MaterialProperty.MP_BASE_COLOR)
        else:
            n=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector)
            n.set_editor_property('constant',unreal.LinearColor(*p['color']))
            unreal.MaterialEditingLibrary.connect_material_property(n,'',unreal.MaterialProperty.MP_BASE_COLOR)
        if p.get('emissionTexture'):texture('emissionTexture',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        constant(unreal.MaterialProperty.MP_ROUGHNESS,p['roughness']);constant(unreal.MaterialProperty.MP_METALLIC,p['metallic'])
        glass=any(word in m.get_name().lower() for word in ('glass','clear','acrylic'))
        if glass:
            m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT);m.set_editor_property('two_sided',True)
            constant(unreal.MaterialProperty.MP_OPACITY,.14)
        unreal.MaterialEditingLibrary.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.log('DESKTOP_MATERIAL_PALETTE_PASSED=132')
