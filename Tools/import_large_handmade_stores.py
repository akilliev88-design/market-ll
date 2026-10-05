"""Import three authored buildings with independent roofs and project surface materials."""
from pathlib import Path
import json,unreal
root=Path(unreal.Paths.project_dir()).resolve()
unreal.SystemLibrary.execute_console_command(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'Interchange.FeatureFlags.Import.FBX 0')
for folder in sorted((root/'AssetInbox/Environment/Stores/HandmadeLarge').iterdir()):
    info=json.loads((folder/'equipment.json').read_text());sid=info['id'];target='/Game/Stores/Handmade/Large/'+sid
    for fbx in sorted(folder.glob('*.fbx')):
        name=fbx.stem;roof='Roof' in name
        options=unreal.FbxImportUI()
        for k,v in dict(import_mesh=True,import_as_skeletal=False,import_materials=True,import_textures=False,create_physics_asset=False).items():options.set_editor_property(k,v)
        data=options.get_editor_property('static_mesh_import_data')
        for k,v in dict(combine_meshes=True,generate_lightmap_u_vs=True,auto_generate_collision=roof,one_convex_hull_per_ucx=True,convert_scene=True,convert_scene_unit=True,transform_vertex_to_absolute=True).items():data.set_editor_property(k,v)
        task=unreal.AssetImportTask()
        for k,v in dict(filename=str(fbx),destination_path=target,destination_name=name,automated=True,replace_existing=True,replace_existing_settings=True,save=True,options=options).items():task.set_editor_property(k,v)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(target+'/'+name)
        if not mesh:raise RuntimeError(name)
        if not roof:
            agg=mesh.get_editor_property('body_setup').get_editor_property('agg_geom')
            count=sum(len(agg.get_editor_property(k)) for k in ('box_elems','sphere_elems','sphyl_elems','convex_elems'))
            if count!=info['collision']['pieces']:raise RuntimeError(f'{name} collision {count}')
        # Persistent material instances use the same textured master as the game.
        slots=mesh.get_editor_property('static_materials')
        for slot in slots:
            n=str(slot.get_editor_property('material_slot_name'))
            path=target+'/MI_'+n
            m=unreal.load_asset(path)
            if not m:m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('MI_'+n,target,unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
            glass='Glass' in n
            master=unreal.load_asset('/Game/Materials/Miras/M_MirasAcrylic' if glass else '/Game/Materials/Miras/M_MirasSurface')
            unreal.MaterialEditingLibrary.set_material_instance_parent(m,master)
            colors={'Floor':(.82,.82,.79),'Wall':(.78,.78,.74),'Metal':(.03,.05,.055),'Glass':(.97,.98,1),'Accent':(.025,.14,.12),'Concrete':(.45,.46,.46),'Light':(.9,.95,1)}
            key=next((k for k in colors if ('HL_'+k) in n),'Wall');c=colors[key]
            unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(m,'Color',unreal.LinearColor(*c,1))
            for param,value in [('Roughness',.08 if glass else .45),('Metallic',.5 if key=='Metal' else 0),('Opacity',.14),('UseTexture',1 if key=='Floor' else 0),('TileCm',120),('Wear',.15),('Emissive',4 if key=='Light' else 0)]:unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(m,param,value)
            if key=='Floor':unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(m,'BaseTex',unreal.load_asset('/Game/Materials/Miras/Textures/T_Floor_Terrazzo'))
            slot.set_editor_property('material_interface',m);unreal.EditorAssetLibrary.save_loaded_asset(m)
        mesh.set_editor_property('static_materials',slots);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        unreal.log('LARGE_IMPORTED='+name)
unreal.log('LARGE_BUILDINGS_IMPORT_PASSED=3')
