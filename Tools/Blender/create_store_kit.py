"""Build the first realistic modular interior kit for Miras Market in Blender 5.2."""

import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def root():
    args = sys.argv
    return Path(args[args.index("--") + 1]).resolve() if "--" in args else Path.cwd().resolve()


ROOT = root()
BASE = ROOT / "AssetInbox" / "Environment" / "StoreKit"
PREVIEWS = ROOT / "Docs" / "Images" / "StoreKit"


def reset():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for group in (bpy.data.meshes, bpy.data.curves, bpy.data.materials, bpy.data.cameras, bpy.data.lights):
        for block in list(group):
            if block.users == 0:
                group.remove(block)


def mat(name, color, metallic=0.0, roughness=0.45, transmission=0.0, emission=None):
    value = bpy.data.materials.new(name)
    value.use_nodes = True
    value.node_tree.nodes.clear()
    shader = value.node_tree.nodes.new("ShaderNodeBsdfPrincipled")
    output = value.node_tree.nodes.new("ShaderNodeOutputMaterial")
    value.node_tree.links.new(shader.outputs["BSDF"], output.inputs["Surface"])
    shader.inputs["Base Color"].default_value = (*color, 1)
    shader.inputs["Metallic"].default_value = metallic
    shader.inputs["Roughness"].default_value = roughness
    if "Transmission Weight" in shader.inputs:
        shader.inputs["Transmission Weight"].default_value = transmission
    if transmission:
        shader.inputs["Alpha"].default_value = 0.32
        value.surface_render_method = "DITHERED"
    if emission:
        shader.inputs["Emission Color"].default_value = (*emission, 1)
        shader.inputs["Emission Strength"].default_value = 5.0
    return value


def cube(name, location, dimensions, material=None, bevel=0.004, export=True, rotation=(0, 0, 0)):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if material:
        obj.data.materials.append(material)
    if bevel:
        mod = obj.modifiers.new("EdgeBevel", "BEVEL")
        mod.width = bevel
        mod.segments = 2
        mod.limit_method = "ANGLE"
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=mod.name)
    obj["miras_export"] = export
    return obj


def cylinder(name, location, radius, depth, material, rotation=(0, 0, 0), vertices=32):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.data.materials.append(material)
    obj["miras_export"] = True
    return obj


def sphere(name, location, radius, material, scale=(1, 1, 1), segments=16, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, radius=radius, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    obj["miras_export"] = True
    return obj


def join(parts, mesh_name, asset_id, dimensions_mm):
    bpy.ops.object.select_all(action="DESELECT")
    for part in parts:
        part.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = mesh_name
    obj.data.name = mesh_name
    bpy.context.scene.cursor.location = (0, 0, 0)
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR", center="MEDIAN")
    obj["equipment_id"] = asset_id
    obj["dimensions_mm"] = ",".join(str(x) for x in dimensions_mm)
    return obj


def collision(mesh_name, index, location, dimensions):
    obj = cube(f"UCX_{mesh_name}_{index:02d}", location, dimensions, None, 0, False)
    obj.display_type = "WIRE"
    obj.hide_render = True
    return obj


def look_at(obj, point):
    obj.rotation_euler = (Vector(point) - obj.location).to_track_quat("-Z", "Y").to_euler()


def preview(asset_dir, model, camera_location, target):
    floor = mat("MI_PreviewFloor", (0.21, 0.15, 0.10), 0, 0.28)
    cube("PreviewFloor", (0, 0, -0.035), (6, 6, 0.06), floor, 0, False)
    bpy.ops.object.camera_add(location=camera_location)
    camera = bpy.context.object
    camera.data.lens = 52
    look_at(camera, target)
    bpy.context.scene.camera = camera
    for name, location, energy, color, size in (
        ("Key", (2.7, -3.0, 3.4), 1200, (1.0, .82, .64), 2.5),
        ("Fill", (-2.4, -1.2, 2.4), 800, (.62, .78, 1.0), 2.2),
        ("Rim", (0, 2.5, 3.0), 950, (1.0, .66, .38), 2.0),
    ):
        bpy.ops.object.light_add(type="AREA", location=location)
        light = bpy.context.object
        light.name = name
        light.data.energy = energy
        light.data.shape = "RECTANGLE"
        light.data.size = size
        light.data.color = color
        look_at(light, target)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 768
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(asset_dir / "preview.png")
    scene.world.color = (.012, .016, .019)
    scene.view_settings.look = "AgX - Medium High Contrast"
    bpy.ops.render.render(write_still=True)


def save_asset(folder, mesh_name, model, collisions, metadata, camera_location, target):
    asset_dir = BASE / folder
    source_dir = asset_dir / "Source"
    asset_dir.mkdir(parents=True, exist_ok=True)
    source_dir.mkdir(parents=True, exist_ok=True)
    PREVIEWS.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    model.select_set(True)
    for item in collisions:
        item.hide_set(False)
        item.select_set(True)
    bpy.context.view_layer.objects.active = model
    bpy.ops.export_scene.fbx(
        filepath=str(asset_dir / f"{mesh_name}.fbx"), use_selection=True,
        apply_unit_scale=True, apply_scale_options="FBX_SCALE_UNITS",
        axis_forward="-Y", axis_up="Z", use_mesh_modifiers=True,
        mesh_smooth_type="FACE", add_leaf_bones=False, bake_anim=False,
    )
    (asset_dir / "equipment.json").write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding="utf-8")
    preview(asset_dir, model, camera_location, target)
    bpy.ops.wm.save_as_mainfile(filepath=str(source_dir / f"{folder}.blend"))
    print(f"MIRAS_STORE_KIT_ASSET={asset_dir}")


