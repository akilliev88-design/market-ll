"""Import Blender-authored environment assets into Unreal Engine."""

from pathlib import Path

import unreal


PROJECT_ROOT = Path(unreal.Paths.project_dir()).resolve()
SOURCE = PROJECT_ROOT / "AssetInbox" / "Environment" / "Shelves" / "Gondola_1200" / "SM_Gondola_1200.fbx"
DESTINATION = "/Game/Environment/Shelves/Gondola_1200"


def main():
    if not SOURCE.exists():
        raise RuntimeError(f"Environment source is missing: {SOURCE}")

    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("create_physics_asset", False)
    static_options = options.get_editor_property("static_mesh_import_data")
    static_options.set_editor_property("combine_meshes", True)
    static_options.set_editor_property("generate_lightmap_u_vs", True)
    # UE 5.8 Interchange maps this legacy flag to the master "Import Collisions"
    # switch, so it must stay enabled even when custom UCX meshes are supplied.
    static_options.set_editor_property("auto_generate_collision", True)
    static_options.set_editor_property("one_convex_hull_per_ucx", True)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(SOURCE))
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("destination_name", "SM_Gondola_1200")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", options)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = list(task.get_editor_property("imported_object_paths"))
    mesh_path = f"{DESTINATION}/SM_Gondola_1200.SM_Gondola_1200"
    mesh = unreal.load_asset(mesh_path)
    if mesh is None:
        raise RuntimeError(f"Static mesh was not imported. Objects: {imported}")
    unreal.EditorAssetLibrary.save_directory(DESTINATION, only_if_is_dirty=False, recursive=True)
    unreal.log(f"MIRAS_ENVIRONMENT_IMPORTED={mesh_path}")


main()
