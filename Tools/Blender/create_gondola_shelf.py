"""Create the first production-ready modular store fixture for Miras Market.

Run:
  blender --background --python Tools/Blender/create_gondola_shelf.py -- <project-root>
"""

import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


ASSET_ID = "gondola_double_1200"
MESH_NAME = "SM_Gondola_1200"


def project_root() -> Path:
    args = sys.argv
    if "--" in args and len(args) > args.index("--") + 1:
        return Path(args[args.index("--") + 1]).resolve()
    return Path.cwd().resolve()


ROOT = project_root()
ASSET_DIR = ROOT / "AssetInbox" / "Environment" / "Shelves" / "Gondola_1200"
SOURCE_DIR = ASSET_DIR / "Source"
PREVIEW_DIR = ROOT / "Docs" / "Images"


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.materials, bpy.data.cameras, bpy.data.lights):
        for block in list(datablocks):
            if block.users == 0:
                datablocks.remove(block)


def material(name, color, metallic=0.0, roughness=0.45):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    mat.node_tree.nodes.clear()
    shader = mat.node_tree.nodes.new("ShaderNodeBsdfPrincipled")
    output = mat.node_tree.nodes.new("ShaderNodeOutputMaterial")
    shader.location = (-220, 0)
    output.location = (80, 0)
    mat.node_tree.links.new(shader.outputs["BSDF"], output.inputs["Surface"])
    shader.inputs["Base Color"].default_value = (*color, 1.0)
    shader.inputs["Metallic"].default_value = metallic
    shader.inputs["Roughness"].default_value = roughness
    return mat


def cube(name, location, dimensions, mat, bevel=0.006, visible=True):
    bpy.ops.mesh.primitive_cube_add(location=location)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if mat:
        obj.data.materials.append(mat)
    if bevel > 0:
        modifier = obj.modifiers.new("EdgeBevel", "BEVEL")
        modifier.width = bevel
        modifier.segments = 3
        modifier.limit_method = "ANGLE"
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj["miras_export"] = visible
    return obj


