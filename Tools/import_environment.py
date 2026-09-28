"""Import Blender-authored environment assets into Unreal Engine."""

from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.project_dir()).resolve()
ASSETS = [
    (PROJECT_ROOT / "AssetInbox" / "Environment" / "Shelves" / "Gondola_1200" / "SM_Gondola_1200.fbx", "/Game/Environment/Shelves/Gondola_1200", "SM_Gondola_1200"),
    (PROJECT_ROOT / "AssetInbox" / "Environment" / "StoreKit" / "WallShelf_2400" / "SM_WallShelf_2400.fbx", "/Game/Environment/StoreKit/WallShelf_2400", "SM_WallShelf_2400"),
    (PROJECT_ROOT / "AssetInbox" / "Environment" / "StoreKit" / "BulkIsland_1600" / "SM_BulkIsland_1600.fbx", "/Game/Environment/StoreKit/BulkIsland_1600", "SM_BulkIsland_1600"),
    (PROJECT_ROOT / "AssetInbox" / "Environment" / "StoreKit" / "CeilingBay_6000" / "SM_CeilingBay_6000.fbx", "/Game/Environment/StoreKit/CeilingBay_6000", "SM_CeilingBay_6000"),
]


def main():
    for source, destination, name in ASSETS:
        if not source.exists():
            raise RuntimeError(f"Environment source is missing: {source}")
        options = unreal.FbxImportUI()
        options.set_editor_property("import_mesh", True)
        options.set_editor_property("import_as_skeletal", False)
        options.set_editor_property("import_materials", True)
        options.set_editor_property("import_textures", False)
        options.set_editor_property("create_physics_asset", False)
        static_options = options.get_editor_property("static_mesh_import_data")
        static_options.set_editor_property("combine_meshes", True)
        static_options.set_editor_property("generate_lightmap_u_vs", True)
        static_options.set_editor_property("auto_generate_collision", True)
        static_options.set_editor_property("one_convex_hull_per_ucx", True)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source))
        task.set_editor_property("destination_path", destination)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("replace_existing_settings", True)
        task.set_editor_property("save", True)
        task.set_editor_property("options", options)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        imported = list(task.get_editor_property("imported_object_paths"))
        mesh_path = f"{destination}/{name}.{name}"
        if unreal.load_asset(mesh_path) is None:
            raise RuntimeError(f"Static mesh was not imported: {mesh_path}. Objects: {imported}")
        unreal.EditorAssetLibrary.save_directory(destination, only_if_is_dirty=False, recursive=True)
        unreal.log(f"MIRAS_ENVIRONMENT_IMPORTED={mesh_path}")


main()
