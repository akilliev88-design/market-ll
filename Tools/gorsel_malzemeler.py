"""Miras Market surface library for Unreal Engine 5.8 (run by GORSEL_HAZIRLA.cmd).

1. Imports the procedural PNG textures from AssetInbox/Textures/Miras (Tools/doku_uret.py) and,
   when present, the free CC0 photo textures from AssetInbox/Textures/Harici.
2. Builds M_MirasSurface: world-aligned (triplanar) texture x Color tint, Roughness / Metallic /
   Specular / Emissive parameters. Works on any mesh without UVs (procedural boxes, kit meshes).
3. Builds M_MirasAcrylic: clear translucent plastic for bulk-food bins.

The game (MarketVisuals.cpp) loads these by path and falls back to M_Market colors if missing.
Every graph connection is checked; any failure raises and the command exits with an error.
"""

from pathlib import Path

import unreal

PROJECT_ROOT = Path(unreal.Paths.project_dir()).resolve()
TEXTURE_SOURCE = PROJECT_ROOT / "AssetInbox" / "Textures" / "Miras"
BASE = "/Game/Materials/Miras"
TEXTURE_DIR = BASE + "/Textures"
EXTERNAL_SOURCE = PROJECT_ROOT / "AssetInbox" / "Textures" / "Harici"
# Free CC0 textures (ambientCG / Poly Haven, Docs/Environment/FAB_PAKET_LISTESI.md). Optional: a missing
# folder is skipped and the game keeps the procedural texture for that surface.
EXTERNAL = {
    "Terrazzo004": ("_Color", "_Roughness"),
    "beige_wall_001": ("_diff_", "_rough_"),
    "american_walnut_veneer": ("_diff_", "_rough_"),
    "ash_veneer": ("_diff_", "_rough_"),
    "Cardboard004": ("_Color", "_Roughness"),
}
TEXTURES = ["T_Floor_Terrazzo", "T_Wood_Walnut", "T_Wood_Oak", "T_Food_Hazelnut", "T_Food_Chickpea", "T_Food_Lentil", "T_Food_Pistachio", "T_Macro_Variation"]

MEL = unreal.MaterialEditingLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def fail(message):
    raise RuntimeError(f"MIRAS_MATERIALS_ERROR={message}")


def import_textures():
    for name in TEXTURES:
        source = TEXTURE_SOURCE / f"{name}.png"
        if not source.exists():
            fail(f"texture source missing: {source} (run: python Tools/doku_uret.py)")
        import_one(source, name, not name.startswith("T_Macro"))  # macro mask is linear data


def import_one(source, name, srgb):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", TEXTURE_DIR)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    TOOLS.import_asset_tasks([task])
    texture = unreal.load_asset(f"{TEXTURE_DIR}/{name}.{name}")
    if texture is None:
        fail(f"texture not imported: {name}")
    texture.set_editor_property("srgb", srgb)
    # Linear maps keep default compression so their sampler type matches the RoughTex/MacroTex defaults.
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    unreal.log(f"MIRAS_TEXTURE_IMPORTED={name}")


def import_external():
    """T_Ext_<folder>_BC (colour, sRGB) and T_Ext_<folder>_R (roughness, linear)."""
    count = 0
    for folder, (color_key, rough_key) in EXTERNAL.items():
        directory = EXTERNAL_SOURCE / folder
        if not directory.is_dir():
            unreal.log_warning(f"MIRAS_MATERIALS_NOTE=external texture folder missing, skipped: {directory}")
            continue
        files = sorted(directory.glob("*.png")) + sorted(directory.glob("*.jpg"))
        color = next((f for f in files if color_key.lower() in f.name.lower()), None)
        rough = next((f for f in files if rough_key.lower() in f.name.lower()), None)
        if color is None:
            unreal.log_warning(f"MIRAS_MATERIALS_NOTE=no colour map in {directory}, skipped")
            continue
        import_one(color, f"T_Ext_{folder}_BC", True)
        if rough is not None:
            import_one(rough, f"T_Ext_{folder}_R", False)
        count += 1
    unreal.log(f"MIRAS_EXTERNAL_TEXTURES={count}")
    return count


def new_material(name):
    path = f"{BASE}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    material = TOOLS.create_asset(name, BASE, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        fail(f"could not create {path}")
    material.set_editor_property("used_with_instanced_static_meshes", True)
    return material


def node(material, cls, x, y, **props):
    expression = MEL.create_material_expression(material, cls, x, y)
    if expression is None:
        fail(f"could not create {cls}")
    for key, value in props.items():
        expression.set_editor_property(key, value)
    return expression


def link(source, output, target, input_name):
    if not MEL.connect_material_expressions(source, output, target, input_name):
        fail(f"link {source.get_name()}[{output}] -> {target.get_name()}[{input_name}]")


