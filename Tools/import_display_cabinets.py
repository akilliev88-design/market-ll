"""Import only the reference-inspired cabinet library, preserving other authored assets."""
from pathlib import Path
import json,unreal
root=Path(unreal.Paths.project_dir()).resolve()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
unreal.SystemLibrary.execute_console_command(world,'Interchange.FeatureFlags.Import.FBX 0')
assets=unreal.AssetToolsHelpers.get_asset_tools()
for p in sorted((root/'AssetInbox/Environment/Stores').rglob('equipment.json')):
    data=json.loads(p.read_text(encoding='utf-8'))
    if 'reference' not in data:continue
    name=Path(data['mesh']).stem;target='/Game/Stores/Equipment'
    options=unreal.FbxImportUI()
    for k,v in dict(import_mesh=True,import_as_skeletal=False,import_materials=True,import_textures=False,create_physics_asset=False).items():options.set_editor_property(k,v)
    imp=options.get_editor_property('static_mesh_import_data')
    for k,v in dict(combine_meshes=True,auto_generate_collision=False,one_convex_hull_per_ucx=True,convert_scene=True,convert_scene_unit=True,transform_vertex_to_absolute=True).items():imp.set_editor_property(k,v)
    task=unreal.AssetImportTask()
    for k,v in dict(filename=str(p.parent/data['mesh']),destination_path=target,destination_name=name,automated=True,replace_existing=True,replace_existing_settings=True,save=True,options=options).items():task.set_editor_property(k,v)
    assets.import_asset_tasks([task]);mesh=unreal.load_asset(target+'/'+name)
    assert mesh,name
    agg=mesh.get_editor_property('body_setup').get_editor_property('agg_geom');count=sum(len(agg.get_editor_property(k)) for k in ('box_elems','sphere_elems','sphyl_elems','convex_elems'))
    assert count>=data['collision']['pieces'],(name,count)
    for slot in mesh.get_editor_property('static_materials'):
        mat=slot.get_editor_property('material_interface')
        if isinstance(mat,unreal.Material):
            unreal.MaterialEditingLibrary.set_material_usage(mat,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
            if 'Glass' in mat.get_name():
                mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT);mat.set_editor_property('two_sided',True)
                opacity=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant)
                opacity.set_editor_property('r',.16);unreal.MaterialEditingLibrary.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
                rough=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.08);unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
            unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh);unreal.log('SIM_CABINET_IMPORTED='+name+f' UCX={count}')
unreal.EditorAssetLibrary.save_directory('/Game/Stores',only_if_is_dirty=True,recursive=True)
unreal.log('SIM_CABINET_IMPORT_COMPLETE')