def wall_shelf():
    reset()
    dark = mat("MI_WallShelf_DarkMetal", (.035, .043, .042), .68, .29)
    wood = mat("MI_WallShelf_WarmWood", (.26, .085, .028), 0, .34)
    rail = mat("MI_WallShelf_PriceRail", (.82, .24, .025), .08, .30)
    back = mat("MI_WallShelf_Back", (.075, .085, .08), .42, .43)
    parts = [cube("Back", (0, .205, 1.12), (2.40, .04, 2.20), back, .005)]
    for x in (-1.175, 0, 1.175):
        parts.append(cube("Post", (x, .17, 1.10), (.05, .10, 2.20), dark, .005))
    for level, z in enumerate((.14, .50, .86, 1.22, 1.58, 1.94)):
        depth = .48 if level < 2 else .42
        parts.append(cube("Shelf", (0, -.02, z), (2.35, depth, .028), dark, .006))
        # Ticket strip hangs from the shelf edge. It must not form a raised barrier in front of products.
        parts.append(cube("Rail", (0, -.255 if level < 2 else -.225, z - .012), (2.35, .018, .040), rail, .003))
    parts += [cube("Base", (0, .02, .07), (2.40, .46, .14), wood, .009), cube("Header", (0, .18, 2.13), (2.40, .12, .18), wood, .009)]
    model = join(parts, "SM_WallShelf_2400", "wall_shelf_2400", (2400, 500, 2250))
    collisions = [collision("SM_WallShelf_2400", 0, (0, .2, 1.1), (2.4, .12, 2.2)), collision("SM_WallShelf_2400", 1, (0, 0, .12), (2.4, .5, .24))]
    zones = [{"face":"front", "level":i, "widthMm":2300, "depthMm":400 if i > 1 else 460, "heightMm":330} for i in range(6)]
    save_asset("WallShelf_2400", "SM_WallShelf_2400", model, collisions, {
        "schemaVersion":1, "id":"wall_shelf_2400", "displayName":"Duvar Reyonu 2400", "family":"wall_shelf",
        "dimensionsMm":{"width":2400,"depth":518,"height":2220}, "customerFaces":["front"], "frontAxis":"-Y",
        "shelfZones":zones, "materials":["DarkMetal","WarmWood","PriceRail","Back"], "collision":"2 UCX boxes"
    }, (3.5, -4.5, 2.8), (0, 0, 1.05))


