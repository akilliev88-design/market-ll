"""Unreal editor Python: import new G-088 assets with custom collision and HISM material usage."""
from pathlib import Path
import json
import unreal
root=Path(unreal.Paths.project_dir()).resolve()
# FbxImportUI is the legacy factory's configuration. Interchange in 5.8 ignored its UCX settings.
# This switch is local to this editor process, not a change to the project's engine settings.
unreal.SystemLibrary.execute_console_command(
    unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),
    'Interchange.FeatureFlags.Import.FBX 0')
for p in sorted((root/'AssetInbox/Environment/Stores').rglob('equipment.json')):
    obj=json.loads(p.read_text(encoding='utf-8'))
    name=Path(obj['mesh']).stem
    target='/Game/Stores/Equipment' if 'planogram' in obj else '/Game/Stores/Shells'
    # A fresh generated mesh avoids preserving collision from an earlier import with different units.
    if unreal.EditorAssetLibrary.does_asset_exist(target+'/'+name):
        if not unreal.EditorAssetLibrary.delete_asset(target+'/'+name): raise RuntimeError('Could not replace generated mesh: '+name)
    options=unreal.FbxImportUI()
    for k,v in dict(import_mesh=True,import_as_skeletal=False,import_materials=True,import_textures=False,create_physics_asset=False).items(): options.set_editor_property(k,v)
    data=options.get_editor_property('static_mesh_import_data')
    for k,v in dict(combine_meshes=True,generate_lightmap_u_vs=True,auto_generate_collision=False,one_convex_hull_per_ucx=True,convert_scene=True,convert_scene_unit=True,transform_vertex_to_absolute=True).items(): data.set_editor_property(k,v)
    task=unreal.AssetImportTask()
    for k,v in dict(filename=str(p.parent/obj['mesh']),destination_path=target,destination_name=name,automated=True,replace_existing=True,replace_existing_settings=True,save=True,options=options).items(): task.set_editor_property(k,v)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=unreal.load_asset(target+'/'+name)
    if not mesh: raise RuntimeError('Import missing: '+name)
    aggregate=mesh.get_editor_property('body_setup').get_editor_property('agg_geom')
    count=sum(len(aggregate.get_editor_property(k)) for k in ('box_elems','sphere_elems','sphyl_elems','convex_elems'))
    if count<obj['collision']['pieces']:
        raise RuntimeError(f'UCX collision missing: {name}, {count} < {obj["collision"]["pieces"]}')
    for sm in mesh.get_editor_property('static_materials'):
        material=sm.get_editor_property('material_interface')
        if isinstance(material,unreal.Material):
            unreal.MaterialEditingLibrary.set_material_usage(material,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
            unreal.MaterialEditingLibrary.recompile_material(material)
            unreal.EditorAssetLibrary.save_loaded_asset(material)
    unreal.log('SIM_STORE_IMPORTED='+mesh.get_path_name()+f', UCX={count}')
unreal.EditorAssetLibrary.save_directory('/Game/Stores',only_if_is_dirty=True,recursive=True)
