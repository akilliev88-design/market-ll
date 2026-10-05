"""Keep the generated low-resolution collision separate from the visible HQ mesh."""
from pathlib import Path
import json
import unreal
root = Path(unreal.Paths.project_dir()).resolve()
source = root / 'Saved/ImageBlaster/image-blaster/worlds/mahalle-market/output/world/0-world.glb'
target = '/Game/Stores/Generated/MahalleCollision'
existing = unreal.EditorAssetLibrary.list_assets(target, recursive=True, include_folder=False) if unreal.EditorAssetLibrary.does_directory_exist(target) else []
if not any(isinstance(unreal.load_asset(p), unreal.StaticMesh) for p in existing):
    task = unreal.AssetImportTask()
    for key, value in dict(filename=str(source), destination_path=target, automated=True, replace_existing=False, save=True).items():
        task.set_editor_property(key, value)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
meshes = []
for path in unreal.EditorAssetLibrary.list_assets(target, recursive=True, include_folder=False):
    asset = unreal.load_asset(path)
    if isinstance(asset, unreal.StaticMesh):
        asset.get_editor_property('body_setup').set_editor_property('collision_trace_flag', unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
        meshes.append(asset.get_path_name())
if not meshes:
    raise RuntimeError('Collision GLB did not import.')
(root / 'Saved/ImageBlaster/collision-assets.json').write_text(json.dumps(meshes, indent=2), encoding='utf-8')
unreal.log('GENERATED_COLLISION_IMPORTED=' + json.dumps(meshes))