def bulk_island():
    reset()
    wood = mat("MI_BulkIsland_Wood", (.25, .075, .024), 0, .29)
    black = mat("MI_BulkIsland_Black", (.018, .022, .021), .35, .35)
    glass = mat("MI_BulkIsland_Clear", (.32, .42, .39), 0, .12, .75)
    foods = [
        mat("MI_Bulk_Amber", (.58, .24, .035), 0, .68), mat("MI_Bulk_Gold", (.70, .46, .08), 0, .72),
        mat("MI_Bulk_Green", (.20, .34, .08), 0, .70), mat("MI_Bulk_Brown", (.22, .075, .025), 0, .76),
    ]
    parts = [cube("Base", (0, 0, .18), (1.60, .90, .36), wood, .018), cube("Toe", (0, -.45, .10), (1.62, .045, .18), black, .006)]
    for row, z in enumerate((.48, .83, 1.18)):
        y = -.18 + row * .06
        parts.append(cube("Tier", (0, y, z), (1.56, .64, .075), wood, .009))
        for col in range(4):
            x = -.585 + col * .39
            parts.append(cube("Bin", (x, y-.20, z+.15), (.35, .34, .25), glass, .012, rotation=(math.radians(-10),0,0)))
            parts.append(cube("Food", (x, y-.17, z+.135), (.29, .25, .15), foods[(row+col)%len(foods)], .025, rotation=(math.radians(-10),0,0)))
    parts += [cube("TopSign", (0, .18, 1.58), (1.30, .08, .30), wood, .012), cube("SignRail", (0, .13, 1.38), (1.48, .055, .055), black, .006)]
    model = join(parts, "SM_BulkIsland_1600", "bulk_island_1600", (1600, 900, 1730))
    collisions = [collision("SM_BulkIsland_1600", 0, (0, 0, .30), (1.6, .9, .60)), collision("SM_BulkIsland_1600", 1, (0, .1, 1.0), (1.5, .55, 1.15))]
    save_asset("BulkIsland_1600", "SM_BulkIsland_1600", model, collisions, {
        "schemaVersion":1, "id":"bulk_island_1600", "displayName":"Şeffaf Hazneli Kuru Gıda Adası", "family":"bulk_island",
        "dimensionsMm":{"width":1620,"depth":1017,"height":1730}, "customerFaces":["front"], "frontAxis":"-Y",
        "shelfZones":[{"face":"front","level":i,"widthMm":1450,"depthMm":330,"heightMm":300} for i in range(3)],
        "materials":["Wood","Black","Clear","BulkColors"], "collision":"2 UCX boxes"
    }, (3.0, -4.2, 2.4), (0, 0, .82))


def ceiling_bay():
    reset()
    black = mat("MI_Ceiling_BlackSteel", (.018, .022, .024), .72, .27)
    duct = mat("MI_Ceiling_Galvanized", (.24, .27, .28), .82, .24)
    light = mat("MI_Ceiling_Light", (.82, .74, .54), .05, .18, emission=(1.0, .76, .46))
    sprinkler = mat("MI_Ceiling_SprinklerRed", (.34, .012, .008), .52, .36)
    parts = []
    for x in (-2.9, 0, 2.9):
        parts.append(cube("MainBeam", (x, 0, .02), (.10, 6.0, .10), black, .008))
    for y in (-2.9, -1.45, 0, 1.45, 2.9):
        parts.append(cube("CrossBeam", (0, y, .06), (6.0, .07, .07), black, .006))
    parts.append(cylinder("Duct", (0, .25, -.12), .24, 5.8, duct, rotation=(0, math.radians(90), 0), vertices=40))
    # Real open-ceiling services: perforated cable tray, red sprinkler main, drops and black conduits.
    for x in (-1.25, -1.05, 1.05, 1.25):
        parts.append(cube("CableTrayRail", (x, 0, -.18), (.035, 5.70, .05), duct, .004))
    for y in (-2.7, -2.1, -1.5, -.9, -.3, .3, .9, 1.5, 2.1, 2.7):
        parts.append(cube("CableTrayRung", (1.15, y, -.18), (.30, .028, .035), duct, .003))
    parts.append(cylinder("SprinklerMain", (-1.75, 0, -.12), .035, 5.75, sprinkler, rotation=(math.radians(90), 0, 0), vertices=20))
    for y in (-2.1, -.7, .7, 2.1):
        parts.append(cylinder("SprinklerDrop", (-1.75, y, -.27), .018, .30, sprinkler, vertices=16))
        parts.append(cylinder("SprinklerHead", (-1.75, y, -.43), .055, .025, sprinkler, vertices=20))
    for x in (-2.35, 2.35):
        parts.append(cylinder("ElectricalConduit", (x, 0, -.12), .018, 5.65, black, rotation=(math.radians(90), 0, 0), vertices=16))
    for x in (-2.05, 2.05):
        for y in (-1.8, 0, 1.8):
            parts.append(cylinder("LightDrop", (x, y, -.14), .012, .34, black, vertices=12))
    for x in (-2.05, 2.05):
        for y in (-1.8, 0, 1.8):
            parts.append(cube("LuminaireBody", (x, y, -.22), (1.25, .14, .08), black, .006))
            parts.append(cube("Luminaire", (x, y, -.265), (1.18, .095, .018), light, .004))
    model = join(parts, "SM_CeilingBay_6000", "ceiling_bay_6000", (6000, 6000, 600))
    collisions = []
    save_asset("CeilingBay_6000", "SM_CeilingBay_6000", model, collisions, {
        "schemaVersion":1, "id":"ceiling_bay_6000", "displayName":"Açık Tavan Modülü 6000", "family":"ceiling",
        "dimensionsMm":{"width":6000,"depth":6000,"height":600}, "customerFaces":[], "frontAxis":"-Y", "shelfZones":[],
        "materials":["BlackSteel","Galvanized","EmissiveLight"], "collision":"none"
    }, (7.4, -8.2, 5.3), (0, 0, -.05))


