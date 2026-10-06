"""Unreal editor Python: build the offline Plex atlas used by all world text."""
import ctypes
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve()
source = root / 'Content/Slate/Fonts/IBMPlexSans-SemiBold.ttf'
gdi = ctypes.WinDLL('gdi32')
gdi.AddFontResourceExW.argtypes = [ctypes.c_wchar_p, ctypes.c_uint, ctypes.c_void_p]
gdi.RemoveFontResourceExW.argtypes = gdi.AddFontResourceExW.argtypes
if not gdi.AddFontResourceExW(str(source), 0x10, None):
    raise RuntimeError('Plex font could not be registered for this editor process')
try:
    factory = unreal.TrueTypeFontFactory()
    options = factory.get_editor_property('import_options')
    data = options.get_editor_property('data')
    for key, value in dict(font_name='IBM Plex Sans SemiBold', height=32.0,
                           enable_antialiasing=True, use_distance_field_alpha=True,
                           distance_field_scale_factor=8, texture_page_width=1024,
                           texture_page_max_height=1024,
                           chars=''.join(chr(n) for n in range(32, 127)) + 'ÇĞİÖŞÜçğıöşü₺×·•—–…’',
                           include_ascii_range=True).items():
        data.set_editor_property(key, value)
    options.set_editor_property('data', data)
    asset = unreal.load_asset('/Game/Stores/Fonts/F_PlexTurkish')
    if not asset:
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            'F_PlexTurkish', '/Game/Stores/Fonts', unreal.Font, factory)
    if not asset:
        raise RuntimeError('Offline font creation failed')
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    material = unreal.load_asset('/Game/Stores/Fonts/M_PlexText')
    if not material:
        material = unreal.EditorAssetLibrary.duplicate_asset('/Engine/EngineMaterials/DefaultTextMaterialOpaque', '/Game/Stores/Fonts/M_PlexText')
    for node in unreal.MaterialEditingLibrary.get_material_expressions(material):
        if isinstance(node, unreal.MaterialExpressionFontSampleParameter):
            node.set_editor_property('font', asset)
            node.set_editor_property('font_texture_page', 0)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    unreal.log('SIM_TURKISH_FONT_CREATED=' + asset.get_path_name())
finally:
    gdi.RemoveFontResourceExW(str(source), 0x10, None)
