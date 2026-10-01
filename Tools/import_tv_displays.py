"""Import only TV fixtures and separate product meshes; preserve other store assets."""
import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve()
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
unreal.SystemLibrary.execute_console_command(world, 'Interchange.FeatureFlags.Import.FBX 0')
paths = [root / 'AssetInbox/Environment/Stores' / eid / 'equipment.json'
         for eid in ('tv_wall_4800', 'tv_plinth_2400', 'tv_island_3000')]
paths += sorted((root / 'AssetInbox/Products/Televisions').glob('tv_*/product.json'))
texture_task = unreal.AssetImportTask()
for key, value in dict(filename=str(root / 'AssetInbox/Products/Televisions/T_TV_DemoLandscape.png'),
                       destination_path='/Game/Stores/Televisions', destination_name='T_TV_DemoLandscape',
                       automated=True, replace_existing=True, save=True).items():
    texture_task.set_editor_property(key, value)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([texture_task])
texture = unreal.load_asset('/Game/Stores/Televisions/T_TV_DemoLandscape')
assert texture
for path in paths:
    meta = json.loads(path.read_text(encoding='utf-8'))
    fixture = 'planogram' in meta
    target = '/Game/Stores/Equipment' if fixture else '/Game/Stores/Televisions'
    name = Path(meta['mesh']).stem
    options = unreal.FbxImportUI()
    for key, value in dict(import_mesh=True, import_as_skeletal=False, import_materials=True,
                           import_textures=False, create_physics_asset=False).items():
        options.set_editor_property(key, value)
    data = options.get_editor_property('static_mesh_import_data')
    for key, value in dict(combine_meshes=True, auto_generate_collision=False, one_convex_hull_per_ucx=True,
                           convert_scene=True, convert_scene_unit=True, transform_vertex_to_absolute=True).items():
        data.set_editor_property(key, value)
    task = unreal.AssetImportTask()
    for key, value in dict(filename=str(path.parent / meta['mesh']), destination_path=target,
                           destination_name=name, automated=True, replace_existing=True,
                           replace_existing_settings=True, save=True, options=options).items():
        task.set_editor_property(key, value)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(target + '/' + name)
    assert mesh, name
    bounds = mesh.get_bounding_box()
    dims = bounds.max - bounds.min
    expected = meta['dimensionsMm'] if fixture else dict(width=meta['depthMm'], depth=meta['widthMm'], height=meta['heightMm'])
    for actual, key in zip((dims.x, dims.y, dims.z), ('width', 'depth', 'height')):
        assert abs(actual - expected[key] / 10) < 2, (name, key, actual, expected[key])
    if fixture:
        agg = mesh.get_editor_property('body_setup').get_editor_property('agg_geom')
        count = sum(len(agg.get_editor_property(key)) for key in ('box_elems', 'sphere_elems', 'sphyl_elems', 'convex_elems'))
        assert count == meta['collision']['pieces'], (name, count)
    for slot in mesh.get_editor_property('static_materials'):
        material = slot.get_editor_property('material_interface')
        if not isinstance(material, unreal.Material): continue
        unreal.MaterialEditingLibrary.set_material_usage(material, unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
        mat_name = material.get_name()
        if mat_name in ('MI_TV_LED', 'MI_TV_ProductScreen'):
            unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
            if mat_name == 'MI_TV_ProductScreen':
                sample = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureSample)
                sample.set_editor_property('texture', texture)
                unreal.MaterialEditingLibrary.connect_material_property(sample, 'RGB', unreal.MaterialProperty.MP_BASE_COLOR)
                unreal.MaterialEditingLibrary.connect_material_property(sample, 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            else:
                emission = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant3Vector)
                emission.set_editor_property('constant', unreal.LinearColor(4, 4.6, 5, 1))
                unreal.MaterialEditingLibrary.connect_material_property(emission, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            rough = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant)
            rough.set_editor_property('r', .17 if mat_name == 'MI_TV_ProductScreen' else .3)
            unreal.MaterialEditingLibrary.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    unreal.log('MIRAS_TV_IMPORTED=' + name)
unreal.log('MIRAS_TV_IMPORT_COMPLETE')