def to_property(source, output, prop):
    if not MEL.connect_material_property(source, output, prop):
        fail(f"property link {source.get_name()}[{output}] -> {prop}")


def scalar(material, name, value, x, y):
    return node(material, unreal.MaterialExpressionScalarParameter, x, y, parameter_name=name, default_value=value)


def mask(material, source, x, y, r=False, g=False, b=False):
    expression = node(material, unreal.MaterialExpressionComponentMask, x, y, r=r, g=g, b=b, a=False)
    link(source, "", expression, "")
    return expression


def binary(material, cls, a, a_out, b, b_out, x, y):
    expression = node(material, cls, x, y)
    link(a, a_out, expression, "A")
    link(b, b_out, expression, "B")
    return expression


def triplanar(m, texture, tile, weights, y):
    """World-aligned projection of a texture object; blends the three planar samples by n^2."""
    world = node(m, unreal.MaterialExpressionWorldPosition, -1200, y)
    uvw = binary(m, unreal.MaterialExpressionDivide, world, "", tile, "", -1050, y)
    uvs = (mask(m, uvw, -900, y - 100, g=True, b=True),   # faces looking along X use Y,Z
           mask(m, uvw, -900, y + 20, r=True, b=True),    # faces looking along Y use X,Z
           mask(m, uvw, -900, y + 140, r=True, g=True))   # floors / tops use X,Y
    parts = []
    for i, uv in enumerate(uvs):
        sample = node(m, unreal.MaterialExpressionTextureSample, -700, y - 150 + i * 170)
        link(uv, "", sample, "UVs")
        link(texture, "", sample, "Tex")
        parts.append(binary(m, unreal.MaterialExpressionMultiply, sample, "RGB", weights[i], "", -450, y - 150 + i * 170))
    sum_xy = binary(m, unreal.MaterialExpressionAdd, parts[0], "", parts[1], "", -300, y - 60)
    return binary(m, unreal.MaterialExpressionAdd, sum_xy, "", parts[2], "", -180, y + 40)


def build_surface():
    m = new_material("M_MirasSurface")
    default_texture = unreal.load_asset(f"{TEXTURE_DIR}/T_Floor_Terrazzo.T_Floor_Terrazzo")
    macro_texture = unreal.load_asset(f"{TEXTURE_DIR}/T_Macro_Variation.T_Macro_Variation")
    color = node(m, unreal.MaterialExpressionVectorParameter, -1400, -300, parameter_name="Color", default_value=unreal.LinearColor(1, 1, 1, 1))
    use_texture = scalar(m, "UseTexture", 0.0, -1400, -150)
    tile = scalar(m, "TileCm", 100.0, -1400, 0)
    texture = node(m, unreal.MaterialExpressionTextureObjectParameter, -1400, 150, parameter_name="BaseTex", texture=default_texture)
    wear = scalar(m, "Wear", 0.0, -1400, 700)
    macro_tile = scalar(m, "MacroTileCm", 350.0, -1400, 850)
    macro_tex = node(m, unreal.MaterialExpressionTextureObjectParameter, -1400, 1000, parameter_name="MacroTex", texture=macro_texture)
    use_rough_tex = scalar(m, "UseRoughTex", 0.0, -1400, 1300)
    rough_tex = node(m, unreal.MaterialExpressionTextureObjectParameter, -1400, 1450, parameter_name="RoughTex", texture=macro_texture)

    normal = node(m, unreal.MaterialExpressionVertexNormalWS, -900, 400)
    squared = binary(m, unreal.MaterialExpressionMultiply, normal, "", normal, "", -750, 400)  # n^2 sums to 1
    weights = (mask(m, squared, -600, 360, r=True), mask(m, squared, -600, 440, g=True), mask(m, squared, -600, 520, b=True))

    base_tex = triplanar(m, texture, tile, weights, 0)
    macro = mask(m, triplanar(m, macro_tex, macro_tile, weights, 900), -60, 900, r=True)

    # Base = Color * lerp(1, texture, UseTexture) * lerp(1, lerp(.72, 1.18, macro), Wear)
    blend = node(m, unreal.MaterialExpressionLinearInterpolate, -60, 0, const_a=1.0)
    link(base_tex, "", blend, "B")
    link(use_texture, "", blend, "Alpha")
    tone = node(m, unreal.MaterialExpressionLinearInterpolate, 60, 900, const_a=0.72, const_b=1.18)
    link(macro, "", tone, "Alpha")
    worn = node(m, unreal.MaterialExpressionLinearInterpolate, 200, 900, const_a=1.0)
    link(tone, "", worn, "B")
    link(wear, "", worn, "Alpha")
    color_rgb = mask(m, color, -1250, -300, r=True, g=True, b=True)
    tinted = binary(m, unreal.MaterialExpressionMultiply, color_rgb, "", blend, "", 90, -100)
    base = binary(m, unreal.MaterialExpressionMultiply, tinted, "", worn, "", 340, -60)
    to_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)

    emissive_strength = scalar(m, "Emissive", 0.0, -60, 250)
    emissive = binary(m, unreal.MaterialExpressionMultiply, tinted, "", emissive_strength, "", 340, 250)
    to_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    # Roughness + (lerp(-.15, .15, macro) * Wear): smudges and scuffs break up clean reflections.
    delta = node(m, unreal.MaterialExpressionLinearInterpolate, 60, 1100, const_a=-0.15, const_b=0.15)
    link(macro, "", delta, "Alpha")
    delta_worn = binary(m, unreal.MaterialExpressionMultiply, delta, "", wear, "", 200, 1100)
    # Photo-scanned roughness map (external textures) replaces the flat Roughness value when UseRoughTex = 1.
    rough_map = mask(m, triplanar(m, rough_tex, tile, weights, 1500), -60, 1500, r=True)
    rough_base = node(m, unreal.MaterialExpressionLinearInterpolate, 90, 1400)
    link(scalar(m, "Roughness", 0.5, -60, 1350), "", rough_base, "A")
    link(rough_map, "", rough_base, "B")
    link(use_rough_tex, "", rough_base, "Alpha")
    rough = binary(m, unreal.MaterialExpressionAdd, rough_base, "", delta_worn, "", 340, 400)
    to_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    to_property(scalar(m, "Metallic", 0.0, 90, 500), "", unreal.MaterialProperty.MP_METALLIC)
    to_property(scalar(m, "Specular", 0.5, 90, 600), "", unreal.MaterialProperty.MP_SPECULAR)
    MEL.recompile_material(m)
    unreal.EditorAssetLibrary.save_loaded_asset(m)
    unreal.log("MIRAS_MATERIAL_READY=M_MirasSurface")


