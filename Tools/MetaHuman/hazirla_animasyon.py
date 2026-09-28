"""Create the shared Miras Market MetaHuman IK retargeter and animation copies.

Run through METAHUMAN_ANIMASYON_HAZIRLA.cmd while the Unreal Editor is closed.
The script is idempotent: existing assets are updated and animation copies are overwritten.
"""

import unreal


ROOT = "/Game/MetaHumans/Animasyon"
SOURCE_MESH_PATH = "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"
TARGET_MESH_PATH = "/Game/MetaHumans/MH_Teyze/Body/SKM_NewMetaHumanCharacter_BodyMesh"
ANIMATION_PATHS = (
    "/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd",
    "/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle",
)


def require(path):
    value = unreal.load_asset(path)
    if not value:
        raise RuntimeError(f"Gerekli varlik bulunamadi: {path}")
    return value


def load_or_create_ik(name, mesh):
    path = f"{ROOT}/{name}"
    rig = unreal.load_asset(path)
    if not rig:
        rig = unreal.IKRigDefinitionFactory.create_new_ik_rig_asset(ROOT, name)
    if not rig:
        raise RuntimeError(f"IK Rig olusturulamadi: {path}")
    controller = unreal.IKRigController.get_controller(rig)
    controller.set_skeletal_mesh(mesh)
    if not controller.apply_auto_generated_retarget_definition():
        raise RuntimeError(f"Otomatik IK zincirleri olusturulamadi: {path}")
    unreal.EditorAssetLibrary.save_loaded_asset(rig)
    return rig


def load_or_create_retargeter(source_rig, target_rig, source_mesh, target_mesh):
    name = "RTG_Mannequin_MarketMetaHuman"
    path = f"{ROOT}/{name}"
    retargeter = unreal.load_asset(path)
    if not retargeter:
        tools = unreal.AssetToolsHelpers.get_asset_tools()
        retargeter = tools.create_asset(name, ROOT, unreal.IKRetargeter, unreal.IKRetargetFactory())
    if not retargeter:
        raise RuntimeError(f"IK Retargeter olusturulamadi: {path}")

    controller = unreal.IKRetargeterController.get_controller(retargeter)
    controller.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, source_rig)
    controller.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, target_rig)
    controller.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, source_mesh)
    controller.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, target_mesh)
    controller.remove_all_ops()
    controller.add_default_ops()
    controller.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
    unreal.EditorAssetLibrary.save_loaded_asset(retargeter)
    return retargeter


def main():
    unreal.EditorAssetLibrary.make_directory(ROOT)
    source_mesh = require(SOURCE_MESH_PATH)
    target_mesh = require(TARGET_MESH_PATH)
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    animations = []
    for path in ANIMATION_PATHS:
        require(path)
        name = path.rsplit("/", 1)[-1]
        asset_data = registry.get_asset_by_object_path(f"{path}.{name}")
        if not asset_data or not asset_data.is_valid():
            raise RuntimeError(f"Animasyon kaydi bulunamadi: {path}")
        animations.append(asset_data)

    source_rig = load_or_create_ik("IK_Mannequin", source_mesh)
    target_rig = load_or_create_ik("IK_MarketMetaHuman", target_mesh)
    retargeter = load_or_create_retargeter(source_rig, target_rig, source_mesh, target_mesh)

    created = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        animations,
        source_mesh,
        target_mesh,
        retargeter,
        target_path=ROOT,
        include_referenced_assets=False,
        overwrite_existing_files=True,
    )
    if len(created) != len(animations):
        raise RuntimeError(f"Animasyon donusumu eksik: {len(created)}/{len(animations)}")
    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    names = ", ".join(str(asset.asset_name) for asset in created)
    unreal.log(f"MIRAS_METAHUMAN_ANIMATION_OK={names}")


main()
