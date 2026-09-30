"""Editable floor material, kept in Codex-owned Content/Stores."""
import unreal
path='/Game/Stores/Materials/M_EditableSurface'
m=unreal.load_asset(path)
if not m:
    m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_EditableSurface','/Game/Stores/Materials',unreal.Material,unreal.MaterialFactoryNew())
    color=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionVectorParameter,-300,0);color.set_editor_property('parameter_name','Color');color.set_editor_property('default_value',unreal.LinearColor(.65,.65,.60,1));unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
    rough=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionScalarParameter,-300,180);rough.set_editor_property('parameter_name','Roughness');rough.set_editor_property('default_value',.65);unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES);unreal.MaterialEditingLibrary.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.log('MIRAS_EDITABLE_FLOOR_READY')