def checkout_lane():
    reset()
    wood = mat("MI_Checkout_Wood", (.30, .11, .035), 0, .34)
    metal = mat("MI_Checkout_Black", (.025, .030, .030), .55, .28)
    rubber = mat("MI_Checkout_Rubber", (.018, .020, .020), 0, .83)
    steel = mat("MI_Checkout_PaintedMetal", (.52, .55, .54), .68, .24)
    glass = mat("MI_Checkout_Glass", (.17, .30, .32), 0, .10, .55)
    paper = mat("MI_Checkout_Paper", (.88, .84, .72), 0, .72)
    parts = [
        cube("Cabinet", (0, .03, .40), (2.50, .82, .80), wood, .018),
        cube("ToeKick", (0, -.405, .10), (2.42, .035, .20), metal, .004),
        cube("TopFrame", (0, -.01, .835), (2.50, .86, .07), steel, .010),
        cube("Conveyor", (-.47, -.10, .875), (1.12, .48, .035), rubber, .005),
        cube("ConveyorLeft", (-1.07, -.10, .89), (.07, .54, .08), metal, .006),
        cube("ConveyorRight", (.13, -.10, .89), (.07, .54, .08), metal, .006),
        cube("ScannerGlass", (.34, -.11, .885), (.31, .29, .025), glass, .005),
        cube("BaggingWell", (.91, -.04, .87), (.55, .58, .055), rubber, .010),
        cube("CashDrawer", (.36, .17, .99), (.43, .32, .14), metal, .008),
        cube("ScreenPost", (.47, .18, 1.13), (.045, .045, .28), metal, .004),
        cube("Screen", (.47, .16, 1.28), (.36, .055, .24), metal, .010, rotation=(math.radians(-8), 0, 0)),
        cube("CustomerDisplayPost", (.78, -.27, 1.01), (.035, .035, .24), metal, .003),
        cube("CustomerDisplay", (.78, -.28, 1.15), (.22, .045, .14), metal, .007),
        cube("ReceiptPrinter", (.72, .19, .97), (.22, .24, .12), metal, .008),
        cube("Receipt", (.72, .06, 1.04), (.12, .08, .006), paper, .001),
        cube("BumperFront", (0, -.45, .48), (2.50, .075, .10), rubber, .012),
    ]
    model = join(parts, "SM_CheckoutLane_2500", "checkout_lane_2500", (2500, 900, 1400))
    collisions = [
        collision("SM_CheckoutLane_2500", 0, (-.48, .03, .43), (1.52, .82, .86)),
        collision("SM_CheckoutLane_2500", 1, (.80, .03, .43), (.94, .82, .86)),
    ]
    save_asset("CheckoutLane_2500", "SM_CheckoutLane_2500", model, collisions, {
        "schemaVersion":1, "id":"checkout_lane_2500", "displayName":"Gercekci Market Kasasi 2500", "family":"checkout",
        "dimensionsMm":{"width":2500,"depth":928,"height":1400}, "customerFaces":["front"], "frontAxis":"-Y",
        "shelfZones":[], "materials":["Wood","Black","Rubber","PaintedMetal","Glass","Paper"], "collision":"2 UCX boxes"
    }, (3.5, -4.4, 2.35), (0, 0, .65))