def join_visible(objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    model = bpy.context.object
    model.name = MESH_NAME
    model.data.name = MESH_NAME
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR", center="MEDIAN")
    model["equipment_id"] = ASSET_ID
    model["dimensions_mm"] = "1200,900,1600"
    model["customer_faces"] = "+Y,-Y"
    return model


def look_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def build_model():
    painted = material("MI_Shelf_PaintedMetal", (0.055, 0.062, 0.060), 0.72, 0.30)
    back = material("MI_Shelf_BackPanel", (0.12, 0.13, 0.12), 0.55, 0.42)
    wood = material("MI_Shelf_WarmWood", (0.24, 0.075, 0.026), 0.0, 0.34)
    rail = material("MI_Shelf_PriceRail", (0.86, 0.30, 0.035), 0.05, 0.28)
    rubber = material("MI_Shelf_Rubber", (0.012, 0.014, 0.013), 0.0, 0.70)

    visible = []
    visible.append(cube("BackPanel", (0, 0, 0.86), (1.10, 0.035, 1.36), back, 0.004))
    visible.append(cube("TopCap", (0, 0, 1.575), (1.20, 0.10, 0.05), painted, 0.009))

    for x in (-0.575, 0.575):
        visible.append(cube("Upright", (x, 0, 0.80), (0.05, 0.075, 1.55), painted, 0.005))
        for z in (0.08, 0.20, 0.32, 0.44, 0.56, 0.68, 0.80, 0.92, 1.04, 1.16, 1.28, 1.40):
            visible.append(cube("UprightSlot", (x, -0.041, z), (0.012, 0.009, 0.035), rubber, 0.002))
            visible.append(cube("UprightSlot", (x, 0.041, z), (0.012, 0.009, 0.035), rubber, 0.002))

    shelf_levels = (0.18, 0.52, 0.86, 1.20)
    for face in (-1, 1):
        y = face * 0.235
        front_y = face * 0.440
        visible.append(cube("BasePlinth", (0, face * 0.25, 0.075), (1.20, 0.40, 0.15), wood, 0.012))
        visible.append(cube("BaseRubber", (0, face * 0.432, 0.035), (1.20, 0.035, 0.07), rubber, 0.006))
        for index, z in enumerate(shelf_levels):
            depth = 0.43 if index == 0 else 0.40
            shelf_y = face * (0.23 if index == 0 else 0.215)
            visible.append(cube("ShelfBoard", (0, shelf_y, z), (1.16, depth, 0.026), painted, 0.008))
            visible.append(cube("PriceRail", (0, front_y, z + 0.038), (1.16, 0.020, 0.070), rail, 0.005))

    # Small top sign carrier: the actual category sign remains data-driven in Unreal.
    for face in (-1, 1):
        visible.append(cube("SignCarrier", (0, face * 0.055, 1.46), (1.08, 0.018, 0.15), wood, 0.006))

    model = join_visible(visible)

    collisions = []
    collision_specs = [
        ((0, 0, 0.80), (1.20, 0.12, 1.60)),
        ((0, -0.25, 0.10), (1.20, 0.40, 0.20)),
        ((0, 0.25, 0.10), (1.20, 0.40, 0.20)),
    ]
    for index, (location, dimensions) in enumerate(collision_specs):
        collision = cube(f"UCX_{MESH_NAME}_{index:02d}", location, dimensions, None, 0, False)
        collision.display_type = "WIRE"
        collision.hide_render = True
        collisions.append(collision)

    return model, collisions


def setup_preview(model):
    ground_mat = material("PreviewFloor", (0.28, 0.16, 0.085), 0.0, 0.24)
    ground = cube("PreviewFloor", (0, 0, -0.035), (5.0, 5.0, 0.06), ground_mat, 0.0, False)

    bpy.ops.object.camera_add(location=(2.6, -3.1, 2.25))
    camera = bpy.context.object
    camera.name = "PreviewCamera"
    camera.data.lens = 52
    look_at(camera, (0, 0, 0.72))
    bpy.context.scene.camera = camera

    def area(name, location, energy, size, color):
        bpy.ops.object.light_add(type="AREA", location=location)
        light = bpy.context.object
        light.name = name
        light.data.energy = energy
        light.data.shape = "RECTANGLE"
        light.data.size = size
        light.data.color = color
        look_at(light, (0, 0, 0.75))

    area("Key", (1.9, -2.2, 3.0), 1150, 2.4, (1.0, 0.83, 0.66))
    area("Fill", (-2.0, -1.0, 1.8), 750, 2.0, (0.66, 0.78, 1.0))
    area("Rim", (0.2, 2.1, 2.6), 950, 1.8, (1.0, 0.72, 0.45))

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 768
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.render.filepath = str(PREVIEW_DIR / "gondola_1200_preview.png")
    scene.world.color = (0.018, 0.022, 0.028)
    scene.view_settings.look = "AgX - Medium High Contrast"
    bpy.ops.render.render(write_still=True)
    return ground, camera


def export_asset(model, collisions):
    ASSET_DIR.mkdir(parents=True, exist_ok=True)
    SOURCE_DIR.mkdir(parents=True, exist_ok=True)
    PREVIEW_DIR.mkdir(parents=True, exist_ok=True)

    bpy.ops.object.select_all(action="DESELECT")
    model.select_set(True)
    for collision in collisions:
        collision.hide_set(False)
        collision.select_set(True)
    bpy.context.view_layer.objects.active = model
    bpy.ops.export_scene.fbx(
        filepath=str(ASSET_DIR / f"{MESH_NAME}.fbx"),
        use_selection=True,
        apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_UNITS",
        axis_forward="-Y",
        axis_up="Z",
        use_mesh_modifiers=True,
        mesh_smooth_type="FACE",
        add_leaf_bones=False,
        bake_anim=False,
    )

    metadata = {
        "schemaVersion": 1,
        "id": ASSET_ID,
        "displayName": "Çift Yüz Gondol Rafı 1200",
        "family": "gondola_double",
        "source": "Blender procedural master",
        "dimensionsMm": {"width": 1200, "depth": 900, "height": 1600},
        "customerFaces": ["front", "back"],
        "frontAxis": "-Y",
        "shelfZones": [
            {"face": face, "level": level, "widthMm": 1120, "depthMm": 370, "heightMm": 310}
            for face in ("front", "back")
            for level in range(4)
        ],
        "anchors": {
            "left": [-600, 0, 0],
            "right": [600, 0, 0],
            "frontEndcap": [0, -450, 0],
            "backEndcap": [0, 450, 0],
        },
        "materials": [
            "MI_Shelf_PaintedMetal",
            "MI_Shelf_BackPanel",
            "MI_Shelf_WarmWood",
            "MI_Shelf_PriceRail",
            "MI_Shelf_Rubber",
        ],
        "collision": "3 UCX boxes",
        "lodPolicy": "LOD0 master; LOD1/LOD2 will be generated during Unreal import",
    }
    (ASSET_DIR / "equipment.json").write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding="utf-8")


def main():
    reset_scene()
    model, collisions = build_model()
    export_asset(model, collisions)
    setup_preview(model)
    bpy.ops.wm.save_as_mainfile(filepath=str(SOURCE_DIR / "Gondola_1200.blend"))
    print(f"MIRAS_ASSET_READY={ASSET_DIR}")


if __name__ == "__main__":
    try:
        main()
    except Exception:
        import traceback
        traceback.print_exc()
        raise SystemExit(1)