def build_acrylic():
    m = new_material("M_MirasAcrylic")
    m.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    m.set_editor_property("two_sided", True)
    try:
        m.set_editor_property("translucency_lighting_mode", unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
    except Exception as error:  # optional quality setting; the material still works without it
        unreal.log_warning(f"MIRAS_MATERIALS_NOTE=translucency lighting mode not set: {error}")
    color = node(m, unreal.MaterialExpressionVectorParameter, -400, -100, parameter_name="Color", default_value=unreal.LinearColor(.9, .97, .95, 1))
    to_property(mask(m, color, -250, -100, r=True, g=True, b=True), "", unreal.MaterialProperty.MP_BASE_COLOR)
    to_property(scalar(m, "Opacity", 0.14, -400, 60), "", unreal.MaterialProperty.MP_OPACITY)
    to_property(scalar(m, "Roughness", 0.04, -400, 160), "", unreal.MaterialProperty.MP_ROUGHNESS)
    to_property(scalar(m, "Specular", 0.8, -400, 260), "", unreal.MaterialProperty.MP_SPECULAR)
    MEL.recompile_material(m)
    unreal.EditorAssetLibrary.save_loaded_asset(m)
    unreal.log("MIRAS_MATERIAL_READY=M_MirasAcrylic")


def enable_instancing():
    """Shelf stock is drawn with instanced meshes: every master material must allow it,
    otherwise Unreal renders the default grey material in game."""
    changed = 0
    for root in ("/Game/Products", "/Game/Materials", "/Game/Environment"):
        if not unreal.EditorAssetLibrary.does_directory_exist(root):
            continue
        for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False):
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if not isinstance(asset, unreal.Material):
                continue
            if asset.get_editor_property("used_with_instanced_static_meshes"):
                continue
            asset.set_editor_property("used_with_instanced_static_meshes", True)
            MEL.recompile_material(asset)
            unreal.EditorAssetLibrary.save_loaded_asset(asset)
            changed += 1
            unreal.log(f"MIRAS_INSTANCING_ENABLED={asset.get_path_name()}")
    unreal.log(f"MIRAS_INSTANCING_READY={changed} materials updated")


def main():
    import_textures()
    external = import_external()
    build_surface()
    build_acrylic()
    enable_instancing()
    for path in (f"{BASE}/M_MirasSurface.M_MirasSurface", f"{BASE}/M_MirasAcrylic.M_MirasAcrylic"):
        if unreal.load_asset(path) is None:
            fail(f"missing after save: {path}")
    unreal.log(f"MIRAS_MATERIALS_READY=2 materials, 8 textures, {external} external texture sets")


main()