def office_desk():
    reset()
    wood = mat("MI_Desk_Wood", (.25, .075, .024), 0, .34)
    metal = mat("MI_Desk_Black", (.022, .026, .025), .38, .34)
    top = mat("MI_Desk_CounterTop", (.60, .46, .31), 0, .30)
    paper = mat("MI_Desk_Paper", (.88, .84, .72), 0, .75)
    parts = [
        cube("Top", (0, 0, .75), (1.80, .80, .055), top, .010),
        cube("LeftPedestal", (-.69, .10, .37), (.36, .54, .74), wood, .012),
        cube("RightPedestal", (.69, .10, .37), (.36, .54, .74), wood, .012),
        cube("BackPanel", (0, .34, .40), (1.48, .035, .58), wood, .006),
        cube("MonitorFoot", (0, .09, .80), (.30, .20, .025), metal, .005),
        cube("MonitorNeck", (0, .12, .92), (.05, .05, .25), metal, .004),
        cube("Monitor", (0, .11, 1.13), (.58, .065, .34), metal, .012, rotation=(math.radians(-4), 0, 0)),
        cube("Keyboard", (0, -.21, .80), (.46, .17, .025), metal, .005),
        cube("MousePad", (.42, -.20, .79), (.24, .20, .012), metal, .005),
        sphere("Mouse", (.42, -.20, .82), .04, metal, scale=(.75, 1.0, .45), segments=16, rings=8),
        cube("PaperStack", (-.48, -.20, .80), (.22, .30, .018), paper, .003, rotation=(0, 0, math.radians(-5))),
        cube("DrawerLine1", (-.69, -.185, .28), (.29, .012, .012), metal, .002),
        cube("DrawerLine2", (-.69, -.185, .49), (.29, .012, .012), metal, .002),
    ]
    model = join(parts, "SM_OfficeDesk_1800", "office_desk_1800", (1800, 800, 1300))
    collisions = [
        collision("SM_OfficeDesk_1800", 0, (0, 0, .75), (1.8, .8, .08)),
        collision("SM_OfficeDesk_1800", 1, (-.69, .10, .37), (.36, .54, .74)),
        collision("SM_OfficeDesk_1800", 2, (.69, .10, .37), (.36, .54, .74)),
    ]
    save_asset("OfficeDesk_1800", "SM_OfficeDesk_1800", model, collisions, {
        "schemaVersion":1, "id":"office_desk_1800", "displayName":"Yonetim Masasi 1800", "family":"office_desk",
        "dimensionsMm":{"width":1800,"depth":800,"height":1300}, "customerFaces":["front"], "frontAxis":"-Y",
        "shelfZones":[], "materials":["Wood","Black","CounterTop","Paper"], "collision":"3 UCX boxes"
    }, (2.8, -3.5, 2.1), (0, 0, .58))


def refrigerated_wall():
    reset()
    metal = mat("MI_Fridge_Black", (.025, .030, .032), .58, .27)
    steel = mat("MI_Fridge_PaintedMetal", (.66, .69, .67), .62, .24)
    glass = mat("MI_Fridge_Glass", (.20, .34, .36), 0, .08, .67)
    light = mat("MI_Fridge_Light", (.82, .86, .78), 0, .18, emission=(.82, .90, 1.0))
    milk = mat("MI_Fridge_ProductWhite", (.75, .78, .72), 0, .65)
    drink = mat("MI_Fridge_ProductBlue", (.02, .18, .35), .05, .55)
    dairy = mat("MI_Fridge_ProductRed", (.55, .025, .018), .05, .58)
    parts = [
        cube("Cabinet", (0, .05, 1.125), (3.00, .70, 2.25), metal, .018),
        cube("Interior", (0, -.285, 1.10), (2.86, .045, 1.87), steel, .004),
        cube("TopHeader", (0, -.36, 2.12), (3.00, .06, .26), metal, .007),
        cube("KickPlate", (0, -.36, .12), (3.00, .06, .24), steel, .006),
    ]
    for z in (.40, .72, 1.04, 1.36, 1.68):
        parts.append(cube("InteriorShelf", (0, -.31, z), (2.84, .42, .025), steel, .003))
        for col in range(15):
            x = -1.31 + col * .187
            product_mat = (milk, drink, dairy)[(col + int(z * 10)) % 3]
            # Products sit behind the glass plane; the earlier front position enlarged the cabinet bounds.
            parts.append(cube("ChilledProduct", (x, -.29, z + .115), (.12, .10, .21), product_mat, .010))
    for door in range(3):
        x = -.99 + door * .99
        parts.append(cube("GlassDoor", (x, -.372, 1.15), (.94, .035, 1.78), glass, .006))
        for edge_x in (x - .47, x + .47):
            parts.append(cube("DoorFrame", (edge_x, -.395, 1.15), (.028, .045, 1.82), metal, .003))
        parts.append(cube("DoorHandle", (x + .36, -.43, 1.15), (.035, .045, .68), metal, .005))
        parts.append(cube("DoorLight", (x - .43, -.415, 1.15), (.018, .018, 1.70), light, .002))
    model = join(parts, "SM_RefrigeratedWall_3000", "refrigerated_wall_3000", (3000, 750, 2250))
    collisions = [collision("SM_RefrigeratedWall_3000", 0, (0, .04, 1.125), (3.0, .70, 2.25))]
    save_asset("RefrigeratedWall_3000", "SM_RefrigeratedWall_3000", model, collisions, {
        "schemaVersion":1, "id":"refrigerated_wall_3000", "displayName":"Uc Kapili Sogutucu 3000", "family":"refrigerated_wall",
        "dimensionsMm":{"width":3000,"depth":920,"height":2250}, "customerFaces":["front"], "frontAxis":"-Y",
        "shelfZones":[], "materials":["Black","PaintedMetal","Glass","Light","ProductWhite","ProductBlue","ProductRed"], "collision":"1 UCX box"
    }, (4.0, -4.8, 2.65), (0, 0, 1.05))


