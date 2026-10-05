"""Import standalone deliveries into isolated per-model packages, preserving existing assets."""
from pathlib import Path
import json, unreal
root=Path(unreal.Paths.project_dir()).resolve()
base=root/'AssetInbox/Environment/Stores/Desktop'
report=json.loads((base/'geometry-audit.json').read_text())
if report['completed']!=report['total'] or any(x['overlapPairsAfter'] for x in report['assets']):raise RuntimeError('Source geometry audit incomplete')
unreal.SystemLibrary.execute_console_command(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'Interchange.FeatureFlags.Import.FBX 0')
done=[]
for entry in json.loads((base/'manifest.json').read_text(encoding='utf-8'))['assets']:
    folder=root/entry['folder'];info=json.loads((folder/'equipment.json').read_text(encoding='utf-8'))
    name=Path(info['mesh']).stem;target='/Game/Stores/Desktop/'+entry['id']
    options=unreal.FbxImportUI()
    for k,v in dict(import_mesh=True,import_as_skeletal=False,import_materials=True,import_textures=True,create_physics_asset=False).items():options.set_editor_property(k,v)
    data=options.static_mesh_import_data
    for k,v in dict(combine_meshes=True,generate_lightmap_u_vs=True,auto_generate_collision=False,one_convex_hull_per_ucx=True,convert_scene=True,convert_scene_unit=True,transform_vertex_to_absolute=True).items():data.set_editor_property(k,v)
    task=unreal.AssetImportTask()
    for k,v in dict(filename=str(folder/info['mesh']),destination_path=target,destination_name=name,automated=True,replace_existing=True,replace_existing_settings=True,save=True,options=options).items():task.set_editor_property(k,v)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=unreal.load_asset(target+'/'+name)
    if not mesh:raise RuntimeError('Missing '+name)
    agg=mesh.get_editor_property('body_setup').get_editor_property('agg_geom')
    count=sum(len(agg.get_editor_property(k)) for k in ('box_elems','sphere_elems','sphyl_elems','convex_elems'))
    if count!=info['collision']['pieces']:raise RuntimeError(f'Collision mismatch {name}: {count} vs {info["collision"]["pieces"]}')
    for slot in mesh.get_editor_property('static_materials'):
        m=slot.get_editor_property('material_interface')
        if not isinstance(m,unreal.Material):continue
        m.set_editor_property('used_with_instanced_static_meshes',True)
        n=m.get_name().lower()
        if any(word in n for word in ('glass','clear','acrylic')):
            m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT);m.set_editor_property('two_sided',True)
            for prop,value in ((unreal.MaterialProperty.MP_OPACITY,.14),(unreal.MaterialProperty.MP_ROUGHNESS,.08)):
                node=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant)
                node.set_editor_property('r',value);unreal.MaterialEditingLibrary.connect_material_property(node,'',prop)
        unreal.MaterialEditingLibrary.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    done.append(dict(id=entry['id'],mesh=mesh.get_path_name(),collisionPieces=count))
    (base/'unreal-import.json').write_text(json.dumps(dict(assets=done,completed=len(done),total=len(report['assets'])),indent=2))
    unreal.log('DESKTOP_IMPORTED '+entry['id'])
unreal.log('DESKTOP_IMPORT_PASSED='+str(len(done)))
