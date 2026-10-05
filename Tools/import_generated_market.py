"""Import the generated HQ textured market into a separate Unreal asset folder."""
from pathlib import Path
import json
import unreal
root = Path(unreal.Paths.project_dir()).resolve()
source = root / 'Saved/ImageBlaster/UnrealImport/mahalle-market-textured.glb'
if not source.exists():
    raise RuntimeError('HQ textured GLB is not ready.')
target = '/Game/Stores/Generated/MahalleMarket'
existing = unreal.EditorAssetLibrary.list_assets(target, recursive=True, include_folder=False) if unreal.EditorAssetLibrary.does_directory_exist(target) else []
if not any(isinstance(unreal.load_asset(p), unreal.StaticMesh) for p in existing):
    task = unreal.AssetImportTask()
    for key, value in dict(filename=str(source), destination_path=target, automated=True, replace_existing=False, save=True).items():
        task.set_editor_property(key, value)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
paths = unreal.EditorAssetLibrary.list_assets(target, recursive=True, include_folder=False)
meshes = []
for path in paths:
    asset = unreal.load_asset(path)
    if isinstance(asset, unreal.StaticMesh):
        body = asset.get_editor_property('body_setup')
        body.set_editor_property('collision_trace_flag', unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        meshes.append(asset.get_path_name())
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
if not meshes:
    raise RuntimeError('No static mesh imported from HQ GLB.')
for path in meshes:
    mesh = unreal.load_asset(path)
    for index, slot in enumerate(mesh.get_editor_property('static_materials')):
        original = slot.get_editor_property('material_interface')
        textures = []
        if isinstance(original, unreal.MaterialInstanceConstant):
            for name in unreal.MaterialEditingLibrary.get_texture_parameter_names(original):
                if any(word in str(name).lower() for word in ('basecolor', 'base_color', 'albedo', 'diffuse')):
                    texture = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(original, name)
                    if texture:
                        textures.append(texture)
        elif isinstance(original, unreal.Material):
            textures = list(unreal.MaterialEditingLibrary.get_used_textures(original))
            color = [t for t in textures if any(w in t.get_name().lower() for w in ('basecolor', 'base_color', 'albedo', 'diffuse'))]
            textures = color or textures
        if len(textures) != 1:
            unreal.log_warning(f'GENERATED_MATERIAL_REVIEW: {original}, textures={len(textures)}')
            continue
        name = 'M_Unlit_' + mesh.get_name() + '_' + str(index)
        material = unreal.load_asset(target + '/' + name)
        if not material:
            material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, target, unreal.Material, unreal.MaterialFactoryNew())
            material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
            material.set_editor_property('two_sided', True)
            sample = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureSample, 0, 0)
            sample.set_editor_property('texture', textures[0])
            unreal.MaterialEditingLibrary.connect_material_property(sample, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.MaterialEditingLibrary.recompile_material(material)
            unreal.EditorAssetLibrary.save_loaded_asset(material)
        mesh.set_material(index, material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
inspection = json.loads((root / 'Saved/ImageBlaster/textured-inspection.json').read_text(encoding='utf-8'))
floor = inspection['floor_at_source_camera_m']
if floor is None:
    raise RuntimeError('No floor below source camera; inspect mesh before walkthrough.')
collision_meshes = json.loads((root / 'Saved/ImageBlaster/collision-assets.json').read_text(encoding='utf-8'))
manifest = dict(meshes=meshes, collision_meshes=collision_meshes, floor_cm=floor*100, source=str(source))
(root / 'Saved/ImageBlaster/unreal-manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
unreal.EditorAssetLibrary.save_directory(target, only_if_is_dirty=True, recursive=True)
unreal.log('GENERATED_MARKET_IMPORTED=' + json.dumps(manifest))