def produce_island():
    reset()
    wood = mat("MI_Produce_Wood", (.28, .095, .025), 0, .40)
    metal = mat("MI_Produce_Black", (.022, .027, .025), .48, .34)
    rail = mat("MI_Produce_PriceRail", (.82, .78, .68), 0, .48)
    green = mat("MI_Produce_Green", (.08, .32, .035), 0, .72)
    red = mat("MI_Produce_Red", (.54, .025, .018), 0, .66)
    orange = mat("MI_Produce_Orange", (.82, .22, .018), 0, .64)
    yellow = mat("MI_Produce_Yellow", (.72, .54, .025), 0, .68)
    produce_mats = (green, red, orange, yellow)
    parts = [
        cube("Base", (0, 0, .18), (2.40, 1.20, .36), wood, .018),
        cube("Toe", (0, -.585, .10), (2.28, .035, .18), metal, .005),
        cube("CenterSpine", (0, .08, .79), (2.22, .12, 1.05), metal, .010),
    ]
    for side in (-1, 1):
        y = side * .34
        tilt = math.radians(side * 12)
        for row, z in enumerate((.48, .84, 1.18)):
            parts.append(cube("ProduceTray", (0, y, z), (2.24, .48, .09), wood, .012, rotation=(tilt, 0, 0)))
            parts.append(cube("ProduceRail", (0, y - side * .25, z + .02), (2.18, .035, .09), rail, .004, rotation=(tilt, 0, 0)))
            for col in range(10):
                x = -1.0 + col * .22
                mat_index = (row * 2 + (0 if side < 0 else 1)) % len(produce_mats)
                for depth_index in range(2):
                    py = y - side * (.035 + depth_index * .13)
                    pz = z + .12 + depth_index * .025
                    parts.append(sphere("Produce", (x + (depth_index % 2) * .035, py, pz), .085,
                        produce_mats[mat_index], scale=(1.05, .92, .88), segments=12, rings=6))
    model = join(parts, "SM_ProduceIsland_2400", "produce_island_2400", (2400, 1200, 1400))
    collisions = [
        collision("SM_ProduceIsland_2400", 0, (0, 0, .24), (2.4, 1.2, .48)),
        collision("SM_ProduceIsland_2400", 1, (0, 0, .86), (2.28, .62, .90)),
    ]
    save_asset("ProduceIsland_2400", "SM_ProduceIsland_2400", model, collisions, {
        "schemaVersion":1, "id":"produce_island_2400", "displayName":"Manav Adasi 2400", "family":"produce_island",
        "dimensionsMm":{"width":2400,"depth":1200,"height":1400}, "customerFaces":["front","back"], "frontAxis":"-Y",
        "shelfZones":[], "materials":["Wood","Black","PriceRail","Green","Red","Orange","Yellow"], "collision":"2 UCX boxes"
    }, (3.5, -4.5, 2.45), (0, 0, .65))


if __name__ == "__main__":
    wall_shelf()
    bulk_island()
    ceiling_bay()
    checkout_lane()
    office_desk()
    refrigerated_wall()
    produce_island()
    print("MIRAS_STORE_KIT_READY=7")
