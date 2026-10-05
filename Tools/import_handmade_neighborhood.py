"""Run with Unreal Editor Python; imports only the handmade neighborhood shell."""
from pathlib import Path
import json
import unreal
root=Path(unreal.Paths.project_dir()).resolve()
folder=root/'AssetInbox/Environment/Stores/HandmadeNeighborhood'
meta=json.loads((folder/'equipment.json').read_text())
name=Path(meta['mesh']).stem
target='/Game/Stores/Handmade/Neighborhood'
unreal.SystemLibrary.execute_console_command(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'Interchange.FeatureFlags.Import.FBX 0')
options=unreal.FbxImportUI()
for k,v in dict(import_mesh=True,import_as_skeletal=False,import_materials=True,import_textures=False,create_physics_asset=False).items(): options.set_editor_property(k,v)
data=options.get_editor_property('static_mesh_import_data')
for k,v in dict(combine_meshes=True,generate_lightmap_u_vs=True,auto_generate_collision=False,one_convex_hull_per_ucx=True,convert_scene=True,convert_scene_unit=True,transform_vertex_to_absolute=True).items(): data.set_editor_property(k,v)
task=unreal.AssetImportTask()
for k,v in dict(filename=str(folder/meta['mesh']),destination_path=target,destination_name=name,automated=True,replace_existing=True,replace_existing_settings=True,save=True,options=options).items(): task.set_editor_property(k,v)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(target+'/'+name)
if not mesh: raise RuntimeError('Handmade shell import missing')
agg=mesh.get_editor_property('body_setup').get_editor_property('agg_geom')
count=sum(len(agg.get_editor_property(k)) for k in ('box_elems','sphere_elems','sphyl_elems','convex_elems'))
if count!=meta['collision']['pieces']: raise RuntimeError(f'Collision mismatch {count} vs {meta["collision"]["pieces"]}')
for sm in mesh.get_editor_property('static_materials'):
    m=sm.get_editor_property('material_interface')
    if isinstance(m,unreal.Material):
        unreal.MaterialEditingLibrary.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
        unreal.MaterialEditingLibrary.recompile_material(m)
        unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
(folder/'unreal-import.json').write_text(json.dumps({'mesh':mesh.get_path_name(),'collisionPieces':count},indent=2))
unreal.log('HANDMADE_IMPORT_PASSED hulls='+str(count))
